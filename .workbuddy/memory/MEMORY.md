# BareZynq7000 项目长期约定

## 项目性质
XC7Z010 (Zynq-7000, Cortex-A9 双核，只用 CPU0) 裸机工程，QSPI **XIP** 运行，无 DDR、无 OS。
工具链：`${sourceDir}/toolchain/arm-none-eabi-gcc`，构建 `cmake --preset BareZynq7000-Debug`（Ninja，Debug 默认，-Og）。
`-mcpu=cortex-a9 -marm -mfloat-abi=hard -mfpu=neon`，`-Wall -Wextra -Wshadow -Wpedantic`。

## CMSIS 范围（2026-09-28 用户明确）
**只要 CMSIS-Core(A)，不要 CMSIS-Driver 框架。**
source/cmsis 为 vendored 的 ARM.CMSIS Core(A) 子集（14 个文件），仅用于：
编译器 intrinsics（`__DSB/__ISB/__DMB`）、CP15 访问器（`__get/__set_SCTLR/TTBR0/DACR/ACTLR/VBAR`）、
GIC API（`GIC_*`，基址 0xF8F01000/0xF8F00100）、PL310 L2C API、L1C/MMU 辅助。
`source/soc/zynq_cmsis.h` 是唯一设备适配头（设备宏 + IRQn_Type），必须在 include `core_ca.h` 之前定义。
**禁止**引入 CMSIS-Driver（`Driver_*.h`）、RTOS2、RTE_Components 等任何驱动/框架层。
本地有/arm/ CMSIS_6 tag 6.3.0 完整副本：`C:\WorkSpace\CMSIS_6-6.3.0`，用于核对 vendored 子集是否被改动
（`diff --strip-trailing-cr C:/WorkSpace/CMSIS_6-6.3.0/CMSIS/Core/Include/<f> source/cmsis/<f>`；
A-profile 头文件在 `CMSIS/Core/Include/a-profile/`，**不是** `Core/A/Include`）。
核对结论：13 个 vendored 文件全部与 6.3.0 逐字节一致，零本地改动。
上游 6.3.0 的 `cmsis_version.h` 里 Core(A) 版本宏本身写的就是 6.1.0（组件版本滞后于 tag），不是错误。

## 硬件与启动的硬约束（踩过坑，勿重犯）
- QSPI 重配窗口（clock_init/qspi_clock_init）CPU 必须在 `.fast`(OCM) 或有缓存兜底，禁止 MMU/cache 关闭时裸取指 flash → 否则总线挂死。顺序固定：`mmu_init → mmu_enable(内含 l2_enable) → xip_warm_cache → clock_init → qspi_clock_init`。
- **L2 投机预取必须关**：AUX_CNT bit28/29(IPFE/DPFE) 与 ACTLR bit1，否则 QSPI 重配那几微秒预取踩死 flash → Prefetch Abort。只保留需求式行填充。
- MMU 页表在 OCM3 (0xFFFF0000) `non-cache` 段；OCR 地址常量表见 zynq_soc_ip.h。
- J-Link 诊断：不要用 SLCR PSS_RST_CTRL 软 POR（`w4 0xF8000008 0xDF0D; w4 0xF8000244 0x1`），会让 JTAG 永久无法 halt，必须断电。用物理断电或 JFlash 自带 reset。

## PL 配置（devcfg/PCAP）
驱动：`source/soc/zynq_devcfg.c/.h`，按 UG585 ch.6 PCAP 示例 11 步实现。**一条 DMA 命令搬完整帧**（FSBL / U-Boot zynqpl.c 同款：
`SRC_LEN = DEST_LEN = bytes/4`，`DEST_ADDR = 0xFFFFFFFF`），CPU 只轮询 `INT_STS[DMA_DONE]` 与 `[PCFG_DONE]`。
对 4KB 规则的正确理解：只需源地址 **64 字节对齐**（对齐是"不跨 4KB 边界"的手段，DMA 内部发 64B burst，4KB 是 64 的整数倍）——不要拆成分块传输。
必备动作：`SLCR->PCAP_CLK_CTRL |= 1`（PCAP 时钟，放在 load 里以支持重配置）、`CTRL[PCFG_POR_CNT_4K]`(bit29) 清 0（走完整 64k POR）、
DMA 完成 ack 要同时写 `INT_STS[13]` 与 `STATUS[DMA_DONE_CNT]`(29:28)、可用 `MCTRL[PCFG_POR_B]`(bit8) 判断 PL 是否供电。
参考实现：`C:\WorkSpace\emPowerZynq\BSP\SEGGER\XC7Z007S_emPowerZynq\Setup\BSP_FPGA.c`（`BSP_FPGA_Init`/`_SendFPGAConfigData`）。
电平转换按 UG585 ch.2 走 0x0A →（配置）→ 0x0F（SEGGER 直接用 0x0F，不跟）。
比特流：`source/bitstream/bitstream.bin`，构建时用 objcopy 链入 `.bitstream` 段（4KB 对齐，符号 `__bitstream_start__/__bitstream_end__`），
DevC DMA 直接从 QSPI 线性窗读取，不搬进 RAM。`.bin` 是 PCAP 所需的 byte-swapped 序，**禁止字节交换**。
硬件事实：Zynq-7000 没有 APB 可写的 PCAP 数据 FIFO，配置数据只能经 DevC DMA 引擎，"手动写"= CPU 分块排队+轮询。
每次 DMA 传输必须 64 字节对齐且不跨 4KB 边界。main.c 里 LED 500ms 慢闪=PL 配置成功，100ms 快闪=失败（查 `g_pl_status`）。

## 代码风格
- `zynq_soc_ip.h` 结构体成员注释放 `;` 之后（`TYPE NAME; /* 注释 */`），全文件统一。
- 改 C 代码后至少用 `-fsyntax-only` 或完整 cmake 构建验证；涉及启动路径的改动需用户上板实测确认。
