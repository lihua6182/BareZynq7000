#ifndef BOARD_H
#define BOARD_H

#include "zynq_tick.h"
#include "zynq_devcfg.h"
#include "zynq_irq.h"
#include "zynq_soc_ip.h"

#define LED_MIO  (24u)              /* MIO pin number of the LED              */
#define LED_BIT  (1UL << LED_MIO)   /* LED bit within GPIO bank 0 (MIO 0..31) */

/*
 * Board-stage bring-up, called at the top of main() once the soc stage has
 * run (soc_init: MMU, clocks, QSPI linear mode). Programs the PL from the
 * packed bitstream generated in this folder (bitstream.bin ->
 * board_fpga.c via the VSCode task "Convert PL bitstream"), so the rest
 * of main() runs with the PL up. Poll-based; needs no GIC or timer tick.
 */
void board_init(void);

/*
 * Outcome of the PL programming in board_init(), kept volatile so both
 * main() and a debugger can read the latest value directly.
 */
extern volatile enum devcfg_status pl_status;

#endif /* BOARD_H */
