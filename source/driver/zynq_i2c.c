/**
 * @file zynq_i2c.c
 * @brief I2C 驱动: 需要多次寄存器访问的部分 (时钟门控、复位、SCL
 *        搜索、阻塞主机传输)。
 */

#include "zynq_i2c.h"

#include <stddef.h>

#include "zynq_clock.h"

/* APER_CLK_CTRL 的 CPU_1x 时钟门: bit 18 I2C0, bit 19 I2C1。 */
#define I2C0_CLK_GATE_BIT (1u << 18)
#define I2C1_CLK_GATE_BIT (1u << 19)

/* 会判失败的中断位 (除 COMP/DATA/SLV_RDY 之外的粘滞位)。 */
#define I2C_IXR_ERROR_Msk                                                                                              \
    (I2C_IXR_NACK_Msk | I2C_IXR_ARB_LOST_Msk | I2C_IXR_RX_OVR_Msk | I2C_IXR_TX_OVR_Msk | I2C_IXR_RX_UNF_Msk |          \
     I2C_IXR_TO_Msk)

/* 超时寄存器给最大 0xFF (SCL 被拉低超过约 255 个 SCL 周期判 TO)。 */
#define I2C_TIMEOUT_CYCLES (0xFFu)

/* HOLD 提前释放点: FIFO 深度 - 2, 让剩余尾巴自然 NACK+STOP。 */
#define I2C_HOLD_RELEASE (I2C_FIFO_DEPTH - 2u)

/* 单次等待的轮询上限 (强序总线周期): 100kHz 下一 FIFO (16B) 约
 * 1.5ms, 余量充足。 */
#define I2C_POLL_LIMIT (1000000u)

/* APER 时钟门位 (按控制器)。 */
static uint32_t gate_bit(I2C_Type *i2c)
{
    return (i2c == I2C1) ? I2C1_CLK_GATE_BIT : I2C0_CLK_GATE_BIT;
}

/* SLCR I2C_RST_CTRL 复位位: I2C0 = bit0, I2C1 = bit1 (仅 APB 域)。 */
static uint32_t reset_bit(I2C_Type *i2c)
{
    return (i2c == I2C1) ? (1u << 1) : (1u << 0);
}

/* 有错误粘滞位则返回 1。 */
static int error_pending(I2C_Type *i2c)
{
    return ((i2c_irq_status(i2c) & I2C_IXR_ERROR_Msk) != 0u) ? 1 : 0;
}

/* 等传输完成 (COMP); 错误位优先, 轮询超限也判失败。 */
static int wait_complete(I2C_Type *i2c)
{
    uint32_t polls = I2C_POLL_LIMIT;

    while ((i2c_irq_status(i2c) & I2C_IXR_COMP_Msk) == 0u)
    {
        if (error_pending(i2c) != 0)
        {
            return -1;
        }
        if (--polls == 0u)
        {
            return -1;
        }
    }
    return 0;
}

/* 等总线空闲 (STOP 已发出); 错误位优先。 */
static int wait_idle(I2C_Type *i2c)
{
    uint32_t polls = I2C_POLL_LIMIT;

    while (i2c_bus_active(i2c) != 0)
    {
        if (error_pending(i2c) != 0)
        {
            return -1;
        }
        if (--polls == 0u)
        {
            return -1;
        }
    }
    return 0;
}

/* 清 HOLD: 传输自然收尾 (STOP), 或从 HOLD 暂停态放行。 */
static void release_bus(I2C_Type *i2c)
{
    i2c->CR = i2c->CR & ~I2C_CR_HOLD_Msk;
}

/* 中止传输: 放行 + 清 FIFO; ISR 现场留给调用方判因。 */
static void abort_transfer(I2C_Type *i2c)
{
    i2c->CR = (i2c->CR & ~I2C_CR_HOLD_Msk) | I2C_CR_CLR_FIFO_Msk;
}

/**
 * @brief  打开 CPU_1x 时钟门 (寄存器可读写的前提)。
 * @param  i2c I2C0 或 I2C1。
 */
void i2c_enable_clock(I2C_Type *i2c)
{
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->APER_CLK_CTRL |= gate_bit(i2c);
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  关掉 CPU_1x 时钟门。
 * @param  i2c I2C0 或 I2C1。
 */
void i2c_disable_clock(I2C_Type *i2c)
{
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->APER_CLK_CTRL &= ~gate_bit(i2c);
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  复位控制器 (SLCR I2C_RST_CTRL 的 CPU1x 位)。
 * @param  i2c I2C0 或 I2C1。
 */
void i2c_reset(I2C_Type *i2c)
{
    uint32_t bit = reset_bit(i2c);
    uint32_t i;

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->I2C_RST_CTRL |= bit;
    for (i = 0u; i < 100u; i++)
    {
        __asm volatile("nop");
    }
    SLCR->I2C_RST_CTRL &= ~bit;
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  选一对分频 (div_a 0..3, div_b 0..63) 给不超过 scl_hz 的最高
 *         SCL: SCL = CPU_1x / (22 * (div_a+1) * (div_b+1))。
 * @param  i2c    I2C0 或 I2C1。
 * @param  scl_hz 目标 SCL, Hz。
 * @return 0; -1 = 目标为 0 / 超 400kHz / 低于可达下限或 CPU_1x 未知。
 */
int i2c_set_scl(I2C_Type *i2c, uint32_t scl_hz)
{
    struct clock_freqs clocks;
    uint32_t div_a;
    uint32_t div_b;
    uint32_t best_a = 0u;
    uint32_t best_b = 0u;
    uint32_t best = 0u;
    uint32_t actual;

    if ((scl_hz == 0u) || (scl_hz > I2C_SCL_MAX))
    {
        return -1;
    }
    clock_freqs_get(&clocks);
    if (clocks.cpu_1x == 0u)
    {
        return -1;
    }
    for (div_a = 0u; div_a <= 3u; div_a++)
    {
        for (div_b = 0u; div_b <= 63u; div_b++)
        {
            actual = clocks.cpu_1x / (22u * (div_a + 1u) * (div_b + 1u));
            if ((actual <= scl_hz) && (actual > best))
            {
                best = actual;
                best_a = div_a;
                best_b = div_b;
            }
        }
    }
    if (best == 0u)
    {
        return -1; /* 最低档 (4x64) 仍快于目标 */
    }
    i2c->CR = (i2c->CR & ~(I2C_CR_DIV_A_Msk | I2C_CR_DIV_B_Msk)) | (best_a << I2C_CR_DIV_A_Pos) |
              (best_b << I2C_CR_DIV_B_Pos);
    return 0;
}

/**
 * @brief  初始化控制器: 复位, 主机发送默认态 (7 位寻址 + ACK), 超时
 *         最大档, 中断全屏蔽。
 * @param  i2c    I2C0 或 I2C1。
 * @param  scl_hz 目标 SCL, Hz。
 * @return 0; -1 = scl_hz 不可达。
 */
int i2c_init(I2C_Type *i2c, uint32_t scl_hz)
{
    i2c_enable_clock(i2c);
    i2c_reset(i2c);

    /* CLR_FIFO 自清; 传输函数每次会重写方向位。 */
    i2c->CR = I2C_CR_MS_Msk | I2C_CR_NEA_Msk | I2C_CR_ACKEN_Msk | I2C_CR_CLR_FIFO_Msk;
    if (i2c_set_scl(i2c, scl_hz) != 0)
    {
        return -1;
    }
    i2c->TIME_OUT = I2C_TIMEOUT_CYCLES;
    i2c_irq_disable(i2c, I2C_IXR_ALL_INTR_Msk);
    i2c_irq_clear(i2c, I2C_IXR_ALL_INTR_Msk);
    return 0;
}

/**
 * @brief  阻塞主机发送, 1..255 字节。
 * @param  i2c  I2C0 或 I2C1。
 * @param  addr 右对齐 7 位从地址。
 * @param  tx   发送缓冲。
 * @param  len  字节数。
 * @return 0; -1 = 参数非法 / 总线忙 / NACK / 仲裁丢失 / 超时等。
 * @note   超过一 FIFO 时先置 HOLD 保总线 (块间不 STOP), 按 TXDV 排空
 *         节奏续填; 完成后放 HOLD 由硬件发 STOP。失败时清 FIFO 并保留
 *         ISR 现场供判因。
 */
int i2c_write(I2C_Type *i2c, uint32_t addr, const uint8_t *tx, uint32_t len)
{
    uint32_t remaining = len;
    uint32_t chunk;
    uint32_t i;
    uint32_t polls = I2C_POLL_LIMIT;
    int hold;

    if ((tx == NULL) || (len == 0u) || (len > I2C_MAX_TRANSFER) || (addr > I2C_ADDR_MAX) ||
        (i2c_bus_active(i2c) != 0))
    {
        return -1;
    }

    hold = (len > I2C_FIFO_DEPTH) ? 1 : 0;
    i2c->CR = I2C_CR_MS_Msk | I2C_CR_NEA_Msk | I2C_CR_ACKEN_Msk | I2C_CR_CLR_FIFO_Msk |
              ((hold != 0) ? I2C_CR_HOLD_Msk : 0u);
    i2c_irq_clear(i2c, I2C_IXR_ALL_INTR_Msk);

    /* 先填首块再写地址: 写 ADDR 即启动传输 (ch.20 写流程)。 */
    chunk = (remaining < I2C_FIFO_DEPTH) ? remaining : I2C_FIFO_DEPTH;
    for (i = 0u; i < chunk; i++)
    {
        i2c_send_byte(i2c, tx[i]);
    }
    tx += chunk;
    remaining -= chunk;
    i2c->ADDR = addr;

    /* TXDV=0 表示 FIFO 已排空, 此时整块续填安全。 */
    while (remaining > 0u)
    {
        if (error_pending(i2c) != 0)
        {
            goto fail;
        }
        if (i2c_tx_active(i2c) != 0)
        {
            if (--polls == 0u)
            {
                goto fail;
            }
            continue;
        }
        chunk = (remaining < I2C_FIFO_DEPTH) ? remaining : I2C_FIFO_DEPTH;
        for (i = 0u; i < chunk; i++)
        {
            i2c_send_byte(i2c, tx[i]);
        }
        tx += chunk;
        remaining -= chunk;
        polls = I2C_POLL_LIMIT;
    }

    if (wait_complete(i2c) != 0)
    {
        goto fail;
    }
    release_bus(i2c);
    if (wait_idle(i2c) != 0)
    {
        goto fail;
    }
    return 0;

fail:
    abort_transfer(i2c);
    return -1;
}

/**
 * @brief  阻塞主机接收, 1..255 字节。
 * @param  i2c  I2C0 或 I2C1。
 * @param  addr 右对齐 7 位从地址。
 * @param  rx   接收缓冲。
 * @param  len  字节数。
 * @return 0; -1 = 参数非法 / 总线忙 / NACK / 溢出 / 超时等。
 * @note   长度先写 TRANS_SIZE 再写 ADDR (ch.20 定序); 超一 FIFO 时置
 *         HOLD, 尾 14 字节前释放, 收尾 NACK+STOP 由控制器自动完成。
 */
int i2c_read(I2C_Type *i2c, uint32_t addr, uint8_t *rx, uint32_t len)
{
    uint32_t got = 0u;
    uint32_t polls = I2C_POLL_LIMIT;
    int hold;

    if ((rx == NULL) || (len == 0u) || (len > I2C_MAX_TRANSFER) || (addr > I2C_ADDR_MAX) ||
        (i2c_bus_active(i2c) != 0))
    {
        return -1;
    }

    hold = (len > I2C_FIFO_DEPTH) ? 1 : 0;
    i2c->CR = I2C_CR_MS_Msk | I2C_CR_NEA_Msk | I2C_CR_ACKEN_Msk | I2C_CR_RD_WR_Msk | I2C_CR_CLR_FIFO_Msk |
              ((hold != 0) ? I2C_CR_HOLD_Msk : 0u);
    i2c_irq_clear(i2c, I2C_IXR_ALL_INTR_Msk);
    i2c->TRANS_SIZE = len;
    i2c->ADDR = addr; /* 启动: 地址 + 读方向 */

    while (got < len)
    {
        if (error_pending(i2c) != 0)
        {
            goto fail;
        }
        if (i2c_rx_ready(i2c) == 0)
        {
            if (--polls == 0u)
            {
                goto fail;
            }
            continue;
        }
        rx[got] = i2c_recv_byte(i2c);
        got++;
        polls = I2C_POLL_LIMIT;
        if ((hold != 0) && ((len - got) <= I2C_HOLD_RELEASE))
        {
            hold = 0;
            release_bus(i2c);
        }
    }

    if (wait_complete(i2c) != 0)
    {
        goto fail;
    }
    release_bus(i2c);
    if (wait_idle(i2c) != 0)
    {
        goto fail;
    }
    return 0;

fail:
    abort_transfer(i2c);
    return -1;
}

/**
 * @brief  组合传输: 写 tx (≤16B) 后 repeated start 读 rx (1..255B)。
 * @param  i2c    I2C0 或 I2C1。
 * @param  addr   右对齐 7 位从地址 (写读同地址)。
 * @param  tx     写段缓冲, 1..16 字节。
 * @param  tx_len 写段长度。
 * @param  rx     读段缓冲。
 * @param  rx_len 读段长度。
 * @return 0; -1 = 参数非法 / 总线忙 / NACK / 超时等。
 * @note   第一段全程 HOLD (写完后 SCL 拉低, 不发 STOP), 第二段换读
 *         方向后重写 ADDR 产生 repeated start; 写段限 16B 保证单次
 *         FIFO 排空即可等到 COMP, 两段之间不出现空窗。ISR 在两段
 *         之间清一次。
 */
int i2c_write_read(I2C_Type *i2c, uint32_t addr, const uint8_t *tx, uint32_t tx_len, uint8_t *rx, uint32_t rx_len)
{
    uint32_t i;
    uint32_t got = 0u;
    uint32_t polls = I2C_POLL_LIMIT;
    int hold = 1;

    if ((tx == NULL) || (rx == NULL) || (tx_len == 0u) || (tx_len > I2C_FIFO_DEPTH) || (rx_len == 0u) ||
        (rx_len > I2C_MAX_TRANSFER) || (addr > I2C_ADDR_MAX) || (i2c_bus_active(i2c) != 0))
    {
        return -1;
    }

    /* 第一段: 写 + HOLD, 结束时总线挂在暂停态 (无 STOP)。 */
    i2c->CR = I2C_CR_MS_Msk | I2C_CR_NEA_Msk | I2C_CR_ACKEN_Msk | I2C_CR_CLR_FIFO_Msk | I2C_CR_HOLD_Msk;
    i2c_irq_clear(i2c, I2C_IXR_ALL_INTR_Msk);
    for (i = 0u; i < tx_len; i++)
    {
        i2c_send_byte(i2c, tx[i]);
    }
    i2c->ADDR = addr;
    if (wait_complete(i2c) != 0)
    {
        goto fail;
    }

    /* 第二段: 不释放 HOLD, 换读方向 + 长度后重写 ADDR => repeated start。 */
    i2c_irq_clear(i2c, I2C_IXR_ALL_INTR_Msk);
    i2c->CR = I2C_CR_MS_Msk | I2C_CR_NEA_Msk | I2C_CR_ACKEN_Msk | I2C_CR_RD_WR_Msk | I2C_CR_CLR_FIFO_Msk |
              I2C_CR_HOLD_Msk;
    i2c->TRANS_SIZE = rx_len;
    i2c->ADDR = addr;

    while (got < rx_len)
    {
        if (error_pending(i2c) != 0)
        {
            goto fail;
        }
        if (i2c_rx_ready(i2c) == 0)
        {
            if (--polls == 0u)
            {
                goto fail;
            }
            continue;
        }
        rx[got] = i2c_recv_byte(i2c);
        got++;
        polls = I2C_POLL_LIMIT;
        if ((hold != 0) && ((rx_len - got) <= I2C_HOLD_RELEASE))
        {
            hold = 0;
            release_bus(i2c);
        }
    }

    if (wait_complete(i2c) != 0)
    {
        goto fail;
    }
    release_bus(i2c);
    if (wait_idle(i2c) != 0)
    {
        goto fail;
    }
    return 0;

fail:
    abort_transfer(i2c);
    return -1;
}
