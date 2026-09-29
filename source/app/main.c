/**
 * @file main.c
 * @brief Application entry: board, tick, console and a heartbeat LED.
 */

#include "board.h"

/**
 * @brief  Application entry point.
 * @return Does not return.
 */
int main(void) {
    board_init();

    irq_init();
    tick_init();

    mio_set_function(LED_MIO, GPIO_MIO24_GPIO);
    mio_set_function(11, GPIO_MIO11_UART0);
    mio_set_function(10, GPIO_MIO10_UART0);
    mio_enable_pull_up(11);
    mio_enable_pull_up(10);

    gpio_enable_clock();
    gpio_direction_output(LED_MIO, 0);

    uart_init(UART0, 115200);

    uart_puts(UART0, "pl_status=");
    uart_putdec(UART0, (uint32_t)pl_status);
    uart_putc(UART0, '\n');

    for (;;) {
        gpio_toggle(LED_MIO);
        uart_puts(UART0, "tick ");
        uart_putdec(UART0, tick_get());
        uart_putc(UART0, '\n');
        tick_mdelay(pl_status == DEVCFG_OK ? 500 : 100);
    }
}
