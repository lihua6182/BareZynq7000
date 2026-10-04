#ifndef BOARD_H
#define BOARD_H

#include "board_fpga.h"
#include "zynq_irq.h"
#include "zynq_mio.h"
#include "zynq_soc.h"
#include "zynq_tick.h"
#include "zynq_uart.h"

#define LED_MIO (24u) /* MIO pin number of the LED */

/* Board bring-up after soc_init: configure the PL, start its clock,
   release its resets. Poll-based, no GIC or tick needed. */
void board_init(void);

/* Outcome of the PL programming in board_init(). */
enum pl_status board_pl_status(void);

#endif /* BOARD_H */
