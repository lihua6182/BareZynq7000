#include "zynq_uart.h"

#include <stddef.h>

#include "zynq_clock.h"

#define UART0_CLK_GATE_BIT (1U << 20)
#define UART1_CLK_GATE_BIT (1U << 21)

void uart_send_byte(struct uart *uart, uint8_t c)
{
    uart->fifo = (uint32_t)c;
}

uint8_t uart_recv_byte(struct uart *uart)
{
    return (uint8_t)(uart->fifo & 0xFFU);
}

bool uart_tx_ready(struct uart *uart)
{
    return (uart->sr & UART_SR_TXFULL) == 0U;
}

bool uart_tx_done(struct uart *uart)
{
    return (uart->sr & UART_SR_TXEMPTY) != 0U;
}

bool uart_rx_ready(struct uart *uart)
{
    return (uart->sr & UART_SR_RXEMPTY) == 0U;
}

void uart_enable(struct uart *uart)
{
    uart->cr = (uart->cr & ~UART_CR_EN_DIS_Msk) | UART_CR_RX_EN | UART_CR_TX_EN;
}

void uart_disable(struct uart *uart)
{
    uart->cr = (uart->cr & ~UART_CR_EN_DIS_Msk) | UART_CR_RX_DIS | UART_CR_TX_DIS;
}

void uart_enable_tx(struct uart *uart)
{
    uart->cr = (uart->cr & ~UART_CR_EN_DIS_Msk) | UART_CR_TX_EN;
}

void uart_disable_tx(struct uart *uart)
{
    uart->cr = (uart->cr & ~UART_CR_EN_DIS_Msk) | UART_CR_TX_DIS;
}

void uart_enable_rx(struct uart *uart)
{
    uart->cr = (uart->cr & ~UART_CR_EN_DIS_Msk) | UART_CR_RX_EN;
}

void uart_disable_rx(struct uart *uart)
{
    uart->cr = (uart->cr & ~UART_CR_EN_DIS_Msk) | UART_CR_RX_DIS;
}

void uart_reset_tx(struct uart *uart)
{
    uart->cr |= UART_CR_TXRST;
}

void uart_reset_rx(struct uart *uart)
{
    uart->cr |= UART_CR_RXRST;
}

void uart_set_baud_gen(struct uart *uart, uint32_t baudgen)
{
    uart->baudgen = baudgen & UART_BAUDGEN_MASK_Msk;
}

void uart_set_baud_div(struct uart *uart, uint32_t bauddiv)
{
    uart->bauddiv = bauddiv & UART_BAUDDIV_MASK_Msk;
}

uint32_t uart_irq_status(struct uart *uart)
{
    return (uart->isr & UART_IXR_MASK_Msk);
}

uint32_t uart_irq_mask(struct uart *uart)
{
    return (uart->imr & UART_IXR_MASK_Msk);
}

void uart_irq_enable(struct uart *uart, uint32_t mask)
{
    uart->ier = (mask & UART_IXR_MASK_Msk);
}

void uart_irq_disable(struct uart *uart, uint32_t mask)
{
    uart->idr = (mask & UART_IXR_MASK_Msk);
}

void uart_irq_clear(struct uart *uart, uint32_t mask)
{
    uart->isr = (mask & UART_IXR_MASK_Msk);
}

void uart_set_rx_timeout(struct uart *uart, uint32_t n)
{
    uart->rxtout = (n & UART_RXTOUT_MASK_Msk);
}

void uart_set_rx_watermark(struct uart *uart, uint32_t n)
{
    uart->rxwm = (n & UART_RXWM_MASK_Msk);
}

void uart_set_tx_watermark(struct uart *uart, uint32_t n)
{
    uart->txwm = (n & UART_TXWM_MASK_Msk);
}

static int uart_find_baud_divisors(uint32_t ref_hz, uint32_t baud, uint32_t *baudgen, uint32_t *bauddiv)
{
    uint32_t best_err = 0xFFFFFFFFU;
    uint32_t div;
    uint32_t gen;
    uint32_t denominator;
    uint32_t actual;
    uint32_t err;

    for (div = 4U; div <= 255U; div++)
    {
        denominator = baud * (div + 1U);
        gen = (ref_hz + denominator / 2U) / denominator;
        if ((gen == 0U) || (gen > 0xFFFFU))
        {
            continue;
        }
        actual = ref_hz / (gen * (div + 1U));
        err = (actual > baud) ? (actual - baud) : (baud - actual);
        if (err < best_err)
        {
            best_err = err;
            *baudgen = gen;
            *bauddiv = div;
        }
    }
    return (best_err == 0xFFFFFFFFU) ? -1 : 0;
}

static uint32_t uart_clock_gate_bit(struct uart *uart)
{
    return (uart == uart1) ? UART1_CLK_GATE_BIT : UART0_CLK_GATE_BIT;
}

void uart_enable_clock(struct uart *uart)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->aper_clk_ctrl |= uart_clock_gate_bit(uart);
    slcr->lock = SLCR_LOCK_KEY;
}

void uart_disable_clock(struct uart *uart)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->aper_clk_ctrl &= ~uart_clock_gate_bit(uart);
    slcr->lock = SLCR_LOCK_KEY;
}

void uart_set_databits(struct uart *uart, uint32_t bits)
{
    uint32_t mr = uart->mr & ~UART_MR_CHARLEN_Msk;

    if (bits == UART_DATABITS_6)
    {
        mr |= UART_MR_CHARLEN_6_BIT;
    }
    else if (bits == UART_DATABITS_7)
    {
        mr |= UART_MR_CHARLEN_7_BIT;
    }
    else
    {
        mr |= UART_MR_CHARLEN_8_BIT;
    }
    uart->mr = mr;
}

void uart_set_parity(struct uart *uart, uint32_t parity)
{
    uint32_t mr = uart->mr & ~UART_MR_PARITY_Msk;

    if (parity == UART_PARITY_ODD)
    {
        mr |= UART_MR_PARITY_ODD;
    }
    else if (parity == UART_PARITY_EVEN)
    {
        mr |= UART_MR_PARITY_EVEN;
    }
    else
    {
        mr |= UART_MR_PARITY_NONE;
    }
    uart->mr = mr;
}

void uart_set_stopbits(struct uart *uart, uint32_t bits)
{
    uint32_t mr = uart->mr & ~UART_MR_STOPMODE_Msk;

    if (bits == UART_STOPBITS_2)
    {
        mr |= UART_MR_STOPMODE_2_BIT;
    }
    else
    {
        mr |= UART_MR_STOPMODE_1_BIT;
    }
    uart->mr = mr;
}

int uart_set_baud(struct uart *uart, uint32_t baud)
{
    struct clock_freqs clocks;
    uint32_t baudgen;
    uint32_t bauddiv;

    if (baud == 0U)
    {
        return -1;
    }
    clock_freqs_get(&clocks);
    if (uart_find_baud_divisors(clocks.uart_ref, baud, &baudgen, &bauddiv) != 0)
    {
        return -1;
    }
    uart_set_baud_gen(uart, baudgen);
    uart_set_baud_div(uart, bauddiv);
    return 0;
}

int uart_init(struct uart *uart, uint32_t baud)
{
    uart_enable_clock(uart);

    uart->cr = UART_CR_RXRST | UART_CR_TXRST;

    uart_set_databits(uart, UART_DATABITS_8);
    uart_set_parity(uart, UART_PARITY_NONE);
    uart_set_stopbits(uart, UART_STOPBITS_1);

    if (uart_set_baud(uart, baud) != 0)
    {
        return -1;
    }

    uart_irq_disable(uart, UART_IXR_MASK_Msk);
    uart_irq_clear(uart, UART_IXR_MASK_Msk);
    uart->cr = UART_CR_RX_EN | UART_CR_TX_EN;
    return 0;
}

void uart_putc(struct uart *uart, int c)
{
    while (!uart_tx_ready(uart))
    {
    }
    uart_send_byte(uart, (uint8_t)c);
}

int uart_getc(struct uart *uart)
{
    while (!uart_rx_ready(uart))
    {
    }
    return (int)uart_recv_byte(uart);
}

int uart_try_getc(struct uart *uart)
{
    if (!uart_rx_ready(uart))
    {
        return -1;
    }
    return (int)uart_recv_byte(uart);
}

void uart_puts(struct uart *uart, const char *s)
{
    if (s == NULL)
    {
        return;
    }
    while (*s != '\0')
    {
        uart_putc(uart, (int)*s);
        s++;
    }
}
