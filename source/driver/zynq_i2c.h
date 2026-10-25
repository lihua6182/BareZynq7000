#ifndef ZYNQ_I2C_H
#define ZYNQ_I2C_H

#include "zynq_soc.h"

#define I2C_SCL_MAX (400000U)

#define I2C_MAX_TRANSFER (0xFFU)

#define I2C_ADDR_MAX (0x7FU)

void i2c_send_byte(struct i2c *i2c, uint8_t c);

uint8_t i2c_recv_byte(struct i2c *i2c);

bool i2c_rx_ready(struct i2c *i2c);

bool i2c_tx_active(struct i2c *i2c);

bool i2c_bus_active(struct i2c *i2c);

uint32_t i2c_irq_status(struct i2c *i2c);

uint32_t i2c_irq_mask(struct i2c *i2c);

void i2c_irq_enable(struct i2c *i2c, uint32_t mask);

void i2c_irq_disable(struct i2c *i2c, uint32_t mask);

void i2c_irq_clear(struct i2c *i2c, uint32_t mask);

void i2c_enable_clock(struct i2c *i2c);

void i2c_disable_clock(struct i2c *i2c);

void i2c_reset(struct i2c *i2c);

int i2c_set_scl(struct i2c *i2c, uint32_t scl_hz);

int i2c_init(struct i2c *i2c, uint32_t scl_hz);

int i2c_write(struct i2c *i2c, uint32_t addr, const uint8_t *tx, uint32_t len);

int i2c_read(struct i2c *i2c, uint32_t addr, uint8_t *rx, uint32_t len);

int i2c_write_read(struct i2c *i2c, uint32_t addr, const uint8_t *tx, uint32_t tx_len, uint8_t *rx, uint32_t rx_len);

#endif
