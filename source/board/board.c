/**
 * @file board.c
 * @brief Board bring-up: PL configuration and PL fabric clocks.
 */

#include "board.h"

#include "board_fpga.h"
#include "board_fpga_bitstream.h"
#include "zynq_clock.h"

/* Outcome of the PL programming; non-static so a debugger can read it
   directly. */
volatile enum devcfg_status pl_status = DEVCFG_NO_IMAGE;

/**
 * @brief  Configure the PL, give the fabric a clock, release its resets.
 *
 * Programs the FPGA before anything drives PS->PL signals, then starts
 * FCLK0 and releases the PL resets. pl_status holds the outcome;
 * DEVCFG_NO_IMAGE means no bitstream was linked in.
 */
void board_init(void) {
    /* Configure the PL (UG585 ch.6 PCAP flow) before anything drives
       PS->PL signals. fpga_img[] comes from bitstream.bin via the VSCode
       task (tools/convert_bitstream.py); a zero length means the --stub
       pair and carries no image. fpga_load() leaves the PL logic held in
       reset, so a half-configured design never runs. */
    if (FPGA_IMG_LEN != 0u) {
        pl_status = fpga_load(fpga_img, FPGA_IMG_LEN);
    }

    /* FCLK0 = IO PLL / (10 * 1) = 100 MHz for the PL fabric, gated on
       around the update (the source mux is not glitch-free). The clock
       has to run before the resets come off: released-reset logic with
       no clock is not clocked at all. */
    fclk_config(FCLK0, FCLK_SRC_IO, 10U, 1U, true);

    /* Let the fabric run. slcr.FPGA_RST_CTRL resets to 0xF: all four
       FCLKRESETN outputs stay asserted, so PL logic - here the
       SmartConnect and the AXI GPIO at 0x41200000 - never leaves reset.
       The fabric then looks configured (PCAP DONE asserts) while GP0
       accesses hang the bus. The FSBL clears this register after
       programming the PL; same order here. */
    fpga_reset_release_all();
}
