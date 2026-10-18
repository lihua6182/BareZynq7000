#ifndef ZYNQ_SPI_H
#define ZYNQ_SPI_H

#include "zynq_soc.h"

/*
 * Cadence SPI 驱动 (UG585 ch.17): 主机模式 + 手动片选, 轮询收发,
 * spi_* 前缀, 控制器对象 (SPI0 / SPI1) 走第一个参数。寄存器级原语是
 * static inline 不阻塞; 参考时钟、SCLK 搜索与阻塞传输在 zynq_spi.c。
 *
 * SCLK = spi_ref / 2^(baud_div+1) (baud_div 1..7, 即 ref/4..ref/256),
 * 参考时钟由 spi_enable_clock() 固定为 IO PLL/5 = 200MHz —— ch.17 要求
 * spi_ref 始终高于 CPU_1x (111MHz, 复位值 /63 是违规的)。引脚复用是
 * 板级的活 (mio_set_function())。
 *
 *   spi_init(SPI1, SPI_MODE_0, 1000000);
 *   spi_select(SPI1, SPI_CS0);
 *   spi_transfer(SPI1, cmd, NULL, 2);     // 发命令
 *   spi_transfer(SPI1, NULL, buf, 4);     // 读回
 *   spi_deselect(SPI1);
 *
 * @note spi_transfer() 的节奏按 FIFO 水位 1 设计 (spi_init 写回默认值):
 *       spi_tx_empty() = TX FIFO 空, spi_rx_ready() = RX FIFO 非空。
 */

/* spi_init() 的时钟模式参数: CPOL/CPHA 组合 (与 Linux SPI_MODE_n 同号)。 */
#define SPI_MODE_0 (0u) /* CPOL=0, CPHA=0 */
#define SPI_MODE_1 (1u) /* CPOL=0, CPHA=1 */
#define SPI_MODE_2 (2u) /* CPOL=1, CPHA=0 */
#define SPI_MODE_3 (3u) /* CPOL=1, CPHA=1 */

/* spi_select() 的片选号: SS0..SS2 (SSDECEN=0, 单热编码低有效)。 */
#define SPI_CS0 (0u)
#define SPI_CS1 (1u)
#define SPI_CS2 (2u)

/* tx == NULL 时 spi_transfer() 往 MOSI 打的填充字节。 */
#define SPI_DUMMY_TX (0x00u)

/* ------------------------------------------------------------------ */
/* Data                                                               */
/* ------------------------------------------------------------------ */

/* 压一个字节进 TX FIFO; 自动启动 (Man_start_en=0), 控制器随即移位发出。 */
static inline void spi_send_byte(SPI_Type *spi, uint8_t c)
{
    spi->TXD = (uint32_t)c;
}

/* 从 RX FIFO 取一个字节; 先看 spi_rx_ready()。 */
static inline uint8_t spi_recv_byte(SPI_Type *spi)
{
    return (uint8_t)(spi->RXD & 0xFFu);
}

/* TX FIFO 已降到水位以下 (水位 1 => 空), 可以安全整块续填。 */
static inline int spi_tx_empty(SPI_Type *spi)
{
    return ((spi->SR & SPI_IXR_TXOW_Msk) != 0u) ? 1 : 0;
}

/* RX FIFO 里有字节可取 (水位 1 => 非空)。 */
static inline int spi_rx_ready(SPI_Type *spi)
{
    return ((spi->SR & SPI_IXR_RXNEMPTY_Msk) != 0u) ? 1 : 0;
}

/* ------------------------------------------------------------------ */
/* Slave select (manual mode: SSFORCE=1, SSDECEN=0)                   */
/* ------------------------------------------------------------------ */

/* 拉低 SS[cs] 选中从设备 (cs = SPI_CS0..SPI_CS2), 其余片选保持高。 */
static inline void spi_select(SPI_Type *spi, uint32_t cs)
{
    spi->CR = (spi->CR & ~SPI_CR_SSCTRL_Msk) | (SPI_CR_SSCTRL_Msk & ~(1u << (SPI_CR_SSCTRL_Pos + cs)));
}

/* 释放全部片选 (SS[2:0] 全高)。 */
static inline void spi_deselect(SPI_Type *spi)
{
    spi->CR = (spi->CR & ~SPI_CR_SSCTRL_Msk) | SPI_CR_SSCTRL_Msk;
}

/* ------------------------------------------------------------------ */
/* Enable                                                             */
/* ------------------------------------------------------------------ */

/* 使能控制器 (SPI_EN=1); 关闭时输出三态。 */
static inline void spi_enable(SPI_Type *spi)
{
    spi->ER = SPI_ER_ENABLE_Msk;
}

/* 关控制器 (SPI_EN=0)。 */
static inline void spi_disable(SPI_Type *spi)
{
    spi->ER = 0u;
}

/* ------------------------------------------------------------------ */
/* Interrupt                                                          */
/* ------------------------------------------------------------------ */

/* 当前状态位 (SPI_IXR_*), 含粘滞位; TXUF/MODF/RXOVR 写 1 清。 */
static inline uint32_t spi_irq_status(SPI_Type *spi)
{
    return (spi->SR & SPI_IXR_MASK_Msk);
}

/* 已使能的源 (IMR 位 = 1)。 */
static inline uint32_t spi_irq_mask(SPI_Type *spi)
{
    return (spi->IMR & SPI_IXR_MASK_Msk);
}

/* 使能给定的 SPI_IXR_* 源。 */
static inline void spi_irq_enable(SPI_Type *spi, uint32_t mask)
{
    spi->IER = (mask & SPI_IXR_MASK_Msk);
}

/* 屏蔽给定的 SPI_IXR_* 源。 */
static inline void spi_irq_disable(SPI_Type *spi, uint32_t mask)
{
    spi->IDR = (mask & SPI_IXR_MASK_Msk);
}

/* 清粘滞状态位 (写 1 清)。 */
static inline void spi_irq_clear(SPI_Type *spi, uint32_t mask)
{
    spi->SR = (mask & SPI_IXR_MASK_Msk);
}

/* ------------------------------------------------------------------ */
/* FIFO levels                                                        */
/* ------------------------------------------------------------------ */

/* TX 水位: 低于它 TXOW 置位 (复位默认 1)。改动会改 spi_transfer() 节奏。 */
static inline void spi_set_tx_watermark(SPI_Type *spi, uint32_t n)
{
    spi->TXWR = (n & SPI_TXWR_MASK_Msk);
}

/* RX 水位: 达到它 RXNEMPTY 置位 (复位默认 1)。 */
static inline void spi_set_rx_watermark(SPI_Type *spi, uint32_t n)
{
    spi->RXWR = (n & SPI_RXWR_MASK_Msk);
}

/* ------------------------------------------------------------------ */
/* Configuration                                                      */
/* ------------------------------------------------------------------ */

/* BAUD_RATE_DIV 位域 (1..7 => SCLK = ref/4 .. ref/256); spi_set_sclk() 用。 */
static inline void spi_set_baud_div(SPI_Type *spi, uint32_t baud_div)
{
    spi->CR = (spi->CR & ~SPI_CR_PRESC_Msk) | ((baud_div << SPI_CR_PRESC_Pos) & SPI_CR_PRESC_Msk);
}

/* 打开参考时钟: APER_CLK_CTRL 的 CPU_1x 门 + SPI_CLK_CTRL 固定
   IO PLL/5 = 200MHz。 */
void spi_enable_clock(SPI_Type *spi);

/* 关掉本控制器的参考时钟门 (共享分频寄存器不动)。 */
void spi_disable_clock(SPI_Type *spi);

/* 复位脉冲 (SLCR SPI_RST_CTRL 的 REF + CPU1x 两位), 清空两个 FIFO。 */
void spi_reset(SPI_Type *spi);

/* 选一档不超过 sclk_hz 的 SCLK 写进 BAUD_RATE_DIV; 0 成功,
   低于参考时钟/256 返回 -1。 */
int spi_set_sclk(SPI_Type *spi, uint32_t sclk_hz);

/* 复位 + 主机模式 (SPI_MODE_n) + 手动片选 (全释放) + 水位 1 + 中断
   全屏蔽 + 使能; 返回 0, sclk_hz 不可达返回 -1。 */
int spi_init(SPI_Type *spi, uint32_t mode, uint32_t sclk_hz);

/* ------------------------------------------------------------------ */
/* Blocking transfer                                                  */
/* ------------------------------------------------------------------ */

/*
 * 全双工阻塞传输: 整块 (≤128B) 填 TX FIFO 后整块收 RX, 块间自动续。
 * tx == NULL 发 SPI_DUMMY_TX, rx == NULL 丢弃; 不动片选 —— 一次从设备
 * 会话由 spi_select() / spi_deselect() 包住。返回 0; -1 = FIFO 等待
 * 超限 (控制器未使能 / 参考时钟关)。
 */
int spi_transfer(SPI_Type *spi, const uint8_t *tx, uint8_t *rx, uint32_t len);

#endif /* ZYNQ_SPI_H */
