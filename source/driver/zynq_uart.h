#ifndef ZYNQ_UART_H
#define ZYNQ_UART_H

#include "zynq_soc.h"

#define UART_DATABITS_6 (6U)
#define UART_DATABITS_7 (7U)
#define UART_DATABITS_8 (8U)

#define UART_PARITY_NONE (0U)
#define UART_PARITY_ODD  (1U)
#define UART_PARITY_EVEN (2U)

#define UART_STOPBITS_1 (1U)
#define UART_STOPBITS_2 (2U)

static inline void uart_send_byte(struct uart *uart, uint8_t c)
{
    uart->fifo = (uint32_t)c;
}

static inline uint8_t uart_recv_byte(struct uart *uart)
{
    return (uint8_t)(uart->fifo & 0xFFU);
}

static inline bool uart_tx_ready(struct uart *uart)
{
    return (uart->sr & UART_SR_TXFULL) == 0U;
}

static inline bool uart_tx_done(struct uart *uart)
{
    return (uart->sr & UART_SR_TXEMPTY) != 0U;
}

static inline bool uart_rx_ready(struct uart *uart)
{
    return (uart->sr & UART_SR_RXEMPTY) == 0U;
}

static inline void uart_enable(struct uart *uart)
{
    uart->cr = (uart->cr & ~UART_CR_EN_DIS_Msk) | UART_CR_RX_EN | UART_CR_TX_EN;
}

static inline void uart_disable(struct uart *uart)
{
    uart->cr = (uart->cr & ~UART_CR_EN_DIS_Msk) | UART_CR_RX_DIS | UART_CR_TX_DIS;
}

static inline void uart_enable_tx(struct uart *uart)
{
    uart->cr = (uart->cr & ~UART_CR_EN_DIS_Msk) | UART_CR_TX_EN;
}

static inline void uart_disable_tx(struct uart *uart)
{
    uart->cr = (uart->cr & ~UART_CR_EN_DIS_Msk) | UART_CR_TX_DIS;
}

static inline void uart_enable_rx(struct uart *uart)
{
    uart->cr = (uart->cr & ~UART_CR_EN_DIS_Msk) | UART_CR_RX_EN;
}

static inline void uart_disable_rx(struct uart *uart)
{
    uart->cr = (uart->cr & ~UART_CR_EN_DIS_Msk) | UART_CR_RX_DIS;
}

static inline void uart_reset_tx(struct uart *uart)
{
    uart->cr |= UART_CR_TXRST;
}

static inline void uart_reset_rx(struct uart *uart)
{
    uart->cr |= UART_CR_RXRST;
}

static inline void uart_set_baud_gen(struct uart *uart, uint32_t baudgen)
{
    uart->baudgen = baudgen & UART_BAUDGEN_MASK_Msk;
}

static inline void uart_set_baud_div(struct uart *uart, uint32_t bauddiv)
{
    uart->bauddiv = bauddiv & UART_BAUDDIV_MASK_Msk;
}

static inline uint32_t uart_irq_status(struct uart *uart)
{
    return (uart->isr & UART_IXR_MASK_Msk);
}

static inline uint32_t uart_irq_mask(struct uart *uart)
{
    return (uart->imr & UART_IXR_MASK_Msk);
}

static inline void uart_irq_enable(struct uart *uart, uint32_t mask)
{
    uart->ier = (mask & UART_IXR_MASK_Msk);
}

static inline void uart_irq_disable(struct uart *uart, uint32_t mask)
{
    uart->idr = (mask & UART_IXR_MASK_Msk);
}

static inline void uart_irq_clear(struct uart *uart, uint32_t mask)
{
    uart->isr = (mask & UART_IXR_MASK_Msk);
}

static inline void uart_set_rx_timeout(struct uart *uart, uint32_t n)
{
    uart->rxtout = (n & UART_RXTOUT_MASK_Msk);
}

static inline void uart_set_rx_watermark(struct uart *uart, uint32_t n)
{
    uart->rxwm = (n & UART_RXWM_MASK_Msk);
}

static inline void uart_set_tx_watermark(struct uart *uart, uint32_t n)
{
    uart->txwm = (n & UART_TXWM_MASK_Msk);
}

void uart_enable_clock(struct uart *uart);

void uart_disable_clock(struct uart *uart);

void uart_set_databits(struct uart *uart, uint32_t bits);

void uart_set_parity(struct uart *uart, uint32_t parity);

void uart_set_stopbits(struct uart *uart, uint32_t bits);

int uart_set_baud(struct uart *uart, uint32_t baud);

int uart_init(struct uart *uart, uint32_t baud);

void uart_putc(struct uart *uart, int c);

int uart_getc(struct uart *uart);

int uart_try_getc(struct uart *uart);

void uart_puts(struct uart *uart, const char *s);

#endif
