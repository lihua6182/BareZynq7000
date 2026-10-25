#ifndef ZYNQ_I2C_H
#define ZYNQ_I2C_H

#include "zynq_soc.h"

#define I2C_SCL_MAX (400000U)

#define I2C_MAX_TRANSFER (0xFFU)

#define I2C_ADDR_MAX (0x7FU)

static inline void i2c_send_byte(struct i2c *i2c, uint8_t c)
{
    i2c->data = (uint32_t)c;
}

static inline uint8_t i2c_recv_byte(struct i2c *i2c)
{
    return (uint8_t)(i2c->data & 0xFFU);
}

static inline bool i2c_rx_ready(struct i2c *i2c)
{
    return (i2c->sr & I2C_SR_RXDV_Msk) != 0U;
}

static inline bool i2c_tx_active(struct i2c *i2c)
{
    return (i2c->sr & I2C_SR_TXDV_Msk) != 0U;
}

static inline bool i2c_bus_active(struct i2c *i2c)
{
    return (i2c->sr & I2C_SR_BA_Msk) != 0U;
}

static inline uint32_t i2c_irq_status(struct i2c *i2c)
{
    return (i2c->isr & I2C_IXR_ALL_INTR_Msk);
}

static inline uint32_t i2c_irq_mask(struct i2c *i2c)
{
    return (i2c->imr & I2C_IXR_ALL_INTR_Msk);
}

static inline void i2c_irq_enable(struct i2c *i2c, uint32_t mask)
{
    i2c->ier = (mask & I2C_IXR_ALL_INTR_Msk);
}

static inline void i2c_irq_disable(struct i2c *i2c, uint32_t mask)
{
    i2c->idr = (mask & I2C_IXR_ALL_INTR_Msk);
}

static inline void i2c_irq_clear(struct i2c *i2c, uint32_t mask)
{
    i2c->isr = (mask & I2C_IXR_ALL_INTR_Msk);
}

void i2c_enable_clock(struct i2c *i2c);

void i2c_disable_clock(struct i2c *i2c);

void i2c_reset(struct i2c *i2c);

int i2c_set_scl(struct i2c *i2c, uint32_t scl_hz);

int i2c_init(struct i2c *i2c, uint32_t scl_hz);

int i2c_write(struct i2c *i2c, uint32_t addr, const uint8_t *tx, uint32_t len);

int i2c_read(struct i2c *i2c, uint32_t addr, uint8_t *rx, uint32_t len);

int i2c_write_read(struct i2c *i2c, uint32_t addr, const uint8_t *tx, uint32_t tx_len, uint8_t *rx, uint32_t rx_len);

#endif
