#include "zynq_i2c.h"

#include <stddef.h>

#include "zynq_clock.h"

#define I2C0_CLK_GATE_BIT (1U << 18)
#define I2C1_CLK_GATE_BIT (1U << 19)

#define I2C_IXR_ERROR_Msk                                                                                              \
    (I2C_IXR_NACK_Msk | I2C_IXR_ARB_LOST_Msk | I2C_IXR_RX_OVR_Msk | I2C_IXR_TX_OVR_Msk | I2C_IXR_RX_UNF_Msk |          \
     I2C_IXR_TO_Msk)

#define I2C_TIMEOUT_CYCLES (0xFFU)

#define I2C_HOLD_RELEASE (I2C_FIFO_DEPTH - 2U)

#define I2C_POLL_LIMIT (1000000U)

static uint32_t i2c_gate_bit(struct i2c *i2c)
{
    return (i2c == i2c1) ? I2C1_CLK_GATE_BIT : I2C0_CLK_GATE_BIT;
}

static uint32_t i2c_reset_bit(struct i2c *i2c)
{
    return (i2c == i2c1) ? (1U << 1) : (1U << 0);
}

static int i2c_error_pending(struct i2c *i2c)
{
    return ((i2c_irq_status(i2c) & I2C_IXR_ERROR_Msk) != 0U) ? 1 : 0;
}

/* Poll reg until (reg & mask) == value; -1 on bus error or timeout. */
static int i2c_wait_bits(struct i2c *i2c, const volatile uint32_t *reg, uint32_t mask, uint32_t value)
{
    uint32_t polls = I2C_POLL_LIMIT;

    while ((*reg & mask) != value)
    {
        if (i2c_error_pending(i2c) != 0)
        {
            return -1;
        }
        if (--polls == 0U)
        {
            return -1;
        }
    }
    return 0;
}

static int i2c_wait_complete(struct i2c *i2c)
{
    return i2c_wait_bits(i2c, &i2c->isr, I2C_IXR_COMP_Msk, I2C_IXR_COMP_Msk);
}

static int i2c_wait_idle(struct i2c *i2c)
{
    return i2c_wait_bits(i2c, &i2c->sr, I2C_SR_BA_Msk, 0U);
}

static void i2c_release_bus(struct i2c *i2c)
{
    i2c->cr = i2c->cr & ~I2C_CR_HOLD_Msk;
}

static void i2c_abort_transfer(struct i2c *i2c)
{
    i2c->cr = (i2c->cr & ~I2C_CR_HOLD_Msk) | I2C_CR_CLR_FIFO_Msk;
}

static void i2c_cr_write(struct i2c *i2c, uint32_t bits)
{
    uint32_t div = i2c->cr & (I2C_CR_DIV_A_Msk | I2C_CR_DIV_B_Msk);

    i2c->cr = div | bits;
}

/* Drain len bytes, releasing HOLD once the last I2C_HOLD_RELEASE bytes are in
   the FIFO so the transfer can complete. */
static int i2c_read_bytes(struct i2c *i2c, uint8_t *rx, uint32_t len, int hold)
{
    uint32_t got = 0U;
    uint32_t polls = I2C_POLL_LIMIT;

    while (got < len)
    {
        if (i2c_error_pending(i2c) != 0)
        {
            return -1;
        }
        if (!i2c_rx_ready(i2c))
        {
            if (--polls == 0U)
            {
                return -1;
            }
            continue;
        }
        rx[got] = i2c_recv_byte(i2c);
        got++;
        polls = I2C_POLL_LIMIT;
        if ((hold != 0) && ((len - got) <= I2C_HOLD_RELEASE))
        {
            hold = 0;
            i2c_release_bus(i2c);
        }
    }
    return 0;
}

void i2c_enable_clock(struct i2c *i2c)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->aper_clk_ctrl |= i2c_gate_bit(i2c);
    slcr->lock = SLCR_LOCK_KEY;
}

void i2c_disable_clock(struct i2c *i2c)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->aper_clk_ctrl &= ~i2c_gate_bit(i2c);
    slcr->lock = SLCR_LOCK_KEY;
}

void i2c_reset(struct i2c *i2c)
{
    uint32_t bit = i2c_reset_bit(i2c);
    uint32_t i;

    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->i2c_rst_ctrl |= bit;
    for (i = 0U; i < 100U; i++)
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
    uint32_t best_a = 0U;
    uint32_t best_b = 0U;
    uint32_t best = 0U;
    uint32_t actual;

    if ((scl_hz == 0U) || (scl_hz > I2C_SCL_MAX))
    {
        return -1;
    }
    clock_freqs_get(&clocks);
    if (clocks.cpu_1x == 0U)
    {
        return -1;
    }
    for (div_a = 0U; div_a <= 3U; div_a++)
    {
        for (div_b = 0U; div_b <= 63U; div_b++)
        {
            actual = clocks.cpu_1x / (22U * (div_a + 1U) * (div_b + 1U));
            if ((actual <= scl_hz) && (actual > best))
            {
                best = actual;
                best_a = div_a;
                best_b = div_b;
            }
        }
    }
    if (best == 0U)
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

    if ((tx == NULL) || (len == 0U) || (len > I2C_MAX_TRANSFER) || (addr > I2C_ADDR_MAX) || (i2c_bus_active(i2c) != 0))
    {
        return -1;
    }

    hold = (len > I2C_FIFO_DEPTH) ? 1 : 0;
    i2c_cr_write(i2c, I2C_CR_MS_Msk | I2C_CR_NEA_Msk | I2C_CR_ACKEN_Msk | I2C_CR_CLR_FIFO_Msk |
                          ((hold != 0) ? I2C_CR_HOLD_Msk : 0U));
    i2c_irq_clear(i2c, I2C_IXR_ALL_INTR_Msk);

    chunk = (remaining < I2C_FIFO_DEPTH) ? remaining : I2C_FIFO_DEPTH;
    for (i = 0U; i < chunk; i++)
    {
        i2c_send_byte(i2c, tx[i]);
    }
    tx += chunk;
    remaining -= chunk;
    i2c->addr = addr;

    while (remaining > 0U)
    {
        if (i2c_error_pending(i2c) != 0)
        {
            goto fail;
        }
        if (i2c_tx_active(i2c) != 0)
        {
            if (--polls == 0U)
            {
                goto fail;
            }
            continue;
        }
        chunk = (remaining < I2C_FIFO_DEPTH) ? remaining : I2C_FIFO_DEPTH;
        for (i = 0U; i < chunk; i++)
        {
            i2c_send_byte(i2c, tx[i]);
        }
        tx += chunk;
        remaining -= chunk;
        polls = I2C_POLL_LIMIT;
    }

    if (i2c_wait_complete(i2c) != 0)
    {
        goto fail;
    }
    i2c_release_bus(i2c);
    if (i2c_wait_idle(i2c) != 0)
    {
        goto fail;
    }
    return 0;

fail:
    i2c_abort_transfer(i2c);
    return -1;
}

int i2c_read(struct i2c *i2c, uint32_t addr, uint8_t *rx, uint32_t len)
{
    int hold = (len > I2C_FIFO_DEPTH) ? 1 : 0;

    if ((rx == NULL) || (len == 0U) || (len > I2C_MAX_TRANSFER) || (addr > I2C_ADDR_MAX) || (i2c_bus_active(i2c) != 0))
    {
        return -1;
    }

    i2c_cr_write(i2c, I2C_CR_MS_Msk | I2C_CR_NEA_Msk | I2C_CR_ACKEN_Msk | I2C_CR_RD_WR_Msk | I2C_CR_CLR_FIFO_Msk |
                          ((hold != 0) ? I2C_CR_HOLD_Msk : 0U));
    i2c_irq_clear(i2c, I2C_IXR_ALL_INTR_Msk);
    i2c->trans_size = len;
    i2c->addr = addr;

    if (i2c_read_bytes(i2c, rx, len, hold) != 0)
    {
        goto fail;
    }

    if (i2c_wait_complete(i2c) != 0)
    {
        goto fail;
    }
    i2c_release_bus(i2c);
    if (i2c_wait_idle(i2c) != 0)
    {
        goto fail;
    }
    return 0;

fail:
    i2c_abort_transfer(i2c);
    return -1;
}

int i2c_write_read(struct i2c *i2c, uint32_t addr, const uint8_t *tx, uint32_t tx_len, uint8_t *rx, uint32_t rx_len)
{
    uint32_t i;

    if ((tx == NULL) || (rx == NULL) || (tx_len == 0U) || (tx_len > I2C_FIFO_DEPTH) || (rx_len == 0U) ||
        (rx_len > I2C_MAX_TRANSFER) || (addr > I2C_ADDR_MAX) || (i2c_bus_active(i2c) != 0))
    {
        return -1;
    }

    i2c_cr_write(i2c, I2C_CR_MS_Msk | I2C_CR_NEA_Msk | I2C_CR_ACKEN_Msk | I2C_CR_CLR_FIFO_Msk | I2C_CR_HOLD_Msk);
    i2c_irq_clear(i2c, I2C_IXR_ALL_INTR_Msk);
    for (i = 0U; i < tx_len; i++)
    {
        i2c_send_byte(i2c, tx[i]);
    }
    i2c->addr = addr;
    if (i2c_wait_complete(i2c) != 0)
    {
        goto fail;
    }

    i2c_irq_clear(i2c, I2C_IXR_ALL_INTR_Msk);
    i2c_cr_write(i2c, I2C_CR_MS_Msk | I2C_CR_NEA_Msk | I2C_CR_ACKEN_Msk | I2C_CR_RD_WR_Msk | I2C_CR_CLR_FIFO_Msk |
                          I2C_CR_HOLD_Msk);
    i2c->trans_size = rx_len;
    i2c->addr = addr;

    if (i2c_read_bytes(i2c, rx, rx_len, 1) != 0)
    {
        goto fail;
    }

    if (i2c_wait_complete(i2c) != 0)
    {
        goto fail;
    }
    i2c_release_bus(i2c);
    if (i2c_wait_idle(i2c) != 0)
    {
        goto fail;
    }
    return 0;

fail:
    i2c_abort_transfer(i2c);
    return -1;
}
