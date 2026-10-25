#include "zynq_mio.h"

#include <stddef.h>

#define MIO_CLK_GATE_BIT (1U << 22)

static volatile uint32_t *mio_reg_at(uint32_t byte_offset)
{
    return &((volatile uint32_t *)(uintptr_t)gpio)[byte_offset / 4U];
}

static volatile uint32_t *mio_reg_bank(uint32_t byte_offset, uint32_t bank)
{
    return mio_reg_at(byte_offset + (0x040U * bank));
}

static volatile uint32_t *mio_reg_mdata(uint32_t bank, uint32_t half)
{
    return mio_reg_at((0x008U * bank) + (0x004U * half));
}

static volatile uint32_t *mio_reg_data(uint32_t bank)
{
    return mio_reg_at(0x040U + (0x004U * bank));
}

static volatile uint32_t *mio_reg_data_ro(uint32_t bank)
{
    return mio_reg_at(0x060U + (0x004U * bank));
}

static volatile uint32_t *mio_reg_dirm(uint32_t bank)
{
    return mio_reg_bank(0x204U, bank);
}

static volatile uint32_t *mio_reg_oen(uint32_t bank)
{
    return mio_reg_bank(0x208U, bank);
}

static volatile uint32_t *mio_reg_int_mask(uint32_t bank)
{
    return mio_reg_bank(0x20CU, bank);
}

static volatile uint32_t *mio_reg_int_en(uint32_t bank)
{
    return mio_reg_bank(0x210U, bank);
}

static volatile uint32_t *mio_reg_int_dis(uint32_t bank)
{
    return mio_reg_bank(0x214U, bank);
}

static volatile uint32_t *mio_reg_int_stat(uint32_t bank)
{
    return mio_reg_bank(0x218U, bank);
}

static volatile uint32_t *mio_reg_int_type(uint32_t bank)
{
    return mio_reg_bank(0x21CU, bank);
}

static volatile uint32_t *mio_reg_int_pol(uint32_t bank)
{
    return mio_reg_bank(0x220U, bank);
}

static volatile uint32_t *mio_reg_int_any(uint32_t bank)
{
    return mio_reg_bank(0x224U, bank);
}

#define MIO_BANK(pin)     (((pin) < MIO_PAD_COUNT) ? ((pin) >> 5U) : (2U + (((pin) - (uint32_t)MIO_PAD_COUNT) >> 5U)))
#define MIO_BANK_BIT(pin) (((pin) < MIO_PAD_COUNT) ? ((pin) & 0x1FU) : (((pin) - (uint32_t)MIO_PAD_COUNT) & 0x1FU))
#define MIO_BIT(pin)      (1U << MIO_BANK_BIT(pin))

#define MIO_MDATA_HALF(pin) ((MIO_BANK_BIT(pin) >> 4U) & 0x1U)
#define MIO_MDATA_BIT(pin)  (1U << (MIO_BANK_BIT(pin) & 0xFU))

bool mio_is_output(uint32_t pin)
{
    return (*mio_reg_dirm(MIO_BANK(pin)) & MIO_BIT(pin)) != 0U;
}

bool mio_is_driver_enabled(uint32_t pin)
{
    return (*mio_reg_oen(MIO_BANK(pin)) & MIO_BIT(pin)) != 0U;
}

void mio_enable_driver(uint32_t pin)
{
    *mio_reg_oen(MIO_BANK(pin)) |= MIO_BIT(pin);
}

void mio_disable_driver(uint32_t pin)
{
    *mio_reg_oen(MIO_BANK(pin)) &= ~MIO_BIT(pin);
}

void mio_set_value(uint32_t pin, int value)
{
    uint32_t bit = MIO_MDATA_BIT(pin);

    *mio_reg_mdata(MIO_BANK(pin), MIO_MDATA_HALF(pin)) = ((0xFFFFU ^ bit) << 16) | ((value != 0) ? bit : 0U);
}

void mio_set_high(uint32_t pin)
{
    uint32_t bit = MIO_MDATA_BIT(pin);

    *mio_reg_mdata(MIO_BANK(pin), MIO_MDATA_HALF(pin)) = ((0xFFFFU ^ bit) << 16) | bit;
}

void mio_set_low(uint32_t pin)
{
    uint32_t bit = MIO_MDATA_BIT(pin);

    *mio_reg_mdata(MIO_BANK(pin), MIO_MDATA_HALF(pin)) = ((0xFFFFU ^ bit) << 16);
}

void mio_toggle(uint32_t pin)
{
    uint32_t bank = MIO_BANK(pin);
    uint32_t bit = MIO_MDATA_BIT(pin);
    uint32_t cur = (*mio_reg_data(bank) >> MIO_BANK_BIT(pin)) & 1U;

    *mio_reg_mdata(bank, MIO_MDATA_HALF(pin)) = ((0xFFFFU ^ bit) << 16) | ((cur != 0U) ? 0U : bit);
}

int mio_get_output_latch(uint32_t pin)
{
    return ((*mio_reg_data(MIO_BANK(pin)) & MIO_BIT(pin)) != 0U) ? 1 : 0;
}

int mio_get_value(uint32_t pin)
{
    return (int)((*mio_reg_data_ro(MIO_BANK(pin)) >> MIO_BANK_BIT(pin)) & 1U);
}

void mio_irq_enable(uint32_t pin)
{
    *mio_reg_int_en(MIO_BANK(pin)) = MIO_BIT(pin);
}

void mio_irq_disable(uint32_t pin)
{
    *mio_reg_int_dis(MIO_BANK(pin)) = MIO_BIT(pin);
}

bool mio_irq_enabled(uint32_t pin)
{
    return (*mio_reg_int_mask(MIO_BANK(pin)) & MIO_BIT(pin)) != 0U;
}

bool mio_irq_pending(uint32_t pin)
{
    return (*mio_reg_int_stat(MIO_BANK(pin)) & MIO_BIT(pin)) != 0U;
}

void mio_irq_clear(uint32_t pin)
{
    *mio_reg_int_stat(MIO_BANK(pin)) = MIO_BIT(pin);
}

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
