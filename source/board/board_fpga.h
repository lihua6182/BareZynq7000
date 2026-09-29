#ifndef BOARD_FPGA_H
#define BOARD_FPGA_H

/*
 * Board FPGA layer: everything the PS does to the PL.
 *
 *   configuration   DevC/PCAP bridge, UG585 ch.6   (fpga_load)
 *   control plane   resets, level shifters, status  (UG585 ch.2, ch.6)
 *
 * The image itself lives one level down, in board_fpga_bitstream.c -
 * generated data, not code. Keeping the two apart means a bitstream
 * swap touches only the generated pair.
 *
 * Two image forms are accepted, picked by the first word:
 *
 *   packed  "ZBSP" magic + 16-byte header + SEGGER-style RLE payload
 *           (tools/convert_bitstream.py); decoded through a 4 KB
 *           staging buffer, one DMA command per chunk
 *   direct  raw PCAP-form bitstream, one DMA command, FSBL style
 *
 * A single full-image DMA command is legal despite the 4 KB rule: the
 * rule is enforced by 64-byte alignment (64-byte bursts never straddle
 * a page), so the image handed in must be 64-byte aligned. Completion
 * is polled on INT_STS - no GIC or timer tick dependency.
 *
 * Four PS->PL reset lines, FCLKRESETN[3:0], are driven by
 * slcr.FPGA_RST_CTRL bits 0-3; every other bit of that register is
 * reserved. The bit is active high in the register - 1 asserts the reset
 * and drives the pin low. The register resets to 0x01F33F0F, so all four
 * lines stay asserted after boot: PL logic never leaves reset until
 * fpga_reset_release() / fpga_reset_release_all() clears them. The PL
 * then looks configured (DONE asserts) while AXI accesses to it hang the
 * bus. The FSBL clears the whole register after programming the PL; the
 * same order is required here.
 *
 * Bring-up or reconfiguration order:
 *
 *   fpga_config_clear();              -- optional, wipes the old design
 *   st = fpga_load(img, len);         -- PS->PL shifters, PCAP, DONE
 *   fclk_config(FCLK0, ...);          -- fabric clock first ...
 *   fpga_reset_release_all();         -- ... then release the logic
 *
 * Register-level primitives are static inline and take a reset line
 * index; the ones that can fail return 0 (ok) / -1 (bad index). The SLCR
 * accepts writes only between its unlock and lock keys, so every SLCR
 * touch is a full unlock/write/lock sequence. PL clocks are not here -
 * they belong to the clock tree, see zynq_clock.h fclk_*().
 */

#include <stdbool.h>
#include <stdint.h>

#include "zynq_soc_ip.h"

enum devcfg_status {
    DEVCFG_OK = 0,
    DEVCFG_ERR_SRC_ALIGN,    /* bitstream base is not 64-byte aligned  */
    DEVCFG_ERR_LENGTH,       /* byte count is not a multiple of 4      */
    DEVCFG_ERR_NO_SYNC,      /* no PCAP sync word: wrong byte order?   */
    DEVCFG_ERR_HDR,          /* packed header magic/lengths invalid    */
    DEVCFG_ERR_STREAM,       /* RLE payload malformed/length mismatch  */
    DEVCFG_ERR_CRC,          /* decompressed image failed CRC-32       */
    DEVCFG_ERR_PL_NO_POWER,  /* MCTRL[PCFG_POR_B] = 0: PL not powered  */
    DEVCFG_ERR_PL_NOT_RESET, /* PCFG_INIT did not clear after PROG_B   */
    DEVCFG_ERR_PL_NOT_READY, /* PCFG_INIT never asserted: PL not ready */
    DEVCFG_ERR_CMD_Q_FULL,   /* DMA command queue stayed full          */
    DEVCFG_ERR_DMA,          /* DMA command / AXI error / FIFO overflow*/
    DEVCFG_ERR_DMA_TIMEOUT,  /* DMA_DONE never arrived                 */
    DEVCFG_ERR_NO_DONE,      /* DMA done, but PL DONE never asserted   */
    DEVCFG_NO_IMAGE,         /* boot(): this build embeds no bitstream */
};

/* Number of PS->PL reset lines (FCLKRESETN[3:0]). */
#define FPGA_RESET_NR (4U)

/* slcr.LVL_SHFTR_EN values, UG585 ch.2 power-up sequence. */
#define FPGA_LVL_OFF      (0x0U) /* PL powered down: all shifters off  */
#define FPGA_LVL_PS_TO_PL (0xAU) /* before programming: PS->PL only    */
#define FPGA_LVL_ALL      (0xFU) /* after DONE: both directions        */

/* ------------------------------------------------------------------ */
/* PL reset lines (slcr.FPGA_RST_CTRL)                                 */
/* ------------------------------------------------------------------ */

/* Assert reset line n (drive FCLKRESETn low). Returns 0, or -1 when n
   is not one of the four lines. */
static inline int fpga_reset_assert(uint32_t n) {
    if (n >= FPGA_RESET_NR) {
        return -1;
    }
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->FPGA_RST_CTRL |= (1UL << n);
    SLCR->LOCK = SLCR_LOCK_KEY;
    return 0;
}

/* Release reset line n (drive FCLKRESETn high). Returns 0, or -1 when n
   is not one of the four lines. */
static inline int fpga_reset_release(uint32_t n) {
    if (n >= FPGA_RESET_NR) {
        return -1;
    }
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->FPGA_RST_CTRL &= ~(1UL << n);
    SLCR->LOCK = SLCR_LOCK_KEY;
    return 0;
}

/* Is reset line n asserted? Non-zero when asserted, -1 when n is not one
   of the four lines. */
static inline int fpga_reset_is_asserted(uint32_t n) {
    if (n >= FPGA_RESET_NR) {
        return -1;
    }
    return (int)((SLCR->FPGA_RST_CTRL >> n) & 1UL);
}

/* State of all four lines as a bit mask: bit n set = reset asserted. */
static inline uint32_t fpga_reset_state(void) {
    return SLCR->FPGA_RST_CTRL & 0xFUL;
}

/* Write the four lines at once from a mask: bit n set = assert. */
static inline void fpga_reset_set(uint32_t mask) {
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->FPGA_RST_CTRL = mask & 0xFUL;
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/* Assert all four lines: PL logic held in reset, pads unchanged. */
static inline void fpga_reset_assert_all(void) {
    fpga_reset_set(0xFUL);
}

/* Release all four lines: the whole fabric starts running. The fabric
   clock must already be running (fclk_config), and no PS->PL AXI
   transaction may be in flight - UG585 ch.6 warns that resetting PL
   logic with pending transactions leaves them uncompleted in the PS. */
static inline void fpga_reset_release_all(void) {
    fpga_reset_set(0x0UL);
}

/* Assert line n, wait a few hundred CPU cycles, release it: the software
   reset of UG585 (write 0 -> write 1 -> write 0 on FPGA_RST_CTRL). The
   pulse is generated with a NOP loop, so it works before the tick
   exists; for a longer pulse call fpga_reset_assert(), tick_mdelay() and
   fpga_reset_release() yourself. Returns 0, or -1 on a bad index. */
int fpga_reset_pulse(uint32_t n);

/* ------------------------------------------------------------------ */
/* PS-PL level shifters (slcr.LVL_SHFTR_EN, UG585 ch.2)               */
/* ------------------------------------------------------------------ */

/* Set the shifter enables directly: FPGA_LVL_OFF / _PS_TO_PL / _ALL. */
static inline void fpga_lvl_set(uint32_t v) {
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->LVL_SHFTR_EN = v;
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/* Current shifter enable value. */
static inline uint32_t fpga_lvl_get(void) {
    return SLCR->LVL_SHFTR_EN;
}

/* All shifters off: no PS-PL signal crosses. UG585 ch.2 step 1 of the
   shutdown sequence; also the state to leave the register in while the
   PL is powered down. */
static inline void fpga_lvl_off(void) {
    fpga_lvl_set(FPGA_LVL_OFF);
}

/* PS->PL direction only: UG585 ch.2 step 2, the state the PL must be in
   while it is being programmed. */
static inline void fpga_lvl_ps_to_pl(void) {
    fpga_lvl_set(FPGA_LVL_PS_TO_PL);
}

/* Both directions: UG585 ch.2 step 5, after the PL asserts DONE. PL->PS
   signals (interrupts, AXI masters) are blocked until this is set. */
static inline void fpga_lvl_all(void) {
    fpga_lvl_set(FPGA_LVL_ALL);
}

/* ------------------------------------------------------------------ */
/* PL status (devcfg)                                                 */
/* ------------------------------------------------------------------ */

/* Is the PL powered? devcfg.MCTRL[PCFG_POR_B], UG585 ch.6: every other
   devcfg operation is meaningless while this reads 0. */
static inline bool fpga_is_powered(void) {
    return ((DEVCFG->MCTRL & DEVCFG_MCTRL_PCFG_POR_B_Msk) != 0U);
}

/* Is the PL ready for configuration data? devcfg.STATUS[PCFG_INIT]:
   0 during housecleaning / while PROG_B is held, 1 once the PL has
   cleared its configuration memory and locked its clocks. */
static inline bool fpga_is_ready(void) {
    return ((DEVCFG->STATUS & DEVCFG_STATUS_PCFG_INIT_Msk) != 0U);
}

/* Has the PL asserted DONE? devcfg.INT_STS[PCFG_DONE]: set once the
   configuration succeeded. Reading INT_STS does not clear it - the bit
   is write-1-to-clear. */
static inline bool fpga_is_done(void) {
    return ((DEVCFG->INT_STS & DEVCFG_IXR_PCFG_DONE_Msk) != 0U);
}

/* ------------------------------------------------------------------ */
/* Configuration                                                      */
/* ------------------------------------------------------------------ */

/*
 * Erase the current design: pulse PCFG_PROG_B to run PL housecleaning
 * and wait for the PL to come back ready (devcfg.CTRL[PCFG_PROG_B] ->
 * STATUS[PCFG_INIT] 1 -> 0 -> 1). Returns DEVCFG_OK once the PL is empty
 * and ready, DEVCFG_ERR_PL_NOT_RESET when it never left reset, or
 * DEVCFG_ERR_PL_NOT_READY when it never came back.
 *
 * fpga_load() does this itself; call it directly to wipe the PL without
 * loading a new image (before shutting the level shifters off, for
 * instance). No AXI transaction may be in flight.
 */
enum devcfg_status fpga_config_clear(void);

/*
 * Configure the PL from an image: prepare the DevC/PCAP bridge, hold the
 * PL logic in reset, push the image, wait for DONE and open the level
 * shifters in both directions.
 *
 * 'img' is a ZBSP packed blob or a raw bitstream - the first word picks
 * the path, only the packed blob starts with the "ZBSP" magic. The raw
 * form must be 64-byte aligned and 'len' a multiple of 4. PCAP consumes
 * little-endian words: a raw image must be the "byte swapped .bin" form
 * (kernel zynq-fpga.c) with the 0xAA995566 sync word appearing as
 * 66 55 99 AA, else the load stops at DEVCFG_ERR_NO_SYNC.
 *
 * Returns DEVCFG_OK, or the first error - see enum devcfg_status. On
 * failure the reset lines stay asserted, so a half-configured design
 * never runs. The fabric still needs a clock and a reset release before
 * its logic does anything:
 *
 *   if (fpga_load(img, len) == DEVCFG_OK) {
 *       fclk_config(FCLK0, FCLK_SRC_IO, 10, 1, true);
 *       fpga_reset_release_all();
 *   }
 */
enum devcfg_status fpga_load(const void* img, uint32_t len);

#endif /* BOARD_FPGA_H */
