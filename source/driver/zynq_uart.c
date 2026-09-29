/**
 * @file zynq_uart.c
 * @brief UART driver implementation (UG585 ch.19).
 *
 * Blocking I/O, format setup, baud rate search and clock gating - the
 * parts that need more than one register access. FIFO and status
 * primitives are static inline in zynq_uart.h.
 */

#include "zynq_uart.h"

#include <stddef.h>

#include "zynq_clock.h"

/* APER_CLK_CTRL ref clock gates: bit 20 UART0, bit 21 UART1. */
#define APER_UART0_CLK (1UL << 20)
#define APER_UART1_CLK (1UL << 21)

/**
 * @brief  Find the CD/BDIV pair closest to a requested baud rate.
 * @param  ref  UART reference clock in Hz.
 * @param  baud Requested baud rate.
 * @param  cd   Out: BAUDGEN divisor, 1..65535.
 * @param  bdiv Out: BAUDDIV divisor, 4..255.
 * @return 0 when a pair exists, -1 when no setting comes close.
 *
 * baud = ref / (cd * (bdiv + 1)). Scans bdiv, rounds cd, keeps the pair
 * with the least absolute error (UG585 ch.19 "Baud Rate"). bdiv below 4
 * is not a valid setting, hence the start of the scan.
 */
static int baud_divs(uint32_t ref, uint32_t baud, uint32_t* cd, uint32_t* bdiv) {
    uint32_t best_err = 0xFFFFFFFFUL;
    uint32_t b;
    uint32_t c;
    uint32_t denom;
    uint32_t actual;
    uint32_t err;

    for (b = 4U; b <= 255U; b++) {
        denom = baud * (b + 1U);
        c = (ref + denom / 2U) / denom;
        if ((c == 0U) || (c > 0xFFFFUL)) {
            continue;
        }
        actual = ref / (c * (b + 1U));
        err = (actual > baud) ? (actual - baud) : (baud - actual);
        if (err < best_err) {
            best_err = err;
            *cd = c;
            *bdiv = b;
        }
    }
    return (best_err == 0xFFFFFFFFUL) ? -1 : 0;
}

/**
 * @brief  Get the APER_CLK_CTRL gate bit of a controller.
 * @param  uart UART0 or UART1.
 * @return The clock-gate bit for that controller.
 */
static uint32_t aper_gate(UART_Type* uart) {
    return (uart == UART1) ? APER_UART1_CLK : APER_UART0_CLK;
}

/**
 * @brief  Ungate a controller's reference clock.
 * @param  uart UART0 or UART1.
 */
void uart_enable_clock(UART_Type* uart) {
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->APER_CLK_CTRL |= aper_gate(uart);
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  Gate a controller's reference clock.
 * @param  uart UART0 or UART1.
 */
void uart_disable_clock(UART_Type* uart) {
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->APER_CLK_CTRL &= ~aper_gate(uart);
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  Select the character length.
 * @param  uart UART0 or UART1.
 * @param  bits UART_DATABITS_6, _7 or _8; anything else means 8 bits.
 */
void uart_set_databits(UART_Type* uart, uint32_t bits) {
    uint32_t mr = uart->MR & ~UART_MR_CHARLEN_Msk;

    if (bits == UART_DATABITS_6) {
        mr |= UART_MR_CHARLEN_6_BIT;
    } else if (bits == UART_DATABITS_7) {
        mr |= UART_MR_CHARLEN_7_BIT;
    } else {
        mr |= UART_MR_CHARLEN_8_BIT;
    }
    uart->MR = mr;
}

/**
 * @brief  Select the parity mode.
 * @param  uart   UART0 or UART1.
 * @param  parity UART_PARITY_ODD, _EVEN or _NONE; anything else means none.
 */
void uart_set_parity(UART_Type* uart, uint32_t parity) {
    uint32_t mr = uart->MR & ~UART_MR_PARITY_Msk;

    if (parity == UART_PARITY_ODD) {
        mr |= UART_MR_PARITY_ODD;
    } else if (parity == UART_PARITY_EVEN) {
        mr |= UART_MR_PARITY_EVEN;
    } else {
        mr |= UART_MR_PARITY_NONE;
    }
    uart->MR = mr;
}

/**
 * @brief  Select the number of stop bits.
 * @param  uart UART0 or UART1.
 * @param  bits UART_STOPBITS_2, or anything else for one stop bit.
 */
void uart_set_stopbits(UART_Type* uart, uint32_t bits) {
    uint32_t mr = uart->MR & ~UART_MR_STOPMODE_Msk;

    if (bits == UART_STOPBITS_2) {
        mr |= UART_MR_STOPMODE_2_BIT;
    } else {
        mr |= UART_MR_STOPMODE_1_BIT;
    }
    uart->MR = mr;
}

/**
 * @brief  Program the baud rate from the live reference clock.
 * @param  uart UART0 or UART1.
 * @param  baud Requested baud rate.
 * @return 0 on success, -1 when baud is 0 or unreachable.
 *
 * Safe to call while running: only the two divisor registers change.
 */
int uart_set_baud(UART_Type* uart, uint32_t baud) {
    struct clk_freqs clks;
    uint32_t cd;
    uint32_t bdiv;

    if (baud == 0U) {
        return -1;
    }
    clk_freqs_get(&clks);
    if (baud_divs(clks.uart_ref, baud, &cd, &bdiv) != 0) {
        return -1;
    }
    uart_set_baud_gen(uart, cd);
    uart_set_baud_div(uart, bdiv);
    return 0;
}

/**
 * @brief  Initialize a controller for 8N1 polled I/O.
 * @param  uart UART0 or UART1.
 * @param  baud Requested baud rate.
 * @return 0 on success, -1 when the baud rate is unreachable.
 *
 * MIO routing and clock setup are the caller's job; this gates the ref
 * clock on, resets both paths, programs the format and the divisors and
 * leaves every interrupt masked.
 */
int uart_init(UART_Type* uart, uint32_t baud) {
    uart_enable_clock(uart);

    /* Both resets in one write: the enable bits go with them, so the
       port is quiet while the format and the divisors are programmed. */
    uart->CR = UART_CR_RXRST | UART_CR_TXRST;

    uart_set_databits(uart, UART_DATABITS_8);
    uart_set_parity(uart, UART_PARITY_NONE);
    uart_set_stopbits(uart, UART_STOPBITS_1);

    if (uart_set_baud(uart, baud) != 0) {
        return -1;
    }

    uart_irq_disable(uart, UART_IXR_MASK_Msk); /* poll mode */
    uart_irq_clear(uart, UART_IXR_MASK_Msk);   /* drop stale flags */
    uart->CR = UART_CR_RX_EN | UART_CR_TX_EN;
    return 0;
}

/**
 * @brief  Send one character, blocking until the FIFO has room.
 * @param  uart UART0 or UART1.
 * @param  c    Character to send.
 */
void uart_putc(UART_Type* uart, int c) {
    while (uart_tx_ready(uart) == 0) {
    }
    uart_send_byte(uart, (uint8_t)c);
}

/**
 * @brief  Receive one character, blocking until one arrives.
 * @param  uart UART0 or UART1.
 * @return The received byte, 0..255.
 */
int uart_getc(UART_Type* uart) {
    while (uart_rx_ready(uart) == 0) {
    }
    return (int)uart_recv_byte(uart);
}

/**
 * @brief  Receive one character without blocking.
 * @param  uart UART0 or UART1.
 * @return The received byte 0..255, or -1 when the FIFO is empty.
 */
int uart_try_getc(UART_Type* uart) {
    if (uart_rx_ready(uart) == 0) {
        return -1;
    }
    return (int)uart_recv_byte(uart);
}

/**
 * @brief  Send a NUL-terminated string.
 * @param  uart UART0 or UART1.
 * @param  s    String to send; NULL is ignored.
 */
void uart_puts(UART_Type* uart, const char* s) {
    if (s == NULL) {
        return;
    }
    while (*s != '\0') {
        uart_putc(uart, (int)*s);
        s++;
    }
}

/**
 * @brief  Send a block of bytes.
 * @param  uart UART0 or UART1.
 * @param  buf  First byte to send.
 * @param  len  Number of bytes.
 * @return Number of bytes sent, or -1 when buf is NULL.
 */
int uart_write(UART_Type* uart, const void* buf, uint32_t len) {
    const uint8_t* p = (const uint8_t*)buf;
    uint32_t i;

    if (p == NULL) {
        return -1;
    }
    for (i = 0U; i < len; i++) {
        uart_putc(uart, (int)p[i]);
    }
    return (int)i;
}

/**
 * @brief  Print an unsigned value in decimal.
 * @param  uart UART0 or UART1.
 * @param  v    Value to print.
 */
void uart_putdec(UART_Type* uart, uint32_t v) {
    char buf[10]; /* 4294967295 */
    uint32_t n = 0U;

    do {
        buf[n] = (char)('0' + (v % 10U));
        v /= 10U;
        n++;
    } while (v != 0U);
    while (n != 0U) {
        n--;
        uart_putc(uart, (int)buf[n]);
    }
}

/**
 * @brief  Print an unsigned value as 8 uppercase hex digits.
 * @param  uart UART0 or UART1.
 * @param  v    Value to print.
 */
void uart_puthex(UART_Type* uart, uint32_t v) {
    static const char hex[] = "0123456789ABCDEF";
    uint32_t i;

    for (i = 0U; i < 8U; i++) {
        uart_putc(uart, (int)hex[(v >> 28U) & 0xFU]);
        v <<= 4U;
    }
}
