/**
 * @file board.c
 * @brief Board bring-up: PL configuration and PL fabric clocks.
 */

#include "board.h"

#include "board_fpga.h"
#include "board_fpga_bitstream.h"
#include "zynq_clock.h"

/* Outcome of the last PL programming; query via board_pl_status(). */
static enum devcfg_status pl_status = DEVCFG_NO_IMAGE;

/**
 * @brief  Query the outcome of the PL programming done by board_init().
 * @return DEVCFG_OK, DEVCFG_NO_IMAGE (no bitstream linked), else the
 *         first load error.
 */
enum devcfg_status board_pl_status(void) {
    return pl_status;
}

/**
 * @brief  Configure the PL, start its clock, release its resets.
 *
 * On a failed or missing image nothing is started: the reset lines stay
 * asserted so a half-configured design never runs.
 */
void board_init(void) {
    /* fpga_img[] is generated from bitstream.bin; zero length = --stub. */
    if (FPGA_IMG_LEN != 0u) {
        pl_status = fpga_load(fpga_img, FPGA_IMG_LEN);
    }

    if (pl_status == DEVCFG_OK) {
        /* FCLK0 = IO PLL / 10 = 100 MHz; clock before reset release. */
        fclk_config(FCLK0, FCLK_SRC_IO, 10U, 1U, true);

        /* FPGA_RST_CTRL resets to 0xF; the FSBL clears it after DONE. */
        fpga_reset_release_all();
    }
}
