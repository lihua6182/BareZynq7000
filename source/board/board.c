#include "board.h"

#if BOARD_PL_ENABLE
#include "board_pl_bitstream.h"
#endif
#include "zynq_clock.h"
#include "zynq_mio.h"
#include "zynq_uart.h"

#if BOARD_PL_ENABLE
static enum pl_status pl_load_status = PL_UNLOADED;
#else
static enum pl_status pl_load_status = PL_DISABLED;
#endif

void board_console_init(void)
{
    mio_enable_clock();
    mio_set_function(BOARD_UART_TX_MIO, MIO_FN_UART);
    mio_set_function(BOARD_UART_RX_MIO, MIO_FN_UART);
    mio_enable_pull_up(BOARD_UART_TX_MIO);
    mio_enable_pull_up(BOARD_UART_RX_MIO);
    uart_init(BOARD_UART, BOARD_UART_BAUD_HZ);
}

int _write(int file, char *ptr, int len)
{
    int idx;

    (void)file;

    for (idx = 0; idx < len; idx++)
    {
        uart_putc(BOARD_UART, ptr[idx]);
    }
    return len;
}

int _read(int file, char *ptr, int len)
{
    int idx;

    (void)file;

    for (idx = 0; idx < len; idx++)
    {
        ptr[idx] = (char)uart_getc(BOARD_UART);
    }
    return len;
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

enum pl_status board_pl_status(void)
{
    return pl_load_status;
}

void board_init(void)
{
#if BOARD_PL_ENABLE
    pl_load_status = pl_load(pl_img, PL_IMG_LEN);

    if (pl_load_status == PL_OK)
    {
        clock_fclk_config(fclk0, fclk_src_io, 10U, 1U, true);
        clock_fclk_config(fclk1, fclk_src_io, 20U, 1U, true);
        pl_reset_release_all();
    }
#endif
}
