
#include "zynq_spi.h"

#include <stddef.h>

#include "zynq_clock.h"

#define SPI0_CLK_GATE_BIT (1u << 14)
#define SPI1_CLK_GATE_BIT (1u << 15)

#define SPI_CLK_DIVISOR_5 (5u << 8)

#define SPI_POLL_LIMIT (1000000u)

static uint32_t gate_bit(struct spi *spi)
{
    return (spi == spi1) ? SPI1_CLK_GATE_BIT : SPI0_CLK_GATE_BIT;
}

static uint32_t reset_bits(struct spi *spi)
{
    return (spi == spi1) ? ((1u << 3) | (1u << 1)) : ((1u << 2) | (1u << 0));
}

void spi_enable_clock(struct spi *spi)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->aper_clk_ctrl |= gate_bit(spi);
    slcr->spi_clk_ctrl = (slcr->spi_clk_ctrl & 0x3u) | SPI_CLK_DIVISOR_5 | ((spi == spi1) ? 2u : 1u);
    slcr->lock = SLCR_LOCK_KEY;
}

void spi_disable_clock(struct spi *spi)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->aper_clk_ctrl &= ~gate_bit(spi);
    slcr->spi_clk_ctrl &= ~((spi == spi1) ? 2u : 1u);
    slcr->lock = SLCR_LOCK_KEY;
}

void spi_reset(struct spi *spi)
{
    uint32_t bits = reset_bits(spi);
    uint32_t i;

    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->spi_rst_ctrl |= bits;
    for (i = 0u; i < 100u; i++)
    {
        asm volatile("nop");
    }
    slcr->spi_rst_ctrl &= ~bits;
    slcr->lock = SLCR_LOCK_KEY;
}

int spi_set_sclk(struct spi *spi, uint32_t sclk_hz)
{
    struct clock_freqs clocks;
    uint32_t baud_div;

    if (sclk_hz == 0u)
    {
        return -1;
    }
    clock_freqs_get(&clocks);
    if (clocks.spi_ref == 0u)
    {
        return -1;
    }

    for (baud_div = 1u; baud_div <= SPI_CR_PRESC_MAXIMUM; baud_div++)
    {
        if ((clocks.spi_ref >> (baud_div + 1u)) <= sclk_hz)
        {
            spi_set_baud_div(spi, baud_div);
            return 0;
        }
    }
    return -1;
}

int spi_init(struct spi *spi, uint32_t mode, uint32_t sclk_hz)
{
    uint32_t cr;

    spi_enable_clock(spi);
    spi_reset(spi);

    cr = SPI_CR_MSTREN_Msk | SPI_CR_SSFORCE_Msk | SPI_CR_SSCTRL_Msk;
    if ((mode & 1u) != 0u)
    {
        cr |= SPI_CR_CPHA_Msk;
    }
    if ((mode & 2u) != 0u)
    {
        cr |= SPI_CR_CPOL_Msk;
    }
    spi->cr = cr;

    if (spi_set_sclk(spi, sclk_hz) != 0)
    {
        return -1;
    }

    spi_set_tx_watermark(spi, 1u);
    spi_set_rx_watermark(spi, 1u);
    spi_irq_disable(spi, SPI_IXR_MASK_Msk);
    spi_irq_clear(spi, SPI_IXR_TXUF_Msk | SPI_IXR_MODF_Msk | SPI_IXR_RXOVR_Msk);
    spi_enable(spi);
    return 0;
}

int spi_transfer(struct spi *spi, const uint8_t *tx, uint8_t *rx, uint32_t len)
{
    uint8_t sink[SPI_FIFO_DEPTH];
    uint32_t chunk;
    uint32_t i;
    uint32_t polls;

    while (len > 0u)
    {
        uint8_t *dst = (rx != NULL) ? rx : sink;

        chunk = (len < SPI_FIFO_DEPTH) ? len : SPI_FIFO_DEPTH;

        polls = SPI_POLL_LIMIT;
        while (spi_tx_empty(spi) == 0)
        {
            if (--polls == 0u)
            {
                return -1;
            }
        }
        if (tx != NULL)
        {
            for (i = 0u; i < chunk; i++)
            {
                spi_send_byte(spi, tx[i]);
            }
        }
        else
        {
            for (i = 0u; i < chunk; i++)
            {
                spi_send_byte(spi, (uint8_t)SPI_DUMMY_TX);
            }
        }
        for (i = 0u; i < chunk; i++)
        {
            polls = SPI_POLL_LIMIT;
            while (spi_rx_ready(spi) == 0)
            {
                if (--polls == 0u)
                {
                    return -1;
                }
            }
            dst[i] = spi_recv_byte(spi);
        }
        if (tx != NULL)
        {
            tx += chunk;
        }
        if (rx != NULL)
        {
            rx += chunk;
        }
        len -= chunk;
    }
    return 0;
}
