/**
 * @file zynq_usb.c
 * @brief USB0 (ChipIdea usb2) 控制器侧原语 + CherryUSB dcache 钩子。
 *        板级接线 (ULPI 引脚、USB3300 PHY 复位、时钟路由、GIC) 在
 *        board_usb.c 的 usb_dc_low_level_init() 钩子里。
 */

#include "zynq_usb.h"

#include "zynq_cache.h"
#include "zynq_soc.h"
#include "zynq_tick.h"

/**
 * @brief  经 ULPI viewport (usb.ULPI_VIEWPORT) 读外置 PHY 的 ULPI 寄存器
 *         (板上是 USB3300)。
 * @param  addr ULPI 寄存器地址 (如 0x00 VID_H, 0x04 Function Control)。
 * @param  val  回读字节。
 * @return 0 成功; -1 超时 (10ms 内 RUN 未清零 = ULPI 链路无应答)。
 * @note   依赖 tick_init()。链路死 (PHY 未出复位/无 60MHz 时钟/引脚错) 时
 *         RUN 位会一直为 1, 由此可把故障定位到 PHY 侧而不是控制器侧。
 */
int usb_ulpi_read_reg(uint8_t addr, uint8_t *val)
{
    uint32_t t0 = tick_get();

    USB0->ULPI_VIEWPORT = (1UL << 30) | ((uint32_t)addr << 16); /* RUN | addr, R/W=0 读 */
    while ((USB0->ULPI_VIEWPORT & (1UL << 30)) != 0U)
    {
        if ((tick_get() - t0) > 10u)
        {
            *val = 0u;
            return -1;
        }
    }
    *val = (uint8_t)(USB0->ULPI_VIEWPORT >> 8);
    return 0;
}

/**
 * @brief  探测 ULPI 链路: 读 PHY 厂商 ID (SMSC/Microchip 0x0424)。
 * @return 0 链路通且 VID 正确; 1 有应答但 VID 异常; -1 无应答。
 * @note   实测这颗 USB3300 的 VID 高/低字节排布与 ULPI 规范相反
 *         (0x00 读回 0x24, 0x01 读回 0x04), 两种顺序都认。
 */
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

/* ------------------------------------------------------------------ */
/* CherryUSB dcache 钩子 (CONFIG_USB_DCACHE_ENABLE, 见 common/usb_dcache.h) */
/* ------------------------------------------------------------------ */

/*
 * USB DMA 走中央互连、不嗅探 PL310, 一致性是 zynq_cache 的双层维护;
 * 这里只做名字/签名的适配。调用点 (chipidea port): usbd_ep_start_write
 * -> clean, usbd_ep_start_read 与 RX 完成中断 -> invalidate。
 */
void usb_dcache_clean(uintptr_t addr, size_t size)
{
    cache_clean((const void *)(uintptr_t)addr, size);
}

void usb_dcache_invalidate(uintptr_t addr, size_t size)
{
    cache_invalidate((const void *)(uintptr_t)addr, size);
}
