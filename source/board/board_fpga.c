/**
 * @file board_fpga.c
 * @brief PL configuration over the DevC/PCAP bridge and the PL control
 *        plane (UG585 ch.6, ch.2).
 *
 * Two halves, both PL-only:
 *
 *   control plane  reset lines, level shifters, status - the primitives
 *                  are static inline in board_fpga.h, what is left here
 *                  needs a loop or a sequence (reset pulse, PROG_B
 *                  handshake)
 *   configuration  the DevC PCAP transfer: unpack a ZBSP image or push a
 *                  raw bitstream, then wait for DONE
 *
 * The image is not here: board_fpga_bitstream.c holds the generated
 * array, board.c decides when to load it.
 */

#include "board_fpga.h"

#include <string.h>

#include "zynq_common.h"
#include "zynq_soc_ip.h"

/* slcr.PCAP_CLK_CTRL bit 0 (CLKACT); the reset value already sets it. */
#define PCAP_CLK_CTRL_CLKACT (0x1UL)

/* Busy-poll limits (one devcfg read ~100 ns): ~1 s for the PL power-up
   / PROG_B handshake, ~10 s safety net per DMA command. A fixed number
   of strongly-ordered bus cycles instead of wall time, so this works
   before the tick and the GIC exist. */
#define INIT_POLL_LIMIT (8000000UL)
#define DMA_POLL_LIMIT  (60000000UL)

/* Reset pulse width in loop iterations: enough to hold FCLKRESETn low
   for far longer than one fabric clock period at any PLL setting. */
#define RESET_PULSE_CYCLES (1024UL)

/* Transport-level errors that abort a load (UG585 step 10). */
#define XFER_ERR_MASK (DEVCFG_IXR_DMA_CMD_ERR_Msk |   \
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
 * Packed image format (tools/convert_bitstream.py): 16-byte LE header
 * plus an RLE payload in the SEGGER BSP_FPGA.c encoding, holding the
 * PCAP-form image so the decoder streams straight into the DevC DMA.
 * The "ZBSP" magic doubles as the load-path selector: a bitgen image
 * starts with 0xFFFFFFFF padding and can never match it.
 *
 *   0x00 magic      "ZBSP"
 *   0x04 raw_len    uncompressed image length, % 4 == 0
 *   0x08 packed_len RLE payload length
 *   0x0C crc32      CRC-32 (EDB88320) over the raw image
 */
#define ZBSP_MAGIC   (0x5A425350UL)
#define HDR_BYTES    (16UL)
#define CRC_POLY     (0xEDB88320UL)

/* The sync word sits within the first 128 bytes of every known
   7-series layout, so a fixed scan window is enough. */
#define SYNC_SCAN_BYTES (128UL)

/* Staging buffer between the RLE decoder and the DevC DMA: 64-byte
   aligned so DMA bursts never straddle a 4 KB page, cacheable OCM so
   each decoded chunk must be cleaned to RAM before the DMA reads it. */
#define STAGE_BYTES (4096UL)
#define L1_CACHE_BYTES (32UL) /* Cortex-A9 L1 D-cache line */
static uint32_t stage_buf[STAGE_BYTES / 4UL] __attribute__((aligned(64)));

struct packed_hdr {
    uint32_t magic;
    uint32_t raw_len;
    uint32_t packed_len;
    uint32_t crc32;
};

/**
 * @brief  Burn CPU cycles.
 * @param  n Number of iterations.
 *
 * The counter is volatile so the loop survives -Og; it is a busy wait
 * by design, used only for the few hundred cycles a reset pulse needs.
 */
static void delay_cycles(uint32_t n) {
    volatile uint32_t i;

    for (i = 0U; i < n; i++) {
        /* spin */
    }
}

/**
 * @brief  Wait for the PL to reach a given PCFG_INIT state.
 * @param  level State to wait for: 0 (in reset) or 1 (ready).
 * @return DEVCFG_OK, or DEVCFG_ERR_PL_NOT_RESET / _NOT_READY on timeout.
 */
static enum devcfg_status wait_init(uint32_t level) {
    uint32_t i;

    for (i = 0U; i < INIT_POLL_LIMIT; i++) {
        if (((DEVCFG->STATUS & DEVCFG_STATUS_PCFG_INIT_Msk) != 0U) ==
            (level != 0U)) {
            return DEVCFG_OK;
        }
    }
    return (level == 0U) ? DEVCFG_ERR_PL_NOT_RESET : DEVCFG_ERR_PL_NOT_READY;
}

/**
 * @brief  Pulse one PS->PL reset line.
 * @param  n Reset line index, 0..3 (FCLKRESETN[n]).
 * @return 0 on success, -1 when n is not one of the four lines.
 *
 * Assert, hold, release: the software reset of UG585 (write 0, write 1,
 * write 0 to slcr.FPGA_RST_CTRL), done on a single line so the other
 * three keep running. PL logic reset this way must not have pending AXI
 * transactions.
 */
int fpga_reset_pulse(uint32_t n) {
    if (n >= FPGA_RESET_NR) {
        return -1;
    }
    if (fpga_reset_assert(n) != 0) {
        return -1;
    }
    delay_cycles(RESET_PULSE_CYCLES);
    return fpga_reset_release(n);
}

/**
 * @brief  Erase the current PL design.
 * @return DEVCFG_OK, or the reason the PL never came back.
 *
 * Pulses devcfg.CTRL[PCFG_PROG_B] (1 -> 0 -> 1) to run PL
 * housecleaning and waits for STATUS[PCFG_INIT] to fall and rise again -
 * INIT = 1 is the PL saying its configuration memory is clear and its
 * clocks are locked. Drops the stale DONE flag on the way, so a later
 * load is not fooled by the previous design's completion.
 */
enum devcfg_status fpga_config_clear(void) {
    uint32_t ctrl;
    enum devcfg_status st;

    if (fpga_is_powered() == false) {
        return DEVCFG_ERR_PL_NO_POWER;
    }

    /* Read-modify-write: CTRL holds PCAP mode, PR and the POR count
       alongside PROG_B. The devcfg region is strongly ordered, so the
       writes reach the bridge in program order without barriers. */
    ctrl = DEVCFG->CTRL;
    DEVCFG->CTRL = ctrl | DEVCFG_CTRL_PCFG_PROG_B_Msk;
    DEVCFG->CTRL = ctrl & ~DEVCFG_CTRL_PCFG_PROG_B_Msk;

    st = wait_init(0U);
    if (st != DEVCFG_OK) {
        return st;
    }

    DEVCFG->CTRL = ctrl | DEVCFG_CTRL_PCFG_PROG_B_Msk;
    DEVCFG->INT_STS = DEVCFG_IXR_PCFG_DONE_Msk; /* W1C: stale DONE */
    return wait_init(1U);
}

/**
 * @brief  Acknowledge the last DMA completion.
 *
 * Clears the DMA_DONE flag and the DMA_DONE_CNT field (both cleared by
 * writing 1, UG585), so a stale completion cannot be mistaken for the
 * one being waited on.
 */
static void dma_ack(void) {
    DEVCFG->INT_STS = DEVCFG_IXR_DMA_DONE_Msk;
    DEVCFG->STATUS |= DEVCFG_STATUS_DMA_DONE_CNT_Msk;
}

/**
 * @brief  Check an image for the bitstream sync word.
 * @param  img Start of the image.
 * @param  len Length of the image in bytes.
 * @return Non-zero when the sync word is present, 0 otherwise.
 *
 * Looks for 0xAA995566 in little-endian byte order (66 55 99 AA) - the
 * PCAP-consumable "byte swapped .bin" the kernel's zynq-fpga driver
 * requires. bitgen's native big-endian .bin is the classic mistake: the
 * DMA pushes all 2 MB, the PL ignores it, and the only symptom is a
 * generic NO_DONE seconds later.
 */
static int has_sync(const uint8_t* img, uint32_t len) {
    uint32_t limit;
    uint32_t i;

    limit = (len < SYNC_SCAN_BYTES) ? len : SYNC_SCAN_BYTES;
    for (i = 0U; (i + 4UL) <= limit; i++) {
        if ((img[i] == 0x66U) && (img[i + 1U] == 0x55U) &&
            (img[i + 2U] == 0x99U) && (img[i + 3U] == 0xAAU)) {
            return 1;
        }
    }
    return 0;
}

/* Table-driven CRC-32 (reflected, poly 0xEDB88320 - the zlib polynomial
   SEGGER_CRC_Calc_EDB88320 also uses): ~5x faster than bitwise over a
   ~2 MB image. crc runs without the final XOR so it carries across
   staging-buffer chunks; finish with ~crc. */
static uint32_t crc_table[256];

/**
 * @brief  Build the reflected CRC-32 table.
 *
 * Called once per load, before the first crc_update().
 */
static void crc_init(void) {
    uint32_t i;
    uint32_t v;
    uint32_t b;

    for (i = 0U; i < 256U; i++) {
        v = i;
        for (b = 0U; b < 8U; b++) {
            v = ((v & 1UL) != 0UL) ? (CRC_POLY ^ (v >> 1U)) : (v >> 1U);
        }
        crc_table[i] = v;
    }
}

/**
 * @brief  Fold bytes into a running CRC-32.
 * @param  crc Running CRC, without the final inversion.
 * @param  p   Bytes to fold in.
 * @param  len Number of bytes.
 * @return The updated CRC; invert it (~crc) to get the final value.
 */
static uint32_t crc_update(uint32_t crc, const uint8_t* p, uint32_t len) {
    uint32_t i;

    for (i = 0U; i < len; i++) {
        crc = (crc >> 8U) ^ crc_table[(crc ^ p[i]) & 0xFFU];
    }
    return crc;
}

/**
 * @brief  Clean the decoded prefix of the staging buffer to RAM.
 * @param  len Number of valid bytes in the buffer.
 *
 * The DMA reads via the ACP, coherent with L2 but not the Cortex-A9 L1.
 * No invalidate needed - the CPU never reads the buffer back.
 */
static void stage_clean(uint32_t len) {
    uint32_t addr = (uint32_t)(uintptr_t)stage_buf;
    uint32_t end = addr + ((len + (L1_CACHE_BYTES - 1UL)) & ~(L1_CACHE_BYTES - 1UL));

    while (addr < end) {
        L1C_CleanDCacheMVA((void*)(uintptr_t)addr);
        addr += L1_CACHE_BYTES;
    }
    __DSB();
}

/**
 * @brief  Prepare the DevC/PCAP bridge for a load.
 *
 * UG585 ch.2 step 2 (PS->PL level shifters on), the DevC unlock word,
 * clearing every pending interrupt, and PCAP mode with the full-length
 * POR. Called from fpga_load(); the PCAP clock is re-armed per load in
 * begin().
 */
static void devcfg_init(void) {
    uint32_t ctrl;

    /* UG585 ch.2 step 2: PS->PL level shifters on. */
    fpga_lvl_ps_to_pl();

    /* DevC APB registers reject writes until the unlock word is written
       once (UG585 devcfg.UNLOCK). */
    DEVCFG->UNLOCK = DEVCFG_UNLOCK_DATA;

    /* Clear every pending DevC interrupt. */
    DEVCFG->INT_STS = 0xFFFFFFFFUL;

    /* PCAP bridge on, PCAP (not ICAP) path, full rate (QUARTER is only
       for AES). CTRL[14:13] are "reserved, always write with 1", hence
       the read-modify-write. */
    ctrl = DEVCFG->CTRL;
    ctrl |= DEVCFG_CTRL_PCAP_MODE_Msk | DEVCFG_CTRL_PCAP_PR_Msk;
    ctrl &= ~(DEVCFG_CTRL_PCAP_RATE_EN_Msk | DEVCFG_CTRL_PCFG_POR_CNT_4K_Msk);
    DEVCFG->CTRL = ctrl;
}

/**
 * @brief  Bring the PL and the PCAP bridge into the "ready for data"
 *         state.
 * @return DEVCFG_OK, or the reason it failed.
 *
 * UG585 ch.6 steps 1-6 minus the DMA: check PL power, gate the PCAP
 * clock on, run the PROG_B housecleaning handshake (fpga_config_clear),
 * check the command queue and drop stale state. Shared by both load
 * paths.
 */
static enum devcfg_status begin(void) {
    enum devcfg_status st;

    /* The PL must be powered before the DevC/PCAP can be used at all:
       MCTRL[PCFG_POR_B] (UG585 ch.6). Reported here instead of
       surfacing as a PCFG_INIT timeout later. */
    if (fpga_is_powered() == false) {
        return DEVCFG_ERR_PL_NO_POWER;
    }

    /* Enable the PCAP clock here rather than in devcfg_init(), so a
       second load cannot start with the clock gated. */
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->PCAP_CLK_CTRL |= PCAP_CLK_CTRL_CLKACT;
    SLCR->LOCK = SLCR_LOCK_KEY;

    /* Erase the old design and wait for the PL to come back ready. */
    st = fpga_config_clear();
    if (st != DEVCFG_OK) {
        return st;
    }

    /* The DMA command queue must have room; PCAP loopback off. */
    if ((DEVCFG->STATUS & DEVCFG_STATUS_DMA_CMD_Q_F_Msk) != 0U) {
        return DEVCFG_ERR_CMD_Q_FULL;
    }
    DEVCFG->MCTRL &= ~DEVCFG_MCTRL_PCAP_LPBK_Msk;

    /* Drop any completion left over from an earlier transfer. */
    dma_ack();

    return DEVCFG_OK;
}

/**
 * @brief  Move one block of configuration data to the PCAP.
 * @param  src   Source address; must be 64-byte aligned and must not
 *               straddle a 4 KB boundary.
 * @param  words Length in 32-bit words.
 * @return DEVCFG_OK, or the transport error that aborted the transfer.
 *
 * UG585 ch.6 steps 8-10 for one DMA command: queue, wait, ack. The
 * register order matters - writing DMA_DEST_LEN last is what queues the
 * command and starts the transfer - and the magic destination address
 * routes the data to the PCAP. A transport error aborts early: a
 * bitstream that faults halfway leaves the PL unconfigured anyway.
 */
static enum devcfg_status dma_xfer(const void* src, uint32_t words) {
    uint32_t i;
    uint32_t sts;

    if ((DEVCFG->STATUS & DEVCFG_STATUS_DMA_CMD_Q_F_Msk) != 0U) {
        return DEVCFG_ERR_CMD_Q_FULL;
    }

    DEVCFG->DMA_SRC_ADDR = (uint32_t)(uintptr_t)src;
    DEVCFG->DMA_DEST_ADDR = DEVCFG_DMA_INVALID_ADDRESS; /* route to PCAP */
    DEVCFG->DMA_SRC_LEN = words;
    __DSB();
    DEVCFG->DMA_DEST_LEN = words; /* triggers the transfer */

    for (i = 0U; i < DMA_POLL_LIMIT; i++) {
        sts = DEVCFG->INT_STS;
        if ((sts & XFER_ERR_MASK) != 0U) {
            DEVCFG->INT_STS = sts; /* W1C: drop the latched error bits */
            return DEVCFG_ERR_DMA;
        }
        if ((sts & DEVCFG_IXR_DMA_DONE_Msk) != 0U) {
            break;
        }
    }
    if (i == DMA_POLL_LIMIT) {
        return DEVCFG_ERR_DMA_TIMEOUT;
    }

    dma_ack();
    return DEVCFG_OK;
}

/**
 * @brief  Close out a load: wait for PL DONE, then open the level
 *         shifters.
 * @return DEVCFG_OK, or DEVCFG_ERR_NO_DONE when the PL never asserted
 *         DONE.
 *
 * UG585 ch.6 step 11 + ch.2 step 5.
 */
static enum devcfg_status finish(void) {
    uint32_t i;

    for (i = 0U; i < DMA_POLL_LIMIT; i++) {
        if ((DEVCFG->INT_STS & DEVCFG_IXR_PCFG_DONE_Msk) != 0U) {
            break;
        }
    }
    DEVCFG->INT_STS = DEVCFG_IXR_PCFG_DONE_Msk;
    if (i == DMA_POLL_LIMIT) {
        return DEVCFG_ERR_NO_DONE;
    }

    /* UG585 ch.2 step 5: the PL drives PS inputs now. */
    fpga_lvl_all();

    return DEVCFG_OK;
}

/**
 * @brief  Load a raw PCAP-form bitstream in one DMA command.
 * @param  img Raw image; must be 64-byte aligned.
 * @param  len Length in bytes, a multiple of 4.
 * @return DEVCFG_OK, or the reason it failed.
 *
 * The FSBL-style direct path: fallback and debug escape hatch, since the
 * build normally packs the image.
 */
static enum devcfg_status load_direct(const uint8_t* img, uint32_t len) {
    if (has_sync(img, len) == 0) {
        return DEVCFG_ERR_NO_SYNC;
    }
    return dma_xfer(img, len / 4UL);
}

/**
 * @brief  Parse and validate a ZBSP packed header.
 * @param  img Start of the packed image.
 * @param  len Length of the whole image in bytes.
 * @param  hdr Out: the decoded header.
 * @return DEVCFG_OK, or DEVCFG_ERR_HDR when the header is malformed.
 *
 * Pure memory work, so a malformed blob is rejected before any PL
 * hardware is touched.
 */
static enum devcfg_status packed_hdr(const uint8_t* img, uint32_t len,
                                     struct packed_hdr* hdr) {
    struct packed_hdr h;

    if (len < HDR_BYTES) {
        return DEVCFG_ERR_HDR;
    }
    memcpy(&h, img, sizeof(h));
    if (h.magic != ZBSP_MAGIC) {
        return DEVCFG_ERR_HDR;
    }
    /* raw_len must be a non-empty word stream; packed_len must fit in
       the image (len is normally exactly 16 + packed_len; slack is
       tolerated for robustness). */
    if ((h.raw_len < 4UL) || ((h.raw_len & 3UL) != 0UL)) {
        return DEVCFG_ERR_HDR;
    }
    if ((h.packed_len == 0UL) ||
        (h.packed_len > (len - HDR_BYTES))) {
        return DEVCFG_ERR_HDR;
    }
    *hdr = h;
    return DEVCFG_OK;
}

/**
 * @brief  RLE-decode a packed image and stream it to the PCAP.
 * @param  img Start of the packed image (header included).
 * @param  hdr Header already validated by packed_hdr().
 * @return DEVCFG_OK, or the reason it failed.
 *
 * One DMA command per decoded chunk; SEGGER's BSP_FPGA.c is the
 * reference for this loop. Two DevC constraints drive the tail handling:
 * the DMA moves whole 32-bit words (a < 4 remainder carries to the next
 * round) and needs a 64-byte aligned source (stage_buf is, and never
 * moves). The decoder is paranoid on purpose: every marker read is
 * bounds-checked, the decoded total against raw_len, and the result
 * against the header CRC.
 */
static enum devcfg_status load_packed(const uint8_t* img,
                                      const struct packed_hdr* hdr) {
    const uint8_t* pay = img + HDR_BYTES;
    const uint8_t* pay_end = pay + hdr->packed_len;
    uint8_t* stage = (uint8_t*)stage_buf;
    uint32_t used = 0UL; /* bytes in the staging buffer (incl. <4 carry) */
    uint32_t decoded = 0UL;
    uint32_t crc = 0xFFFFFFFFUL;
    int synced = 0;
    int stream_done = 0;
    enum devcfg_status r;

    crc_init();
    for (;;) {
        /* Decode markers into the free tail of the buffer; every marker
           emits at most 128 bytes, so "128 free" always makes progress. */
        while ((stream_done == 0) && ((used + 128UL) <= STAGE_BYTES)) {
            uint32_t c0 = *pay++;
            if (c0 == 0UL) { /* end marker */
                stream_done = 1;
            } else if (c0 <= 0x3FUL) { /* literal run */
                if ((pay + c0) > pay_end) {
                    return DEVCFG_ERR_STREAM;
                }
                memcpy(&stage[used], pay, c0);
                pay += c0;
                used += c0;
                decoded += c0;
            } else if (c0 <= 0x7FUL) { /* byte repeat run */
                uint32_t n = c0 - 0x3FUL;
                if (pay >= pay_end) {
                    return DEVCFG_ERR_STREAM;
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
            if (decoded > hdr->raw_len) {
                return DEVCFG_ERR_STREAM; /* payload exceeds declared image */
            }
        }

        /* Check the byte order once, on the first decoded bytes. */
        if ((!synced) && (used != 0UL)) {
            uint32_t scan = (used < SYNC_SCAN_BYTES) ? used : SYNC_SCAN_BYTES;
            if (has_sync(stage, scan) == 0) {
                return DEVCFG_ERR_NO_SYNC;
            }
            synced = 1;
        }

        /* DMA the whole words out; carry the <4 remainder to the front
           for the next round. raw_len is word-aligned, so the carry is
           empty at a clean end of stream. */
        {
            uint32_t words = used / 4UL;
            uint32_t rem = used & 3UL;

            if (words != 0UL) {
                stage_clean(words * 4UL);
                crc = crc_update(crc, stage, words * 4UL);
                r = dma_xfer(stage_buf, words);
                if (r != DEVCFG_OK) {
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
        /* Buffer full with payload still flowing: go around and DMA. */
    }

    if (decoded != hdr->raw_len) {
        return DEVCFG_ERR_STREAM; /* end marker hit too early */
    }
    if (pay != pay_end) {
        return DEVCFG_ERR_STREAM; /* trailing bytes past the end marker */
    }
    if ((~crc) != hdr->crc32) {
        return DEVCFG_ERR_CRC;
    }
    return DEVCFG_OK;
}

/**
 * @brief  Configure the PL from an image.
 * @param  img Start of the image: a ZBSP packed blob or a raw bitstream.
 *             The first path needs no alignment, the second must be
 *             64-byte aligned.
 * @param  len Length in bytes, a multiple of 4.
 * @return DEVCFG_OK, or the reason it failed.
 *
 * Requires devcfg_init() to have run. The first word picks the path:
 * only the packed blob starts with the "ZBSP" magic.
 */
static enum devcfg_status devcfg_load(const void* img, uint32_t len) {
    uintptr_t addr = (uintptr_t)img;
    const uint8_t* p = (const uint8_t*)img;
    uint32_t magic;
    enum devcfg_status r;

    if ((addr & 0x3FU) != 0U) {
        return DEVCFG_ERR_SRC_ALIGN;
    }
    if ((len == 0U) || ((len & 0x3U) != 0U)) {
        return DEVCFG_ERR_LENGTH;
    }

    /* The first word picks the path: only the packed blob starts with
       the "ZBSP" magic. */
    memcpy(&magic, p, sizeof(magic));
    if (magic == ZBSP_MAGIC) {
        struct packed_hdr hdr;

        r = packed_hdr(p, len, &hdr);
        if (r != DEVCFG_OK) {
            return r;
        }
        r = begin();
        if (r != DEVCFG_OK) {
            return r;
        }
        r = load_packed(p, &hdr);
    } else {
        r = begin();
        if (r != DEVCFG_OK) {
            return r;
        }
        r = load_direct(p, len);
    }
    if (r != DEVCFG_OK) {
        return r;
    }
    return finish();
}

/**
 * @brief  Configure the PL from an image.
 * @param  img Start of the image: a ZBSP packed blob or a raw bitstream.
 * @param  len Length in bytes, a multiple of 4.
 * @return DEVCFG_OK, or the reason it failed.
 *
 * Prepares the DevC/PCAP bridge, asserts every reset line so a
 * half-configured design cannot run, pushes the image, waits for DONE
 * and opens the level shifters in both directions. The reset lines stay
 * asserted on return - give the fabric a clock and release them only
 * when the design is ready to run.
 */
enum devcfg_status fpga_load(const void* img, uint32_t len) {
    /* UG585 ch.2 step 2: PS->PL level shifters on, DevC unlocked, PCAP
       mode selected. Runs before the image is touched so a rejected
       image leaves the bridge in a usable state. */
    devcfg_init();

    /* Hold the PL logic: the bits that arrive before the last DMA
       command are a valid partial design, and would start running. */
    fpga_reset_assert_all();

    return devcfg_load(img, len);
}
