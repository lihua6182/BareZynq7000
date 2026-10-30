/**
 * @file main.c
 * @brief CDC ACM 双向吞吐测速: CherryUSB 设备栈跑在 BOARD_USB (usb1) 上,
 *        外置 ULPI PHY 接 MIO40..51 (接线与 PHY 复位见 board_usb.c)。
 *        OUT 端点是 sink (只计数丢弃), IN 端点是 source (DTR 控制的固定图案流),
 *        相位协议与 tools/test_cdc_speed.py 对齐:
 *        DTR=0 主机下行测速 (设备排水), DTR=1 主机上行测速 (设备持续压 IN)。
 */

#include <stdio.h>
#include <string.h>

#include "board.h"
#include "usbd_core.h"
#include "usbd_cdc_acm.h"
#include "zynq_soc.h"
#include "zynq_tick.h"
#include "zynq_usb.h"

#define CDC_IN_EP  0x81
#define CDC_OUT_EP 0x02
#define CDC_INT_EP 0x83

#define USBD_VID       0xFFFF
#define USBD_PID       0xFFFF
#define USBD_MAX_POWER 100

#define USB_CONFIG_SIZE (9 + CDC_ACM_DESCRIPTOR_LEN)

/* 高速 (ULPI, 512 字节 bulk) / 全速 (64 字节) 两份配置描述符。 */
static const uint8_t cdc_config_desc_hs[] = {
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, 512, 0x02)};

static const uint8_t cdc_config_desc_fs[] = {
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, 64, 0x02)};

static const uint8_t cdc_device_desc[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01)};

static const uint8_t cdc_device_qualifier[] = {
    0x0a, USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
};

static const char *cdc_strings[] = {
    (const char[]){0x09, 0x04}, /* LangID */
    "BareZynq7000",             /* Manufacturer */
    "CDC speed test",           /* Product */
    "000000000001",             /* Serial */
};

static const uint8_t *device_descriptor_callback(uint8_t speed)
{
    (void)speed;
    return cdc_device_desc;
}

static const uint8_t *config_descriptor_callback(uint8_t speed)
{
    return (speed == USB_SPEED_HIGH) ? cdc_config_desc_hs : cdc_config_desc_fs;
}

static const uint8_t *device_qualifier_callback(uint8_t speed)
{
    (void)speed;
    return cdc_device_qualifier;
}

static const char *string_descriptor_callback(uint8_t speed, uint8_t index)
{
    (void)speed;
    if (index >= (sizeof(cdc_strings) / sizeof(char *)))
    {
        return NULL;
    }
    return cdc_strings[index];
}

static const struct usb_descriptor cdc_descriptor = {
    .device_descriptor_callback = device_descriptor_callback,
    .config_descriptor_callback = config_descriptor_callback,
    .device_quality_descriptor_callback = device_qualifier_callback,
    .string_descriptor_callback = string_descriptor_callback};

/* 测速缓冲: WB 缓存的 .bss (32 字节对齐), 一致性由 usb_dcache_* 钩子
 * (L1+PL310 双层维护, 见 zynq_usb.c) 负责。(read=下行收, write=上行发) */
static USB_MEM_ALIGNX uint8_t cdc_read_buf[4096];
static USB_MEM_ALIGNX uint8_t cdc_write_buf[4096];

/* 运行状态 (IRQ 写, main 读)。 */
static volatile uint32_t cdc_rx_bytes;
static volatile uint32_t cdc_tx_bytes;
static volatile bool cdc_configured;
static volatile bool cdc_dtr;      /* SET_CONTROL_LINE_STATE 的 DTR 位 */
static volatile bool cdc_tx_busy;  /* IN 端点有一笔写未完成 */
static volatile uint8_t cdc_speed; /* USB_SPEED_* */

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
    (void)busid;

    switch (event)
    {
    case USBD_EVENT_CONFIGURED:
        cdc_configured = true;
        cdc_speed = usbd_get_port_speed(busid);
        /* 挂上第一笔 OUT 读, 之后收完即丢再挂 (下行排水) */
        usbd_ep_start_read(busid, CDC_OUT_EP, cdc_read_buf, sizeof(cdc_read_buf));
        break;
    case USBD_EVENT_RESET:
        cdc_configured = false;
        cdc_tx_busy = false; /* 在途写随总线复位作废 */
        break;
    case USBD_EVENT_DISCONNECTED:
        cdc_configured = false;
        cdc_tx_busy = false;
        break;
    default:
        break;
    }
}

/* OUT 完成: 只计数, 立刻挂下一笔读, 持续排空主机侧管道。 */
static void cdc_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    (void)ep;

    cdc_rx_bytes += nbytes;
    usbd_ep_start_read(busid, CDC_OUT_EP, cdc_read_buf, sizeof(cdc_read_buf));
}

/* IN 写完成: 清 busy, 由主循环续写下一笔 (上行持续流)。 */
static void cdc_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    (void)ep;

    cdc_tx_bytes += nbytes;
    cdc_tx_busy = false;
}

/* tools/test_cdc_speed.py 的相位控制: DTR=0 下行测速 (只收丢), DTR=1 上行测速。 */
void usbd_cdc_acm_set_dtr(uint8_t busid, uint8_t intf, bool dtr)
{
    (void)busid;
    (void)intf;

    cdc_dtr = dtr;
}

static struct usbd_endpoint cdc_out_ep = {.ep_addr = CDC_OUT_EP, .ep_cb = cdc_bulk_out};

static struct usbd_endpoint cdc_in_ep = {.ep_addr = CDC_IN_EP, .ep_cb = cdc_bulk_in};

static struct usbd_interface intf0;
static struct usbd_interface intf1;

/**
 * @brief 注册描述符/接口/端点后 usbd_initialize(); 底层 (ULPI 引脚、PHY、
 *        时钟、GIC) 由钩子 usb_dc_low_level_init() 一并拉起。
 * @note  依赖 tick 已在跑 (PHY 复位延迟用 tick_mdelay)。
 */
static void cdc_speed_test_init(void)
{
    usbd_desc_register(0, &cdc_descriptor);
    usbd_add_interface(0, usbd_cdc_acm_init_intf(0, &intf0));
    usbd_add_interface(0, usbd_cdc_acm_init_intf(0, &intf1));
    usbd_add_endpoint(0, &cdc_out_ep);
    usbd_add_endpoint(0, &cdc_in_ep);
    usbd_initialize(0, (uintptr_t)BOARD_USB, usbd_event_handler);
}

static bool usb_phy_wait_ok(void)
{
    int st;
    int i;

    for (i = 0; i < 3; i++)
    {
        st = usb_phy_detect(BOARD_USB);
        if (st == 0)
        {
            return true;
        }
        if (i < 2)
        {
            usb_dc_init(0);
        }
    }
    if (st == 1)
    {
        printf("usb: PHY 应答但 VID 非 0424 -> 查 PHY 型号/ULPI 连线\n");
    }
    else
    {
        printf("usb: ULPI 无应答 -> 查 PHY 供电/晶振/60MHz/复位脚\n");
    }
    return false;
}

/**
 * @brief 周期性打印测速状态 (累计字节 + 本轮增量)。
 */
static void cdc_report(void)
{
    static uint32_t last_rx;
    static uint32_t last_tx;
    uint32_t rx = cdc_rx_bytes;
    uint32_t tx = cdc_tx_bytes;

    printf("cdc: %s dtr=%u speed=%s rx=%lu (+%lu) tx=%lu (+%lu)\n", cdc_configured ? "configured" : "idle   ",
           cdc_dtr ? 1U : 0U, (cdc_speed == USB_SPEED_HIGH) ? "high" : "full", (unsigned long)rx,
           (unsigned long)(rx - last_rx), (unsigned long)tx, (unsigned long)(tx - last_tx));
    last_rx = rx;
    last_tx = tx;
}

/**
 * @brief 应用入口: 控制台/LED 就绪后拉起 CDC, 主循环纯轮询。
 * @return 不返回。
 */
int main(void)
{
    uint32_t t_led = 0U;
    uint32_t t_last = 0U;

    board_init();

    printf("cdc speed test: console up, usb1 ULPI MIO40-51\n");

    memset(cdc_write_buf, 'U', sizeof(cdc_write_buf)); /* 上行图案, 内容无关 */
    cdc_speed_test_init();
    (void)usb_phy_wait_ok();

    /* 纯轮询: 续写检查每圈都跑 (上行测速要压满 IN 管道), LED/报告按 tick 分频。 */
    for (;;)
    {
        /* 上行测速相 (DTR=1): 上一笔完成即续写, 始终压满 IN 管道。 */
        if (cdc_configured && cdc_dtr && !cdc_tx_busy)
        {
            if (usbd_ep_start_write(0, CDC_IN_EP, cdc_write_buf, sizeof(cdc_write_buf)) == 0)
            {
                cdc_tx_busy = true;
            }
        }

        if ((tick_get() - t_led) >= 500U)
        {
            t_led = tick_get();
            board_led_toggle();
        }

        if ((tick_get() - t_last) >= 1000U)
        {
            t_last = tick_get();
            cdc_report();
        }
    }
}
