/**
 * @file main.c
 * @brief Application entry: board, tick, console and a heartbeat LED.
 */

#include <stdio.h>

#include "board.h"

/* AXI BRAM window in the PL design (block design address editor). */
#define BRAM_BASE  (0x40000000u)
#define BRAM_WORDS (2048u) /* 8 KB / 4 */

/* Address-mixed test word: Knuth's multiplicative hash XOR a marker. */
#define BRAM_PAT(i) ((((uint32_t)(i)) * 2654435761u) ^ 0xA5A5A5A5u)

/* newlib console binding: syscalls.c's _write/_read loop over these. */
int __io_putchar(int ch) {
    uart_putc(UART0, ch);
    return ch;
}

int __io_getchar(void) {
    return uart_getc(UART0);
}

/**
 * @brief  Write-and-verify test over the AXI BRAM window.
 * @return True when every location read back both patterns.
 *
 * Two passes: the pattern and its complement, so both a stuck bit and
 * its inverted twin fail. The first mismatch is printed with address,
 * expected and actual word. Runs only when the PL is configured - a GP0
 * access with no live slave hangs the bus.
 */
static bool bram_test(void) {
    volatile uint32_t* bram = (volatile uint32_t*)BRAM_BASE;
    uint32_t pass;
    uint32_t i;

    for (pass = 0U; pass < 2U; pass++) {
        uint32_t flip = (pass == 0U) ? 0U : 0xFFFFFFFFU;

        for (i = 0U; i < BRAM_WORDS; i++) {
            bram[i] = BRAM_PAT(i) ^ flip;
        }
        for (i = 0U; i < BRAM_WORDS; i++) {
            if (bram[i] != (BRAM_PAT(i) ^ flip)) {
                printf("bram @%08X want %08X got %08X\n",
                       (unsigned)(BRAM_BASE + (i * 4U)),
                       (unsigned)(BRAM_PAT(i) ^ flip), (unsigned)bram[i]);
                return false;
            }
        }
    }
    return true;
}

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

    printf("pl_status=%u\n", (unsigned)board_pl_status());

    if (board_pl_status() == DEVCFG_OK) {
        printf("bram_test %s\n", bram_test() ? "OK" : "FAIL");
    }

    for (;;) {
        gpio_toggle(LED_MIO);
        printf("tick %u\n", (unsigned)tick_get());
        tick_mdelay(board_pl_status() == DEVCFG_OK ? 500 : 100);
    }
}
