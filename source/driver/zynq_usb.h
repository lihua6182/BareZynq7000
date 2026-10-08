#ifndef ZYNQ_USB_H
#define ZYNQ_USB_H

#include "zynq_common.h"

/*
 * USB0 控制器侧: ChipIdea usb2 2.20a (UG585 ch.15) 的寄存器原语, 外加
 * CherryUSB dcache 钩子的适配。板级接线 —— MIO 28..39 的 ULPI 引脚复用、
 * USB3300 PHY 复位 (MIO 47, 高有效)、USB0_CLK_CTRL 的 MIO 时钟输入选择、
 * GIC —— 在 source/board/board.c 的 usb_dc_low_level_init() (CherryUSB
 * 钩子的板级实现, 见 board.h)。
 *
 * 应用侧只用 middleware/cherryusb 的 usbd_* API:
 *
 *     usbd_desc_register(0, &my_descriptor);
 *     usbd_add_interface(0, usbd_cdc_acm_init_intf(0, &intf));
 *     usbd_add_endpoint(0, &ep);
 *     usbd_initialize(0, USB0_BASE, my_event_handler);
 *
 * usbd_initialize() -> usb_dc_init() -> usb_dc_low_level_init()(board.c)
 * 会把 ULPI 引脚复用、USB 时钟门控、PHY 复位、GIC 中断全部做好, 无需手动
 * 先调板级接线。
 */

/* 经 ULPI viewport 读外置 PHY 的 ULPI 寄存器 (超时返回 -1)。 */
int usb_ulpi_read_reg(uint8_t addr, uint8_t *val);

/* 探测 ULPI 链路: 0 通且 VID 正确; 1 有应答但 VID 异常; -1 无应答。 */
int usb_phy_link_check(void);

#endif /* ZYNQ_USB_H */
