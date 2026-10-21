
#include "zynq_i2c.h"

#include <stddef.h>

#include "zynq_clock.h"

#define I2C0_CLK_GATE_BIT (1u << 18)
#define I2C1_CLK_GATE_BIT (1u << 19)

#define I2C_IXR_ERROR_Msk                                                                                              \
    (I2C_IXR_NACK_Msk | I2C_IXR_ARB_LOST_Msk | I2C_IXR_RX_OVR_Msk | I2C_IXR_TX_OVR_Msk | I2C_IXR_RX_UNF_Msk |          \
     I2C_IXR_TO_Msk)

#define I2C_TIMEOUT_CYCLES (0xFFu)

#define I2C_HOLD_RELEASE (I2C_FIFO_DEPTH - 2u)

#define I2C_POLL_LIMIT (1000000u)

static uint32_t gate_bit(struct i2c *i2c)
{
    return (i2c == i2c1) ? I2C1_CLK_GATE_BIT : I2C0_CLK_GATE_BIT;
}

static uint32_t reset_bit(struct i2c *i2c)
{
    return (i2c == i2c1) ? (1u << 1) : (1u << 0);
}

static int error_pending(struct i2c *i2c)
{
    return ((i2c_irq_status(i2c) & I2C_IXR_ERROR_Msk) != 0u) ? 1 : 0;
}

static int wait_complete(struct i2c *i2c)
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

static int wait_idle(struct i2c *i2c)
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

static void release_bus(struct i2c *i2c)
{
    i2c->cr = i2c->cr & ~I2C_CR_HOLD_Msk;
}

static void abort_transfer(struct i2c *i2c)
{
    i2c->cr = (i2c->cr & ~I2C_CR_HOLD_Msk) | I2C_CR_CLR_FIFO_Msk;
}

static void i2c_cr_write(struct i2c *i2c, uint32_t bits)
{
    uint32_t div = i2c->cr & (I2C_CR_DIV_A_Msk | I2C_CR_DIV_B_Msk);

    i2c->cr = div | bits;
}

void i2c_enable_clock(struct i2c *i2c)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->aper_clk_ctrl |= gate_bit(i2c);
    slcr->lock = SLCR_LOCK_KEY;
}

void i2c_disable_clock(struct i2c *i2c)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->aper_clk_ctrl &= ~gate_bit(i2c);
    slcr->lock = SLCR_LOCK_KEY;
}

void i2c_reset(struct i2c *i2c)
{
    uint32_t bit = reset_bit(i2c);
    uint32_t i;

    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->i2c_rst_ctrl |= bit;
    for (i = 0u; i < 100u; i++)
    {
        asm volatile("nop");
    }
    slcr->i2c_rst_ctrl &= ~bit;
    slcr->lock = SLCR_LOCK_KEY;
}

int i2c_set_scl(struct i2c *i2c, uint32_t scl_hz)
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
        return -1;
    }
    i2c->cr = (i2c->cr & ~(I2C_CR_DIV_A_Msk | I2C_CR_DIV_B_Msk)) | (best_a << I2C_CR_DIV_A_Pos) |
              (best_b << I2C_CR_DIV_B_Pos);
    return 0;
}

int i2c_init(struct i2c *i2c, uint32_t scl_hz)
{
    i2c_enable_clock(i2c);
    i2c_reset(i2c);

    i2c_cr_write(i2c, I2C_CR_MS_Msk | I2C_CR_NEA_Msk | I2C_CR_ACKEN_Msk | I2C_CR_CLR_FIFO_Msk);
    if (i2c_set_scl(i2c, scl_hz) != 0)
    {
        return -1;
    }
    i2c->time_out = I2C_TIMEOUT_CYCLES;
    i2c_irq_disable(i2c, I2C_IXR_ALL_INTR_Msk);
    i2c_irq_clear(i2c, I2C_IXR_ALL_INTR_Msk);
    return 0;
}

int i2c_write(struct i2c *i2c, uint32_t addr, const uint8_t *tx, uint32_t len)
{
    uint32_t remaining = len;
    uint32_t chunk;
    uint32_t i;
    uint32_t polls = I2C_POLL_LIMIT;
    int hold;

    if ((tx == NULL) || (len == 0u) || (len > I2C_MAX_TRANSFER) || (addr > I2C_ADDR_MAX) || (i2c_bus_active(i2c) != 0))
    {
        return -1;
    }

    hold = (len > I2C_FIFO_DEPTH) ? 1 : 0;
    i2c_cr_write(i2c, I2C_CR_MS_Msk | I2C_CR_NEA_Msk | I2C_CR_ACKEN_Msk | I2C_CR_CLR_FIFO_Msk |
                          ((hold != 0) ? I2C_CR_HOLD_Msk : 0u));
    i2c_irq_clear(i2c, I2C_IXR_ALL_INTR_Msk);

    chunk = (remaining < I2C_FIFO_DEPTH) ? remaining : I2C_FIFO_DEPTH;
    for (i = 0u; i < chunk; i++)
    {
        i2c_send_byte(i2c, tx[i]);
    }
    tx += chunk;
    remaining -= chunk;
    i2c->addr = addr;

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

int i2c_read(struct i2c *i2c, uint32_t addr, uint8_t *rx, uint32_t len)
{
    uint32_t got = 0u;
    uint32_t polls = I2C_POLL_LIMIT;
    int hold;

    if ((rx == NULL) || (len == 0u) || (len > I2C_MAX_TRANSFER) || (addr > I2C_ADDR_MAX) || (i2c_bus_active(i2c) != 0))
    {
        return -1;
    }

    hold = (len > I2C_FIFO_DEPTH) ? 1 : 0;
    i2c_cr_write(i2c, I2C_CR_MS_Msk | I2C_CR_NEA_Msk | I2C_CR_ACKEN_Msk | I2C_CR_RD_WR_Msk | I2C_CR_CLR_FIFO_Msk |
                          ((hold != 0) ? I2C_CR_HOLD_Msk : 0u));
    i2c_irq_clear(i2c, I2C_IXR_ALL_INTR_Msk);
    i2c->trans_size = len;
    i2c->addr = addr;

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

int i2c_write_read(struct i2c *i2c, uint32_t addr, const uint8_t *tx, uint32_t tx_len, uint8_t *rx, uint32_t rx_len)
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

    i2c_cr_write(i2c, I2C_CR_MS_Msk | I2C_CR_NEA_Msk | I2C_CR_ACKEN_Msk | I2C_CR_CLR_FIFO_Msk | I2C_CR_HOLD_Msk);
    i2c_irq_clear(i2c, I2C_IXR_ALL_INTR_Msk);
    for (i = 0u; i < tx_len; i++)
    {
        i2c_send_byte(i2c, tx[i]);
    }
    i2c->addr = addr;
    if (wait_complete(i2c) != 0)
    {
        goto fail;
    }

    i2c_irq_clear(i2c, I2C_IXR_ALL_INTR_Msk);
    i2c_cr_write(i2c, I2C_CR_MS_Msk | I2C_CR_NEA_Msk | I2C_CR_ACKEN_Msk | I2C_CR_RD_WR_Msk | I2C_CR_CLR_FIFO_Msk |
                          I2C_CR_HOLD_Msk);
    i2c->trans_size = rx_len;
    i2c->addr = addr;

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
