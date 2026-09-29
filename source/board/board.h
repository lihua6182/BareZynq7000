#ifndef BOARD_H
#define BOARD_H

#include "board_fpga.h"
#include "zynq_gpio.h"
#include "zynq_irq.h"
#include "zynq_soc_ip.h"
#include "zynq_tick.h"
#include "zynq_uart.h"

#define LED_MIO (24u) /* MIO pin number of the LED */

/* Board-stage bring-up, called after soc_init: configures the PL from
   the packed image, starts its clock, releases its resets. Poll-based;
   needs no GIC or timer tick. */
void board_init(void);

/* Outcome of the PL programming in board_init(). */
enum devcfg_status board_pl_status(void);

#endif /* BOARD_H */
