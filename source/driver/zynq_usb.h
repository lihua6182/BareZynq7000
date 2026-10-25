#ifndef ZYNQ_USB_H
#define ZYNQ_USB_H

#include "zynq_common.h"

int usb_ulpi_read_reg(uint8_t addr, uint8_t *val);

int usb_phy_detect(void);

#endif
