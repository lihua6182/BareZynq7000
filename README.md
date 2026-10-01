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
5. `soc_init()`按UG585第25章的PLL更新序列把时钟配置到UG585表25-3（6:2:1模式）示例1的状态：CPU 667MHz、外设总线CPU_1x 111MHz、QSPI参考200MHz，Flash实际SCLK=100MHz（W25Q128JV四线0x6B读，Config_reg波特率÷2 + LPBK_DLY_ADJ内部回环采样DLY=00 + MIO8/9反馈时钟环回路由）。MMU开启后、QSPI重配前，`xip_warm_cache()`把整个XIP镜像读入统一的L2缓存——实测这块板经刚重配的QSPI线性口的读不可靠，预热保证重配窗口内零flash取指。QSPI参数切换在OCM的`.fast`段中按UG585第12章流程执行：退出linear模式→排空FIFO→改时钟并脉冲复位两个控制器复位域→重编程控制器寄存器→重新进入linear模式。时钟参数：晶振频率（`PS_CLK_HZ`，33.333MHz）在`source/soc/zynq_common.h`中配置，是全工程唯一硬编码的频率；其余所有时钟频率（PLL/CPU/外设参考时钟等）在运行时由`zynq_clock.c`的`clock_freqs_get()`从SLCR寄存器读出。

## 源码结构

```
source/
  app/     可执行目标 app：main.c + newlib 桩（syscalls/sysmem）
  board/   板级：board_fpga（PL 配置 + PL 控制面，手写）+ board_fpga_bitstream（PL 镜像，生成物）
  driver/  PS 外设：zynq_mio（MIO pad/pinmux + GPIO 控制器，模块词 mio_）、zynq_uart
  soc/     zynq_startup.s、zynq_soc（MMU/L2/时钟/QSPI 启动）、zynq_clock（时钟树）、
           zynq_irq（GIC）、zynq_tick（TTC0）、zynq_boot_header、寄存器头 + CMSIS Core(A) 子集
```
前三者在 CMake 里是 `INTERFACE` 库（只传源文件与包含路径，不产生归档），只有 `app` 是真目标。
`doc/code_notes.md` 是源码注释的"长文版"——背景、取舍、与 FSBL/SEGGER/内核的对照都放在那里（UG585 等 PDF 被 `.gitignore` 排除）。

## 板级现状

- LED：MIO 24，推挽输出；PL 配置成功时 500ms 慢闪，失败时 100ms 快闪（`board_pl_status()` 给出失败原因）。
- 串口：UART0，MIO 11 = TX / MIO 10 = RX，115200 8N1，轮询收发，打印 PL 配置结果与 tick 计数。
- tick：TTC0 计数器 0 的 interval 中断（SPI 42），1ms，全部经 IRQ 投递（不用 FIQ）。
- PL：上电即由 `board_init()` 经 DevC/PCAP 配置，FCLK0 = IO PLL / 10 = 100MHz，随后释放 PL 复位（`slcr.FPGA_RST_CTRL` 复位值为 0xF，不清则 GP0 访问挂死）。对 PL 的所有操作走 `board_fpga.h` 的 API：
  `fpga_reset_assert/release/pulse`、`fpga_reset_assert_all/release_all`、`fpga_lvl_ps_to_pl/all/off`、
  `fpga_is_powered/ready/done`、`fpga_config_clear()`（PROG_B 清配置）、`fpga_load()`（配置并保持在复位）；
  配置结果类型是 `enum pl_status`（`PL_OK` / `PL_ERR_*` / `PL_NO_IMAGE`）。
- 当前 PL 设计（Vivado `C:\WorkSpace\fpga\project_1`）：SmartConnect + **axi_bram_ctrl @ 0x4000_0000，8 KB**，
  复位全接 `FCLK_RESET0_N`。`main()` 在 PL 配置成功后对它跑一遍 `bram_test()`（8 KB 写读回，图案 + 其取反两遍）。
- proto_coproc 协议协处理器 @ 0x4000_0000（MioLink 工程 IP v1.2，PL 时钟域 = FCLK0 = 100MHz；
  注意这里与上面 `project_1`/`bram_test` 是两代 PL 设计，**当前板上跑的是 MioLink 这一代**）。
  `main()` 跑 `stress_run()`：stress_prog 让 SHF 引擎连续输出 SPI 字流 30 s
  （**SHF div=1 → SCK=FCLK0/4=25MHz**、133 拍/字），每秒打印 words/s 与引擎效率。
  **注意**：该地址需要 MioLink 工程重新生成的比特流（convert_bitstream.py 流程不变），
  旧的 BRAM-only 镜像下这个地址无从站，访问会挂死 GP0 总线。
  gpio[7:0] 的管脚约束（XDC）在 MioLink 工程里：**gpio[0]=G19(SCK) / gpio[1]=E18(MOSI)** … gpio[7]=G15。

## 构建与烧写

```
cmake --preset BareZynq7000-Debug     # 配置（Ninja，Debug，-Og）
cmake --build build                    # 产物 build/source/app/app.{elf,bin,hex}
```
另有 `BareZynq7000-Release`（`-O2`）——**两个 preset 共用同一个 `build/` 目录**，切换时要重新 configure；
上板实测跑的是 Release 档。构建末尾会自动校验 BootROM 头。烧写用 J-Flash 工程 `scripts/jflash/jflash_zynq7000.jflash`。

烧写的硬性规矩（实测定案，见项目记忆）：J-Flash 的 flash loader 装进低 OCM，会覆盖
`.fast`/`.data`/`.bss`，而挪走 `RAMAddr` 的方案实测无效（J-Flash 只接受 `0x20` 这类窗口，
高段窗口顶部又撞系统栈顶）。因此 **`RAMAddr=0x20/0x20000` 保持原样，每次 J-Flash 烧写后必须
断电重启**（或至少 `SetPC` 到 `reset_handler` 全量重启），绝不能从 halt 直接 `g` 恢复运行。
烧写失败（exit 1）会把板子留在坏状态，继续重试无效，先断电再来。

PL 比特流：`source/board/bitstream.bin`（Vivado 原生 .bin）经
`python tools/convert_bitstream.py <in.bin> <out.c> <out.h>` 生成 `board_fpga_bitstream.c/h`（ZBSP：RLE + CRC32），
VSCode task 名 "Convert PL bitstream"。脚本内部负责字节序转换，不要手工交换。

板级代码与镜像数据是分开的两个文件：`board_fpga.c/h` 是手写代码（PCAP 传输 + PL 控制面），
`board_fpga_bitstream.c/h` 是生成的数据（只有 `fpga_img[]` 与 `FPGA_IMG_LEN`），换比特流只重新生成后者。

## 代码约定

- 外设 API 走 linux/posix 风格，小写下划线，一个功能一个函数：
  `mio_direction_output/mio_set_value/mio_get_value/mio_toggle/mio_set_function/mio_enable_pull_up/...`、
  `uart_putc/uart_puts/uart_getc/uart_try_getc/uart_set_baud/...`。
  寄存器级原子操作 `static inline` 放头文件，会失败的配置函数放 `.c` 返回 0/-1。
- **命名要能自解释**：不看注释就应知道它做什么、操作谁。据此定的词表：
  `pl_*` 是 PL 这一层，PCAP 传输的内部步骤一律 `pcap_*`
  （`pcap_open → pcap_start → pcap_send_block → pcap_wait_pl_done`，另有 `pcap_ack_dma_done`），
  数量宏统一 `*_COUNT`（`MIO_PIN_COUNT` 118 / `MIO_PAD_COUNT` 54 / `FPGA_RESET_LINE_COUNT` 4 / `IRQ_ID_COUNT` 96），
  带单位的量写进名字（`tick_ms`、`ref_hz`、`byte_offset`、`word_count`），
  寄存器值参数跟寄存器同名（`uart_set_baud_gen(uart, baudgen)`、`uart_set_baud_div(uart, bauddiv)`），
  寄存器与位域名保持 UG585 原名（`zynq_soc_ip.h`，改名会丢掉"查手册即所得"）。
- **公共 API 的第一个词必须是模块名**（2026-09-30 用户定）：同一个头文件里的接口只能有一个模块词，
  调用点不看头文件也能知道它属于哪一层。当前的模块词表：

  | 模块词 | 头文件 | 说明 |
  |---|---|---|
  | `mio_` | `zynq_mio.h` | PS 引脚域：MIO pad/pinmux（SLCR，ch.6，pad 0..53）+ GPIO 控制器（ch.14，pin 0..117 含 EMIO），一个模块两半（pin n == pad n, n<54） |
  | `uart_` | `zynq_uart.h` | Cadence UART |
  | `clock_` | `zynq_clock.h` | 时钟树读回（`clock_freqs_get`）+ PL 时钟路由（`clock_fclk_config` / `clock_fclk_enable` / `clock_fclk_freq_get`） |
  | `irq_` | `zynq_irq.h` | GIC 分发 |
  | `tick_` | `zynq_tick.h` | TTC0 毫秒 tick |
  | `board_` | `board.h` | 板级 bring-up |
  | `fpga_` | `board_fpga.h`、`board_fpga_bitstream.h` | PL 配置与控制面 / PL 镜像数据 |
  | `soc_` | `zynq_soc.c` | SoC 启动（MMU/L2/时钟/QSPI） |

  **模块词与文件名一一对应**：`zynq_clock.c/h` ↔ `clock_`、`zynq_soc.c` ↔ `soc_`、`zynq_mio.c/h` ↔ `mio_`……
  两者对不上时**改文件名**（如 `zynq_system.c` → `zynq_soc.c`、`zynq_gpio.c` 并入 `zynq_mio.c`），
  不要为迁就文件名把 API 拆成第二个词。zynq_mio 是“一个模块两半”：pad 半（mux/pull-up/slew，pad 0..53）
  与 pin 半（GPIO 控制器，pin 0..117），两套号码在 0..53 重合，头文件顶注写明。
  文件内的 `static` 辅助函数不加模块词（作用域只在本文件，模块由文件名给），但同一模块内词汇要一致
  ——例如 FCLK 相关一律用 `fclk` 这个词（`enum fclk` / `FCLK0..3` / `clock_fclk_*`）。
- **注释写给人看（2026-09-30 用户定）**：一句话说清做什么/参数是谁，硬约束一行 `@note`；
  背景、取舍、排雷记录放 `doc/code_notes.md`，不进源码。`.c` 函数注释保持 Doxygen 骨架
  （`@brief` / `@param` / `@return`），每文件带 `@file`。
- 中断处理函数**不要**放进 `.fast` 段（该段从地址 0 起址，函数地址会是 0，被带 NULL 检查的注册接口拒收）。

