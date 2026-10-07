/**
 * @file zynq_mio.c
 * @brief MIO/GPIO pin driver: multi-register paths and SLCR writes.
 *        Register primitives are static inline in zynq_mio.h.
 */

#include "zynq_mio.h"

#include <stddef.h>

/* APER_CLK_CTRL bit 22: the GPIO ref clock gate. */
#define MIO_CLK_GATE_BIT (1UL << 22)

/**
 * @brief  Ungate the GPIO ref clock (APER_CLK_CTRL bit 22); call once
 *         before any pin access - gated, every read returns 0.
 */
void mio_enable_clock(void)
{
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->APER_CLK_CTRL |= MIO_CLK_GATE_BIT;
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  Gate the GPIO ref clock again; pin configuration is lost.
 */
void mio_disable_clock(void)
{
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->APER_CLK_CTRL &= ~MIO_CLK_GATE_BIT;
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  Switch a pin to output and drive it to value.
 * @param  pin   GPIO pin, 0..117.
 * @param  value Initial level: 0 = low, else high.
 * @return 0 on success, -1 when pin is out of range.
 *
 * @note Drives the level first, then switches: the other order glitches
 *       the pad with the stale output latch.
 */
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

/**
 * @brief  Switch a pin to input (direction and output driver off).
 * @param  pin GPIO pin, 0..117.
 * @return 0 on success, -1 when pin is out of range.
 */
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

/**
 * @brief  Select the interrupt condition of a pin.
 * @param  pin  GPIO pin, 0..117.
 * @param  type MIO_IRQ_TYPE_*; an unknown value changes nothing.
 */
void mio_set_irq_type(uint32_t pin, uint32_t type)
{
    uint32_t bank = MIO_BANK(pin);
    uint32_t bit = MIO_BIT(pin);
    uint32_t int_type;
    uint32_t int_polarity;
    uint32_t int_any;

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

    *mio_reg_int_type(bank) = (*mio_reg_int_type(bank) & ~bit) | ((int_type != 0U) ? bit : 0UL);
    *mio_reg_int_pol(bank) = (*mio_reg_int_pol(bank) & ~bit) | ((int_polarity != 0U) ? bit : 0UL);
    *mio_reg_int_any(bank) = (*mio_reg_int_any(bank) & ~bit) | ((int_any != 0U) ? bit : 0UL);
}

/* ------------------------------------------------------------------ */
/* MIO pad configuration (SLCR.MIO_PIN_xx)                            */
/* ------------------------------------------------------------------ */
/* MIO pads bank 0 covers MIO 0..15, bank 1 MIO 16..53 (UG585 ch.6). */
#define MIO_BANK1_FIRST_PAD (16U)

/**
 * @brief  Get the pad-control register of an MIO pad.
 * @param  pad MIO pad, 0..53.
 * @return Pointer to SLCR.MIO_PIN_xx, or NULL when out of range.
 */
static volatile uint32_t *mio_pad_reg(uint32_t pad)
{
    if (pad >= MIO_PAD_COUNT)
    {
        return NULL;
    }
    return &SLCR->MIO_PIN_00 + pad;
}

/**
 * @brief  Read-modify-write one pad register inside the SLCR unlock /
 *         lock window; bits outside mask survive.
 * @param  pad   MIO pad, 0..53.
 * @param  mask  Bits to replace.
 * @param  value Replacement bits, already shifted into position.
 * @return 0 on success, -1 when out of range.
 */
static int mio_pad_rmw(uint32_t pad, uint32_t mask, uint32_t value)
{
    volatile uint32_t *reg = mio_pad_reg(pad);

    if (reg == NULL)
    {
        return -1;
    }

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    *reg = (*reg & ~mask) | value;
    SLCR->LOCK = SLCR_LOCK_KEY;
    return 0;
}

/**
 * @brief  Route a pad to one of its functions.
 * @param  pad MIO pad, 0..53.
 * @param  fn  One of the MIOxx_* selects; the bank IO level is added.
 * @return 0 on success, -1 when out of range.
 *
 * @note Plain store - pad options revert to reset values, set them
 *       again afterwards.
 */
int mio_set_function(uint32_t pad, uint32_t fn)
{
    volatile uint32_t *reg = mio_pad_reg(pad);

    if (reg == NULL)
    {
        return -1;
    }

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    *reg = fn | ((pad < MIO_BANK1_FIRST_PAD) ? MIO_BANK0_LVL : MIO_BANK1_LVL);
    SLCR->LOCK = SLCR_LOCK_KEY;
    return 0;
}

/**
 * @brief  Enable the pad's weak pull-up.
 * @param  pad MIO pad, 0..53.
 * @return 0 on success, -1 when out of range.
 */
int mio_enable_pull_up(uint32_t pad)
{
    return mio_pad_rmw(pad, MIO_PIN_OPT_ENA_PULLUP, MIO_PIN_OPT_ENA_PULLUP);
}

/**
 * @brief  Disable the pad's weak pull-up.
 * @param  pad MIO pad, 0..53.
 * @return 0 on success, -1 when out of range.
 */
int mio_disable_pull_up(uint32_t pad)
{
    return mio_pad_rmw(pad, MIO_PIN_OPT_ENA_PULLUP, 0UL);
}

/**
 * @brief  Enable the pad's input receiver.
 * @param  pad MIO pad, 0..53.
 * @return 0 on success, -1 when out of range.
 */
int mio_enable_receiver(uint32_t pad)
{
    return mio_pad_rmw(pad, MIO_PIN_OPT_DIS_RCVR, 0UL);
}

/**
 * @brief  Disable the pad's input receiver (saves power on pins that
 *         are output-only).
 * @param  pad MIO pad, 0..53.
 * @return 0 on success, -1 when out of range.
 */
int mio_disable_receiver(uint32_t pad)
{
    return mio_pad_rmw(pad, MIO_PIN_OPT_DIS_RCVR, MIO_PIN_OPT_DIS_RCVR);
}

/**
 * @brief  Select the fast slew rate.
 * @param  pad MIO pad, 0..53.
 * @return 0 on success, -1 when out of range.
 */
int mio_set_slew_fast(uint32_t pad)
{
    return mio_pad_rmw(pad, MIO_PIN_OPT_ENA_SPEED_FAST, MIO_PIN_OPT_ENA_SPEED_FAST);
}

/**
 * @brief  Select the slow slew rate (reset default).
 * @param  pad MIO pad, 0..53.
 * @return 0 on success, -1 when out of range.
 */
int mio_set_slew_slow(uint32_t pad)
{
    return mio_pad_rmw(pad, MIO_PIN_OPT_ENA_SPEED_FAST, 0UL);
}
