#ifndef BOARD_H
#define BOARD_H

#include "board_fpga.h"
#include "zynq_irq.h"
#include "zynq_mio.h"
#include "zynq_soc.h"
#include "zynq_tick.h"
#include "zynq_uart.h"

/* ------------------------------------------------------------------ */
/* 板级引脚配置: 改这里即可换 LED / 控制台串口的引脚                   */
/* ------------------------------------------------------------------ */

/* LED: 推挽输出 (任意 MIO 脚, GPIO 为 L0 复用, 全脚一致)。 */
#define BOARD_LED_MIO (24u)

/* 控制台串口: UART 控制器 + 收发脚 (UART 复用全脚一致, 收发脚须为
 * 同一控制器的合法引脚对, UG585 Table 2-4)。 */
#define BOARD_UART         (UART0)
#define BOARD_UART_TX_MIO  (11u)
#define BOARD_UART_RX_MIO  (10u)
#define BOARD_UART_BAUD_HZ (115200u)

/**
 * @brief  控制台串口就绪: 收发脚复用 + 上拉 + uart_init(BOARD_UART_BAUD_HZ)。
 * @note   供 printf/uart_putc 前调用; 与 board_init() 无依赖, 可在其前。
 */
void board_console_init(void);

/** @brief  LED 脚复用为 GPIO 推挽输出, 初态低。依赖 board_console_init()
 *          或任何先行的 mio_enable_clock() (此处自调, 幂等)。 */
void board_led_init(void);

/** @brief  LED 电平翻转。 */
void board_led_toggle(void);

/* Board bring-up after soc_init: configure the PL, start its clock,
   release its resets. Poll-based, no GIC or tick needed. */
void board_init(void);

/* Outcome of the PL programming in board_init(). */
enum pl_status board_pl_status(void);

#endif /* BOARD_H */
