/**
 * @file zynq_spi.c
 * @brief SPI 驱动: 需要多次寄存器访问的部分 (时钟门控、复位、SCLK
 *        搜索、阻塞传输)。
 */

#include "zynq_spi.h"

#include <stddef.h>

#include "zynq_clock.h"

/* APER_CLK_CTRL 的 CPU_1x 时钟门: bit 14 SPI0, bit 15 SPI1。 */
#define SPI0_CLK_GATE_BIT (1u << 14)
#define SPI1_CLK_GATE_BIT (1u << 15)

/* SPI_CLK_CTRL (SPI0/1 共享) 固定 IO PLL/5 = 200MHz: ch.17 要求参考
 * 时钟高于 CPU_1x (111MHz), 且 /4../256 的 2 的幂阶梯给出
 * 50MHz..781kHz。DIVISOR 在 [13:8], SRCSEL=0 (IO PLL) 在 [5:4]。 */
#define SPI_CLK_DIVISOR_5 (5u << 8)

/* 单次 FIFO 等待的轮询上限 (强序总线周期): 最慢档下一整块 128 字节
 * 约 1.3ms, 余量充足。 */
#define SPI_POLL_LIMIT (1000000u)

/* APER 时钟门位 (按控制器)。 */
static uint32_t gate_bit(SPI_Type *spi)
{
    return (spi == SPI1) ? SPI1_CLK_GATE_BIT : SPI0_CLK_GATE_BIT;
}

/* SLCR SPI_RST_CTRL 复位位: SPI0 = REF(2)|CPU1x(0), SPI1 = REF(3)|CPU1x(1)。 */
static uint32_t reset_bits(SPI_Type *spi)
{
    return (spi == SPI1) ? ((1u << 3) | (1u << 1)) : ((1u << 2) | (1u << 0));
}

/**
 * @brief  打开参考时钟: APER 门 + SPI_CLK_CTRL 选择 IO PLL/5。
 * @param  spi SPI0 或 SPI1。
 * @note   SPI_CLK_CTRL 两控制器共享, 每次调用都把 DIVISOR/SRCSEL 重写
 *         为 IO PLL/5 = 200MHz (复位值 /63 低于 CPU_1x, 违反 ch.17),
 *         只保留另一个控制器的 CLKACT。
 */
void spi_enable_clock(SPI_Type *spi)
{
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->APER_CLK_CTRL |= gate_bit(spi);
    SLCR->SPI_CLK_CTRL = (SLCR->SPI_CLK_CTRL & 0x3u) | SPI_CLK_DIVISOR_5 | ((spi == SPI1) ? 2u : 1u);
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  关掉本控制器的参考时钟门。
 * @param  spi SPI0 或 SPI1。
 */
void spi_disable_clock(SPI_Type *spi)
{
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->APER_CLK_CTRL &= ~gate_bit(spi);
    SLCR->SPI_CLK_CTRL &= ~((spi == SPI1) ? 2u : 1u);
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  复位控制器: REF + CPU1x 两个复位域一起脉冲, 清空两个 FIFO。
 * @param  spi SPI0 或 SPI1。
 */
void spi_reset(SPI_Type *spi)
{
    uint32_t bits = reset_bits(spi);
    uint32_t i;

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->SPI_RST_CTRL |= bits;
    for (i = 0u; i < 100u; i++)
    {
        __asm volatile("nop");
    }
    SLCR->SPI_RST_CTRL &= ~bits;
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  选一档不超过 sclk_hz 的 SCLK 并写入 BAUD_RATE_DIV。
 * @param  spi     SPI0 或 SPI1。
 * @param  sclk_hz 目标 SCLK, Hz。
 * @return 0; -1 = sclk_hz 为 0、参考时钟未开或低于参考时钟/256。
 * @note   BAUD_RATE_DIV=0 不是合法档位 (ch.17), 因此最低只能到 ref/256。
 */
int spi_set_sclk(SPI_Type *spi, uint32_t sclk_hz)
{
    struct clock_freqs clocks;
    uint32_t baud_div;

    if (sclk_hz == 0u)
    {
        return -1;
    }
    clock_freqs_get(&clocks);
    if (clocks.spi_ref == 0u)
    {
        return -1; /* 参考时钟关着, 先 spi_enable_clock() */
    }
    /* 档位率随 baud_div 单调下降, 第一个不超目标的即最高一档。 */
    for (baud_div = 1u; baud_div <= SPI_CR_PRESC_MAXIMUM; baud_div++)
    {
        if ((clocks.spi_ref >> (baud_div + 1u)) <= sclk_hz)
        {
            spi_set_baud_div(spi, baud_div);
            return 0;
        }
    }
    return -1;
}

/**
 * @brief  初始化控制器: 复位, 主机模式, 手动片选 (全释放), 水位 1,
 *         中断全屏蔽, 使能。
 * @param  spi     SPI0 或 SPI1。
 * @param  mode    SPI_MODE_0..SPI_MODE_3 (CPOL/CPHA)。
 * @param  sclk_hz 目标 SCLK, Hz。
 * @return 0; -1 = sclk_hz 不可达。
 * @note   自动启动 (Man_start_en=0) + 手动片选: 写入 TX FIFO 即移位,
 *         片选状态由 spi_select()/spi_deselect() 决定。单主机系统关闭
 *         总线竞争检测 (Modefail_gen_en=0), 片选走 MIO 直连 (SSDECEN=0)。
 */
int spi_init(SPI_Type *spi, uint32_t mode, uint32_t sclk_hz)
{
    uint32_t cr;

    spi_enable_clock(spi);
    spi_reset(spi);

    cr = SPI_CR_MSTREN_Msk | SPI_CR_SSFORCE_Msk | SPI_CR_SSCTRL_Msk;
    if ((mode & 1u) != 0u)
    {
        cr |= SPI_CR_CPHA_Msk;
    }
    if ((mode & 2u) != 0u)
    {
        cr |= SPI_CR_CPOL_Msk;
    }
    spi->CR = cr;

    if (spi_set_sclk(spi, sclk_hz) != 0)
    {
        return -1;
    }

    spi_set_tx_watermark(spi, 1u);
    spi_set_rx_watermark(spi, 1u);
    spi_irq_disable(spi, SPI_IXR_MASK_Msk);
    spi_irq_clear(spi, SPI_IXR_TXUF_Msk | SPI_IXR_MODF_Msk | SPI_IXR_RXOVR_Msk);
    spi_enable(spi);
    return 0;
}

/**
 * @brief  全双工阻塞传输: 整块 (≤128B) 填 TX FIFO, 等 FIFO 排空, 再
 *         整块取回; 超出 128B 自动分块, 片选不松、时钟不换。
 * @param  spi SPI0 或 SPI1。
 * @param  tx  发送缓冲, NULL = 发 SPI_DUMMY_TX。
 * @param  rx  接收缓冲, NULL = 丢弃。
 * @param  len 字节数, 0 = 空操作。
 * @return 0; -1 = FIFO 等待超限 (控制器未使能 / 参考时钟关)。
 * @note   一块的收发字节数相同: RX FIFO 恰好装得下一块, 不会溢出。
 */
int spi_transfer(SPI_Type *spi, const uint8_t *tx, uint8_t *rx, uint32_t len)
{
    uint32_t chunk;
    uint32_t i;
    uint32_t polls;

    while (len > 0u)
    {
        chunk = (len < SPI_FIFO_DEPTH) ? len : SPI_FIFO_DEPTH;

        /* 上一块已移出 FIFO 才整块续填, 满水位写入才安全。 */
        polls = SPI_POLL_LIMIT;
        while (spi_tx_empty(spi) == 0)
        {
            if (--polls == 0u)
            {
                return -1;
            }
        }
        for (i = 0u; i < chunk; i++)
        {
            spi_send_byte(spi, (tx != NULL) ? tx[i] : (uint8_t)SPI_DUMMY_TX);
        }
        for (i = 0u; i < chunk; i++)
        {
            polls = SPI_POLL_LIMIT;
            while (spi_rx_ready(spi) == 0)
            {
                if (--polls == 0u)
                {
                    return -1;
                }
            }
            if (rx != NULL)
            {
                rx[i] = spi_recv_byte(spi);
            }
            else
            {
                (void)spi_recv_byte(spi);
            }
        }
        if (tx != NULL)
        {
            tx += chunk;
        }
        if (rx != NULL)
        {
            rx += chunk;
        }
        len -= chunk;
    }
    return 0;
}
