#ifndef BOARD_PL_H
#define BOARD_PL_H

#include <stdbool.h>
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

#define PL_LVL_OFF      (0x0U)
#define PL_LVL_PS_TO_PL (0xAU)
#define PL_LVL_ALL      (0xFU)

static inline int pl_reset_assert(uint32_t line)
{
    if (line >= PL_RESET_LINE_COUNT)
    {
        return -1;
    }
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->fpga_rst_ctrl |= (1UL << line);
    slcr->lock = SLCR_LOCK_KEY;
    return 0;
}

static inline int pl_reset_release(uint32_t line)
{
    if (line >= PL_RESET_LINE_COUNT)
    {
        return -1;
    }
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->fpga_rst_ctrl &= ~(1UL << line);
    slcr->lock = SLCR_LOCK_KEY;
    return 0;
}

static inline int pl_reset_is_asserted(uint32_t line)
{
    if (line >= PL_RESET_LINE_COUNT)
    {
        return -1;
    }
    return (int)((slcr->fpga_rst_ctrl >> line) & 1UL);
}

static inline uint32_t pl_reset_state(void)
{
    return slcr->fpga_rst_ctrl & 0xFUL;
}

static inline void pl_reset_set(uint32_t mask)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->fpga_rst_ctrl = mask & 0xFUL;
    slcr->lock = SLCR_LOCK_KEY;
}

static inline void pl_reset_assert_all(void)
{
    pl_reset_set(0xFUL);
}

static inline void pl_reset_release_all(void)
{
    pl_reset_set(0x0UL);
}

int pl_reset_pulse(uint32_t line);

static inline void pl_lvl_set(uint32_t value)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->lvl_shftr_en = value;
    slcr->lock = SLCR_LOCK_KEY;
}

static inline uint32_t pl_lvl_get(void)
{
    return slcr->lvl_shftr_en;
}

static inline void pl_lvl_off(void)
{
    pl_lvl_set(PL_LVL_OFF);
}

static inline void pl_lvl_ps_to_pl(void)
{
    pl_lvl_set(PL_LVL_PS_TO_PL);
}

static inline void pl_lvl_all(void)
{
    pl_lvl_set(PL_LVL_ALL);
}

static inline bool pl_is_powered(void)
{
    return ((devcfg->mctrl & DEVCFG_MCTRL_PCFG_POR_B_Msk) != 0U);
}

static inline bool pl_is_ready(void)
{
    return ((devcfg->status & DEVCFG_STATUS_PCFG_INIT_Msk) != 0U);
}

static inline bool pl_is_done(void)
{
    return ((devcfg->int_sts & DEVCFG_IXR_PCFG_DONE_Msk) != 0U);
}

enum pl_status pl_config_clear(void);

enum pl_status pl_load(const void *img, uint32_t len);

#endif
