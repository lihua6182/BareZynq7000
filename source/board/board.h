#ifndef BOARD_H
#define BOARD_H

#include "board_pl.h"
#include "zynq_soc.h"

#ifndef BOARD_PL_ENABLE
#define BOARD_PL_ENABLE (1)
#endif

#define BOARD_LED_MIO (11U)

#define BOARD_UART         (uart1)
#define BOARD_UART_TX_MIO  (36U)
#define BOARD_UART_RX_MIO  (37U)
#define BOARD_UART_BAUD_HZ (115200U)

#define BOARD_USB               (usb1)
#define USB_ULPI_FIRST_MIO      (40U)
#define USB_ULPI_LAST_MIO       (51U)
#define BOARD_USB_PHY_RESET_MIO (38U)

enum pl_status board_pl_status(void);

void board_console_init(void);
void board_led_init(void);
void board_led_toggle(void);
void board_init(void);

#endif
