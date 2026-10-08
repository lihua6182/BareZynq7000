/**
 * @file board.c
 * @brief Board bring-up: console binding, LED, PL configuration and PL
 *        fabric clocks. USB0 的板级接线在 board_usb.c。
 */

#include "board.h"

#include "board_fpga.h"
#include "board_fpga_bitstream.h"
#include "zynq_clock.h"

/* Outcome of the last PL programming; query via board_pl_status(). */
static enum pl_status pl_load_status = PL_NO_IMAGE;

void board_console_init(void)
{
    mio_enable_clock();
    mio_set_function(BOARD_UART_TX_MIO, MIO_FN_UART);
    mio_set_function(BOARD_UART_RX_MIO, MIO_FN_UART);
    mio_enable_pull_up(BOARD_UART_TX_MIO);
    mio_enable_pull_up(BOARD_UART_RX_MIO);
    uart_init(BOARD_UART, BOARD_UART_BAUD_HZ);
}

/* newlib 控制台绑定: syscalls.c 的 _write/_read 循环弱符号
 * __io_putchar/__io_getchar (模板契约, 名字不可改), 这两个强版本把
 * printf 系列绑到板级控制台串口。 */
int __io_putchar(int ch)
{
    uart_putc(BOARD_UART, ch);
    return ch;
}

int __io_getchar(void)
{
    return uart_getc(BOARD_UART);
}

void board_led_init(void)
{
    mio_enable_clock();
    mio_set_function(BOARD_LED_MIO, MIO_FN_GPIO);
    mio_direction_output(BOARD_LED_MIO, 0);
}

void board_led_toggle(void)
{
    mio_toggle(BOARD_LED_MIO);
}

/**
 * @brief  Query the outcome of the PL programming done by board_init().
 * @return PL_OK, PL_NO_IMAGE (no bitstream linked), else the
 *         first load error.
 */
enum pl_status board_pl_status(void)
{
    return pl_load_status;
}

/**
 * @brief  Configure the PL, start its clock, release its resets.
 *
 * On a failed or missing image nothing is started: the reset lines stay
 * asserted so a half-configured design never runs.
 */
void board_init(void)
{
    /* fpga_img[] is generated from bitstream.bin; zero length = --stub. */
    if (FPGA_IMG_LEN != 0u)
    {
        pl_load_status = fpga_load(fpga_img, FPGA_IMG_LEN);
    }

    if (pl_load_status == PL_OK)
    {
        clock_fclk_config(FCLK0, FCLK_SRC_IO, 10U, 1U, true); /* FCLK0 = IO PLL / 10 = 100 MHz */
        clock_fclk_config(FCLK1, FCLK_SRC_IO, 20U, 1U, true); /* FCLK1 = IO PLL / 20 = 50 MHz */
        fpga_reset_release_all();
    }
}
