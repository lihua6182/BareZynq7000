# BareZynq7000

## 简介

裸机状态运行的ZYNQ7000，无FSBL，从BootROM跳转后直接运行。

## 内存分配

RAM:

OCM低192KB(OCM0-2), 0x0000_0000, 数据/.bss/堆/栈, WB缓存

NOCACHE:

OCM高64KB(OCM3),    0xFFFF_0000, MMU页表 + no_cache_ram, 非缓存, DMA可达

FLASH:

QSPI,   0xFC00_0000, 16MB

DDR不使用, MMU中故意不映射(访问即translation fault)。

## 启动流程

1. 芯片上电时，运行BootROM，根据引脚配置从QSPI启动，以32KB为间隔搜索启动镜像。
2. Flash的0KB处(`0xFC00_0000`)放置镜像头(`.boot_header`)。
3. Flash的`0x8C0`处(`0xFC00_08C0`)放置中断向量表。
4. BootROM读取Flash头部的`.boot_header`块，不复制fsbl（XIP），直接跳转到镜像起始处的向量表首项进入`reset_handler`。
5. `soc_init()`按UG585第25章的PLL更新序列把时钟配置到UG585表25-3（6:2:1模式）示例1的状态：CPU 667MHz、外设总线CPU_1x 111MHz、QSPI参考200MHz，Flash实际SCLK=100MHz（W25Q128JV四线0x6B读，Config_reg波特率÷2 + LPBK_DLY_ADJ内部回环采样DLY=00 + MIO8/9反馈时钟环回路由）。MMU开启后、QSPI重配前，`xip_warm_cache()`把整个XIP镜像读入统一的L2缓存——实测这块板经刚重配的QSPI线性口的读不可靠，预热保证重配窗口内零flash取指。QSPI参数切换在OCM的`.fast`段中按UG585第12章流程执行：退出linear模式→排空FIFO→改时钟并脉冲复位两个控制器复位域→重编程控制器寄存器→重新进入linear模式。时钟参数：晶振频率（`PS_CLK_HZ`，33.333MHz）在`source/soc/zynq_common.h`中配置，是全工程唯一硬编码的频率；其余所有时钟频率（PLL/CPU/外设参考时钟等）在运行时由`zynq_clock.c`的`clk_freqs_get()`从SLCR寄存器读出。

## 源码结构

```
source/
  app/     可执行目标 app：main.c + newlib 桩（syscalls/sysmem）
  board/   板级：board_fpga（PL 配置 + PL 控制面，手写）+ board_fpga_bitstream（PL 镜像，生成物）
  driver/  PS 外设：zynq_gpio、zynq_uart
  soc/     启动汇编、MMU/时钟/QSPI、GIC（zynq_irq）、TTC tick、BootROM 头、CMSIS Core(A) 子集
```
前三者在 CMake 里是 `INTERFACE` 库（只传源文件与包含路径，不产生归档），只有 `app` 是真目标。

## 板级现状

- LED：MIO 24，推挽输出；PL 配置成功时 500ms 慢闪，失败时 100ms 快闪（`pl_status` 保存失败原因）。
- 串口：UART0，MIO 11 = TX / MIO 10 = RX，115200 8N1，轮询收发，打印 `pl_status` 与 tick 计数。
- tick：TTC0 计数器 0 的 interval 中断（SPI 42），1ms，全部经 IRQ 投递（不用 FIQ）。
- PL：上电即由 `board_init()` 经 DevC/PCAP 配置，FCLK0 = IO PLL / 10 = 100MHz，随后释放 PL 复位（`slcr.FPGA_RST_CTRL` 复位值为 0xF，不清则 GP0 访问挂死）。对 PL 的所有操作走 `board_fpga.h` 的 API：
  `fpga_reset_assert/release/pulse`、`fpga_reset_assert_all/release_all`、`fpga_lvl_ps_to_pl/all/off`、
  `fpga_is_powered/ready/done`、`fpga_config_clear()`（PROG_B 清配置）、`fpga_load()`（配置并保持在复位）。

## 构建与烧写

```
cmake --preset BareZynq7000-Debug     # 配置（Ninja，Debug，-Og）
cmake --build build                    # 产物 build/source/app/app.{elf,bin,hex}
```
构建末尾会自动校验 BootROM 头。烧写用 J-Flash 工程 `scripts/jflash/jflash_zynq7000.jflash`
（RAM loader 已改到 `RAMAddr=0x20000`，避免压扁低 OCM 里的 .data/.bss）。

PL 比特流：`source/board/bitstream.bin`（Vivado 原生 .bin）经
`python tools/convert_bitstream.py <in.bin> <out.c> <out.h>` 生成 `board_fpga_bitstream.c/h`（ZBSP：RLE + CRC32），
VSCode task 名 "Convert PL bitstream"。脚本内部负责字节序转换，不要手工交换。

板级代码与镜像数据是分开的两个文件：`board_fpga.c/h` 是手写代码（PCAP 传输 + PL 控制面），
`board_fpga_bitstream.c/h` 是生成的数据（只有 `fpga_img[]` 与 `FPGA_IMG_LEN`），换比特流只重新生成后者。

## 代码约定

- 外设 API 走 linux/posix 风格，小写下划线，一个功能一个函数：
  `gpio_direction_output/gpio_set_value/gpio_get_value/gpio_toggle`、
  `mio_set_function/mio_enable_pull_up/...`、`uart_putc/uart_puts/uart_getc/uart_try_getc/uart_set_baud/...`。
  寄存器级原子操作 `static inline` 放头文件，会失败的配置函数放 `.c` 返回 0/-1。
- `.c` 文件的函数注释一律 Doxygen 风格（`@brief` / `@param` / `@return` / `@note`），每文件带 `@file`。
- 中断处理函数**不要**放进 `.fast` 段（该段从地址 0 起址，函数地址会是 0，被带 NULL 检查的注册接口拒收）。

