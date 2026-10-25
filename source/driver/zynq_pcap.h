#ifndef ZYNQ_PCAP_H
#define ZYNQ_PCAP_H

#include <stdbool.h>
#include <stdint.h>

#include "zynq_soc.h"

enum pcap_status
{
    PCAP_OK = 0,
    PCAP_ERR_NO_POWER,
    PCAP_ERR_NOT_RESET,
    PCAP_ERR_NOT_READY,
    PCAP_ERR_CMD_Q_FULL,
    PCAP_ERR_DMA,
    PCAP_ERR_DMA_TIMEOUT,
    PCAP_ERR_NO_DONE,
};

#define PCAP_LVL_OFF      (0x0U)
#define PCAP_LVL_PS_TO_PL (0xAU)
#define PCAP_LVL_ALL      (0xFU)

static inline void pcap_lvl_set(uint32_t value)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->lvl_shftr_en = value;
    slcr->lock = SLCR_LOCK_KEY;
}

static inline uint32_t pcap_lvl_get(void)
{
    return slcr->lvl_shftr_en;
}

static inline void pcap_lvl_off(void)
{
    pcap_lvl_set(PCAP_LVL_OFF);
}

static inline void pcap_lvl_ps_to_pl(void)
{
    pcap_lvl_set(PCAP_LVL_PS_TO_PL);
}

static inline void pcap_lvl_all(void)
{
    pcap_lvl_set(PCAP_LVL_ALL);
}

static inline bool pcap_is_powered(void)
{
    return ((devcfg->mctrl & DEVCFG_MCTRL_PCFG_POR_B_Msk) != 0U);
}

static inline bool pcap_is_ready(void)
{
    return ((devcfg->status & DEVCFG_STATUS_PCFG_INIT_Msk) != 0U);
}

static inline bool pcap_is_done(void)
{
    return ((devcfg->int_sts & DEVCFG_IXR_PCFG_DONE_Msk) != 0U);
}

void pcap_open(void);

enum pcap_status pcap_start(void);

enum pcap_status pcap_send_block(const void *src, uint32_t words);

enum pcap_status pcap_wait_pl_done(void);

#endif
