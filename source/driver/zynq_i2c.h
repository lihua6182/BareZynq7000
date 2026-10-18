#ifndef ZYNQ_I2C_H
#define ZYNQ_I2C_H

#include "zynq_soc.h"

/*
 * Cadence I2C 驱动 (UG585 ch.20): 主机模式 + 7 位地址, 轮询收发,
 * i2c_* 前缀, 控制器对象 (I2C0 / I2C1) 走第一个参数。寄存器级原语是
 * static inline 不阻塞; SCL 搜索与阻塞传输在 zynq_i2c.c。
 *
 * SCL = CPU_1x / (22 * (div_a+1) * (div_b+1)), div_a 0..3, div_b 0..63;
 * 目标速率只接受总线规格内的 (≤ 400kHz)。引脚复用是板级的活
 * (mio_set_function())。
 *
 *   i2c_init(I2C0, 100000);
 *   i2c_write(I2C0, 0x50, page, 2);
 *   i2c_write_read(I2C0, 0x50, &reg, 1, buf, 2);   // 寄存器读
 *
 * @note 传输失败返回 -1 时 ISR 现场保留 (NACK/ARB_LOST/TO/...), 可用
 *       i2c_irq_status() 判因; 下一次传输开头会自行清。
 */

/* 总线规格上限 (ch.20): 标准/快速模式; 高于它的目标速率直接拒绝。 */
#define I2C_SCL_MAX (400000u)

/* 单笔传输字节数上限: 传输长度寄存器 8 位 (ch.20 附录 A)。 */
#define I2C_MAX_TRANSFER (0xFFu)

/* 7 位从地址上限 (右对齐; 正常寻址用 add[6:0])。 */
#define I2C_ADDR_MAX (0x7Fu)

/* ------------------------------------------------------------------ */
/* Data / status                                                      */
/* ------------------------------------------------------------------ */

/* 压一个字节进 TX FIFO (主机发送用)。 */
static inline void i2c_send_byte(I2C_Type *i2c, uint8_t c)
{
    i2c->DATA = (uint32_t)c;
}

/* 从 RX FIFO 取一个字节; 先看 i2c_rx_ready()。 */
static inline uint8_t i2c_recv_byte(I2C_Type *i2c)
{
    return (uint8_t)(i2c->DATA & 0xFFu);
}

/* RX FIFO 有有效数据。 */
static inline int i2c_rx_ready(I2C_Type *i2c)
{
    return ((i2c->SR & I2C_SR_RXDV_Msk) != 0u) ? 1 : 0;
}

/* 接口还有字节没发完 (FIFO 或移位器)。 */
static inline int i2c_tx_active(I2C_Type *i2c)
{
    return ((i2c->SR & I2C_SR_TXDV_Msk) != 0u) ? 1 : 0;
}

/* 总线忙 (有进行中的传输)。 */
static inline int i2c_bus_active(I2C_Type *i2c)
{
    return ((i2c->SR & I2C_SR_BA_Msk) != 0u) ? 1 : 0;
}

/* ------------------------------------------------------------------ */
/* Interrupt                                                          */
/* ------------------------------------------------------------------ */

/* 粘滞状态位 (I2C_IXR_*); 写 1 清。 */
static inline uint32_t i2c_irq_status(I2C_Type *i2c)
{
    return (i2c->ISR & I2C_IXR_ALL_INTR_Msk);
}

/* 已被屏蔽的源 (IMR 位 = 1 表示屏蔽)。 */
static inline uint32_t i2c_irq_mask(I2C_Type *i2c)
{
    return (i2c->IMR & I2C_IXR_ALL_INTR_Msk);
}

/* 使能给定的 I2C_IXR_* 源 (IER 写 1 清屏蔽)。 */
static inline void i2c_irq_enable(I2C_Type *i2c, uint32_t mask)
{
    i2c->IER = (mask & I2C_IXR_ALL_INTR_Msk);
}

/* 屏蔽给定的 I2C_IXR_* 源 (IDR 写 1 置屏蔽)。 */
static inline void i2c_irq_disable(I2C_Type *i2c, uint32_t mask)
{
    i2c->IDR = (mask & I2C_IXR_ALL_INTR_Msk);
}

/* 清状态位 (写 1 清)。 */
static inline void i2c_irq_clear(I2C_Type *i2c, uint32_t mask)
{
    i2c->ISR = (mask & I2C_IXR_ALL_INTR_Msk);
}

/* ------------------------------------------------------------------ */
/* Configuration                                                      */
/* ------------------------------------------------------------------ */

/* 打开 CPU_1x 时钟门 (APER_CLK_CTRL bit 18/19); 门关时寄存器读回全零。 */
void i2c_enable_clock(I2C_Type *i2c);

/* 关掉 CPU_1x 时钟门。 */
void i2c_disable_clock(I2C_Type *i2c);

/* 复位脉冲 (SLCR I2C_RST_CTRL 的 CPU1x 位, 只有 APB 域)。 */
void i2c_reset(I2C_Type *i2c);

/* 选一对分频给不超过 scl_hz 的最高 SCL; 0 成功, 超 400kHz 或低于
   可达下限 (CPU_1x/5632) 返回 -1。 */
int i2c_set_scl(I2C_Type *i2c, uint32_t scl_hz);

/* 复位 + 主机发送默认态 (7 位寻址, ACK 开) + 超时 0xFF + 中断全屏蔽;
   返回 0, scl_hz 不可达返回 -1。 */
int i2c_init(I2C_Type *i2c, uint32_t scl_hz);

/* ------------------------------------------------------------------ */
/* Blocking master transfers                                          */
/* ------------------------------------------------------------------ */

/*
 * 阻塞主机发送: addr 为右对齐 7 位从地址, len 1..255; 超过一 FIFO
 * (16B) 时经 HOLD 保总线, 逐块续填, 末尾自动 STOP。
 * 返回 0; -1 = 参数非法 / 总线忙 / NACK / 仲裁丢失 / 超时 (ISR 留现场)。
 */
int i2c_write(I2C_Type *i2c, uint32_t addr, const uint8_t *tx, uint32_t len);

/* 阻塞主机接收: len 1..255, 末尾由控制器回 NACK + STOP。 */
int i2c_read(I2C_Type *i2c, uint32_t addr, uint8_t *rx, uint32_t len);

/*
 * 组合传输 (无 STOP 间隔, repeated start): 先写 tx (1..16B, 一个
 * FIFO), 再读 rx (1..255B) —— "寄存器地址 + 读回" 的标准姿势。
 */
int i2c_write_read(I2C_Type *i2c, uint32_t addr, const uint8_t *tx, uint32_t tx_len, uint8_t *rx, uint32_t rx_len);

#endif /* ZYNQ_I2C_H */
