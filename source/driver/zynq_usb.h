#ifndef ZYNQ_USB_H
#define ZYNQ_USB_H

#include "zynq_common.h"

/*
 * USB0 底层接线: PS 的 USB 控制器 (ChipIdea usb2 2.20a, UG585 ch.15) 经
 * MIO 28..39 的 ULPI 口外接 USB3300 PHY, PHY 复位脚是 MIO 47 (高有效复位, 运行电平低 —— 本板定型事实)。
 *
 * 设备协议栈是 middleware/cherryusb, 它的板级钩子 usb_dc_low_level_init()/
 * usb_dc_low_level_deinit() 在本文件实现 (引脚/时钟/PHY/GIC 一站式拉起)。
 * 应用侧只需:
 *
 *     usbd_desc_register(0, &my_descriptor);
 *     usbd_add_interface(0, usbd_cdc_acm_init_intf(0, &intf));
 *     usbd_add_endpoint(0, &ep);
 *     usbd_initialize(0, USB0_BASE, my_event_handler);
 *
 * usbd_initialize() -> usb_dc_init() -> usb_dc_low_level_init() 会把 ULPI
 * 引脚复用、USB 时钟门控、PHY 复位、GIC 中断全部做好, 无需手动先调本模块。
 */

void usb_phy_reset_pulse(void);

int usb_ulpi_read_reg(uint8_t addr, uint8_t* val);

int usb_phy_link_check(void);

#endif /* ZYNQ_USB_H */
