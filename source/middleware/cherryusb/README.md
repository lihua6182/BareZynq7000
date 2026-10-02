# CherryUSB (vendored, v1.6.1)

CherryUSB 设备栈最小子集, 只编译设备方向 + CDC ACM + ChipIdea 设备控制器 port:

```
core/usbd_core.{c,h}                  设备协议引擎 (EP0 无线程模式, 全 IRQ 上下文)
common/usb_*.h                        公共定义 (usb_config.h 在本目录)
class/cdc/                            CDC ACM 设备类 (usbd_cdc_acm_*)
port/chipidea/usb_dc_chipidea.{c,..}  ChipIdea usb2 设备控制器驱动 (QHD/QTD)
```

来源: `C:\WorkSpace\CherryDAP\CherryUSB` (git tag v1.6.1) 的对应文件, 原样落库
(license: Apache-2.0 / BSD-3-Clause, 见上游仓库)。与上游的唯一差异在
`port/chipidea/usb_dc_chipidea.c` 的 USBMODE.SLOM 一行: Zynq-7000 的 CI 2.20a
上 1 = 关闭 setup lockout (配 tripwire), 上游按 HPM/i.MX 措辞写的是清零。

Zynq-7000 PS 的 USB 控制器是 ChipIdea usb2 2.20a (TRM App.B: ID 寄存器复位
0xE441FA05, USBCMD@0x140/ENDPTCTRL@0x1C0 一族), 与 HPM 的 USB IP 同族——所以
选 chipidea port 而不是 dwc2。板级接线 (ULPI 引脚 / PHY 复位 / 时钟 / GIC) 在
`driver/zynq_usb.c`。上游: https://github.com/cherry-embedded/CherryUSB
