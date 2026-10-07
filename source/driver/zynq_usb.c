/**
 * @file zynq_usb.c
 * @brief USB0 (ChipIdea usb2) + 外置 USB3300 ULPI PHY 的底层接线。
 */

#include "zynq_usb.h"

#include "zynq_cache.h"
#include "zynq_irq.h"
#include "zynq_mio.h"
#include "zynq_soc.h"
#include "zynq_tick.h"

/* CherryUSB chipidea port 的中断入口 (usb_dc_chipidea.c, 未在公共头声明)。 */
extern void USBD_IRQHandler(uint8_t busid);

/* ULPI 12 根信号 (DATA[7:0]=28..35, DIR/STP/NXT/CLK=36..39), 全部 L1 复用 1,
 * 3.3V bank。ULPI CLK 60MHz, 输出沿统一用 SLEW FAST (MIO_PIN_OPT_ENA_SPEED_FAST)。 */
#define USB_ULPI_FIRST_MIO (28u)
#define USB_ULPI_LAST_MIO  (39u)

/* USB3300 RESET, 高有效复位 (MIO47 直连), GPIO 推挽输出。 */
#define USB_PHY_RESET_MIO (47u)

/* 到达 usb_dc_low_level_init() 时板子已上电很久 (boot + PL 加载), USB3300
 * 外部 24MHz 晶振必然已起振; 这里只给 ULPI 时钟稳定的保守裕量。原 200ms
 * 预等待属过度保守, 实测远小于此。若上板偶发 ULPI 第一笔 viewport 读超时,
 * 调大此值 (cdc_throughput_init 的 5x20ms 链路重试已兜底)。 */
#define USB_PHY_XTAL_WAIT_MS (50u)

/* SLCR.USB0_CLK_CTRL 复位值 0x00101941: SRCSEL[6:4]=100 选择 MIO ULPI 时钟
 * (输入) + 使能, 其余保留位维持复位值 (UG585 App.A, "do not modify" 字段)。 */
#define USB0_CLK_CTRL_ULPI_MIO_IN (0x00101941UL)

/* SLCR.APER_CLK_CTRL[2]: USB0_CPU_1XCLKACT (复位即 1, 显式再开一次)。 */
#define APER_CLK_USB0_CPU_1X (1u << 2)

static void usb_irq_entry(void)
{
    USBD_IRQHandler(0);
}

/* USB3300 的 RESET 引脚 (pin 9) 为高有效复位 (Microchip 00001783C 数据手册:
 * "active high transceiver reset", 只复位收发器, 不动 ULPI 接口与寄存器组,
 * 复位完成后自动清除)。MIO47 直连该脚: 高 = 复位, 低 = 运行。极性是本板
 * 定型事实, 不做运行时搜索。 */

/**
 * @brief  脉冲 USB3300 的 RESET (MIO 47): 拉高 10 ms 复位, 放低运行, 再等
 *         50 ms ULPI CLK (60MHz) 起振——之后控制器寄存器才可访问。
 * @note   依赖 tick_mdelay() (tick_init() 已跑)。
 */
void usb_phy_reset_pulse(void)
{
    mio_enable_clock();
    mio_set_function(USB_PHY_RESET_MIO, MIO47_GPIO);
    mio_direction_output(USB_PHY_RESET_MIO, 1u); /* 复位 (高有效) */
    tick_mdelay(10);
    mio_set_value(USB_PHY_RESET_MIO, 0u); /* 运行 (低) */
    tick_mdelay(50);
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

/**
 * @brief  经 ULPI viewport (usb.ULPI_VIEWPORT) 读 USB3300 的 ULPI 寄存器。
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
 * @brief  CherryUSB 设备栈钩子: USB0 全套底层拉起 (ULPI 引脚 -> 时钟 ->
 *         PHY 复位 -> 控制器 AMBA 复位脉冲 -> GIC)。
 * @note   由 usbd_initialize() -> usb_dc_init() 调用, 应用不要手动先调;
 *         依赖 tick_init() (PHY 复位延迟)。只接了 USB0, 忽略 busid。
 */
void usb_dc_low_level_init(uint8_t busid)
{
    uint32_t pin;

    (void)busid;

    for (pin = USB_ULPI_FIRST_MIO; pin <= USB_ULPI_LAST_MIO; pin++)
    {
        mio_set_function(pin, MIO_FN_USB | MIO_PIN_OPT_ENA_SPEED_FAST);
    }

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->USB0_CLK_CTRL = USB0_CLK_CTRL_ULPI_MIO_IN;
    SLCR->APER_CLK_CTRL |= APER_CLK_USB0_CPU_1X;
    SLCR->LOCK = SLCR_LOCK_KEY;

    /* PHY 晶振/ULPI 时钟起振裕量 (见 USB_PHY_XTAL_WAIT_MS)。只脉冲复位一次
     * —— 原"连发两次 + 200ms 预等待"约 420ms 硬阻塞, 其中 300ms+ 是过度保守
     * 的保险, 链路健康已由 cdc_throughput_init 的 5x20ms 重试兜底。 */
    tick_mdelay(USB_PHY_XTAL_WAIT_MS);
    usb_phy_reset_pulse();

    /* PHY 时钟稳定后再脉冲控制器的 CPU_1x 复位域, 从干净状态开始枚举。 */
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->USB_RST_CTRL = 1u; /* USB0_CPU1X_RST assert */
    SLCR->USB_RST_CTRL = 0u; /* de-assert */
    SLCR->LOCK = SLCR_LOCK_KEY;

    irq_attach(Usb0_IRQn, usb_irq_entry);
    irq_enable(Usb0_IRQn);
}

/**
 * @brief  CherryUSB 设备栈钩子: 撤销 usb_dc_low_level_init() 的接线 (关中断)。
 */
void usb_dc_low_level_deinit(uint8_t busid)
{
    (void)busid;

    irq_disable(Usb0_IRQn);
    irq_detach(Usb0_IRQn);
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
