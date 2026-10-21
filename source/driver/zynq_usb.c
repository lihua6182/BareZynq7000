
#include "zynq_usb.h"

#include "zynq_cache.h"
#include "zynq_soc.h"
#include "zynq_tick.h"

int usb_ulpi_read_reg(uint8_t addr, uint8_t *val)
{
    uint32_t t0 = tick_get();

    usb0->ulpi_viewport = (1UL << 30) | ((uint32_t)addr << 16);
    while ((usb0->ulpi_viewport & (1UL << 30)) != 0U)
    {
        if ((tick_get() - t0) > 10u)
        {
            *val = 0u;
            return -1;
        }
    }
    *val = (uint8_t)(usb0->ulpi_viewport >> 8);
    return 0;
}

int usb_phy_link_check(void)
{
    uint8_t vh = 0u;
    uint8_t vl = 0u;

    if (usb_ulpi_read_reg(0x00, &vh) != 0)
    {
        return -1;
    }
    if (usb_ulpi_read_reg(0x01, &vl) != 0)
    {
        return -1;
    }
    if (vh == 0x04u && vl == 0x24u)
    {
        return 0;
    }
    return (vh == 0x24u && vl == 0x04u) ? 0 : 1;
}

void usb_dcache_clean(uintptr_t addr, size_t size)
{
    cache_clean((const void *)(uintptr_t)addr, size);
}

void usb_dcache_invalidate(uintptr_t addr, size_t size)
{
    cache_invalidate((const void *)(uintptr_t)addr, size);
}
