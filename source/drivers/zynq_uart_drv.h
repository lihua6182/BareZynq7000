#ifndef ZYNQ_UART_DRV_H
#define ZYNQ_UART_DRV_H

#include "zynq_soc_ip.h"

/*
 * Zynq-7000 UART driver, modeled after the Artery AT32 firmware library
 * conventions: usart_type-style pointer parameters, an init struct with a
 * default_para_init filler, flag/interrupt source masks and
 * flag_status/error_status returns.
 *
 * Hardware facts used here (UG585 ch. 19):
 *   baud rate = UART_REF_CLK / (CD x (BDIV + 1))            (Table 19-1)
 *   MR.PAR[5:3]  000 even, 001 odd, 010 forced-0 (space),
 *                011 forced-1 (mark), 1xx no parity
 *   MR.CHRL[7:6] 00 8-bit, 01 7-bit, 10 6-bit, 11 5-bit
 *   MR.NBSTOP[2:1] 00 1 stop, 01 1.5, 10 2 stop bits (11 reserved)
 *   MR.CLKS[0]   0 = UART_REF_CLK, 1 = UART_REF_CLK/8
 *   CR: [0] RXRES [1] TXRES [2] RXEN [3] RXDIS [4] TXEN [5] TXDIS
 *       [6] RSTTO [7] STTBRK [8] STPBRK - the reset bits are self-clearing
 *   SR/IER/IDR/ISR share one bit layout (Table 19-5).
 *
 * The caller must mux the UART TX/RX MIO pins to the UART function via the
 * SLCR MIO_PIN registers before init (board specific).
 */

/* status types (AT32 style) */
typedef enum { FAILED = 0,
               SUCCESS = !FAILED } error_status;
typedef enum { RESET = 0,
               SET = !RESET } flag_status;
typedef enum { DISABLE = 0,
               ENABLE = !DISABLE } confirm_state;

/* UART instances */
typedef UART_Type zynq_uart_type;
#define ZYNQ_UART0 UART0
#define ZYNQ_UART1 UART1

/*
 * The UART reference clock for the baud rate calculation is read back from
 * the SLCR at run time (zynq_clock_freqs_get().uart_ref): zynq_uart_init
 * programs UART_CLK_CTRL to IO PLL / 20 before computing the divisors.
 */

/* zynq_uart_init_type fields - pre-shifted MR bit values (UG585 ch. 19) */
#define ZYNQ_UART_DATA_8BITS (0U << 6)
#define ZYNQ_UART_DATA_7BITS (1U << 6)
#define ZYNQ_UART_DATA_6BITS (2U << 6)
#define ZYNQ_UART_DATA_5BITS (3U << 6)

#define ZYNQ_UART_PARITY_EVEN     (0U << 3)
#define ZYNQ_UART_PARITY_ODD      (1U << 3)
#define ZYNQ_UART_PARITY_FORCED_0 (2U << 3) /* space */
#define ZYNQ_UART_PARITY_FORCED_1 (3U << 3) /* mark  */
#define ZYNQ_UART_PARITY_NONE     (4U << 3) /* 1xx: no parity */

#define ZYNQ_UART_STOP_1_BIT    (0U << 1)
#define ZYNQ_UART_STOP_1_5_BITS (1U << 1)
#define ZYNQ_UART_STOP_2_BITS   (2U << 1)

#define ZYNQ_UART_MODE_TX    (1U << 0)
#define ZYNQ_UART_MODE_RX    (1U << 1)
#define ZYNQ_UART_MODE_TX_RX (ZYNQ_UART_MODE_TX | ZYNQ_UART_MODE_RX)

/* status flags and interrupt sources (SR / IER / IDR / ISR bit layout) */
#define ZYNQ_UART_FLAG_RTRIG   (1UL << 0)  /* Rx FIFO trigger reached      */
#define ZYNQ_UART_FLAG_RXEMPTY (1UL << 1)  /* Rx FIFO empty (raw)          */
#define ZYNQ_UART_FLAG_RXFULL  (1UL << 2)  /* Rx FIFO full (raw)           */
#define ZYNQ_UART_FLAG_TEMPTY  (1UL << 3)  /* Tx FIFO empty (raw)          */
#define ZYNQ_UART_FLAG_TXFULL  (1UL << 4)  /* Tx FIFO full (raw)           */
#define ZYNQ_UART_FLAG_ROVR    (1UL << 5)  /* Rx FIFO overflow             */
#define ZYNQ_UART_FLAG_FRAME   (1UL << 6)  /* framing error                */
#define ZYNQ_UART_FLAG_PARE    (1UL << 7)  /* parity error                 */
#define ZYNQ_UART_FLAG_TOUT    (1UL << 8)  /* Rx timeout                   */
#define ZYNQ_UART_FLAG_TACTIVE (1UL << 11) /* transmitter shifting a char */

/* Control register bits (UG585 ch. 19); the reset bits are self-clearing. */
#define ZYNQ_UART_CR_RXRES  (1UL << 0) /* Rx soft reset      */
#define ZYNQ_UART_CR_TXRES  (1UL << 1) /* Tx soft reset      */
#define ZYNQ_UART_CR_RXEN   (1UL << 2) /* enable Rx path     */
#define ZYNQ_UART_CR_RXDIS  (1UL << 3) /* disable Rx path    */
#define ZYNQ_UART_CR_TXEN   (1UL << 4) /* enable Tx path     */
#define ZYNQ_UART_CR_TXDIS  (1UL << 5) /* disable Tx path    */
#define ZYNQ_UART_CR_RSTTO  (1UL << 6) /* restart Rx timeout */
#define ZYNQ_UART_CR_STTBRK (1UL << 7) /* start break        */
#define ZYNQ_UART_CR_STPBRK (1UL << 8) /* stop break         */

/* init parameters */
typedef struct {
    uint32_t baudrate;         /* requested baud rate                          */
    uint32_t data_word_length; /* ZYNQ_UART_DATA_x_BITS                        */
    uint32_t parity_select;    /* ZYNQ_UART_PARITY_x                           */
    uint32_t stop_bits;        /* ZYNQ_UART_STOP_x                             */
    uint32_t mode;             /* ZYNQ_UART_MODE_x                             */
} zynq_uart_init_type;

void zynq_uart_reset(zynq_uart_type* uart_x);
void zynq_uart_default_para_init(zynq_uart_init_type* init_struct);
error_status zynq_uart_init(zynq_uart_type* uart_x, zynq_uart_init_type* init_struct);
flag_status zynq_uart_flag_get(zynq_uart_type* uart_x, uint32_t flag);
void zynq_uart_flag_clear(zynq_uart_type* uart_x, uint32_t flag);
void zynq_uart_interrupt_enable(zynq_uart_type* uart_x, uint32_t source, confirm_state new_state);
void zynq_uart_data_transmit(zynq_uart_type* uart_x, uint8_t data);
uint8_t zynq_uart_data_receive(zynq_uart_type* uart_x);
void zynq_uart_rx_fifo_level_config(zynq_uart_type* uart_x, uint8_t level);
void zynq_uart_rx_timeout_config(zynq_uart_type* uart_x, uint8_t timeout);
void zynq_uart_putc(zynq_uart_type* uart_x, uint8_t data);
uint8_t zynq_uart_getc(zynq_uart_type* uart_x);

#endif
