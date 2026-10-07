/**
 * @file qspi_probe.c
 * @brief QSPI linear-port characterization: CRC32 the whole image under
 *        candidate port configs, via the non-cacheable alias (single-beat)
 *        or the cached window after invalidate (linefill bursts).
 *
 * Safety argument for poking the (possibly unreliable) port at runtime:
 *  - the per-config window runs with IRQs masked: no ISR fetches flash;
 *  - the reconfig sequence executes from OCM (.fast), like the boot one;
 *  - the CRC loop code is preheated under the golden config before each
 *    sweep, and single-beat alias reads never allocate, so its code lines
 *    cannot be evicted mid-window;
 *  - a bad config therefore yields a wrong CRC (or all zeros), not a
 *    crash; the golden config is restored (from OCM) before IRQs are
 *    re-enabled and before any printf.
 *
 * The burst variant additionally invalidates L1D+L2 over the XIP window
 * inside the window, so every read is a linefill burst through the port.
 * If bursts at the golden config are broken, the post-window printf code
 * itself re-fills from a dead port: a frozen LED is the verdict.
 *
 * Ground truth is host-side: zlib.crc32 of app.bin (same span). The sweep
 * also flags crc == crc32(all-zeros) - the 100 MHz single-beat signature.
 */

#include <stdint.h>
#include <stdio.h>

#include "qspi_probe.h"
#include "zynq_cache.h"
#include "zynq_clock.h"
#include "zynq_soc.h"

/* Non-cacheable RO alias of PA 0xFC000000 (page table entry 0xFB2). */
#define PROBE_ALIAS_BASE (0xFB200000UL)

/* Shipping ("golden") port config: ref = IO PLL/5 = 200 MHz (> CPU_1x),
   SCLK = ref/8 = 25 MHz, LPBK = 0 (direct pad sampling - the MIO8
   feedback path is not routed, loopback capture is dead on this board).
   Proven correct by the 2026-10-07 sweeps + go-live; matches what
   qspi_reconfig() ships. */
#define PROBE_CLK_CTRL ((5UL << 8) | (1UL << 0))
#define PROBE_GOLDEN_CR (PROBE_CR_BASE | (2UL << 3))
#define PROBE_GOLDEN_LPBK (0UL)

/* QSPI CR with BAUD_RATE_DIV[5:3] = 000; the sweep ORs the field in. */
#define PROBE_CR_BASE (0x800238C1UL)

/* BootROM config (captured, known-good): ref = ARM PLL/40 = 33.33 MHz
   both before (PLL bypass) and after (1333/40) sys_clock_init. */
#define PROBE_BOOTROM_REF_KHZ (33333UL)

#define PROBE_DIV_COUNT (4U)  /* /2 /4 /8 /16 of the 200 MHz ref */
#define PROBE_DLY_COUNT (32U) /* DLY[4:0]                        */

/* ------------------------------------------------------------------ */
/* CRC32 (zlib polynomial 0xEDB88320, so the host check is one line)  */
/* ------------------------------------------------------------------ */

static uint32_t probe_crc_table[256];

/* Set by probe_golive(): the system runs on the legal config from then on,
   so probe commands (which restore the dead golden config) must be locked
   out to avoid hanging the board. */
static int probe_frozen;

int probe_keys_frozen(void)
{
    return probe_frozen;
}

static void probe_crc_table_init(void)
{
    uint32_t i;
    int k;

    for (i = 0U; i < 256U; i++)
    {
        uint32_t c = i;
        for (k = 0; k < 8; k++)
        {
            c = ((c & 1U) != 0U) ? (0xEDB88320UL ^ (c >> 1)) : (c >> 1);
        }
        probe_crc_table[i] = c;
    }
}

static inline uint32_t probe_crc_step(uint32_t crc, uint8_t b)
{
    return (crc >> 8) ^ probe_crc_table[(crc ^ b) & 0xFFU];
}

static uint32_t probe_crc_words(volatile const uint32_t *p, uint32_t word_count)
{
    uint32_t crc = 0xFFFFFFFFUL;
    uint32_t i;

    for (i = 0U; i < word_count; i++)
    {
        uint32_t w = p[i];
        crc = probe_crc_step(crc, (uint8_t)(w & 0xFFU));
        crc = probe_crc_step(crc, (uint8_t)((w >> 8) & 0xFFU));
        crc = probe_crc_step(crc, (uint8_t)((w >> 16) & 0xFFU));
        crc = probe_crc_step(crc, (uint8_t)((w >> 24) & 0xFFU));
    }
    return crc ^ 0xFFFFFFFFUL;
}

/* End of everything fetchable from QSPI: same span as xip_warm_cache. */
static uint32_t probe_span(void)
{
    extern uint8_t _sidata[], _sdata[], _edata[];

    return ((uint32_t)(uintptr_t)(_sidata + (_edata - _sdata)) - LQSPI_BASE);
}

static uint32_t probe_word_count(void)
{
    return probe_span() / 4U;
}

/* CRC of the image via the alias: every read reaches the QSPI bus. */
static uint32_t probe_crc_image(void)
{
    return probe_crc_words((volatile const uint32_t *)PROBE_ALIAS_BASE, probe_word_count());
}

/* CRC a zero-filled span: the "port returns all zeros" signature. */
static uint32_t probe_crc_zero_fill(void)
{
    uint32_t word_count = probe_word_count();
    uint32_t crc = 0xFFFFFFFFUL;
    uint32_t i;
    int b;

    for (i = 0U; i < word_count; i++)
    {
        for (b = 0; b < 4; b++)
        {
            crc = probe_crc_step(crc, 0U);
        }
    }
    return crc ^ 0xFFFFFFFFUL;
}

/* ------------------------------------------------------------------ */
/* Parameterized port reconfiguration (runtime twin of qspi_reconfig) */
/* ------------------------------------------------------------------ */

/**
 * @brief  Reconfigure the QSPI linear port with raw register values.
 *         Mirrors qspi_reconfig() in zynq_soc.c; deliberately duplicated
 *         so the boot path stays untouched until measurements justify
 *         changing it.
 * @note   Runs from OCM (.fast) - the CPU may be mid-fetch from the flash
 *         being reconfigured. Caller masks IRQs.
 */
static void probe_apply_raw(uint32_t clk_ctrl, uint32_t cr, uint32_t lpbk) __attribute__((section(".fast"), noinline));
static void probe_apply_raw(uint32_t clk_ctrl, uint32_t cr, uint32_t lpbk)
{
    uint32_t lqspi_cr = QSPI->LQSPI_CR;
    uint32_t i;

    /* 1. Leave linear mode. */
    QSPI->LQSPI_CR = lqspi_cr & ~(1UL << 31); /* LQ_MODE = 0 */

    /* 2. Drain RxFIFO (63 deep): stop once SR[4] = 0. */
    for (i = 0U; i < 64U; i++)
    {
        if ((QSPI->SR & (1U << 4)) == 0U)
        {
            break;
        }
        (void)QSPI->RXD;
    }

    /* 3. Program ref clock, pulse both resets (clears regs). */
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->LQSPI_CLK_CTRL = clk_ctrl;
    SLCR->LQSPI_RST_CTRL = (1U << 1) | /* QSPI_REF_RST    */
                           (1U << 0);  /* LQSPI_CPU1X_RST */
    SLCR->LQSPI_RST_CTRL = 0U;
    for (i = 0U; i < 100U; i++)
    {
        __asm volatile("nop");
    }
    SLCR->LOCK = SLCR_LOCK_KEY;

    /* 4. Reprogram controller, re-enter linear mode. */
    QSPI->CR = cr;
    QSPI->LPBK_DLY_ADJ = lpbk;
    QSPI->LQSPI_CR = lqspi_cr; /* LQ_MODE = 1 */
    QSPI->ER = 1U;             /* SPI_EN = 1  */
}

/* Restore the shipping config (safe endpoint of every probe window). */
static void probe_apply_golden(void)
{
    probe_apply_raw(PROBE_CLK_CTRL, PROBE_GOLDEN_CR, PROBE_GOLDEN_LPBK);
}

/* Sweep loopback DLY (dead on this board - included for completeness). */
#define PROBE_SWEEP_DLY (0x0DU)

/* Sweep-template config: SCLK = 200MHz/2>>div, loopback DLY = dly.
   NOTE: any use enables the (dead) loopback path - sweep only. */
static void probe_apply_config(uint32_t div, uint32_t dly)
{
    probe_apply_raw(PROBE_CLK_CTRL, PROBE_CR_BASE | ((div & 0x7U) << 3), (1UL << 5) | (dly & 0x1FU));
}

static const char *probe_tag(uint32_t crc, uint32_t ref, uint32_t zero_crc)
{
    if (crc == ref)
    {
        return "==";
    }
    return (crc == zero_crc) ? "ZEROS" : "!=";
}

/* ------------------------------------------------------------------ */
/* Commands                                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief  Single-beat sweep via the alias: one line per (div, dly).
 *         Compare against host-side zlib.crc32(app.bin).
 * @note   IRQ-masked windows of a few ms each stall USB; never call
 *         during a CDC throughput run.
 */
void probe_sweep(void)
{
    uint32_t golden;
    uint32_t zero_crc;
    uint32_t div;
    uint32_t dly;

    probe_crc_table_init();
    zero_crc = probe_crc_zero_fill();

    printf("probe: span=%lu bytes via alias 0x%08lX, zeros=%08lX\n", (unsigned long)probe_span(),
           (unsigned long)PROBE_ALIAS_BASE, (unsigned long)zero_crc);

    /* Golden measurement doubles as the preheat: the whole CRC code path
       lands in L1I/L2 here, and alias reads cannot evict it. */
    golden = probe_crc_image();
    printf("probe: golden (shipping) crc=%08lX%s\n", (unsigned long)golden,
           (golden == zero_crc) ? " WARNING: reads return all zeros" : "");

    for (div = 0U; div < PROBE_DIV_COUNT; div++)
    {
        for (dly = 0U; dly < PROBE_DLY_COUNT; dly++)
        {
            uint32_t crc;

            __disable_irq();
            probe_apply_config(div, dly);
            crc = probe_crc_image();
            probe_apply_golden();
            __enable_irq();

            printf("probe: div=%lu sclk=%luMHz dly=%2lu crc=%08lX %s\n", (unsigned long)div,
                   (unsigned long)(200UL / (2UL << div)), (unsigned long)dly, (unsigned long)crc,
                   probe_tag(crc, golden, zero_crc));
        }
    }

    printf("probe: restored, re-read crc=%08lX\n", (unsigned long)probe_crc_image());
}

/**
 * @brief  Burst (linefill) sweep: per divider, invalidate L1D+L2 over the
 *         XIP window and CRC through the cached mapping, so every read is
 *         a burst through the port - the access pattern of code fetches.
 * @note   If the golden burst path is broken, the post-window printf may
 *         never appear (its code re-fills from a dead port): a frozen LED
 *         is the verdict; power-cycle to recover.
 */
void probe_burst(void)
{
    uint32_t span = probe_span();
    uint32_t word_count = probe_word_count();
    uint32_t zero_crc;
    uint32_t div;

    probe_crc_table_init();
    zero_crc = probe_crc_zero_fill();

    printf("probe: burst sweep, span=%lu via cached window (L1D+L2 inv per pass)\n", (unsigned long)span);

    /* Preheat under the golden config: pull this path's code into L1I and
       run cache_invalidate once, so nothing in the window is a first call. */
    cache_invalidate((const void *)LQSPI_BASE, 32U);
    (void)probe_crc_words((volatile const uint32_t *)LQSPI_BASE, 8U);

    for (div = 0U; div < PROBE_DIV_COUNT; div++)
    {
        uint32_t crc;

        __disable_irq();
        probe_apply_config(div, PROBE_SWEEP_DLY);
        cache_invalidate((const void *)LQSPI_BASE, span);
        crc = probe_crc_words((volatile const uint32_t *)LQSPI_BASE, word_count);
        cache_invalidate((const void *)LQSPI_BASE, span);
        probe_apply_golden();
        __enable_irq();

        printf("probe: burst div=%lu sclk=%luMHz crc=%08lX %s\n", (unsigned long)div,
               (unsigned long)(200UL / (2UL << div)), (unsigned long)crc, (crc == zero_crc) ? "ZEROS" : "----");
    }

    printf("probe: burst done (LED frozen = restore-path fetch died, power-cycle)\n");
}

/**
 * @brief  Print the BootROM-era QSPI register snapshot captured at
 *         soc_init() entry - the only known-good port configuration.
 */
void probe_boot_cfg(void)
{
    const soc_qspi_boot_cfg *cfg = soc_qspi_boot_cfg_get();

    printf("probe: bootrom lqspi_clk=%08lX cr=%08lX lqspi_cr=%08lX lpbk=%08lX er=%08lX\n",
           (unsigned long)cfg->lqspi_clk_ctrl, (unsigned long)cfg->cr, (unsigned long)cfg->lqspi_cr,
           (unsigned long)cfg->lpbk_dly_adj, (unsigned long)cfg->er);
}

/**
 * @brief  Replicate the BootROM config at runtime and CRC via the alias,
 *         then climb BAUD_RATE_DIV up from it (ref = ARM PLL/40 = 33.33 MHz
 *         both before and after sys_clock_init, SCLK = ref/2>>div). A line
 *         matching the host truth (printed at build time) is a proven
 *         working config; the fastest such line is the adoption candidate.
 * @note   Bursts at the BootROM config are already proven by the boot-time
 *         warm (its content passes the ZBLZ CRC), so a single-beat pass
 *         here completes the proof for both access patterns.
 */
void probe_bootrom(void)
{
    const soc_qspi_boot_cfg *boot = soc_qspi_boot_cfg_get();
    uint32_t zero_crc;
    uint32_t crc;
    uint32_t div;

    probe_crc_table_init();
    zero_crc = probe_crc_zero_fill();

    /* Preheat every code path used inside the windows (wrapper included)
       while the port is still in a known state. */
    probe_apply_golden();
    (void)probe_crc_image();

    printf("probe: bootrom replication (host truth = crc32 printed at build time)\n");

    __disable_irq();
    probe_apply_raw(boot->lqspi_clk_ctrl, boot->cr, boot->lpbk_dly_adj);
    crc = probe_crc_image();
    probe_apply_golden();
    __enable_irq();

    printf("probe: bootrom exact (sclk=%lukHz) crc=%08lX %s\n", PROBE_BOOTROM_REF_KHZ / 4UL, (unsigned long)crc,
           (crc == zero_crc) ? "ZEROS" : "----");

    /* Climb: BootROM clk + loopback, only BAUD_RATE_DIV varies. */
    for (div = 0U; div < PROBE_DIV_COUNT; div++)
    {
        uint32_t cr = (boot->cr & ~(0x7UL << 3)) | (div << 3);

        __disable_irq();
        probe_apply_raw(boot->lqspi_clk_ctrl, cr, boot->lpbk_dly_adj);
        crc = probe_crc_image();
        probe_apply_golden();
        __enable_irq();

        printf("probe: bootrom-clk div=%lu sclk=%lukHz crc=%08lX %s\n", (unsigned long)div,
               PROBE_BOOTROM_REF_KHZ / (2UL << div), (unsigned long)crc, (crc == zero_crc) ? "ZEROS" : "----");
    }

    printf("probe: bootrom done, restored crc=%08lX\n", (unsigned long)probe_crc_image());
}

/**
 * @brief  Direct-sampling sweep: LPBK_DLY_ADJ = 0 (no loopback, so no
 *         feedback clock is needed), REF in {200, 250} MHz (both > CPU_1x
 *         per the UG585 ch. 12 ratio rule) over all 8 BAUD dividers, plus
 *         one control point on the BootROM clock tree (REF = 33.33 MHz,
 *         ratio-violating at runtime) with LPBK = 0. Every prior failure
 *         maps onto a manual violation (loopback with BAUD != 000, dead
 *         MIO8 feedback path, REF < CPU_1x); this sweep contains only
 *         legal configs, so a line matching the host truth is adoptable.
 */
void probe_direct(void)
{
    static const struct
    {
        uint32_t clk_ctrl;
        uint32_t ref_mhz;
    } refs[] = {
        {(5UL << 8) | 1UL, 200UL}, /* IO PLL/5: current tree    */
        {(4UL << 8) | 1UL, 250UL}, /* IO PLL/4: more ratio room */
    };
    const soc_qspi_boot_cfg *boot = soc_qspi_boot_cfg_get();
    struct clock_freqs freqs;
    uint32_t zero_crc;
    uint32_t r;
    uint32_t div;

    probe_crc_table_init();
    zero_crc = probe_crc_zero_fill();
    clock_freqs_get(&freqs);

    printf("probe: cpu_1x=%luHz qspi_ref=%luHz mio_pin_08=%08lX\n", (unsigned long)freqs.cpu_1x,
           (unsigned long)freqs.qspi_ref, (unsigned long)SLCR->MIO_PIN_08);
    printf("probe: direct sweep, LPBK=0 (host truth = crc32 printed at build time)\n");

    /* Preheat every window code path under the golden config. */
    probe_apply_golden();
    (void)probe_crc_image();

    for (r = 0U; r < (sizeof(refs) / sizeof(refs[0])); r++)
    {
        for (div = 0U; div < 8U; div++)
        {
            uint32_t crc;

            __disable_irq();
            probe_apply_raw(refs[r].clk_ctrl, PROBE_CR_BASE | (div << 3), 0U);
            crc = probe_crc_image();
            probe_apply_golden();
            __enable_irq();

            printf("probe: ref=%luMHz baud=/%lu sclk=%lukHz crc=%08lX %s\n", (unsigned long)refs[r].ref_mhz,
                   (unsigned long)(2UL << div), (unsigned long)(refs[r].ref_mhz * 1000UL / (2UL << div)),
                   (unsigned long)crc, (crc == zero_crc) ? "ZEROS" : "----");
        }
    }

    /* Control: BootROM clock tree + CR but LPBK=0. REF=33.33MHz violates
       the REF > CPU_1x rule at runtime; a pass here would indict USE_LPBK
       instead of the ratio for the replica failure. */
    __disable_irq();
    probe_apply_raw(boot->lqspi_clk_ctrl, boot->cr, 0U);
    {
        uint32_t crc = probe_crc_image();

        probe_apply_golden();
        __enable_irq();
        printf("probe: ref=33MHz baud=/4 sclk=8333kHz crc=%08lX %s (control)\n", (unsigned long)crc,
               (crc == zero_crc) ? "ZEROS" : "----");
    }

    printf("probe: direct done, restored crc=%08lX\n", (unsigned long)probe_crc_image());
}

/* "Legal" config == the shipping config since 2026-10-07: REF = 200 MHz
   (> CPU_1x = 111 MHz), BAUD /8 = SCLK 25 MHz (< FQSPICLK2 ~40 MHz, no
   feedback clock needed), LPBK = 0 (direct pad sampling). */
#define PROBE_LEGAL_CR (PROBE_GOLDEN_CR)

/* ICIALLU + barriers: force every subsequent fetch to miss L1I. The CMSIS
   subset has no icache op, so issue the CP15 write directly. */
static inline void probe_icache_inv_all(void)
{
    __asm volatile("mcr p15, 0, %0, c7, c5, 0" ::"r"(0U) : "memory");
    __DSB();
    __ISB();
}

/**
 * @brief  Word-by-word mismatch scan at the legal config: alias (through
 *         the port) vs cached window (warm-L2 hit = boot-proven truth).
 *         The cached pass runs first (pure warm hits; alias reads never
 *         allocate), then the scan reports only differing words. This
 *         localizes the config-independent corruption exactly, with no
 *         host-side capture needed.
 */
void probe_mismatch(void)
{
#define PROBE_MAX_SAMPLES (24U)
    static uint32_t s_off[PROBE_MAX_SAMPLES];
    static uint32_t s_alias[PROBE_MAX_SAMPLES];
    static uint32_t s_cached[PROBE_MAX_SAMPLES];
    volatile const uint32_t* alias = (volatile const uint32_t*)PROBE_ALIAS_BASE;
    volatile const uint32_t* cached = (volatile const uint32_t*)LQSPI_BASE;
    uint32_t word_count = probe_word_count();
    uint32_t mismatches = 0U;
    uint32_t crc_cached;
    uint32_t crc_alias;
    uint32_t i;

    probe_crc_table_init();

    /* Preheat every window code path under the golden config. */
    probe_apply_golden();
    (void)probe_crc_image();

    printf("probe: mismatch scan @ legal cfg (ref200 baud/8 lpbk0), %lu words\n",
           (unsigned long)word_count);

    __disable_irq();
    probe_apply_raw(PROBE_CLK_CTRL, PROBE_LEGAL_CR, 0U);
    crc_cached = probe_crc_words(cached, word_count); /* warm-L2 reference */
    crc_alias = probe_crc_words(alias, word_count);
    for (i = 0U; i < word_count; i++) {
        uint32_t a = alias[i];
        uint32_t c = cached[i];

        if (a != c) {
            if (mismatches < PROBE_MAX_SAMPLES) {
                s_off[mismatches] = i * 4U;
                s_alias[mismatches] = a;
                s_cached[mismatches] = c;
            }
            mismatches++;
        }
    }
    probe_apply_golden();
    __enable_irq();

    printf("probe: crc_cached=%08lX crc_alias=%08lX mismatches=%lu\n", (unsigned long)crc_cached,
           (unsigned long)crc_alias, (unsigned long)mismatches);
    for (i = 0U; (i < PROBE_MAX_SAMPLES) && (i < mismatches); i++) {
        printf("probe: diff off=%06lX alias=%08lX cached=%08lX\n", (unsigned long)s_off[i],
               (unsigned long)s_alias[i], (unsigned long)s_cached[i]);
    }
}

/* Block size for probe_blocks(): small enough to localize, few enough
   lines to eyeball (160604 / 1024 = 157 blocks). */
#define PROBE_BLOCK_BYTES (1024U)
#define PROBE_MAX_BLOCKS (256U)

/**
 * @brief  Per-block CRC32 dump at the legal config: each block's CRC is an
 *         independent zlib-style CRC32, so the host can localize the
 *         flash-vs-file difference block by block with one line of Python
 *         (crc32(app.bin[i*1024:(i+1)*1024]) per block, diff the lists).
 */
void probe_blocks(void)
{
    static uint32_t s_blk[PROBE_MAX_BLOCKS];
    volatile const uint32_t* alias = (volatile const uint32_t*)PROBE_ALIAS_BASE;
    uint32_t span = probe_span();
    uint32_t block_count = (span + PROBE_BLOCK_BYTES - 1U) / PROBE_BLOCK_BYTES;
    uint32_t b;

    probe_crc_table_init();

    /* Preheat the window code path under the golden config. */
    probe_apply_golden();
    (void)probe_crc_image();

    printf("probe: block crcs @ legal cfg, %lu blocks x %lu bytes\n", (unsigned long)block_count,
           (unsigned long)PROBE_BLOCK_BYTES);

    if (block_count > PROBE_MAX_BLOCKS) {
        printf("probe: too many blocks, abort\n");
        return;
    }

    __disable_irq();
    probe_apply_raw(PROBE_CLK_CTRL, PROBE_LEGAL_CR, 0U);
    for (b = 0U; b < block_count; b++) {
        uint32_t off = b * PROBE_BLOCK_BYTES;
        uint32_t len = span - off;

        if (len > PROBE_BLOCK_BYTES) {
            len = PROBE_BLOCK_BYTES;
        }
        s_blk[b] = probe_crc_words(alias + off / 4U, len / 4U);
    }
    probe_apply_golden();
    __enable_irq();

    for (b = 0U; b < block_count; b++) {
        printf("probe: blk %03lu crc=%08lX\n", (unsigned long)b, (unsigned long)s_blk[b]);
    }
}

/**
 * @brief  Go-live endurance pass: re-apply the (now shipping) config,
 *         evict L1D+L2 over the whole XIP window and invalidate L1I, then
 *         CRC through the cached window - every byte, code included,
 *         re-fills through the port. The system keeps running on the
 *         shipping config afterwards (a frozen LED would be the verdict).
 *         This is the no-warm-up XIP proof; passed 2026-10-07.
 */
void probe_golive(void)
{
    uint32_t span = probe_span();
    uint32_t crc;

    probe_crc_table_init();

    /* Preheat the maintenance + CRC code path once under golden. */
    cache_invalidate((const void*)LQSPI_BASE, 32U);
    (void)probe_crc_words((volatile const uint32_t*)LQSPI_BASE, 8U);

    printf("probe: go-live ref200 baud/8 lpbk0 (25MHz direct), NO restore\n");

    __disable_irq();
    probe_apply_raw(PROBE_CLK_CTRL, PROBE_LEGAL_CR, 0U);
    cache_invalidate((const void*)LQSPI_BASE, span);
    probe_icache_inv_all();
    crc = probe_crc_words((volatile const uint32_t*)LQSPI_BASE, probe_word_count());
    __enable_irq();

    printf("probe: LIVE crc=%08lX (truth = build-time crc32); system stays on legal cfg\n",
           (unsigned long)crc);
    probe_frozen = 1;
}
