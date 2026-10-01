#ifndef BOARD_FPGA_H
#define BOARD_FPGA_H

/*
 * Board FPGA layer: everything the PS does to the PL.
 *
 *   configuration   DevC/PCAP bridge, UG585 ch.6   (fpga_load)
 *   control plane   resets, level shifters, status  (UG585 ch.2, ch.6)
 *
 * The image lives in board_fpga_bitstream.c - generated data, not code;
 * a bitstream swap touches only that pair. Two image forms, picked by
 * the first word:
 *
 *   packed  "ZBSP" magic + header + RLE payload (convert_bitstream.py),
 *           decoded through a staging buffer, one DMA per chunk
 *   direct  raw PCAP-form bitstream, one DMA command (FSBL style)
 *
 * Trap: FPGA_RST_CTRL resets to 0x01F33F0F - all four reset lines
 * asserted. Until fpga_reset_release_all() clears them the PL *looks*
 * configured (DONE asserts) while AXI accesses hang the bus.
 *
 * Bring-up order:
 *
 *   fpga_config_clear();              -- optional, wipes the old design
 *   st = fpga_load(img, len);         -- PS->PL shifters, PCAP, DONE
 *   clock_fclk_config(FCLK0, ...);    -- fabric clock first ...
 *   fpga_reset_release_all();         -- ... then release the logic
 *
 * Failing calls return 0 (ok) / -1 (bad line); every SLCR touch is a
 * full unlock/write/lock sequence. PL clocks are in zynq_clock.h
 * (clock_fclk_*()).
 */

#include <stdbool.h>
#include <stdint.h>

#include "zynq_soc_ip.h"

/* Outcome of the PL configuration, reported by board_pl_status(). */
enum pl_status {
    PL_OK = 0,
    PL_ERR_SRC_ALIGN,    /* image base is not 64-byte aligned      */
    PL_ERR_LENGTH,       /* byte count is not a multiple of 4      */
    PL_ERR_NO_SYNC,      /* no PCAP sync word: wrong byte order?   */
    PL_ERR_HDR,          /* packed header magic/lengths invalid    */
    PL_ERR_STREAM,       /* RLE payload malformed/length mismatch  */
    PL_ERR_CRC,          /* decompressed image failed CRC-32       */
    PL_ERR_NO_POWER,     /* MCTRL[PCFG_POR_B] = 0: PL not powered  */
    PL_ERR_NOT_RESET,    /* PCFG_INIT did not clear after PROG_B   */
    PL_ERR_NOT_READY,    /* PCFG_INIT never asserted: PL not ready */
    PL_ERR_CMD_Q_FULL,   /* DMA command queue stayed full          */
    PL_ERR_DMA,          /* DMA command / AXI error / FIFO overflow*/
    PL_ERR_DMA_TIMEOUT,  /* DMA_DONE never arrived                 */
    PL_ERR_NO_DONE,      /* DMA done, but PL DONE never asserted   */
    PL_NO_IMAGE,         /* boot(): this build embeds no bitstream */
};

/* Number of PS->PL reset lines (FCLKRESETN[3:0]). */
#define FPGA_RESET_LINE_COUNT (4U)

/* slcr.LVL_SHFTR_EN values, UG585 ch.2 power-up sequence. */
#define FPGA_LVL_OFF      (0x0U) /* PL powered down: all shifters off  */
#define FPGA_LVL_PS_TO_PL (0xAU) /* before programming: PS->PL only    */
#define FPGA_LVL_ALL      (0xFU) /* after DONE: both directions        */

/* ------------------------------------------------------------------ */
/* PL reset lines (slcr.FPGA_RST_CTRL)                                 */
/* ------------------------------------------------------------------ */

/* Assert a reset line (drive FCLKRESETn low). Returns 0, or -1 when line
   is not one of the four lines. */
static inline int fpga_reset_assert(uint32_t line) {
    if (line >= FPGA_RESET_LINE_COUNT) {
        return -1;
    }
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->FPGA_RST_CTRL |= (1UL << line);
    SLCR->LOCK = SLCR_LOCK_KEY;
    return 0;
}

/* Release a reset line (drive FCLKRESETn high). Returns 0, or -1 when line
   is not one of the four lines. */
static inline int fpga_reset_release(uint32_t line) {
    if (line >= FPGA_RESET_LINE_COUNT) {
        return -1;
    }
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->FPGA_RST_CTRL &= ~(1UL << line);
    SLCR->LOCK = SLCR_LOCK_KEY;
    return 0;
}

/* Is that reset line asserted? Non-zero when asserted, -1 when line is not
   one of the four lines. */
static inline int fpga_reset_is_asserted(uint32_t line) {
    if (line >= FPGA_RESET_LINE_COUNT) {
        return -1;
    }
    return (int)((SLCR->FPGA_RST_CTRL >> line) & 1UL);
}

/* State of all four lines as a bit mask: bit n set = reset line n asserted. */
static inline uint32_t fpga_reset_state(void) {
    return SLCR->FPGA_RST_CTRL & 0xFUL;
}

/* Write the four lines at once from a mask: bit n set = assert reset line n. */
static inline void fpga_reset_set(uint32_t mask) {
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->FPGA_RST_CTRL = mask & 0xFUL;
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/* Assert all four lines: PL logic held in reset, pads unchanged. */
static inline void fpga_reset_assert_all(void) {
    fpga_reset_set(0xFUL);
}

/* Release all four lines: the fabric starts running. Fabric clock must
   already run; no PS->PL AXI transaction may be in flight (ch.6). */
static inline void fpga_reset_release_all(void) {
    fpga_reset_set(0x0UL);
}

/* Assert / hold / release one line: the ch.6 software reset. NOP-loop
   pulse, works before the tick exists; for a longer pulse use
   fpga_reset_assert() + tick_mdelay() + fpga_reset_release(). */
int fpga_reset_pulse(uint32_t line);

/* ------------------------------------------------------------------ */
/* PS-PL level shifters (slcr.LVL_SHFTR_EN, UG585 ch.2)               */
/* ------------------------------------------------------------------ */

/* Set the shifter enables directly: FPGA_LVL_OFF / _PS_TO_PL / _ALL. */
static inline void fpga_lvl_set(uint32_t value) {
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->LVL_SHFTR_EN = value;
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/* Current shifter enable value. */
static inline uint32_t fpga_lvl_get(void) {
    return SLCR->LVL_SHFTR_EN;
}

/* All shifters off (ch.2 shutdown step 1; also the powered-down state). */
static inline void fpga_lvl_off(void) {
    fpga_lvl_set(FPGA_LVL_OFF);
}

/* PS->PL direction only (ch.2 step 2): the state while programming. */
static inline void fpga_lvl_ps_to_pl(void) {
    fpga_lvl_set(FPGA_LVL_PS_TO_PL);
}

/* Both directions (ch.2 step 5, after DONE): PL->PS signals are blocked
   until this is set. */
static inline void fpga_lvl_all(void) {
    fpga_lvl_set(FPGA_LVL_ALL);
}

/* ------------------------------------------------------------------ */
/* PL status (devcfg)                                                 */
/* ------------------------------------------------------------------ */

/* Is the PL powered? MCTRL[PCFG_POR_B] = 0: nothing else works. */
static inline bool fpga_is_powered(void) {
    return ((DEVCFG->MCTRL & DEVCFG_MCTRL_PCFG_POR_B_Msk) != 0U);
}

/* Ready for configuration data? STATUS[PCFG_INIT]: 0 while housecleaning
   / PROG_B held, 1 once the config memory is clear. */
static inline bool fpga_is_ready(void) {
    return ((DEVCFG->STATUS & DEVCFG_STATUS_PCFG_INIT_Msk) != 0U);
}

/* Configuration succeeded (INT_STS[PCFG_DONE], write-1-to-clear). */
static inline bool fpga_is_done(void) {
    return ((DEVCFG->INT_STS & DEVCFG_IXR_PCFG_DONE_Msk) != 0U);
}

/* ------------------------------------------------------------------ */
/* Configuration                                                      */
/* ------------------------------------------------------------------ */

/*
 * Erase the current design: pulse PCFG_PROG_B, wait for PCFG_INIT
 * 1 -> 0 -> 1. fpga_load() does this itself; call directly to wipe the
 * PL without loading. No AXI transaction may be in flight.
 */
enum pl_status fpga_config_clear(void);

/*
 * Configure the PL from an image: bridge up, logic held in reset, push,
 * wait for DONE, open the level shifters. 'img' is a ZBSP packed blob
 * (no alignment) or a raw bitstream (64-byte aligned, len % 4 == 0,
 * PCAP byte order: sync word 0xAA995566 appears as 66 55 99 AA).
 *
 * On failure the reset lines stay asserted - a half-configured design
 * never runs. The fabric still needs a clock and a reset release:
 *
 *   if (fpga_load(img, len) == PL_OK) {
 *       clock_fclk_config(FCLK0, FCLK_SRC_IO, 10, 1, true);
 *       fpga_reset_release_all();
 *   }
 */
enum pl_status fpga_load(const void* img, uint32_t len);

#endif /* BOARD_FPGA_H */
