/**
 * @file main.c
 * @brief Application entry: board bring-up, then the coprocessor SPI
 *        stress run (spi_stress.s from C:\WorkSpace\PPC, assembled with
 *        its pasm.py, ISA v1.2).
 */

#include <stdio.h>

#include "board.h"
#include "zynq_ppc.h"

/* proto_coproc window in the PL design (MioLink01: coproc only,
 * SmartConnect, 64K segment at the GP0 base). Any access below requires
 * the PL to be configured - otherwise the GP0 bus hangs. */
#define COPROC ((COPROC_Type*)0x40000000u)

/* spi_stress.s via pasm --c stress_prog (lean variant): infinite SPI word
 * stream through the SHF engine. No autopush, no program heartbeat - the
 * RX FIFO stays silent and the host only feeds TX, so the rate limit is
 * the engine itself, not the AXI drain path. Masks are GPIOW=8 layout
 * (out = cfg[7:0], in = cfg[15:8]).
 * Rate: 133 cycles/word at div=1 (SCK = FCLK0/4 = 25MHz while shifting
 * with FCLK0 = 100MHz, 0.75 Mword/s ideal). Words are counted as TX
 * pushes; TX_OVF sticky would flag any overrun. */
static const uint32_t stress_prog[] = {
    0x3C800102,
    0xDC040000,
    0x3CA00001,
    0xDC050001,
    0x3D200003,
    0xA0090000,
    0x3D000037,
    0x3CE00020,
    0xE1A83800,
    0x4C000008,
};
#define STRESS_PROG_WORDS (10u)

/* newlib console binding: syscalls.c's _write/_read loop over these. */
int __io_putchar(int ch) {
    uart_putc(UART0, ch);
    return ch;
}

int __io_getchar(void) {
    return uart_getc(UART0);
}

/**
 * @brief  SPI stress test (lean): run spi_stress.s for STRESS_SECONDS.
 *         The engine consumes one TX word per 133 cycles (32 bits x 4,
 *         div=1); the host pushes as fast as it can and counts the pushes
 *         as words (the engine is faster than the host here, so pushes ≈
 *         words shifted; the in-flight error is bounded by the 64-deep
 *         FIFO). Per-second report: words/s and engine efficiency
 *         (133 cycles/word ideal, measured against CYCLE); efficiency
 *         below 100% is the PS polling bandwidth showing up. Sticky FIFO
 *         error bits in STATUS must stay clear.
 */
#define STRESS_SECONDS         (30u)
#define STRESS_CYCLES_PER_WORD (133u)

static void stress_run(void) {
    uint32_t tx = 0x1A2B3C4Du;
    uint32_t words = 0u;
    uint32_t t0, t_last, w_last, c_last;
    uint32_t st;

    printf("stress: %u s of continuous SPI, SCK=25MHz while shifting\n",
           (unsigned)STRESS_SECONDS);
    coproc_cmd(COPROC, COPROC_CMD_FLUSH_ALL);
    coproc_load(COPROC, stress_prog, STRESS_PROG_WORDS);
    coproc_cmd(COPROC, COPROC_CMD_RUN);

    t0 = tick_get();
    t_last = t0;
    w_last = 0u;
    c_last = COPROC->CYCLE;
    while ((tick_get() - t0) < STRESS_SECONDS * 1000u) {
        if (coproc_tx_ready(COPROC)) {
            coproc_tx_push(COPROC, tx);
            tx++;
            words++;
        }
        while (coproc_rx_ready(COPROC)) { /* lean program: should stay empty */
            (void)coproc_rx_pop(COPROC);
        }
        if ((tick_get() - t_last) >= 1000u) {
            uint32_t dw = words - w_last;
            uint32_t dc = COPROC->CYCLE - c_last;
            w_last = words;
            c_last = COPROC->CYCLE;
            t_last = tick_get();
            printf("stress: %3us  %6u w/s  eff=%u%%  status=%08X\n",
                   (unsigned)((t_last - t0) / 1000u), (unsigned)dw,
                   (unsigned)((dc != 0u)
                                  ? ((unsigned long long)dw *
                                     (STRESS_CYCLES_PER_WORD * 100u)) /
                                        dc
                                  : 0u),
                   (unsigned)COPROC->STATUS);
        }
    }

    /* read stickies before RESET clears them */
    st = COPROC->STATUS;
    coproc_cmd(COPROC, COPROC_CMD_RESET);
    printf("stress done: %u words, %u.%02u MB/s avg\n",
           (unsigned)words,
           (unsigned)(words * 4u / (STRESS_SECONDS * 1000000u)),
           (unsigned)((words * 4u / (STRESS_SECONDS * 10000u)) % 100u));
    printf("stress done: STATUS=%08X - TX_OVF(bit11)/RX_UNF(bit12) must be 0\n",
           (unsigned)st);
}

/**
 * @brief  Application entry point.
 * @return Does not return.
 */
int main(void) {
    board_init();

    irq_init();
    tick_init();

    mio_set_function(LED_MIO, MIO24_GPIO);
    mio_set_function(11, MIO11_UART0);
    mio_set_function(10, MIO10_UART0);
    mio_enable_pull_up(11);
    mio_enable_pull_up(10);

    mio_enable_clock();
    mio_direction_output(LED_MIO, 0);

    uart_init(UART0, 115200);

    printf("pl_status=%u\n", (unsigned)board_pl_status());

    if (board_pl_status() != PL_OK) {
        printf("PL not configured, coproc test skipped (bus would hang)\n");
    } else {
        stress_run();
    }

    for (;;) {
        mio_toggle(LED_MIO);
        tick_mdelay(500);
    }
}
