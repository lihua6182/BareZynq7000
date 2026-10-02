#ifndef ZYNQ_UART_H
#define ZYNQ_UART_H

#include "zynq_soc_ip.h"

/*
 * Poll-mode driver for the two Cadence UARTs (UG585 ch.19), uart_* with
 * the controller (UART0 / UART1) first. Register primitives are static
 * inline and never block; blocking helpers, the baud search and the
 * number printers live in zynq_uart.c. Baud = uart_ref / (baud_gen *
 * (baud_div + 1)). Pin routing is the board's job (mio_set_function()).
 *
 *   uart_init(UART1, 115200);
 *   uart_puts(UART1, "hello\n");
 *   c = uart_getc(UART1);
 */

/* uart_set_databits() argument. */
#define UART_DATABITS_6 (6U)
#define UART_DATABITS_7 (7U)
#define UART_DATABITS_8 (8U)

/* uart_set_parity() argument. */
#define UART_PARITY_NONE (0U)
#define UART_PARITY_ODD  (1U)
#define UART_PARITY_EVEN (2U)

/* uart_set_stopbits() argument; the 1.5-bit mode is not offered. */
#define UART_STOPBITS_1 (1U)
#define UART_STOPBITS_2 (2U)

/* ------------------------------------------------------------------ */
/* Data                                                               */
/* ------------------------------------------------------------------ */

/* Push one byte into the TX FIFO; check uart_tx_ready() first. */
static inline void uart_send_byte(UART_Type* uart, uint8_t c) {
    uart->FIFO = (uint32_t)c;
}

/* Pull one byte out of the RX FIFO. Check uart_rx_ready() first. */
static inline uint8_t uart_recv_byte(UART_Type* uart) {
    return (uint8_t)(uart->FIFO & 0xFFUL);
}

/* Room in the TX FIFO? */
static inline int uart_tx_ready(UART_Type* uart) {
    return ((uart->SR & UART_SR_TXFULL) == 0U) ? 1 : 0;
}

/* TX FIFO empty incl. shift register: last byte is on the wire. */
static inline int uart_tx_done(UART_Type* uart) {
    return ((uart->SR & UART_SR_TXEMPTY) != 0U) ? 1 : 0;
}

/* Byte waiting in the RX FIFO? */
static inline int uart_rx_ready(UART_Type* uart) {
    return ((uart->SR & UART_SR_RXEMPTY) == 0U) ? 1 : 0;
}

/* ------------------------------------------------------------------ */
/* Enable / reset                                                     */
/* ------------------------------------------------------------------ */

/* Turn the receiver and transmitter on. */
static inline void uart_enable(UART_Type* uart) {
    uart->CR = (uart->CR & ~UART_CR_EN_DIS_Msk) | UART_CR_RX_EN | UART_CR_TX_EN;
}

/* Turn both off; in-flight characters are flushed. */
static inline void uart_disable(UART_Type* uart) {
    uart->CR = (uart->CR & ~UART_CR_EN_DIS_Msk) | UART_CR_RX_DIS | UART_CR_TX_DIS;
}

/* Transmitter on. */
static inline void uart_enable_tx(UART_Type* uart) {
    uart->CR = (uart->CR & ~UART_CR_EN_DIS_Msk) | UART_CR_TX_EN;
}

/* Transmitter off. */
static inline void uart_disable_tx(UART_Type* uart) {
    uart->CR = (uart->CR & ~UART_CR_EN_DIS_Msk) | UART_CR_TX_DIS;
}

/* Receiver on. */
static inline void uart_enable_rx(UART_Type* uart) {
    uart->CR = (uart->CR & ~UART_CR_EN_DIS_Msk) | UART_CR_RX_EN;
}

/* Receiver off. */
static inline void uart_disable_rx(UART_Type* uart) {
    uart->CR = (uart->CR & ~UART_CR_EN_DIS_Msk) | UART_CR_RX_DIS;
}

/* Pulse the transmitter reset (self-clearing). */
static inline void uart_reset_tx(UART_Type* uart) {
    uart->CR |= UART_CR_TXRST;
}

/* Pulse the receiver reset (self-clearing). */
static inline void uart_reset_rx(UART_Type* uart) {
    uart->CR |= UART_CR_RXRST;
}

/* ------------------------------------------------------------------ */
/* Baud rate                                                          */
/* ------------------------------------------------------------------ */

/* BAUDGEN, the clock divider: 1..65535, 0 disables the clock. */
static inline void uart_set_baud_gen(UART_Type* uart, uint32_t baudgen) {
    uart->BAUDGEN = baudgen & UART_BAUDGEN_MASK_Msk;
}

/* BAUDDIV, the second divider: the register value is (divisor - 1). */
static inline void uart_set_baud_div(UART_Type* uart, uint32_t bauddiv) {
    uart->BAUDDIV = bauddiv & UART_BAUDDIV_MASK_Msk;
}

/* ------------------------------------------------------------------ */
/* Interrupt                                                          */
/* ------------------------------------------------------------------ */

/* Latched status (UART_IXR_* bits), regardless of the mask. */
static inline uint32_t uart_irq_status(UART_Type* uart) {
    return (uart->ISR & UART_IXR_MASK_Msk);
}

/* Currently unmasked sources (UART_IXR_* bits). */
static inline uint32_t uart_irq_mask(UART_Type* uart) {
    return (uart->IMR & UART_IXR_MASK_Msk);
}

/* Unmask the given UART_IXR_* sources. */
static inline void uart_irq_enable(UART_Type* uart, uint32_t mask) {
    uart->IER = (mask & UART_IXR_MASK_Msk);
}

/* Mask the given UART_IXR_* sources. */
static inline void uart_irq_disable(UART_Type* uart, uint32_t mask) {
    uart->IDR = (mask & UART_IXR_MASK_Msk);
}

/* ISR is write-1-to-clear: write back the bits to drop them. */
static inline void uart_irq_clear(UART_Type* uart, uint32_t mask) {
    uart->ISR = (mask & UART_IXR_MASK_Msk);
}

/* ------------------------------------------------------------------ */
/* FIFO levels                                                        */
/* ------------------------------------------------------------------ */

/* RX timeout in baud-clock quarters; 0 disables it. */
static inline void uart_set_rx_timeout(UART_Type* uart, uint32_t n) {
    uart->RXTOUT = (n & UART_RXTOUT_MASK_Msk);
}

/* RX FIFO level that raises the trigger interrupt. */
static inline void uart_set_rx_watermark(UART_Type* uart, uint32_t n) {
    uart->RXWM = (n & UART_RXWM_MASK_Msk);
}

/* TX FIFO level that raises the trigger interrupt. */
static inline void uart_set_tx_watermark(UART_Type* uart, uint32_t n) {
    uart->TXWM = (n & UART_TXWM_MASK_Msk);
}

/* ------------------------------------------------------------------ */
/* Configuration                                                      */
/* ------------------------------------------------------------------ */

/* Ungate the controller's ref clock (SLCR APER_CLK_CTRL bit 20/21).
   With the clock gated the registers read back as zeros. */
void uart_enable_clock(UART_Type* uart);

/* Gate the ref clock again. */
void uart_disable_clock(UART_Type* uart);

/* Data bits. Anything other than 6/7/8 selects 8. */
void uart_set_databits(UART_Type* uart, uint32_t bits);

/* Parity: none, odd or even. */
void uart_set_parity(UART_Type* uart, uint32_t parity);

/* Stop bits: 1 or 2. Anything else selects 1. */
void uart_set_stopbits(UART_Type* uart, uint32_t bits);

/*
 * Pick the divisor pair with the smallest error and program it. Safe to
 * call on a running port. Returns 0, or -1 when the rate is out of
 * range for the current uart_ref.
 */
int uart_set_baud(UART_Type* uart, uint32_t baud);

/*
 * Reset the port and bring it up at 8N1 with the given baud rate:
 * clock, both resets, format, baud, interrupts masked, then receiver
 * and transmitter on. Returns 0, or -1 when baud cannot be reached.
 */
int uart_init(UART_Type* uart, uint32_t baud);

/* ------------------------------------------------------------------ */
/* Blocking transfer                                                  */
/* ------------------------------------------------------------------ */

/* Send one character, waiting for room in the TX FIFO. */
void uart_putc(UART_Type* uart, int c);

/* Receive one character, waiting for one to arrive: 0..255. */
int uart_getc(UART_Type* uart);

/* Receive one character without waiting: 0..255, or -1 if none. */
int uart_try_getc(UART_Type* uart);

/* Send a NUL-terminated string. */
void uart_puts(UART_Type* uart, const char* s);

#endif
