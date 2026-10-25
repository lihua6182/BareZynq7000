#include "zynq_pcap.h"

#define PCAP_CLK_CTRL_CLKACT (0x1U)

#define PCAP_INIT_POLL_LIMIT     (8000000U)
#define PCAP_DMA_DONE_POLL_LIMIT (60000000U)

#define PCAP_DMA_ERROR_MASK                                                                                            \
    (DEVCFG_IXR_DMA_CMD_ERR_Msk | DEVCFG_IXR_DMA_Q_OV_Msk | DEVCFG_IXR_P2D_LEN_ERR_Msk | DEVCFG_IXR_RX_FIFO_OV_Msk |   \
     DEVCFG_IXR_AXI_WERR_Msk | DEVCFG_IXR_AXI_WTO_Msk | DEVCFG_IXR_AXI_RERR_Msk | DEVCFG_IXR_AXI_RTO_Msk |             \
     DEVCFG_IXR_PCFG_HMAC_ERR_Msk | DEVCFG_IXR_PCFG_SEU_ERR_Msk)

void pcap_lvl_set(uint32_t value)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->lvl_shftr_en = value;
    slcr->lock = SLCR_LOCK_KEY;
}

uint32_t pcap_lvl_get(void)
{
    return slcr->lvl_shftr_en;
}

void pcap_lvl_off(void)
{
    pcap_lvl_set(PCAP_LVL_OFF);
}

void pcap_lvl_ps_to_pl(void)
{
    pcap_lvl_set(PCAP_LVL_PS_TO_PL);
}

void pcap_lvl_all(void)
{
    pcap_lvl_set(PCAP_LVL_ALL);
}

bool pcap_is_powered(void)
{
    return ((devcfg->mctrl & DEVCFG_MCTRL_PCFG_POR_B_Msk) != 0U);
}

bool pcap_is_ready(void)
{
    return ((devcfg->status & DEVCFG_STATUS_PCFG_INIT_Msk) != 0U);
}

bool pcap_is_done(void)
{
    return ((devcfg->int_sts & DEVCFG_IXR_PCFG_DONE_Msk) != 0U);
}

static enum pcap_status pcap_wait_init(bool ready)
{
    uint32_t i;

    for (i = 0U; i < PCAP_INIT_POLL_LIMIT; i++)
    {
        if (pcap_is_ready() == ready)
        {
            return PCAP_OK;
        }
    }
    return ready ? PCAP_ERR_NOT_READY : PCAP_ERR_NOT_RESET;
}

static void pcap_ack_dma_done(void)
{
    devcfg->int_sts = DEVCFG_IXR_DMA_DONE_Msk;
    devcfg->status |= DEVCFG_STATUS_DMA_DONE_CNT_Msk;
}

void pcap_open(void)
{
    uint32_t ctrl;

    pcap_lvl_ps_to_pl();

    devcfg->unlock = DEVCFG_UNLOCK_DATA;

    devcfg->int_sts = 0xFFFFFFFFU;

    ctrl = devcfg->ctrl;
    ctrl |= DEVCFG_CTRL_PCAP_MODE_Msk | DEVCFG_CTRL_PCAP_PR_Msk;
    ctrl &= ~(DEVCFG_CTRL_PCAP_RATE_EN_Msk | DEVCFG_CTRL_PCFG_POR_CNT_4K_Msk);
    devcfg->ctrl = ctrl;
}

static enum pcap_status pcap_config_clear(void)
{
    uint32_t ctrl;
    enum pcap_status st;

    if (pcap_is_powered() == false)
    {
        return PCAP_ERR_NO_POWER;
    }

    ctrl = devcfg->ctrl;
    devcfg->ctrl = ctrl | DEVCFG_CTRL_PCFG_PROG_B_Msk;
    devcfg->ctrl = ctrl & ~DEVCFG_CTRL_PCFG_PROG_B_Msk;

    st = pcap_wait_init(false);
    if (st != PCAP_OK)
    {
        return st;
    }

    devcfg->ctrl = ctrl | DEVCFG_CTRL_PCFG_PROG_B_Msk;
    devcfg->int_sts = DEVCFG_IXR_PCFG_DONE_Msk;
    return pcap_wait_init(true);
}

enum pcap_status pcap_start(void)
{
    enum pcap_status st;

    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->pcap_clk_ctrl |= PCAP_CLK_CTRL_CLKACT;
    slcr->lock = SLCR_LOCK_KEY;

    st = pcap_config_clear();
    if (st != PCAP_OK)
    {
        return st;
    }

    if ((devcfg->status & DEVCFG_STATUS_DMA_CMD_Q_F_Msk) != 0U)
    {
        return PCAP_ERR_CMD_Q_FULL;
    }
    devcfg->mctrl &= ~DEVCFG_MCTRL_PCAP_LPBK_Msk;

    pcap_ack_dma_done();

    return PCAP_OK;
}

enum pcap_status pcap_send_block(const void *src, uint32_t words)
{
    uint32_t i;
    uint32_t int_sts;

    devcfg->dma_src_addr = (uint32_t)(uintptr_t)src;
    devcfg->dma_dest_addr = DEVCFG_DMA_INVALID_ADDRESS;
    devcfg->dma_src_len = words;
    dsb();
    devcfg->dma_dest_len = words;

    for (i = 0U; i < PCAP_DMA_DONE_POLL_LIMIT; i++)
    {
        int_sts = devcfg->int_sts;
        if ((int_sts & PCAP_DMA_ERROR_MASK) != 0U)
        {
            devcfg->int_sts = int_sts;
            return PCAP_ERR_DMA;
        }
        if ((int_sts & DEVCFG_IXR_DMA_DONE_Msk) != 0U)
        {
            break;
        }
    }
    if (i == PCAP_DMA_DONE_POLL_LIMIT)
    {
        return PCAP_ERR_DMA_TIMEOUT;
    }

    pcap_ack_dma_done();
    return PCAP_OK;
}

enum pcap_status pcap_wait_pl_done(void)
{
    uint32_t i;

    for (i = 0U; i < PCAP_DMA_DONE_POLL_LIMIT; i++)
    {
        if ((devcfg->int_sts & DEVCFG_IXR_PCFG_DONE_Msk) != 0U)
        {
            break;
        }
    }
    devcfg->int_sts = DEVCFG_IXR_PCFG_DONE_Msk;
    if (i == PCAP_DMA_DONE_POLL_LIMIT)
    {
        return PCAP_ERR_NO_DONE;
    }

    pcap_lvl_all();

    return PCAP_OK;
}
