#include "zynq_mio.h"

#include <stddef.h>

#define MIO_CLK_GATE_BIT (1U << 22)

void mio_enable_clock(void)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->aper_clk_ctrl |= MIO_CLK_GATE_BIT;
    slcr->lock = SLCR_LOCK_KEY;
}

void mio_disable_clock(void)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->aper_clk_ctrl &= ~MIO_CLK_GATE_BIT;
    slcr->lock = SLCR_LOCK_KEY;
}

int mio_direction_output(uint32_t pin, int value)
{
    uint32_t bank;
    uint32_t bit;

    if (pin >= MIO_PIN_COUNT)
    {
        return -1;
    }
    bank = MIO_BANK(pin);
    bit = MIO_BIT(pin);

    mio_set_value(pin, value);
    *mio_reg_dirm(bank) |= bit;
    *mio_reg_oen(bank) |= bit;
    return 0;
}

int mio_direction_input(uint32_t pin)
{
    uint32_t bank;
    uint32_t bit;

    if (pin >= MIO_PIN_COUNT)
    {
        return -1;
    }
    bank = MIO_BANK(pin);
    bit = MIO_BIT(pin);

    *mio_reg_dirm(bank) &= ~bit;
    *mio_reg_oen(bank) &= ~bit;
    return 0;
}

void mio_set_irq_type(uint32_t pin, uint32_t type)
{
    uint32_t bank;
    uint32_t bit;
    uint32_t int_type;
    uint32_t int_polarity;
    uint32_t int_any;

    if (pin >= MIO_PIN_COUNT)
    {
        return;
    }
    bank = MIO_BANK(pin);
    bit = MIO_BIT(pin);

    switch (type)
    {
    case MIO_IRQ_TYPE_LEVEL_LOW:
        int_type = 0U;
        int_polarity = 0U;
        int_any = 0U;
        break;
    case MIO_IRQ_TYPE_LEVEL_HIGH:
        int_type = 0U;
        int_polarity = 1U;
        int_any = 0U;
        break;
    case MIO_IRQ_TYPE_EDGE_FALLING:
        int_type = 1U;
        int_polarity = 0U;
        int_any = 0U;
        break;
    case MIO_IRQ_TYPE_EDGE_RISING:
        int_type = 1U;
        int_polarity = 1U;
        int_any = 0U;
        break;
    case MIO_IRQ_TYPE_EDGE_BOTH:
        int_type = 1U;
        int_polarity = 1U;
        int_any = 1U;
        break;
    default:
        return;
    }

    *mio_reg_int_type(bank) = (*mio_reg_int_type(bank) & ~bit) | ((int_type != 0U) ? bit : 0U);
    *mio_reg_int_pol(bank) = (*mio_reg_int_pol(bank) & ~bit) | ((int_polarity != 0U) ? bit : 0U);
    *mio_reg_int_any(bank) = (*mio_reg_int_any(bank) & ~bit) | ((int_any != 0U) ? bit : 0U);
}

#define MIO_BANK1_FIRST_PAD (16U)

static volatile uint32_t *mio_pad_reg(uint32_t pad)
{
    if (pad >= MIO_PAD_COUNT)
    {
        return NULL;
    }
    return &slcr->mio_pin_00 + pad;
}

static int mio_pad_rmw(uint32_t pad, uint32_t mask, uint32_t value)
{
    volatile uint32_t *reg = mio_pad_reg(pad);

    if (reg == NULL)
    {
        return -1;
    }

    slcr->unlock = SLCR_UNLOCK_KEY;
    *reg = (*reg & ~mask) | value;
    slcr->lock = SLCR_LOCK_KEY;
    return 0;
}

int mio_set_function(uint32_t pad, uint32_t fn)
{
    volatile uint32_t *reg = mio_pad_reg(pad);

    if (reg == NULL)
    {
        return -1;
    }

    slcr->unlock = SLCR_UNLOCK_KEY;
    *reg = fn | ((pad < MIO_BANK1_FIRST_PAD) ? MIO_BANK0_LVL : MIO_BANK1_LVL);
    slcr->lock = SLCR_LOCK_KEY;
    return 0;
}

int mio_enable_pull_up(uint32_t pad)
{
    return mio_pad_rmw(pad, MIO_PIN_OPT_ENA_PULLUP, MIO_PIN_OPT_ENA_PULLUP);
}

int mio_disable_pull_up(uint32_t pad)
{
    return mio_pad_rmw(pad, MIO_PIN_OPT_ENA_PULLUP, 0U);
}

int mio_enable_receiver(uint32_t pad)
{
    return mio_pad_rmw(pad, MIO_PIN_OPT_DIS_RCVR, 0U);
}

int mio_disable_receiver(uint32_t pad)
{
    return mio_pad_rmw(pad, MIO_PIN_OPT_DIS_RCVR, MIO_PIN_OPT_DIS_RCVR);
}

int mio_set_slew_fast(uint32_t pad)
{
    return mio_pad_rmw(pad, MIO_PIN_OPT_ENA_SPEED_FAST, MIO_PIN_OPT_ENA_SPEED_FAST);
}

int mio_set_slew_slow(uint32_t pad)
{
    return mio_pad_rmw(pad, MIO_PIN_OPT_ENA_SPEED_FAST, 0U);
}
