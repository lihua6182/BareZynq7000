#include "board.h"

#include "zynq_irq.h"
#include "zynq_mio.h"
#include "zynq_soc.h"
#include "zynq_tick.h"

extern void USBD_IRQHandler(uint8_t busid);

#define USB_ULPI_FIRST_MIO (28U)
#define USB_ULPI_LAST_MIO  (39U)

#define USB_PHY_XTAL_WAIT_MS (50U)

#define USB0_CLK_CTRL_ULPI_MIO_IN (0x00101941U)

#define APER_CLK_USB0_CPU_1X (1U << 2)

static void usb_irq_entry(void)
{
    USBD_IRQHandler(0);
}

static void usb_phy_reset_pulse(void)
{
    mio_enable_clock();
    mio_set_function(BOARD_USB_PHY_RESET_MIO, MIO47_GPIO);
    mio_direction_output(BOARD_USB_PHY_RESET_MIO, 1U);
    tick_mdelay(10);
    mio_set_value(BOARD_USB_PHY_RESET_MIO, 0U);
    tick_mdelay(50);
}

void usb_dc_low_level_init(uint8_t busid)
{
    uint32_t pin;

    (void)busid;

    for (pin = USB_ULPI_FIRST_MIO; pin <= USB_ULPI_LAST_MIO; pin++)
    {
        mio_set_function(pin, MIO_FN_USB | MIO_PIN_OPT_ENA_SPEED_FAST);
    }

    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->usb0_clk_ctrl = USB0_CLK_CTRL_ULPI_MIO_IN;
    slcr->aper_clk_ctrl |= APER_CLK_USB0_CPU_1X;
    slcr->lock = SLCR_LOCK_KEY;

    tick_mdelay(USB_PHY_XTAL_WAIT_MS);
    usb_phy_reset_pulse();

    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->usb_rst_ctrl = 1U;
    slcr->usb_rst_ctrl = 0U;
    slcr->lock = SLCR_LOCK_KEY;

    irq_attach(usb0_irq, usb_irq_entry);
    irq_enable(usb0_irq);
}

void usb_dc_low_level_deinit(uint8_t busid)
{
    (void)busid;

    irq_disable(usb0_irq);
    irq_detach(usb0_irq);
}
