/**
 * @file zynq_swd.c
 * @brief SWD 访问层实现: 命令打包 + 阻塞收发 (超时走 1ms tick)。
 *
 * 只做协议搬运, 不含 SWD 时序本身 —— 时序由 PPC 上的 swd_dap 程序按拍产生
 * (见 zynq_swd.h 的协议表)。阻塞等待一律带毫秒超时, 不通就返回
 * false / SWD_ACK_TIMEOUT, 不无限挂死。
 */

#include "zynq_swd.h"

#include "zynq_tick.h"

/**
 * @brief  等 RX FIFO 里攒到 n 个字 (FSTAT[19:12] = rx 计数)。
 * @return 超时返回 false。
 */
static bool swd_wait_rx(COPROC_Type *c, uint32_t n, uint32_t timeout_ms)
{
    uint32_t t0 = tick_get();

    while (((c->FSTAT >> 12) & 0xFFu) < n)
    {
        if ((tick_get() - t0) > timeout_ms)
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief  从 LSB first 位流里取 n 位 (n <= 32)。
 * @param  off 起始位偏移。
 */
static uint32_t swd_bits_get(const uint8_t *bits, uint32_t off, uint32_t n)
{
    uint32_t v = 0u;
    uint32_t i;

    for (i = 0u; i < n; i++)
    {
        uint32_t b = off + i;
        if (((bits[b >> 3] >> (b & 7u)) & 1u) != 0u)
        {
            v |= (1u << i);
        }
    }
    return v;
}

/**
 * @brief  把 n 位写回 LSB first 位流 (n <= 32)。
 */
static void swd_bits_put(uint8_t *bits, uint32_t off, uint32_t n, uint32_t v)
{
    uint32_t i;

    for (i = 0u; i < n; i++)
    {
        uint32_t b = off + i;
        uint8_t mask = (uint8_t)(1u << (b & 7u));
        if (((v >> i) & 1u) != 0u)
        {
            bits[b >> 3] |= mask;
        }
        else
        {
            bits[b >> 3] &= (uint8_t)~mask;
        }
    }
}

void swd_configure(COPROC_Type *c, uint32_t turnaround, uint32_t idle)
{
    /* RUN 前 FIFO 是空的, 直接压即可; 程序启动时先 pull 这两个字 */
    coproc_tx_push(c, turnaround);
    coproc_tx_push(c, idle);
}

bool swd_line_reset(COPROC_Type *c, uint32_t timeout_ms)
{
    /* ADIv5: >=50 拍 SWDIO=1 + >=2 拍 SWDIO=0; 取 51+8 */
    static const uint8_t ones[7] = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0x07u};
    static const uint8_t zero = 0x00u;

    return swd_seq_out(c, 51u, ones, timeout_ms) && swd_seq_out(c, 8u, &zero, timeout_ms);
}

bool swd_jtag_to_swd(COPROC_Type *c, uint32_t timeout_ms)
{
    /* ADIv5: 16 位 0xE79E, LSB first 先发 0x9E */
    static const uint8_t j2s[2] = {0x9Eu, 0xE7u};

    return swd_seq_out(c, 16u, j2s, timeout_ms);
}

bool swd_seq_out(COPROC_Type *c, uint32_t count, const uint8_t *bits, uint32_t timeout_ms)
{
    uint32_t off = 0u;

    if (count == 0u || count > 256u)
    {
        return false;
    }
    while (count != 0u)
    {
        uint32_t n = (count > 32u) ? 32u : count;
        uint32_t word = swd_bits_get(bits, off, n);

        if (!coproc_tx_push_wait(c, SWD_SEQ(SWD_DIR_OUT, n), timeout_ms))
        {
            return false;
        }
        if (!coproc_tx_push_wait(c, word, timeout_ms))
        {
            return false;
        }
        if (!swd_wait_rx(c, 1u, timeout_ms))
        {
            return false;
        }
        (void)coproc_rx_pop(c); /* 回显, 丢弃 */
        off += n;
        count -= n;
    }
    return true;
}

bool swd_seq_in(COPROC_Type *c, uint32_t count, uint8_t *bits, uint32_t timeout_ms)
{
    uint32_t off = 0u;

    if (count == 0u || count > 256u)
    {
        return false;
    }
    while (count != 0u)
    {
        uint32_t n = (count > 32u) ? 32u : count;
        uint32_t word;

        if (!coproc_tx_push_wait(c, SWD_SEQ(SWD_DIR_IN, n), timeout_ms))
        {
            return false;
        }
        /* 输入方向的输入值不参与时序, 主机仍要发数据字保持帧同步 */
        if (!coproc_tx_push_wait(c, 0u, timeout_ms))
        {
            return false;
        }
        if (!swd_wait_rx(c, 1u, timeout_ms))
        {
            return false;
        }
        word = coproc_rx_pop(c);
        swd_bits_put(bits, off, n, word);
        off += n;
        count -= n;
    }
    return true;
}

uint8_t swd_transfer(COPROC_Type *c, uint8_t request, uint32_t *data, uint32_t timeout_ms)
{
    uint32_t ack;
    uint32_t word = (data != 0) ? *data : 0u;

    if (!coproc_tx_push_wait(c, SWD_XFER(request), timeout_ms))
    {
        return SWD_ACK_TIMEOUT;
    }
    if (!coproc_tx_push_wait(c, word, timeout_ms))
    {
        return SWD_ACK_TIMEOUT;
    }
    if (!swd_wait_rx(c, 2u, timeout_ms))
    {
        return SWD_ACK_TIMEOUT;
    }
    ack = coproc_rx_pop(c);
    if (data != 0)
    {
        *data = coproc_rx_pop(c); /* 读=收到的值, 写=回显 */
    }
    else
    {
        (void)coproc_rx_pop(c);
    }
    return (uint8_t)ack;
}
