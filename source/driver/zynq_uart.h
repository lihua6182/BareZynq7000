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

void uart_send_byte(struct uart *uart, uint8_t c);

uint8_t uart_recv_byte(struct uart *uart);

bool uart_tx_ready(struct uart *uart);

bool uart_tx_done(struct uart *uart);

bool uart_rx_ready(struct uart *uart);

void uart_enable(struct uart *uart);

void uart_disable(struct uart *uart);

void uart_enable_tx(struct uart *uart);

void uart_disable_tx(struct uart *uart);

void uart_enable_rx(struct uart *uart);

void uart_disable_rx(struct uart *uart);

void uart_reset_tx(struct uart *uart);

void uart_reset_rx(struct uart *uart);

void uart_set_baud_gen(struct uart *uart, uint32_t baudgen);

void uart_set_baud_div(struct uart *uart, uint32_t bauddiv);

uint32_t uart_irq_status(struct uart *uart);

uint32_t uart_irq_mask(struct uart *uart);

void uart_irq_enable(struct uart *uart, uint32_t mask);

void uart_irq_disable(struct uart *uart, uint32_t mask);

void uart_irq_clear(struct uart *uart, uint32_t mask);

void uart_set_rx_timeout(struct uart *uart, uint32_t n);

void uart_set_rx_watermark(struct uart *uart, uint32_t n);

void uart_set_tx_watermark(struct uart *uart, uint32_t n);

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
