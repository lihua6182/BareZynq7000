#include "zynq_devcfg.h"

#include <string.h>

#include "zynq_common.h"
#include "zynq_soc_ip.h"

/* slcr.LVL_SHFTR_EN values, UG585 ch.2 power-up sequence. */
#define LVL_SHFTR_PS_TO_PL (0xAUL) /* step 2: PS->PL only, before programming */
#define LVL_SHFTR_ALL      (0xFUL) /* step 5: both directions, after DONE     */

/* slcr.PCAP_CLK_CTRL bit 0 (CLKACT). The bitstream datapath is clocked by
   the PCAP clock, a divided-down PCAP_2x (UG585 ch.6 "PCAP Clocking"). Reset
   value 0x00000F01 already has CLKACT set, but the load says it out loud so
   it also works if something else gated the clock off. */
#define PCAP_CLK_CTRL_CLKACT (0x1UL)

/* Busy-poll limits. A devcfg register read over the APB bridge is on the
   order of 100 ns, so these bound the wait at roughly:

     DEVCFG_INIT_POLL_LIMIT  -> ~1 s   (PL power-up, TPOR is max ~50 ms)
     DEVCFG_DMA_POLL_LIMIT   -> ~10 s  (safety net for a stuck DMA)

   The DMA wait is the long one: it covers the whole bitstream transfer plus
   the PCAP programming that follows it, and the loop body is a single
   strongly-ordered load, so the bound is a fixed number of bus cycles
   instead of a wall-clock deadline. That keeps this boot-time path working
   before the timer tick and the GIC exist. The limit is per DMA command, so
   the packed path's ~500 chunked transfers cannot accumulate it. */
#define DEVCFG_INIT_POLL_LIMIT (8000000UL)
#define DEVCFG_DMA_POLL_LIMIT  (60000000UL)

/* Transport-level errors that abort a load (UG585 step 10). */
#define DEVCFG_XFER_ERR_Msk (DEVCFG_IXR_DMA_CMD_ERR_Msk |   \
                             DEVCFG_IXR_DMA_Q_OV_Msk |      \
                             DEVCFG_IXR_P2D_LEN_ERR_Msk |   \
                             DEVCFG_IXR_RX_FIFO_OV_Msk |    \
                             DEVCFG_IXR_AXI_WERR_Msk |      \
                             DEVCFG_IXR_AXI_WTO_Msk |       \
                             DEVCFG_IXR_AXI_RERR_Msk |      \
                             DEVCFG_IXR_AXI_RTO_Msk |       \
                             DEVCFG_IXR_PCFG_HMAC_ERR_Msk | \
                             DEVCFG_IXR_PCFG_SEU_ERR_Msk)

/*
 * Packed image format, produced by tools/rle_bitstream.py (see there for the
 * full story). A 16-byte little-endian header followed by an RLE payload in
 * the SEGGER BSP_FPGA.c encoding; the payload is the PCAP-form image, so the
 * decoder can stream straight into the DevC DMA with no byte-order fixup:
 *
 *   0x00: magic        0x5A425350 "ZBSP" - a bitgen image can never start
 *                      with this word (it starts with 0xFFFFFFFF padding),
 *                      so it doubles as the load-path selector
 *   0x04: raw_bytes    length of the uncompressed PCAP-form image, % 4 == 0
 *   0x08: packed_bytes length of the RLE payload after the header
 *   0x0C: crc32        CRC-32 (EDB88320, zlib-compatible) over the raw image
 */
#define DEVCFG_PACKED_MAGIC     (0x5A425350UL) /* ASCII "ZBSP"            */
#define DEVCFG_PACKED_HDR_BYTES (16UL)
#define DEVCFG_CRC_POLY         (0xEDB88320UL)

/* bitgen pads the image header with bus-detection words (0xFF, 000000BB,
   11220044) before the sync word, and the sync word stays inside the first
   128 bytes in every known 7-series layout - so a fixed scan window is
   enough and the window is never file-sized. */
#define DEVCFG_SYNC_SCAN_BYTES (128UL)

/* Staging buffer between the RLE decoder and the DevC DMA. 4 KB because it
   must hold the widest single marker output (128 bytes) with room to spare,
   and because the per-chunk DMA overhead (a few register writes and one
   poll) is noise at that size. 64-byte aligned: the DMA emits 64-byte
   bursts, and a 64-byte-aligned burst never straddles a 4 KB page - see the
   alignment note in zynq_devcfg.h. Lives in cacheable OCM, so each decoded
   chunk is cleaned to RAM before the DMA touches it. */
#define DEVCFG_STAGE_BYTES (4096UL)
#define DEVCFG_STAGE_LINE  (32UL) /* Cortex-A9 L1 D-cache line */
static uint32_t s_devcfg_stage[DEVCFG_STAGE_BYTES / 4UL] __attribute__((aligned(64)));

typedef struct {
    uint32_t magic;
    uint32_t raw_bytes;
    uint32_t packed_bytes;
    uint32_t crc32;
} devcfg_packed_hdr_t;

/*
 * Acknowledge everything the DevC DMA has finished: the interrupt flag and
 * the DMA_DONE_CNT field. UG585: a finished transfer "is acknowledged by
 * clearing the DMA_DONE_INT interrupt status flag", and the count itself
 * "is cleared by writing a 1 to either bit location". Doing both before
 * queueing a command - as SEGGER's BSP_FPGA.c does - keeps a stale
 * completion from being mistaken for the one being waited on.
 */
static void zynq_devcfg_dma_ack(void) {
    DEVCFG->INT_STS = DEVCFG_IXR_DMA_DONE_Msk;
    DEVCFG->STATUS |= DEVCFG_STATUS_DMA_DONE_CNT_Msk;
}

/* The PCAP consumes 32-bit words little-endian: a loadable image shows the
   0xAA995566 sync word as the bytes 66 55 99 AA (the "byte swapped .bin"
   the kernel's zynq-fpga driver requires; its zynq_fpga_has_sync does the
   same check). bitgen's native big-endian .bin carries it byte-reversed,
   and the failure it produces is nasty: the DMA happily pushes all 2 MB and
   the PL silently ignores the stream, so the only symptom is a generic
   NO_DONE seconds later. Scanning here turns that mistake into a specific
   status; the build embeds the image pre-swapped (tools/rle_bitstream.py),
   so this is the safety net for a hand-replaced image. Returns non-zero
   when img carries the sync word in the little-endian byte order. */
static int zynq_devcfg_has_sync(const uint8_t* img, uint32_t bytes) {
    uint32_t limit;
    uint32_t i;

    limit = (bytes < DEVCFG_SYNC_SCAN_BYTES) ? bytes : DEVCFG_SYNC_SCAN_BYTES;
    for (i = 0U; (i + 4UL) <= limit; i++) {
        if ((img[i] == 0x66U) && (img[i + 1U] == 0x55U) &&
            (img[i + 2U] == 0x99U) && (img[i + 3U] == 0xAAU)) {
            return 1;
        }
    }
    return 0;
}

/*
 * CRC-32 (reflected, poly 0xEDB88320 - the zlib/zmodem polynomial SEGGER's
 * SEGGER_CRC_Calc_EDB88320 also uses), table-driven. The table costs 1 KB of
 * .bss and ~2k cycles to build; over a ~2 MB image the table variant runs
 * ~5x faster than the bitwise loop, which keeps the packed path's integrity
 * check in the low tens of milliseconds. crc runs without the final XOR so
 * it can be carried across staging-buffer chunks; finish with ~crc.
 */
static uint32_t s_devcfg_crc_table[256];

static void zynq_devcfg_crc_init(void) {
    uint32_t i;
    uint32_t v;
    uint32_t b;

    for (i = 0U; i < 256U; i++) {
        v = i;
        for (b = 0U; b < 8U; b++) {
            v = ((v & 1UL) != 0UL) ? (DEVCFG_CRC_POLY ^ (v >> 1U)) : (v >> 1U);
        }
        s_devcfg_crc_table[i] = v;
    }
}

static uint32_t zynq_devcfg_crc_update(uint32_t crc, const uint8_t* p, uint32_t bytes) {
    uint32_t i;

    for (i = 0U; i < bytes; i++) {
        crc = (crc >> 8U) ^ s_devcfg_crc_table[(crc ^ p[i]) & 0xFFU];
    }
    return crc;
}

/* Clean the decoded prefix of the staging buffer to RAM. The DevC DMA reads
   via the ACP, which is coherent with L2 but not with the Cortex-A9 L1, so
   write-back lines the decoder just filled must be pushed down first
   (SEGGER's OS_ARM_DCACHE_CleanRange, for the same reason). The CPU never
   reads the buffer, so invalidation is not needed. */
static void zynq_devcfg_stage_clean(uint32_t bytes) {
    uint32_t addr = (uint32_t)(uintptr_t)s_devcfg_stage;
    uint32_t end = addr + ((bytes + (DEVCFG_STAGE_LINE - 1UL)) & ~(DEVCFG_STAGE_LINE - 1UL));

    while (addr < end) {
        L1C_CleanDCacheMVA((void*)(uintptr_t)addr);
        addr += DEVCFG_STAGE_LINE;
    }
    __DSB();
}

/*
 * UG585 ch.6 steps 1-6 (minus the DMA itself): bring the PL and the PCAP
 * bridge into the "ready for data" state. Shared by both load paths.
 */
static zynq_devcfg_status_t zynq_devcfg_begin(void) {
    uint32_t ctrl;
    uint32_t i;

    /* UG585 ch.6 "Determine the PL State": the PL must be powered before the
       DevC - and therefore the PCAP bridge - can be used at all.
       MCTRL[PCFG_POR_B] is the power flag, so a dead PL is reported here
       instead of surfacing later as a PCFG_INIT timeout. */
    if ((DEVCFG->MCTRL & DEVCFG_MCTRL_PCFG_POR_B_Msk) == 0U) {
        return ZYNQ_DEVCFG_ERR_PL_NO_POWER;
    }

    /* The PCAP datapath runs on slcr.PCAP_CLK_CTRL (UG585 ch.6 "PCAP
       Clocking"). Enable it here rather than in zynq_devcfg_init() so a
       second load (reconfiguration) cannot start with the clock gated. */
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->PCAP_CLK_CTRL |= PCAP_CLK_CTRL_CLKACT;
    SLCR->LOCK = SLCR_LOCK_KEY;

    /* Step 3 (optional in UG585, needed here): the BootROM never brought the
       PL up on this board, so pulse PCFG_PROG_B to run PL housecleaning and
       clear any previous configuration.
       a: 1 -> b: 0 -> c: wait for PCFG_INIT = 0 -> d: 1.
       The devcfg registers live in the strongly-ordered MMU region, so the
       writes reach the bridge in program order without extra barriers. */
    ctrl = DEVCFG->CTRL;
    DEVCFG->CTRL = ctrl | DEVCFG_CTRL_PCFG_PROG_B_Msk;
    DEVCFG->CTRL = ctrl & ~DEVCFG_CTRL_PCFG_PROG_B_Msk;
    for (i = 0U; i < DEVCFG_INIT_POLL_LIMIT; i++) {
        if ((DEVCFG->STATUS & DEVCFG_STATUS_PCFG_INIT_Msk) == 0U) {
            break;
        }
    }
    if (i == DEVCFG_INIT_POLL_LIMIT) {
        return ZYNQ_DEVCFG_ERR_PL_NOT_RESET;
    }
    DEVCFG->CTRL = ctrl | DEVCFG_CTRL_PCFG_PROG_B_Msk;

    /* Step 3e: clear the (now stale) PL configuration done interrupt. */
    DEVCFG->INT_STS = DEVCFG_IXR_PCFG_DONE_Msk;

    /* Step 4: the PL is ready for PCAP data once housecleaning is done. */
    for (i = 0U; i < DEVCFG_INIT_POLL_LIMIT; i++) {
        if ((DEVCFG->STATUS & DEVCFG_STATUS_PCFG_INIT_Msk) != 0U) {
            break;
        }
    }
    if (i == DEVCFG_INIT_POLL_LIMIT) {
        return ZYNQ_DEVCFG_ERR_PL_NOT_READY;
    }

    /* Steps 5 and 6: there must be room in the DMA command queue, and the
       PCAP loopback must be off. */
    if ((DEVCFG->STATUS & DEVCFG_STATUS_DMA_CMD_Q_F_Msk) != 0U) {
        return ZYNQ_DEVCFG_ERR_CMD_Q_FULL;
    }
    DEVCFG->MCTRL &= ~DEVCFG_MCTRL_PCAP_LPBK_Msk;

    /* Drop any completion left over from an earlier transfer before looking
       for the one this load is going to raise. */
    zynq_devcfg_dma_ack();

    return ZYNQ_DEVCFG_OK;
}

/*
 * UG585 ch.6 steps 8-10 for one DMA command: queue it, wait for the data to
 * be consumed, acknowledge it. Used once per staging-buffer chunk on the
 * packed path and once overall on the direct path.
 *
 * 8: the four DMA registers are written in the order UG585 mandates (source
 *    address, destination address, source length, destination length)
 *    because writing DMA_DEST_LEN is what moves the command into the queue
 *    and starts the transfer.
 * 8b: the magic destination address routes the data to the PCAP instead of
 *     to memory.
 * 9/10: wait for DMA_DONE, bail out early on any transport error - a
 *     bitstream that faults halfway through leaves the PL unconfigured, so
 *     there is no point waiting for the rest.
 */
static zynq_devcfg_status_t zynq_devcfg_dma_xfer(const void* src, uint32_t words) {
    uint32_t i;
    uint32_t sts;

    if ((DEVCFG->STATUS & DEVCFG_STATUS_DMA_CMD_Q_F_Msk) != 0U) {
        return ZYNQ_DEVCFG_ERR_CMD_Q_FULL;
    }

    DEVCFG->DMA_SRC_ADDR = (uint32_t)(uintptr_t)src;
    DEVCFG->DMA_DEST_ADDR = DEVCFG_DMA_INVALID_ADDRESS; /* 8b: PCAP */
    DEVCFG->DMA_SRC_LEN = words;
    __DSB();
    DEVCFG->DMA_DEST_LEN = words; /* 8d: triggers the transfer */

    for (i = 0U; i < DEVCFG_DMA_POLL_LIMIT; i++) {
        sts = DEVCFG->INT_STS;
        if ((sts & DEVCFG_XFER_ERR_Msk) != 0U) {
            DEVCFG->INT_STS = sts; /* W1C: drop the latched error bits */
            return ZYNQ_DEVCFG_ERR_DMA;
        }
        if ((sts & DEVCFG_IXR_DMA_DONE_Msk) != 0U) {
            break;
        }
    }
    if (i == DEVCFG_DMA_POLL_LIMIT) {
        return ZYNQ_DEVCFG_ERR_DMA_TIMEOUT;
    }

    /* Acknowledge the transfer the DMA just finished: INT_STS is write-1-
       to-clear and DMA_DONE_CNT only goes back to 0 when software says so. */
    zynq_devcfg_dma_ack();
    return ZYNQ_DEVCFG_OK;
}

/* UG585 ch.6 step 11 plus ch.2 step 5: wait for the PL to assert DONE, then
   enable the PL->PS level shifters so PS and PL signals can be used. */
static zynq_devcfg_status_t zynq_devcfg_finish(void) {
    uint32_t i;

    for (i = 0U; i < DEVCFG_DMA_POLL_LIMIT; i++) {
        if ((DEVCFG->INT_STS & DEVCFG_IXR_PCFG_DONE_Msk) != 0U) {
            break;
        }
    }
    DEVCFG->INT_STS = DEVCFG_IXR_PCFG_DONE_Msk;
    if (i == DEVCFG_DMA_POLL_LIMIT) {
        return ZYNQ_DEVCFG_ERR_NO_DONE;
    }

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->LVL_SHFTR_EN = LVL_SHFTR_ALL;
    SLCR->LOCK = SLCR_LOCK_KEY;

    return ZYNQ_DEVCFG_OK;
}

/*
 * Direct path: the image is the raw PCAP-form bitstream, handed to the DevC
 * DMA in one command, FSBL style. Kept as the fallback and debug escape
 * hatch; the build normally packs the image (magic selector below).
 */
static zynq_devcfg_status_t zynq_devcfg_load_direct(const uint8_t* img, uint32_t bytes) {
    if (zynq_devcfg_has_sync(img, bytes) == 0) {
        return ZYNQ_DEVCFG_ERR_NO_SYNC;
    }
    return zynq_devcfg_dma_xfer(img, bytes / 4UL);
}

/* Parse and validate the packed header. Pure memory work, so a malformed
   blob is rejected before any PL hardware is touched. */
static zynq_devcfg_status_t zynq_devcfg_packed_hdr(const uint8_t* img, uint32_t bytes,
                                                   devcfg_packed_hdr_t* hdr) {
    devcfg_packed_hdr_t h;

    if (bytes < DEVCFG_PACKED_HDR_BYTES) {
        return ZYNQ_DEVCFG_ERR_HDR;
    }
    memcpy(&h, img, sizeof(h));
    if (h.magic != DEVCFG_PACKED_MAGIC) {
        return ZYNQ_DEVCFG_ERR_HDR;
    }
    /* raw_bytes must be a non-empty word stream; packed_bytes must fit in
       the caller-provided image (bytes may exceed 16 + packed_bytes: the
       linker pads __bitstream_end__ up to 4). */
    if ((h.raw_bytes < 4UL) || ((h.raw_bytes & 3UL) != 0UL)) {
        return ZYNQ_DEVCFG_ERR_HDR;
    }
    if ((h.packed_bytes == 0UL) ||
        (h.packed_bytes > (bytes - DEVCFG_PACKED_HDR_BYTES))) {
        return ZYNQ_DEVCFG_ERR_HDR;
    }
    *hdr = h;
    return ZYNQ_DEVCFG_OK;
}

/*
 * Packed path: RLE-decode the payload into the staging buffer and hand each
 * decoded chunk to the DevC DMA. SEGGER's BSP_FPGA.c is the reference for
 * this shape of loop (it stages through 64 bytes where we stage through
 * 4 KB); the same two DevC constraints drive the tail handling:
 *   (1) the DMA only transfers whole 32-bit words - a decoded chunk that is
 *       not a multiple of 4 carries its remainder to the front of the buffer
 *       and continues after the transfer;
 *   (2) the DMA source must be 64-byte aligned - s_devcfg_stage is, and it
 *       never moves.
 * The decoder is deliberately paranoid: every marker read is bounds-checked
 * against the payload end, the decoded total against raw_bytes, and the
 * reassembled image against the header CRC. The sync-word scan of the first
 * decoded bytes keeps the historical "swapped the wrong way" mistake a
 * specific error instead of a NO_DONE after 2 MB of garbage.
 */
static zynq_devcfg_status_t zynq_devcfg_load_packed(const uint8_t* img,
                                                    const devcfg_packed_hdr_t* hdr) {
    const uint8_t* pay = img + DEVCFG_PACKED_HDR_BYTES;
    const uint8_t* pay_end = pay + hdr->packed_bytes;
    uint8_t* stage = (uint8_t*)s_devcfg_stage;
    uint32_t used = 0UL; /* bytes in the staging buffer (incl. <4 carry) */
    uint32_t decoded = 0UL;
    uint32_t crc = 0xFFFFFFFFUL;
    int synced = 0;
    int stream_done = 0;
    zynq_devcfg_status_t r;

    zynq_devcfg_crc_init();
    for (;;) {
        /* Decode markers into the free tail of the staging buffer. Every
           marker emits at most 128 bytes, so "128 bytes free" is enough to
           always make progress. The loop stops only on the end marker or a
           nearly-full buffer; overshooting raw_bytes is caught per marker
           below, which also keeps the end marker reachable when the last
           data marker lands decoded exactly on raw_bytes. */
        while ((stream_done == 0) && ((used + 128UL) <= DEVCFG_STAGE_BYTES)) {
            uint32_t c0 = *pay++;
            if (c0 == 0UL) { /* end marker */
                stream_done = 1;
            } else if (c0 <= 0x3FUL) { /* literal run */
                if ((pay + c0) > pay_end) {
                    return ZYNQ_DEVCFG_ERR_STREAM;
                }
                memcpy(&stage[used], pay, c0);
                pay += c0;
                used += c0;
                decoded += c0;
            } else if (c0 <= 0x7FUL) { /* byte repeat run */
                uint32_t n = c0 - 0x3FUL;
                if (pay >= pay_end) {
                    return ZYNQ_DEVCFG_ERR_STREAM;
                }
                memset(&stage[used], *pay++, n);
                used += n;
                decoded += n;
            } else { /* zero run */
                uint32_t n = c0 - 0x7FUL;
                memset(&stage[used], 0, n);
                used += n;
                decoded += n;
            }
            if (decoded > hdr->raw_bytes) {
                return ZYNQ_DEVCFG_ERR_STREAM; /* payload exceeds declared image */
            }
        }

        if ((!synced) && (used != 0UL)) {
            uint32_t scan = (used < DEVCFG_SYNC_SCAN_BYTES) ? used : DEVCFG_SYNC_SCAN_BYTES;
            if (zynq_devcfg_has_sync(stage, scan) == 0) {
                return ZYNQ_DEVCFG_ERR_NO_SYNC;
            }
            synced = 1;
        }

        /* DMA the whole words out; carry the <4 byte remainder to the front
           for the next round (constraint (1) above). raw_bytes is a multiple
           of 4, so at a clean end-of-stream the carry is always empty. */
        {
            uint32_t words = used / 4UL;
            uint32_t rem = used & 3UL;

            if (words != 0UL) {
                zynq_devcfg_stage_clean(words * 4UL);
                crc = zynq_devcfg_crc_update(crc, stage, words * 4UL);
                r = zynq_devcfg_dma_xfer(s_devcfg_stage, words);
                if (r != ZYNQ_DEVCFG_OK) {
                    return r;
                }
            }
            if (rem != 0UL) {
                memcpy(stage, &stage[words * 4UL], rem);
                used = rem;
            } else {
                used = 0UL;
            }
        }

        if (stream_done != 0) {
            break;
        }
        /* Buffer full with payload still flowing - normal case, go around
           and DMA the chunk. Nothing else to do here. */
    }

    if (decoded != hdr->raw_bytes) {
        return ZYNQ_DEVCFG_ERR_STREAM; /* end marker hit too early */
    }
    if (pay != pay_end) {
        return ZYNQ_DEVCFG_ERR_STREAM; /* trailing bytes past the end marker */
    }
    if ((~crc) != hdr->crc32) {
        return ZYNQ_DEVCFG_ERR_CRC;
    }
    return ZYNQ_DEVCFG_OK;
}

void zynq_devcfg_init(void) {
    uint32_t ctrl;

    /* UG585 ch.2 step 2: enable the PS->PL level shifters before driving
       anything towards the PL. SLCR writes need the unlock/lock pair. */
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->LVL_SHFTR_EN = LVL_SHFTR_PS_TO_PL;
    SLCR->LOCK = SLCR_LOCK_KEY;

    /* The DevC APB registers reject writes until the unlock word has been
       written once (UG585 devcfg.UNLOCK, "First APB access data"). */
    DEVCFG->UNLOCK = DEVCFG_UNLOCK_DATA;

    /* Step 2: clear every pending DevC interrupt. */
    DEVCFG->INT_STS = 0xFFFFFFFFUL;

    /* Step 1: enable the PCAP bridge and select PCAP (not ICAP) as the
       configuration path. Step 7b: non-secure bitstream, so keep the full
       PCAP rate (QUARTER rate is only for the AES path).
       PCFG_POR_CNT_4K is the "short PL power-on-reset" optimisation (UG585
       ch.6 "PL Initialization Time Optimization"); leave it at 0 so the PL
       runs its full 64k POR timer. CTRL[14:13] are "reserved, always write
       with 1", hence read-modify-write instead of a plain store. */
    ctrl = DEVCFG->CTRL;
    ctrl |= DEVCFG_CTRL_PCAP_MODE_Msk | DEVCFG_CTRL_PCAP_PR_Msk;
    ctrl &= ~(DEVCFG_CTRL_PCAP_RATE_EN_Msk | DEVCFG_CTRL_PCFG_POR_CNT_4K_Msk);
    DEVCFG->CTRL = ctrl;
}

zynq_devcfg_status_t zynq_devcfg_load(const void* bitstream, uint32_t bytes) {
    uintptr_t addr = (uintptr_t)bitstream;
    const uint8_t* img = (const uint8_t*)bitstream;
    uint32_t magic;
    zynq_devcfg_status_t r;

    if ((addr & 0x3FU) != 0U) {
        return ZYNQ_DEVCFG_ERR_SRC_ALIGN;
    }
    if ((bytes == 0U) || ((bytes & 0x3U) != 0U)) {
        return ZYNQ_DEVCFG_ERR_LENGTH;
    }

    /* The first word picks the path: the packed blob starts with the "ZBSP"
       magic, which a raw bitgen image can never start with. */
    memcpy(&magic, img, sizeof(magic));
    if (magic == DEVCFG_PACKED_MAGIC) {
        devcfg_packed_hdr_t hdr;

        r = zynq_devcfg_packed_hdr(img, bytes, &hdr);
        if (r != ZYNQ_DEVCFG_OK) {
            return r;
        }
        r = zynq_devcfg_begin();
        if (r != ZYNQ_DEVCFG_OK) {
            return r;
        }
        r = zynq_devcfg_load_packed(img, &hdr);
    } else {
        r = zynq_devcfg_begin();
        if (r != ZYNQ_DEVCFG_OK) {
            return r;
        }
        r = zynq_devcfg_load_direct(img, bytes);
    }
    if (r != ZYNQ_DEVCFG_OK) {
        return r;
    }
    return zynq_devcfg_finish();
}

/* Linker-script symbols: the build appends the packed image (SDK_BITSTREAM
   -> tools/rle_bitstream.py -> .bitstream section, see
   scripts/cmake/function_gcc_arm_bitstream.cmake) to the flash payload.
   An empty range means this build carries no image at all. */
extern const uint8_t __bitstream_start__[];
extern const uint8_t __bitstream_end__[];

volatile zynq_devcfg_status_t g_pl_status = ZYNQ_DEVCFG_NO_IMAGE;

/*
 * Boot-time PL bring-up, on the _start path before main(). The application
 * therefore starts with the PL either configured or with a recorded reason
 * why not; no application code has to remember to program it. Deliberately
 * independent of the GIC and the timer tick (the DMA wait is a bounded
 * cycle poll, see DEVCFG_DMA_POLL_LIMIT), so it can run this early.
 */
void zynq_devcfg_boot(void) {
    uint32_t bytes = (uint32_t)(__bitstream_end__ - __bitstream_start__);

    g_pl_status = ZYNQ_DEVCFG_NO_IMAGE;
    zynq_devcfg_init();
    if (bytes != 0U) {
        g_pl_status = zynq_devcfg_load(__bitstream_start__, bytes);
    }
}
