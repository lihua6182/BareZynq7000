#ifndef BOARD_PL_H
#define BOARD_PL_H

#include <stdint.h>

#include "zynq_soc.h"

enum pl_status
{
    PL_OK = 0,
    PL_ERR_SRC_ALIGN,
    PL_ERR_LENGTH,
    PL_ERR_NO_SYNC,
    PL_ERR_HDR,
    PL_ERR_STREAM,
    PL_ERR_CRC,
    PL_ERR_NO_POWER,
    PL_ERR_NOT_RESET,
    PL_ERR_NOT_READY,
    PL_ERR_CMD_Q_FULL,
    PL_ERR_DMA,
    PL_ERR_DMA_TIMEOUT,
    PL_ERR_NO_DONE,
    PL_UNLOADED,
    PL_DISABLED,
};

#define PL_RESET_LINE_COUNT (4U)

int pl_reset_assert(uint32_t line);

int pl_reset_release(uint32_t line);

int pl_reset_is_asserted(uint32_t line);

uint32_t pl_reset_state(void);

void pl_reset_set(uint32_t mask);

void pl_reset_assert_all(void);

void pl_reset_release_all(void);

int pl_reset_pulse(uint32_t line);

enum pl_status pl_load(const void *img, uint32_t len);

#endif
