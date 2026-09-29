/**
 * @file zynq_gpio.c
 * @brief PS GPIO driver implementation (UG585 ch.14).
 *
 * Only the pieces that cannot be a single register access live here -
 * clock gating, direction+driver handover and the SLCR pad controls,
 * which need the SLCR unlock/lock key around every write. Register
 * level primitives are static inline in zynq_gpio.h.
 */

#include "zynq_gpio.h"

#include <stddef.h>

/* APER_CLK_CTRL bit 22: the GPIO ref clock gate. */
#define APER_GPIO_CLK (1UL << 22)

/* Number of MIO pads (0..53); 54..117 are EMIO and need no pinmux. */
#define MIO_PIN_NR (54U)

/* MIO pads bank 0 covers MIO 0..15, bank 1 MIO 16..53 (UG585 ch.6). */
#define MIO_BANK1_PIN (16U)

/**
 * @brief  Get the SLCR pin-control register of an MIO pad.
 * @param  mio MIO pin index, 0..53.
 * @return Pointer to SLCR.MIO_PIN_xx, or NULL when mio is not an MIO pin.
 *
 * SLCR.MIO_PIN_00 .. MIO_PIN_53 are consecutive, so the pad's control
 * register is one index away.
 */
static volatile uint32_t* mio_reg(uint32_t mio) {
    if (mio >= MIO_PIN_NR) {
        return NULL;
    }
    return &SLCR->MIO_PIN_00 + mio;
}

/**
 * @brief  Read-modify-write one MIO pad register.
 * @param  mio   MIO pin index, 0..53.
 * @param  mask  Bits to replace.
 * @param  value Replacement bits, already shifted into position.
 * @return 0 on success, -1 when mio is not an MIO pin.
 *
 * SLCR accepts writes only between the unlock and lock keys, and every
 * bit outside 'mask' has to survive.
 */
static int mio_rmw(uint32_t mio, uint32_t mask, uint32_t value) {
    volatile uint32_t* reg = mio_reg(mio);

    if (reg == NULL) {
        return -1;
    }

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    *reg = (*reg & ~mask) | value;
    SLCR->LOCK = SLCR_LOCK_KEY;
    return 0;
}

/**
 * @brief  Ungate the GPIO reference clock (SLCR APER_CLK_CTRL bit 22).
 *
 * Call once before any pin access: while gated, every GPIO read returns
 * 0.
 */
void gpio_enable_clock(void) {
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->APER_CLK_CTRL |= APER_GPIO_CLK;
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  Gate the GPIO reference clock again.
 *
 * Pin configuration and output state are lost.
 */
void gpio_disable_clock(void) {
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->APER_CLK_CTRL &= ~APER_GPIO_CLK;
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  Switch a pin to output and drive it to value.
 * @param  pin   Global GPIO number, 0..117.
 * @param  value Initial level: 0 = low, any other value = high.
 * @return 0 on success, -1 when pin is out of range.
 *
 * Sets the level first and switches the pin over afterwards: the other
 * order drives the pad with whatever the output latch held before,
 * which shows up as a glitch on the scope.
 */
int gpio_direction_output(uint32_t pin, int value) {
    uint32_t bank;
    uint32_t bit;

    if (pin >= GPIO_PIN_NR) {
        return -1;
    }
    bank = GPIO_BANK(pin);
    bit = GPIO_BIT(pin);

    gpio_set_value(pin, value);
    GPIO_REGW(GPIO_W_DIRM(bank)) |= bit;
    GPIO_REGW(GPIO_W_OEN(bank)) |= bit;
    return 0;
}

/**
 * @brief  Switch a pin to input (direction and output driver off).
 * @param  pin Global GPIO number, 0..117.
 * @return 0 on success, -1 when pin is out of range.
 */
int gpio_direction_input(uint32_t pin) {
    uint32_t bank;
    uint32_t bit;

    if (pin >= GPIO_PIN_NR) {
        return -1;
    }
    bank = GPIO_BANK(pin);
    bit = GPIO_BIT(pin);

    GPIO_REGW(GPIO_W_DIRM(bank)) &= ~bit;
    GPIO_REGW(GPIO_W_OEN(bank)) &= ~bit;
    return 0;
}

/**
 * @brief  Select the interrupt condition of a pin.
 * @param  pin  Global GPIO number, 0..117.
 * @param  type One of the GPIO_IRQ_TYPE_* values: LEVEL_LOW,
 *              LEVEL_HIGH, EDGE_FALLING, EDGE_RISING, EDGE_BOTH.
 *
 * Expands the single choice onto the INT_TYPE / INT_POLARITY / INT_ANY
 * registers; an unknown value leaves all three untouched.
 */
void gpio_set_irq_type(uint32_t pin, uint32_t type) {
    uint32_t bank = GPIO_BANK(pin);
    uint32_t bit = GPIO_BIT(pin);
    uint32_t edge;
    uint32_t high;
    uint32_t any;

    switch (type) {
        case GPIO_IRQ_TYPE_LEVEL_LOW:
            edge = 0U;
            high = 0U;
            any = 0U;
            break;
        case GPIO_IRQ_TYPE_LEVEL_HIGH:
            edge = 0U;
            high = 1U;
            any = 0U;
            break;
        case GPIO_IRQ_TYPE_EDGE_FALLING:
            edge = 1U;
            high = 0U;
            any = 0U;
            break;
        case GPIO_IRQ_TYPE_EDGE_RISING:
            edge = 1U;
            high = 1U;
            any = 0U;
            break;
        case GPIO_IRQ_TYPE_EDGE_BOTH:
            edge = 1U;
            high = 1U;
            any = 1U;
            break;
        default:
            return;
    }

    GPIO_REGW(GPIO_W_INT_TYPE(bank)) =
        (GPIO_REGW(GPIO_W_INT_TYPE(bank)) & ~bit) | ((edge != 0U) ? bit : 0UL);
    GPIO_REGW(GPIO_W_INT_POL(bank)) =
        (GPIO_REGW(GPIO_W_INT_POL(bank)) & ~bit) | ((high != 0U) ? bit : 0UL);
    GPIO_REGW(GPIO_W_INT_ANY(bank)) =
        (GPIO_REGW(GPIO_W_INT_ANY(bank)) & ~bit) | ((any != 0U) ? bit : 0UL);
}

/**
 * @brief  Route an MIO pin to one of its functions.
 * @param  mio MIO pin index, 0..53.
 * @param  fn  Function encoding, one of the GPIO_MIOxx_* selects; the
 *             bank's IO level (LVL) is added by this function.
 * @return 0 on success, -1 when mio is not an MIO pin.
 *
 * A plain store, so slew rate, receiver and pull-up revert to their
 * reset values - set those with their own calls afterwards.
 */
int mio_set_function(uint32_t mio, uint32_t fn) {
    volatile uint32_t* reg = mio_reg(mio);

    if (reg == NULL) {
        return -1;
    }

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    *reg = fn | ((mio < MIO_BANK1_PIN) ? GPIO_MIO0_LVL : GPIO_MIO1_LVL);
    SLCR->LOCK = SLCR_LOCK_KEY;
    return 0;
}

/**
 * @brief  Enable an MIO pad's weak pull-up.
 * @param  mio MIO pin index, 0..53.
 * @return 0 on success, -1 when mio is not an MIO pin.
 *
 * The Zynq MIO pads have a weak pull-up only, no pull-down (UG585
 * ch.6): without it the pad floats.
 */
int mio_enable_pull_up(uint32_t mio) {
    return mio_rmw(mio, MIO_PIN_OPT_ENA_PULLUP, MIO_PIN_OPT_ENA_PULLUP);
}

/**
 * @brief  Disable an MIO pad's weak pull-up.
 * @param  mio MIO pin index, 0..53.
 * @return 0 on success, -1 when mio is not an MIO pin.
 */
int mio_disable_pull_up(uint32_t mio) {
    return mio_rmw(mio, MIO_PIN_OPT_ENA_PULLUP, 0UL);
}

/**
 * @brief  Enable an MIO pad's input receiver.
 * @param  mio MIO pin index, 0..53.
 * @return 0 on success, -1 when mio is not an MIO pin.
 */
int mio_enable_receiver(uint32_t mio) {
    return mio_rmw(mio, MIO_PIN_OPT_DIS_RCVR, 0UL);
}

/**
 * @brief  Disable an MIO pad's input receiver.
 * @param  mio MIO pin index, 0..53.
 * @return 0 on success, -1 when mio is not an MIO pin.
 *
 * Saves a little power on a pin that is output-only.
 */
int mio_disable_receiver(uint32_t mio) {
    return mio_rmw(mio, MIO_PIN_OPT_DIS_RCVR, MIO_PIN_OPT_DIS_RCVR);
}

/**
 * @brief  Select the fast slew rate on an MIO pad.
 * @param  mio MIO pin index, 0..53.
 * @return 0 on success, -1 when mio is not an MIO pin.
 */
int mio_set_slew_fast(uint32_t mio) {
    return mio_rmw(mio, MIO_PIN_OPT_ENA_SPEED_FAST, MIO_PIN_OPT_ENA_SPEED_FAST);
}

/**
 * @brief  Select the slow slew rate on an MIO pad (reset default).
 * @param  mio MIO pin index, 0..53.
 * @return 0 on success, -1 when mio is not an MIO pin.
 */
int mio_set_slew_slow(uint32_t mio) {
    return mio_rmw(mio, MIO_PIN_OPT_ENA_SPEED_FAST, 0UL);
}
