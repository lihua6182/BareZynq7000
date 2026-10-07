/**
 * @file zynq_uart.c
 * @brief UART driver: the parts that need more than one register access
 *        (blocking I/O, format, baud search, clock gating).
 */

#include "zynq_uart.h"

#include <stddef.h>

#include "zynq_clock.h"

/* APER_CLK_CTRL ref clock gates: bit 20 UART0, bit 21 UART1. */
#define UART0_CLK_GATE_BIT (1UL << 20)
#define UART1_CLK_GATE_BIT (1UL << 21)

/**
 * @brief  Find the BAUDGEN/BAUDDIV pair closest to a baud rate.
 * @param  ref_hz  UART reference clock in Hz.
 * @param  baud    Requested baud rate.
 * @param  baudgen Out: BAUDGEN divisor, 1..65535.
 * @param  bauddiv Out: BAUDDIV divisor, 4..255 (below 4 is invalid).
 * @return 0 when a pair exists, -1 when nothing comes close.
 */
static int find_baud_divisors(uint32_t ref_hz, uint32_t baud, uint32_t *baudgen, uint32_t *bauddiv)
{
    uint32_t best_err = 0xFFFFFFFFUL;
    uint32_t div;
    uint32_t gen;
    uint32_t denominator;
    uint32_t actual;
    uint32_t err;

    for (div = 4U; div <= 255U; div++)
    {
        denominator = baud * (div + 1U);
        gen = (ref_hz + denominator / 2U) / denominator;
        if ((gen == 0U) || (gen > 0xFFFFUL))
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
    return (best_err == 0xFFFFFFFFUL) ? -1 : 0;
}

/**
 * @brief  Get the APER_CLK_CTRL gate bit of a controller.
 * @param  uart UART0 or UART1.
 * @return The clock-gate bit for that controller.
 */
static uint32_t uart_clock_gate_bit(UART_Type *uart)
{
    return (uart == UART1) ? UART1_CLK_GATE_BIT : UART0_CLK_GATE_BIT;
}

/**
 * @brief  Ungate a controller's reference clock.
 * @param  uart UART0 or UART1.
 */
void uart_enable_clock(UART_Type *uart)
{
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->APER_CLK_CTRL |= uart_clock_gate_bit(uart);
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  Gate a controller's reference clock.
 * @param  uart UART0 or UART1.
 */
void uart_disable_clock(UART_Type *uart)
{
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->APER_CLK_CTRL &= ~uart_clock_gate_bit(uart);
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  Select the character length.
 * @param  uart UART0 or UART1.
 * @param  bits UART_DATABITS_6, _7 or _8; anything else means 8 bits.
 */
void uart_set_databits(UART_Type *uart, uint32_t bits)
{
    uint32_t mr = uart->MR & ~UART_MR_CHARLEN_Msk;

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
    uart->MR = mr;
}

/**
 * @brief  Select the parity mode.
 * @param  uart   UART0 or UART1.
 * @param  parity UART_PARITY_ODD, _EVEN or _NONE; anything else means none.
 */
void uart_set_parity(UART_Type *uart, uint32_t parity)
{
    uint32_t mr = uart->MR & ~UART_MR_PARITY_Msk;

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
    uart->MR = mr;
}

/**
 * @brief  Select the number of stop bits.
 * @param  uart UART0 or UART1.
 * @param  bits UART_STOPBITS_2, or anything else for one stop bit.
 */
void uart_set_stopbits(UART_Type *uart, uint32_t bits)
{
    uint32_t mr = uart->MR & ~UART_MR_STOPMODE_Msk;

    if (bits == UART_STOPBITS_2)
    {
        mr |= UART_MR_STOPMODE_2_BIT;
    }
    else
    {
        mr |= UART_MR_STOPMODE_1_BIT;
    }
    uart->MR = mr;
}

/**
 * @brief  Program the baud rate from the live reference clock; only the
 *         two divisor registers change, safe while running.
 * @param  uart UART0 or UART1.
 * @param  baud Requested baud rate.
 * @return 0 on success, -1 when baud is 0 or unreachable.
 */
int uart_set_baud(UART_Type *uart, uint32_t baud)
{
    struct clock_freqs clocks;
    uint32_t baudgen;
    uint32_t bauddiv;

    if (baud == 0U)
    {
        return -1;
    }
    clock_freqs_get(&clocks);
    if (find_baud_divisors(clocks.uart_ref, baud, &baudgen, &bauddiv) != 0)
    {
        return -1;
    }
    uart_set_baud_gen(uart, baudgen);
    uart_set_baud_div(uart, bauddiv);
    return 0;
}

/**
 * @brief  Initialize a controller for 8N1 polled I/O: ref clock on, both
 *         paths reset, format + divisors programmed, interrupts masked.
 * @param  uart UART0 or UART1.
 * @param  baud Requested baud rate.
 * @return 0 on success, -1 when the baud rate is unreachable.
 */
int uart_init(UART_Type *uart, uint32_t baud)
{
    uart_enable_clock(uart);

    /* Both resets in one write; the port stays quiet while the format
       and the divisors are programmed. */
    uart->CR = UART_CR_RXRST | UART_CR_TXRST;

    uart_set_databits(uart, UART_DATABITS_8);
    uart_set_parity(uart, UART_PARITY_NONE);
    uart_set_stopbits(uart, UART_STOPBITS_1);

    if (uart_set_baud(uart, baud) != 0)
    {
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
void uart_putc(UART_Type *uart, int c)
{
    while (uart_tx_ready(uart) == 0)
    {
    }
    uart_send_byte(uart, (uint8_t)c);
}

/**
 * @brief  Receive one character, blocking until one arrives.
 * @param  uart UART0 or UART1.
 * @return The received byte, 0..255.
 */
int uart_getc(UART_Type *uart)
{
    while (uart_rx_ready(uart) == 0)
    {
    }
    return (int)uart_recv_byte(uart);
}

/**
 * @brief  Receive one character without blocking.
 * @param  uart UART0 or UART1.
 * @return The received byte 0..255, or -1 when the FIFO is empty.
 */
int uart_try_getc(UART_Type *uart)
{
    if (uart_rx_ready(uart) == 0)
    {
        return -1;
    }
    return (int)uart_recv_byte(uart);
}

/**
 * @brief  Send a NUL-terminated string.
 * @param  uart UART0 or UART1.
 * @param  s    String to send; NULL is ignored.
 */
void uart_puts(UART_Type *uart, const char *s)
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
