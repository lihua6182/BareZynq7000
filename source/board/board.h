#ifndef BOARD_H
#define BOARD_H

#include "board_fpga.h"
#include "zynq_gpio.h"
#include "zynq_irq.h"
#include "zynq_soc_ip.h"
#include "zynq_tick.h"
#include "zynq_uart.h"

#define LED_MIO (24u) /* MIO pin number of the LED */

/*
 * Board-stage bring-up, called at the top of main() once the soc stage has
 * run (soc_init: MMU, clocks, QSPI linear mode). Configures the PL from
 * the packed image in board_fpga_bitstream.c (generated from bitstream.bin
 * by the VSCode task "Convert PL bitstream"), starts its clock and
 * releases its resets, so the rest of main() runs with the PL up.
 * Poll-based; needs no GIC or timer tick.
 */
void board_init(void);

/*
 * Outcome of the PL programming in board_init(), kept volatile so both
 * main() and a debugger can read the latest value directly.
 */
extern volatile enum devcfg_status pl_status;

#endif /* BOARD_H */
