/**
 * @file main.c
 * @brief Application entry: board bring-up, then a CDC ACM bidirectional
 *        throughput benchmark over CherryUSB, driven by the Zynq-7000 USB0
 *        controller (ChipIdea usb2 2.20a) with the external USB3300 ULPI PHY
 *        on MIO 28..39 (USB3300 reset on MIO 47, active high per datasheet;
 *        wiring owned by board_usb.c's usb_dc_low_level_init()). The OUT
 *        endpoint is a sink (received bytes are counted, then dropped); the
 *        IN endpoint is a source (a fixed pattern is streamed out under DTR
 *        control). This is NOT a true loopback -- see cdc_throughput_init().
 */

#include <stdio.h>

#include "board.h"
#include "usbd_cdc_acm.h"
#include "usbd_core.h"
#include "zynq_soc.h"
#include "zynq_tick.h"
#include "zynq_usb.h"

/* ------------------------------------------------------------------ */
/* CDC ACM throughput benchmark                                        */
/* ------------------------------------------------------------------ */

#define CDC_IN_EP  0x81
#define CDC_OUT_EP 0x02
#define CDC_INT_EP 0x83

#define USBD_VID           0xFFFF
#define USBD_PID           0xFFFF
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

#define USB_CONFIG_SIZE (9 + CDC_ACM_DESCRIPTOR_LEN)

/* 高速 (USB3300, 512 字节 bulk) / 全速 (64 字节) 两份配置描述符。 */
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
    "CDC loopback",             /* Product */
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

static const struct usb_descriptor cdc_descriptor = {.device_descriptor_callback = device_descriptor_callback,
                                                     .config_descriptor_callback = config_descriptor_callback,
                                                     .device_quality_descriptor_callback = device_qualifier_callback,
                                                     .string_descriptor_callback = string_descriptor_callback};

/* 测速缓冲: 放 WB 缓存的低 OCM (.bss, 32 对齐), 一致性由 usb_dcache_* 钩子
 * (L1+PL310 双层维护, 见 zynq_usb.c) 负责。(read=下行收, write=上行发) */
static USB_MEM_ALIGNX uint8_t cdc_read_buf[1024 * 4];
static USB_MEM_ALIGNX uint8_t cdc_write_buf[1024 * 4];

/* 回环状态 (IRQ 写, main 读)。 */
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
        /* 挂上第一个 OUT 读, 之后收完即丢再挂 (下行排水) */
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

/* OUT 完成: 只计数, 立刻挂下一笔读把主机侧管道 continuously 排空。 */
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

/* tools/test_cdc_speed.py 的相位控制: DTR=0 下行测速 (只收丢), DTR=1 上行
 * 测速 (主循环持续压 IN 端点)。 */
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

static void usb_diag_print(void);

/**
 * @brief  CDC ACM 双向吞吐测试初始化: 注册描述符/接口/端点后
 * usbd_initialize(), 底层 (ULPI 引脚、PHY、时钟、GIC) 由 board 的钩子
 * usb_dc_low_level_init() 一并拉起。OUT 是 sink (只计数丢弃), IN 是 source
 * (固定图案), 并非真回环。
 * @note   依赖 tick_init() 已跑 (PHY 复位延迟用 tick_mdelay)。
 */
static void cdc_throughput_init(void)
{
    usbd_desc_register(0, &cdc_descriptor);
    usbd_add_interface(0, usbd_cdc_acm_init_intf(0, &intf0));
    usbd_add_interface(0, usbd_cdc_acm_init_intf(0, &intf1));
    usbd_add_endpoint(0, &cdc_out_ep);
    usbd_add_endpoint(0, &cdc_in_ep);
    usbd_initialize(0, (uintptr_t)BOARD_USB, usbd_event_handler);

    /* 链路健康则静默; 只有 ULPI 无应答才展开诊断 (含极性搜索与重拉栈)。
     * 冷掉电后晶振/ULPI 时钟爬坡有离散性, PHY 偶尔要再等几十 ms 才应答
     * 第一笔 viewport 读 —— 先重试几轮, 避免误报诊断 (实测枚举不受影响)。 */
    int link_ok = 0;
    for (int i = 0; i < 5; i++)
    {
        if (usb_phy_link_check() == 0)
        {
            link_ok = 1;
            break;
        }
        tick_mdelay(20);
    }
    if (!link_ok)
    {
        usb_diag_print();
    }
}

/**
 * @brief  打印 ULPI 寄存器行 (经 viewport 读 USB3300)。
 * @return 0 链路通; -1 超时 (无应答)。
 */
static int usb_ulpi_line_print(void)
{
    uint8_t vh, vl, ph, pl, fctrl, otg;

    if (usb_ulpi_read_reg(0x00, &vh) == 0 && usb_ulpi_read_reg(0x01, &vl) == 0 && usb_ulpi_read_reg(0x02, &ph) == 0 &&
        usb_ulpi_read_reg(0x03, &pl) == 0 && usb_ulpi_read_reg(0x04, &fctrl) == 0 && usb_ulpi_read_reg(0x0A, &otg) == 0)
    {
        printf("usb: ulpi vid=%02X%02X pid=%02X%02X func=%02X otgctl=%02X "
               "ss=%lu\n",
               vh, vl, ph, pl, fctrl, otg, (unsigned long)((USB0->ULPI_VIEWPORT >> 27) & 1u));
        return 0;
    }
    printf("usb: ulpi TIMEOUT - PHY 无应答\n");
    return -1;
}

/**
 * @brief  USB bring-up 诊断: 控制器关键寄存器 + 经 ULPI viewport 读 PHY。
 *         逐行含义:
 *         USBCMD bit0 RS=1 已命令连接 (拉 D+ 上拉); USBSTS 的 URI/PCI/UI
 *         表示主机来的复位/端口变化/传输; PORTSC1 bit0 CCS=1 主机在位,
 *         [27:26] PSPD 协商速率, bit23 PHCD=1 则 PHY 时钟被关;
 *         OTGSC [4:0] 状态: bit0 ID, bit1 AVV, bit2 ASV, bit3 BSV (VBUS 有效),
 *         bit4 BSE (session end) —— BSV=0 且 BSE=1 就是 PHY 没看到 VBUS;
 *         mio 行是 SLCR 引脚回读 ([15:13]=DIS_RCVR/PU [11:9]=IO [8]=SLEW
 *         [7:2]=L3..L0 复用), clkctl 是 USB0_CLK_CTRL (SRCSEL[6:4]=100 选
 *         MIO ULPI 时钟输入)。链路死时给出方向性提示。
 */
static void usb_diag_print(void)
{
    printf("usb: USBCMD=%08lX USBSTS=%08lX USBMODE=%08lX\n", (unsigned long)USB0->USBCMD, (unsigned long)USB0->USBSTS,
           (unsigned long)USB0->USBMODE);
    printf("usb: PORTSC1=%08lX ccs=%lu susp=%lu pr=%lu phcd=%lu pspd=%lu\n", (unsigned long)USB0->PORTSC1,
           (USB0->PORTSC1 >> 0) & 1u, (USB0->PORTSC1 >> 7) & 1u, (USB0->PORTSC1 >> 8) & 1u, (USB0->PORTSC1 >> 23) & 1u,
           (USB0->PORTSC1 >> 26) & 3u);
    printf("usb: OTGSC=%08lX id=%lu avv=%lu asv=%lu bsv=%lu bse=%lu\n", (unsigned long)USB0->OTGSC, USB0->OTGSC & 1u,
           (USB0->OTGSC >> 1) & 1u, (USB0->OTGSC >> 2) & 1u, (USB0->OTGSC >> 3) & 1u, (USB0->OTGSC >> 4) & 1u);
    printf("usb: mio28=%08lX mio35=%08lX mio36=%08lX mio39=%08lX clkctl=%08lX\n", (unsigned long)SLCR->MIO_PIN_28,
           (unsigned long)SLCR->MIO_PIN_35, (unsigned long)SLCR->MIO_PIN_36, (unsigned long)SLCR->MIO_PIN_39,
           (unsigned long)SLCR->USB0_CLK_CTRL);

    if (usb_ulpi_line_print() != 0)
    {
        printf("usb: PHY 无应答 -> 供电/晶振/60MHz/布线\n");
    }
}

/**
 * @brief  周期性打印测速状态。
 */
static void cdc_report(void)
{
    static uint32_t last_rx;
    static uint32_t last_tx;
    uint32_t rx = cdc_rx_bytes;
    uint32_t tx = cdc_tx_bytes;

    printf("cdc: %s dtr=%u speed=%s rx=%lu (+%lu) tx=%lu (+%lu)\n", cdc_configured ? "configured" : "idle   ",
           cdc_dtr ? 1u : 0u, (cdc_speed == USB_SPEED_HIGH) ? "high" : "full", (unsigned long)rx,
           (unsigned long)(rx - last_rx), (unsigned long)tx, (unsigned long)(tx - last_tx));
    last_rx = rx;
    last_tx = tx;
}

/**
 * @brief  Application entry point.
 * @return Does not return.
 */
int main(void)
{
    uint32_t t_led = 0u;
    uint32_t t_last = 0u;

    board_console_init();
    board_led_init();

    /* PL 配置放在控制台之后：加载/失败全程可见。board 路径本无 console
       依赖（反之 console 也不依赖 PL），两向都可换序；tick 已在跑，
       1ms 中断只会给 PCAP 轮询限值添可忽略的抖动。 */
    printf("pl: loading...\n");
    board_init();

    printf("pl_status=%u\n", (unsigned)board_pl_status());

    memset(cdc_write_buf, 'U', sizeof(cdc_write_buf)); /* 上行图案, 内容无关 */
    cdc_throughput_init();

    /* 纯轮询: 续写检查每圈都跑 (上行测速要压满管道), LED/报告按 tick 分频。
     * 主循环不睡眠。 */
    for (;;)
    {
        /* 上行测速相 (DTR=1): 写一笔 2048, 完成即续, 始终压满 IN 管道。 */
        if (cdc_configured && cdc_dtr && !cdc_tx_busy)
        {
            if (usbd_ep_start_write(0, CDC_IN_EP, cdc_write_buf, sizeof(cdc_write_buf)) == 0)
            {
                cdc_tx_busy = true;
            }
        }

        if ((tick_get() - t_led) >= 500u)
        {
            t_led = tick_get();
            board_led_toggle();
        }

        if ((tick_get() - t_last) >= 1000u)
        {
            t_last = tick_get();
            cdc_report();
        }
    }
}
