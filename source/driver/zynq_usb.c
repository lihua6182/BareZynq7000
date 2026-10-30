#include "zynq_usb.h"

#include "zynq_cache.h"
#include "zynq_soc.h"
#include "zynq_tick.h"

int usb_ulpi_read_reg(struct usb *dev, uint8_t addr, uint8_t *val)
{
    uint32_t t0 = tick_get();

    dev->ulpi_viewport = (1U << 30) | ((uint32_t)addr << 16);
    while ((dev->ulpi_viewport & (1U << 30)) != 0U)
    {
        if ((tick_get() - t0) > 10U)
        {
            *val = 0U;
            return -1;
        }
    }
    *val = (uint8_t)(dev->ulpi_viewport >> 8);
    return 0;
}

int usb_phy_detect(struct usb *dev)
{
    uint8_t vid_low = 0U;
    uint8_t vid_high = 0U;

    if (usb_ulpi_read_reg(dev, 0x00, &vid_low) != 0)
    {
        return -1;
    }
    if (usb_ulpi_read_reg(dev, 0x01, &vid_high) != 0)
    {
        return -1;
    }
    /* SMSC/Microchip PHY Vendor ID 0x0424 */
    return (vid_low == 0x24U && vid_high == 0x04U) ? 0 : 1;
}

void usb_dcache_clean(uintptr_t addr, size_t size)
{
    cache_clean((const void *)(uintptr_t)addr, size);
}

void usb_dcache_invalidate(uintptr_t addr, size_t size)
{
    cache_invalidate((const void *)(uintptr_t)addr, size);
}
