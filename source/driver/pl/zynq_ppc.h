#ifndef ZYNQ_PPC_H
#define ZYNQ_PPC_H

/*
 * Driver for the proto_coproc protocol co-processor (a PL AXI4-Lite IP,
 * not a Zynq PS peripheral; lives in driver/pl/). Sources at
 * C:\WorkSpace\SIC1 - see its README.md for the ISA. The IP
 * executes a 32-bit program out of an on-chip BRAM and shifts SPI/SWD/
 * JTAG/QSPI style bit streams on the PL gpio_o/gpio_oe pins with cycle
 * exact timing; data moves through a TX FIFO (pushed here) and an RX
 * FIFO (popped here).
 *
 *   coproc_cmd(COPROC, COPROC_CMD_RESET | COPROC_CMD_FLUSH_ALL);
 *   coproc_load(COPROC, prog, PROG_WORDS);
 *   COPROC->TXDATA = 0xA53C81FF;
 *   coproc_cmd(COPROC, COPROC_CMD_RUN);
 *   coproc_wait_halt(COPROC, 100);
 *   word = coproc_rx_pop(COPROC);
 *
 * Mio/PL pin routing is the PL design's job (gpio_o/gpio_oe external
 * ports); SCK=bit0, MOSI=bit1, MISO=bit2, CS=bit3 in the shipped SPI
 * program.
 */

#include "zynq_common.h"

/* CTRL write command bits (self clearing except RUN). */
#define COPROC_CMD_RUN       (1U << 0)
#define COPROC_CMD_RESET     (1U << 1)
#define COPROC_CMD_FLUSH_TX  (1U << 2)
#define COPROC_CMD_FLUSH_RX  (1U << 3)
#define COPROC_CMD_FLUSH_ALL (COPROC_CMD_RESET | COPROC_CMD_FLUSH_TX | COPROC_CMD_FLUSH_RX)

/* STATUS bits. */
#define COPROC_STAT_RUNNING   (1U << 0)
#define COPROC_STAT_HALTED    (1U << 1)
#define COPROC_STAT_STALL_TX  (1U << 2)
#define COPROC_STAT_STALL_RX  (1U << 3)
#define COPROC_STAT_STALL_DLY (1U << 4)
#define COPROC_STAT_STALL_PIN (1U << 5)
#define COPROC_STAT_STALL_SHF (1U << 6)
#define COPROC_STAT_TX_EMPTY  (1U << 7)
#define COPROC_STAT_TX_FULL   (1U << 8)
#define COPROC_STAT_RX_EMPTY  (1U << 9)
#define COPROC_STAT_RX_FULL   (1U << 10)

/* FSTAT packing: [3:0] flags, [11:4] tx count, [19:12] rx count. */
#define COPROC_FSTAT_TXCNT(v) (((v) >> 4) & 0xFFU)
#define COPROC_FSTAT_RXCNT(v) (((v) >> 12) & 0xFFU)

typedef struct
{                             /* proto_coproc register window (64K) */
    __IO uint32_t PROG[1024]; /* 0x0000 program memory, 1024 x 32 bit */
    __O uint32_t TXDATA;      /* 0x1000 push into the TX FIFO */
    __I uint32_t RXDATA;      /* 0x1004 pop from the RX FIFO */
    __I uint32_t FSTAT;       /* 0x1008 fifo flags + fill counts */
    __IO uint32_t FCTRL;      /* 0x100C irq enables + thresholds */
    __IO uint32_t CTRL;       /* 0x1010 RUN / RESET / FLUSH pulses */
    __I uint32_t STATUS;      /* 0x1014 run/halt/stall/fifo flags */
    __IO uint32_t PC;         /* 0x1018 read current / write new PC */
    __I uint32_t CYCLE;       /* 0x101C cycles counted while running */
    __I uint32_t IR;          /* 0x1020 current instruction (debug) */
    __I uint32_t REG[32];     /* 0x1024 register file window (debug) */
    uint32_t RESERVED0[23];   /* 0x10A4-0x10FF */
    __I uint32_t GPIO_IN;     /* 0x1100 synchronised pin inputs */
    __IO uint32_t GPIO_OUT;   /* 0x1104 output data register */
    __IO uint32_t GPIO_DIR;   /* 0x1108 direction, 1 = output */
    __I uint32_t IRQSTAT;     /* 0x110C enabled irq levels */
} COPROC_Type;

/* ------------------------------------------------------------------ */
/* Data                                                               */
/* ------------------------------------------------------------------ */

/* Push one word into the TX FIFO; check coproc_tx_ready() first. */
static inline void coproc_tx_push(COPROC_Type *c, uint32_t w)
{
    c->TXDATA = w;
}

/* Pop one word from the RX FIFO; check coproc_rx_ready() first. */
static inline uint32_t coproc_rx_pop(COPROC_Type *c)
{
    return c->RXDATA;
}

static inline bool coproc_tx_ready(COPROC_Type *c)
{
    return (c->STATUS & COPROC_STAT_TX_FULL) == 0U;
}

static inline bool coproc_rx_ready(COPROC_Type *c)
{
    return (c->STATUS & COPROC_STAT_RX_EMPTY) == 0U;
}

/* ------------------------------------------------------------------ */
/* Control                                                            */
/* ------------------------------------------------------------------ */

/* Issue a CTRL command (RUN bit persists, RESET/FLUSH self clear). */
static inline void coproc_cmd(COPROC_Type *c, uint32_t cmd)
{
    c->CTRL = cmd;
}

static inline bool coproc_is_halted(COPROC_Type *c)
{
    return (c->STATUS & COPROC_STAT_HALTED) != 0U;
}

/* Blocking helpers (timeouts on the 1 ms tick) live in zynq_ppc.c. */
bool coproc_wait_halt(COPROC_Type *c, uint32_t timeout_ms);
bool coproc_tx_push_wait(COPROC_Type *c, uint32_t w, uint32_t timeout_ms);

/* Load a program image (word array) into the program BRAM. */
static inline void coproc_load(COPROC_Type *c, const uint32_t *img, uint32_t words)
{
    uint32_t i;
    for (i = 0U; i < words; i++)
    {
        c->PROG[i] = img[i];
    }
}

#endif /* ZYNQ_PPC_H */
