#ifndef ZYNQ_SPI_H
#define ZYNQ_SPI_H

#include "zynq_soc.h"

#define SPI_MODE_0 (0u)
#define SPI_MODE_1 (1u)
#define SPI_MODE_2 (2u)
#define SPI_MODE_3 (3u)

#define SPI_CS0 (0u)
#define SPI_CS1 (1u)
#define SPI_CS2 (2u)

#define SPI_DUMMY_TX (0x00u)

static inline void spi_send_byte(struct spi *spi, uint8_t c)
{
    spi->txd = (uint32_t)c;
}

static inline uint8_t spi_recv_byte(struct spi *spi)
{
    return (uint8_t)(spi->rxd & 0xFFu);
}

static inline int spi_tx_empty(struct spi *spi)
{
    return ((spi->sr & SPI_IXR_TXOW_Msk) != 0u) ? 1 : 0;
}

static inline int spi_rx_ready(struct spi *spi)
{
    return ((spi->sr & SPI_IXR_RXNEMPTY_Msk) != 0u) ? 1 : 0;
}

static inline void spi_select(struct spi *spi, uint32_t cs)
{
    spi->cr = (spi->cr & ~SPI_CR_SSCTRL_Msk) | (SPI_CR_SSCTRL_Msk & ~(1u << (SPI_CR_SSCTRL_Pos + cs)));
}

static inline void spi_deselect(struct spi *spi)
{
    spi->cr = (spi->cr & ~SPI_CR_SSCTRL_Msk) | SPI_CR_SSCTRL_Msk;
}

static inline void spi_enable(struct spi *spi)
{
    spi->er = SPI_ER_ENABLE_Msk;
}

static inline void spi_disable(struct spi *spi)
{
    spi->er = 0u;
}

static inline uint32_t spi_irq_status(struct spi *spi)
{
    return (spi->sr & SPI_IXR_MASK_Msk);
}

static inline uint32_t spi_irq_mask(struct spi *spi)
{
    return (spi->imr & SPI_IXR_MASK_Msk);
}

static inline void spi_irq_enable(struct spi *spi, uint32_t mask)
{
    spi->ier = (mask & SPI_IXR_MASK_Msk);
}

static inline void spi_irq_disable(struct spi *spi, uint32_t mask)
{
    spi->idr = (mask & SPI_IXR_MASK_Msk);
}

static inline void spi_irq_clear(struct spi *spi, uint32_t mask)
{
    spi->sr = (mask & SPI_IXR_MASK_Msk);
}

static inline void spi_set_tx_watermark(struct spi *spi, uint32_t n)
{
    spi->txwr = (n & SPI_TXWR_MASK_Msk);
}

static inline void spi_set_rx_watermark(struct spi *spi, uint32_t n)
{
    spi->rxwr = (n & SPI_RXWR_MASK_Msk);
}

static inline void spi_set_baud_div(struct spi *spi, uint32_t baud_div)
{
    spi->cr = (spi->cr & ~SPI_CR_PRESC_Msk) | ((baud_div << SPI_CR_PRESC_Pos) & SPI_CR_PRESC_Msk);
}

void spi_enable_clock(struct spi *spi);

void spi_disable_clock(struct spi *spi);

void spi_reset(struct spi *spi);

int spi_set_sclk(struct spi *spi, uint32_t sclk_hz);

int spi_init(struct spi *spi, uint32_t mode, uint32_t sclk_hz);

int spi_transfer(struct spi *spi, const uint8_t *tx, uint8_t *rx, uint32_t len);

#endif
