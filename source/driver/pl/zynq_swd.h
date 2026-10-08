#ifndef ZYNQ_SWD_H
#define ZYNQ_SWD_H

/**
 * @file zynq_swd.h
 * @brief SWD 访问层 (CMSIS-DAP 风格): 主机侧命令打包 + 阻塞收发。
 *        经 proto_coproc 移位, 不是 PS 外设; 与 zynq_ppc 同在 driver/pl/。
 *
 * PPC 侧程序: C:\WorkSpace\PPC\examples\swd_dap.s (命令驱动, 协议表见其头部):
 *   配置  [turnaround, idle]              (RUN 前推 2 个字)
 *   SEQ   [SWD_SEQ(dir,n), data]  -> 1 字 (发/收 1..32 位, LSB first)
 *   XFER  [SWD_XFER(req),   data] -> 2 字 [ack, data]
 * 本层把任意长度序列自动拆成 <=32 位/条, 并保持 XFER 的 2 字帧。
 *
 * 时序: SWCLK 由 PPC 按 PL 时钟产生 (FCLK0=100MHz 时 写 ~12.5MHz / 读 ~11.1MHz);
 * 目标跑不了就调低 board.c 里 FCLK0 的 IO PLL 分频, SWCLK 按比例下降。
 *
 * 典型用法 (标准初始化序列):
 *   coproc_cmd(COPROC, COPROC_CMD_FLUSH_ALL);
 *   coproc_load(COPROC, swd_dap_prog, SWD_DAP_PROG_WORDS);
 *   swd_configure(COPROC, 1u, 2u);
 *   coproc_cmd(COPROC, COPROC_CMD_RUN);
 *   swd_line_reset(COPROC, 100u);
 *   ack = swd_transfer(COPROC, SWD_DP_RD(SWD_DP_IDCODE), &v, 100u);
 */

#include "zynq_common.h"

#include "zynq_ppc.h"

/* ------------------------------------------------------------------ */
/* PPC (swd_dap.s) 命令字                                             */
/* ------------------------------------------------------------------ */

#define SWD_CMD_SEQ     0x10000000u /* [31:28]=1 | [16]=dir | [12:8]=n */
#define SWD_CMD_XFER    0x20000000u /* [31:28]=2 | [7:0]=request */
#define SWD_SEQ(dir, n) (SWD_CMD_SEQ | ((uint32_t)(dir) << 16) | ((uint32_t)(n) << 8))
#define SWD_XFER(req)   (SWD_CMD_XFER | ((uint32_t)(req) & 0xFFu))

#define SWD_DIR_OUT 0u /* 主机驱动 SWDIO (发送) */
#define SWD_DIR_IN  1u /* 释放 SWDIO, 采样目标 (接收) */

/* ------------------------------------------------------------------ */
/* request 组装 (CMSIS-DAP: bit0=APnDP bit1=RnW bit2=A2 bit3=A3)       */
/* ------------------------------------------------------------------ */

#define SWD_REQ_APnDP   0x01u
#define SWD_REQ_RnW     0x02u
#define SWD_DP_RD(addr) (SWD_REQ_RnW | ((uint32_t)(addr) & 0x0Cu))
#define SWD_DP_WR(addr) (((uint32_t)(addr) & 0x0Cu))
#define SWD_AP_RD(addr) (SWD_REQ_APnDP | SWD_REQ_RnW | ((uint32_t)(addr) & 0x0Cu))
#define SWD_AP_WR(addr) (SWD_REQ_APnDP | ((uint32_t)(addr) & 0x0Cu))

/* 常用寄存器 (ADIv5) 与字段 */
#define SWD_DP_IDCODE   0x00u
#define SWD_DP_CTRLSTAT 0x04u
#define SWD_DP_SELECT   0x08u
#define SWD_AP_CSW      0x00u
#define SWD_AP_TAR      0x04u
#define SWD_AP_DRW      0x0Cu
#define SWD_AP_IDR      0xFCu

#define SWD_CTRLSTAT_PWRUP  0x50000000u /* CSYSPWRUPREQ | CDBGPWRUPREQ */
#define SWD_CTRLSTAT_PWRACK 0xA0000000u /* CSYSPWRUPACK | CDBGPWRUPACK */

/* ------------------------------------------------------------------ */
/* ack 值 (与 CMSIS-DAP 的 SWD_Transfer 返回值同值)                    */
/* ------------------------------------------------------------------ */

#define SWD_ACK_OK      1u    /* OK    */
#define SWD_ACK_WAIT    2u    /* WAIT  */
#define SWD_ACK_FAULT   4u    /* FAULT */
#define SWD_ACK_ERROR   8u    /* 读奇偶错 */
#define SWD_ACK_BAD     7u    /* 线上非法 ack: 无目标 / 线异常 */
#define SWD_ACK_TIMEOUT 0xFEu /* 本层超时: PPC 未回话 */

/* ------------------------------------------------------------------ */
/* API                                                                */
/* ------------------------------------------------------------------ */

/* 启动配置 (RUN 前推): 转弯时钟数 >=1, OK 之后 idle 时钟数 */
void swd_configure(COPROC_Type *c, uint32_t turnaround, uint32_t idle);

/* 线复位: >=50 拍 SWDIO=1 再 >=2 拍 SWDIO=0 (ADIv5 定义) */
bool swd_line_reset(COPROC_Type *c, uint32_t timeout_ms);

/* JTAG→SWD 切换序列 (16 位 0xE79E, LSB first); 目标本来就在 SWD 也无害 */
bool swd_jtag_to_swd(COPROC_Type *c, uint32_t timeout_ms);

/* 序列: count 位 (1..256), 位流 LSB first 取自 / 写回 bits */
bool swd_seq_out(COPROC_Type *c, uint32_t count, const uint8_t *bits, uint32_t timeout_ms);
bool swd_seq_in(COPROC_Type *c, uint32_t count, uint8_t *bits, uint32_t timeout_ms);

/* 单笔 SWD 传输 (CMSIS-DAP SWD_Transfer): 写时 *data 为写值, 读时回填
 * (读方向的输入值被忽略, 但主机仍必须推 1 个数据字保持帧同步)。 */
uint8_t swd_transfer(COPROC_Type *c, uint8_t request, uint32_t *data, uint32_t timeout_ms);

#endif /* ZYNQ_SWD_H */
