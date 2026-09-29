#include "board.h"

#include "board_fpga.h"

/*
 * Outcome of the PL programming below, kept volatile and non-static so both
 * main() and a debugger can read the latest value directly.
 */
volatile enum devcfg_status pl_status = DEVCFG_NO_IMAGE;

void board_init(void) {
    /*
     * Board stage: configure the PL from the packed bitstream (UG585 ch.6
     * PCAP flow) before anything drives PS->PL signals. fpga_img[] is
     * generated from bitstream.bin (same folder) by the VSCode task
     * "Convert PL bitstream" (tools/convert_bitstream.py); a zero-length array
     * means the pair was generated with --stub and carries no image.
     * Poll-based, so it runs fine before the tick and the GIC come up.
     */
    devcfg_init();
    if (FPGA_IMG_LEN != 0u) {
        pl_status = devcfg_load(fpga_img, FPGA_IMG_LEN);
    }
}
