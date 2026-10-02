#ifndef USB_CONFIG_H
#define USB_CONFIG_H

/* CherryUSB v1.6.1 设备栈配置 (模板: 上游 cherryusb_config_template.h)。
 * @note 本工程无 OS: 不定义 CONFIG_USBDEV_EP0_THREAD, EP0 全部在 USB IRQ
 *       上下文处理, 不需要任何 osal 实现。 */

#define CONFIG_USB_PRINTF(...) printf(__VA_ARGS__)

/* 回环测速期间只要告警/错误 (枚举/配置的 INFO 全部关掉; 需要排查时改回
 * USB_DBG_INFO)。错误路径 (传输错误/标准请求失败) 仍然打印。 */
#define CONFIG_USB_DBG_LEVEL USB_DBG_WARNING

/* 缓冲/QTD 指针落 OCM 高 64KB 非缓存区 (flash_xip.ld 的 .no_cache_ram),
 * USB 控制器 DMA 直接访问该区, 无需软件 cache 维护 (不开 DCACHE_ENABLE)。 */
#define USB_NOCACHE_RAM_SECTION __attribute__((section(".no_cache_ram")))

/* DMA 缓冲对齐 (usb_util.h 的 USB_MEM_ALIGNX/USB_ALIGN_UP 依赖)。 */
#define CONFIG_USB_ALIGN_SIZE 4

/* 端点数: CDC ACM 用 EP0 + EP1(IN/OUT bulk) + EP3(int), 4 个足够。
 * @note 每端点占 2 个 QH(64B) + 16 个 QTD(32B), 4 端点共 1.5KB no_cache_ram。 */
#define CONFIG_USBDEV_EP_NUM 4

#define CONFIG_USBDEV_MAX_BUS 1

/* EP0 控制传输缓冲 (模板默认 512)。 */
#ifndef CONFIG_USBDEV_REQUEST_BUFFER_LEN
#define CONFIG_USBDEV_REQUEST_BUFFER_LEN 512
#endif

#endif /* USB_CONFIG_H */
