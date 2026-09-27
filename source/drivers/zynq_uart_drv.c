#include "zynq_uart_drv.h"

#include "zynq_clock.h"

/*
 * Index of the UART in the SLCR-side registers (UART_CLK_CTRL.CLKACT and
 * UART_RST_CTRL come in per-UART pairs).
 */
static uint32_t zynq_uart_index(const zynq_uart_type* uart_x) {
    return (uart_x == ZYNQ_UART0) ? 0U : 1U;
}

/*
 * Soft reset through the SLCR: reset both clock domains of the selected
 * UART (reference and APB). All registers return to their reset values.
 */
void zynq_uart_reset(zynq_uart_type* uart_x) {
    uint32_t rst = (1U << zynq_uart_index(uart_x));

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->UART_RST_CTRL |= rst;
    SLCR->UART_RST_CTRL &= ~rst;
    SLCR->LOCK = SLCR_LOCK_KEY;
}

void zynq_uart_default_para_init(zynq_uart_init_type* init_struct) {
    init_struct->baudrate = 115200U;
    init_struct->data_word_length = ZYNQ_UART_DATA_8BITS;
    init_struct->parity_select = ZYNQ_UART_PARITY_NONE;
    init_struct->stop_bits = ZYNQ_UART_STOP_1_BIT;
    init_struct->mode = ZYNQ_UART_MODE_TX_RX;
}

/*
 * Baud rate divisors: baud = UART_REF_CLK / (CD x (BDIV + 1)) (UG585 Table
 * 19-1). Brute-force the BDIV range for the pair with the smallest relative
 * error; CD is rounded to the nearest value for each BDIV. Accepts anything
 * within 3 %, which every standard baud rate from the 50 MHz reference is
 * well inside (115200 lands at 115207 baud, 0.55 %).
 */
static error_status zynq_uart_baud_divisors_calc(uint32_t ref_clk, uint32_t baudrate, uint16_t* cd, uint8_t* bdiv) {
    uint32_t best_err = 0xFFFFFFFFU;
    uint32_t bdiv_plus_1;
    uint32_t cd_found = 0U;
    uint32_t bdiv_found = 0U;

    if (baudrate == 0U || ref_clk == 0U) {
        return FAILED;
    }

    for (bdiv_plus_1 = 5U; bdiv_plus_1 <= 256U; bdiv_plus_1++) {
        uint32_t product = baudrate * bdiv_plus_1;
        uint32_t cd_calc = (ref_clk + product / 2U) / product;
        uint32_t actual;
        uint32_t err;
        uint32_t err_ppm;

        if (cd_calc == 0U || cd_calc > 65535U) {
            continue;
        }
        actual = ref_clk / (cd_calc * bdiv_plus_1);
        err = (actual > baudrate) ? (actual - baudrate) : (baudrate - actual);
        err_ppm = (err * 100000UL) / baudrate;
        if (err_ppm < best_err) {
            best_err = err_ppm;
            cd_found = cd_calc;
            bdiv_found = bdiv_plus_1 - 1U;
        }
    }

    if (best_err == 0xFFFFFFFFU || best_err > 30000U) { /* > 3 %: not usable */
        return FAILED;
    }
    *cd = (uint16_t)cd_found;
    *bdiv = (uint8_t)bdiv_found;
    return SUCCESS;
}

/*
 * Configure and enable the UART. Programs the SLCR peripheral clock
 * (UART_REF_CLK = IO PLL / ZYNQ_UART_CLK_DIVISOR), the character frame, the
 * baud rate divisors and the FIFO levels, then soft-resets both paths and
 * enables them per the requested mode. Changing the shared UART_CLK_CTRL
 * divisor also shifts the reference clock of the other UART.
 */
error_status zynq_uart_init(zynq_uart_type* uart_x, zynq_uart_init_type* init_struct) {
    zynq_clock_freqs_type clocks;
    uint16_t cd;
    uint8_t bdiv;
    uint32_t mr;
    uint32_t cr;
    uint32_t clk;

    if (init_struct->baudrate == 0U) {
        return FAILED;
    }

    /* Peripheral clock: SRCSEL = IO PLL, DIVISOR, enable this UART. */
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    clk = SLCR->UART_CLK_CTRL;
    clk &= ~((0xFFU << 8) | (0x3U << 4)); /* DIVISOR field and SRCSEL = 00 */
    clk |= (20UL << 8) | (1U << zynq_uart_index(uart_x));
    SLCR->UART_CLK_CTRL = clk;
    SLCR->LOCK = SLCR_LOCK_KEY;

    /* Baud rate divisors from the reference clock just programmed, read
     * back from the SLCR (zynq_clock driver). */
    zynq_clock_freqs_get(&clocks);
    if (zynq_uart_baud_divisors_calc(clocks.uart_ref, init_struct->baudrate, &cd, &bdiv) != SUCCESS) {
        return FAILED;
    }

    /* Both paths must be disabled before touching MR and the baud rate
     * divisors (UG585 ch. 19 "Configure the Baud Rate"). */
    uart_x->CR = ZYNQ_UART_CR_TXDIS | ZYNQ_UART_CR_RXDIS;

    /* Character frame: CLKS = 0 (full reference clock), CHMODE = normal. */
    mr = (init_struct->data_word_length & 0xC0U) |
         (init_struct->parity_select & 0x38U) |
         (init_struct->stop_bits & 0x06U);
    uart_x->MR = mr;

    /* FIFO levels: RxFIFO trigger per byte, Rx timeout disabled. */
    uart_x->RXWM = 1U;
    uart_x->RXTOUT = 0U;

    /* Baud rate divisors. */
    uart_x->BAUDGEN = cd;
    uart_x->BAUDDIV = bdiv;

    /* Soft reset both paths (self-clearing bits), then enable the
     * configured ones. */
    cr = ZYNQ_UART_CR_TXRES | ZYNQ_UART_CR_RXRES;
    if ((init_struct->mode & ZYNQ_UART_MODE_RX) != 0U) {
        cr |= ZYNQ_UART_CR_RXEN;
    }
    if ((init_struct->mode & ZYNQ_UART_MODE_TX) != 0U) {
        cr |= ZYNQ_UART_CR_TXEN;
    }
    uart_x->CR = cr;

    return SUCCESS;
}

flag_status zynq_uart_flag_get(zynq_uart_type* uart_x, uint32_t flag) {
    return ((uart_x->SR & flag) != 0U) ? SET : RESET;
}

/*
 * Error and timeout flags clear through the write-1-to-clear interrupt
 * status register. The raw FIFO bits in SR (TEMPTY/TFUL/...) are read-only
 * and cannot be cleared.
 */
void zynq_uart_flag_clear(zynq_uart_type* uart_x, uint32_t flag) {
    uart_x->ISR = flag;
}

void zynq_uart_interrupt_enable(zynq_uart_type* uart_x, uint32_t source, confirm_state new_state) {
    if (new_state != DISABLE) {
        uart_x->IER = source;
    } else {
        uart_x->IDR = source;
    }
}

void zynq_uart_data_transmit(zynq_uart_type* uart_x, uint8_t data) {
    uart_x->FIFO = data;
}

uint8_t zynq_uart_data_receive(zynq_uart_type* uart_x) {
    return (uint8_t)uart_x->FIFO;
}

/* RxFIFO trigger level for the RTRIG flag/interrupt: 0 disables, 1..63. */
void zynq_uart_rx_fifo_level_config(zynq_uart_type* uart_x, uint8_t level) {
    uart_x->RXWM = (level > 63U) ? 63U : level;
}

/* Rx timeout in bit periods; 0 disables the timeout mechanism. */
void zynq_uart_rx_timeout_config(zynq_uart_type* uart_x, uint8_t timeout) {
    uart_x->RXTOUT = timeout;
}

/* Blocking byte transmit: waits for room in the TxFIFO, then writes. */
void zynq_uart_putc(zynq_uart_type* uart_x, uint8_t data) {
    while ((uart_x->SR & ZYNQ_UART_FLAG_TXFULL) != 0U) {
    }
    uart_x->FIFO = data;
}

/* Blocking byte receive: waits for a byte in the RxFIFO, then reads. */
uint8_t zynq_uart_getc(zynq_uart_type* uart_x) {
    while ((uart_x->SR & ZYNQ_UART_FLAG_RXEMPTY) != 0U) {
    }
    return (uint8_t)uart_x->FIFO;
}
