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
5. `soc_init()`按UG585第25章的PLL更新序列把时钟配置到UG585表25-3（6:2:1模式）示例1的状态：CPU 667MHz、外设总线CPU_1x 111MHz、QSPI参考 IO PLL/5 = 200MHz，Flash实际SCLK=25MHz（W25Q128JV四线0x6B读，BAUD /8 直接采样：CR=`0x800238D1`，LPBK_DLY_ADJ保持复位值；MIO8/9不复用——板上未接线，>40MHz所需的SCLK_FB反馈钟不可用，25MHz是免改板的规格内最快档）。启动顺序：`mmu_init → mmu_and_caches_enable(含L2) → sys_clock_init → qspi_reconfig`；重配窗口（端口死区）内CPU必须处于OCM的`.fast`段执行，L2投机预取保持关闭（否则预取踩死flash）。QSPI参数切换在`.fast`段中按UG585第12章流程执行：退出linear模式→排空FIFO→改时钟并脉冲复位两个控制器复位域→重编程控制器寄存器→重新进入linear模式。曾用`xip_warm_cache()`把整个XIP镜像预灌L2缓存，2026-10-07经qspi_probe五轮上板测量定案后**已删除**——端口在合法配置下可靠（历史排查全记录见`doc/code_notes.md`）。时钟参数：晶振频率（`PS_CLK_HZ`，33.333MHz）在`source/soc/zynq_common.h`中配置，是全工程唯一硬编码的频率；其余所有时钟频率（PLL/CPU/外设参考时钟等）在运行时由`zynq_clock.c`的`clock_freqs_get()`从SLCR寄存器读出。

## 源码结构

```
  app/        可执行目标 app：main.c（CDC ACM 双向吞吐基准）+ newlib 桩（syscalls/sysmem）
  board/      板级：board（bring-up + BOARD_* 引脚宏 + 控制台绑定）、board_usb（USB0 板级接线）、
              board_fpga（PL 配置 + PL 控制面，手写）、board_fpga_bitstream（PL 镜像，生成物）
  driver/     PS 外设：zynq_mio（MIO pad/pinmux + GPIO 控制器，模块词 mio_）、zynq_uart、
              zynq_usb（USB0 控制器原语 + dcache 钩子，模块词 usb_）、zynq_dma（PL330 微码搬运）、
              zynq_cache（L1/PL310 维护）、zynq_ppc（proto_coproc 访问）、zynq_swd（SWD 访问层）
  soc/        zynq_startup.s、zynq_soc（MMU/L2/时钟/QSPI 启动）、zynq_clock（时钟树）、
              zynq_irq（GIC）、zynq_tick（TTC0）、zynq_boot_header、寄存器头 + CMSIS Core(A) 子集
  middleware/ cherryusb（vendored v1.6.1 最小设备栈：usbd_core + CDC ACM + chipidea port）、
              lzma（公有领域 LZMA SDK + 流式封装，ZBLZ 解码）
```
`driver`/`board`/`soc`/`middleware` 在 CMake 里是 `INTERFACE` 库（只传源文件与包含路径，不产生归档），只有 `app` 是真目标。
`doc/code_notes.md` 是源码注释的"长文版"——背景、取舍、与 FSBL/SEGGER/内核的对照都放在那里（UG585 等 PDF 被 `.gitignore` 排除）。

## 板级现状

- LED：MIO 24，推挽输出；PL 配置成功时 500ms 慢闪，失败时 100ms 快闪（`board_pl_status()` 给出失败原因）。
- 串口：UART0，MIO 11 = TX / MIO 10 = RX，115200 8N1，轮询收发，打印 PL 配置结果与 tick 计数。
  newlib 控制台（`__io_putchar/__io_getchar` 强定义）也收在 board.c，`printf` 即走板级串口。
- tick：TTC0 计数器 0 的 interval 中断（SPI 42），1ms，全部经 IRQ 投递（不用 FIQ）。
- PL：上电即由 `board_init()` 经 DevC/PCAP 配置，FCLK0 = IO PLL / 10 = 100MHz，随后释放 PL 复位（`slcr.FPGA_RST_CTRL` 复位值为 0xF，不清则 GP0 访问挂死）。对 PL 的所有操作走 `board_fpga.h` 的 API：
  `fpga_reset_assert/release/pulse`、`fpga_reset_assert_all/release_all`、`fpga_lvl_ps_to_pl/all/off`、
  `fpga_is_powered/ready/done`、`fpga_config_clear()`（PROG_B 清配置）、`fpga_load()`（配置并保持在复位）；
  配置结果类型是 `enum pl_status`（`PL_OK` / `PL_ERR_*` / `PL_NO_IMAGE`）。
- 上一代 PL 设计（Vivado `C:\WorkSpace\fpga\project_1`）：SmartConnect + **axi_bram_ctrl @ 0x4000_0000，8 KB**，
  复位全接 `FCLK_RESET0_N`（配套的 `bram_test()` 已随 `main()` 改造移除）。
- proto_coproc 协议协处理器 @ 0x4000_0000（MioLink 工程 IP v1.2，PL 时钟域 = FCLK0 = 100MHz；
  **当前板上跑的是 MioLink 这一代**）。驱动 `driver/zynq_ppc.*`/`zynq_swd.*` 已入树；
  PPC 侧例程的独立测试镜像（tests/ 下 dma_test/ppc_stress/swd_test）仍在 stash "SWD测试"。
  `main()` 现在跑的是下面的 USB CDC 双向吞吐基准。
- **USB CDC ACM 双向吞吐基准**（`main()` 当前内容）：Zynq-7000 的 USB 控制器是 **ChipIdea usb2 2.20a**
  （TRM App.B：`ID` 复位值 0xE441FA05，USBCMD@0x140/ENDPTCTRL@0x1C0 一族——和 HPM 的 USB IP
  同族，**不是** DWC2，别选错 port），USB0 经 **MIO 28..39 的 ULPI 口**外接 **USB3300**（3.3V bank，
  SLEW FAST），**PHY 复位脚 MIO 47**（USB3300 的 RESET 引脚高有效，直连：高=复位/低=运行；GPIO 推挽）。设备栈是 vendored 的
  **CherryUSB v1.6.1**（`source/middleware/cherryusb/`，`port/chipidea`，无 OS：EP0 全在 IRQ 上下文，
  不用 osal）；板级接线收在 `source/board/`——`BOARD_USB_*` 宏（board.h：控制器实例与 PHY
  复位脚）+ board_usb.c 的 CherryUSB 钩子 `usb_dc_low_level_init()` 板级实现，拉起顺序：
  ULPI 引脚复用 → `USB0_CLK_CTRL=0x00101941`（MIO ULPI 时钟输入）→ `APER_CLK_CTRL[2]`
  → MIO47 复位脉冲 → `USB_RST_CTRL` 脉冲 → GIC SPI 53；控制器侧原语（`usb_ulpi_read_reg`/
  `usb_phy_link_check`，模块词 `usb_`）与 dcache 钩子留在 `driver/zynq_usb.c`，应用侧
  诊断打印在 main.c（`usb_diag_print()`）。
  vendor 副本与上游唯一差异：`USBMODE.SLOM` 按 Zynq 极性写 1（=关闭 setup lockout，配 tripwire）。
  测速模式（配合 `tools/test_cdc_speed.py`，DTR 门控两相）：DTR=0 下行相——收完即丢、
  立即续挂 4096B 读（排水）；DTR=1 上行相——主循环轮询续写 4096B 压满 IN 管道。
  数据缓冲与 QH/QTD 链表都在 `.no_cache_ram`（OCM 高 64KB，DMA 可达，免 cache 维护），
  UART 每 5s 打印 rx/tx 计数。实测（Win10 usbser，高速枚举）：
  **双向 ~30-31 MB/s**（4096B 缓冲；2048B 时 ~28-30 MB/s）。

## 构建与烧写

```
cmake --preset BareZynq7000-Debug     # 配置（Ninja，Debug，-Og）
cmake --build build                    # 产物 build/source/app/app.{elf,bin,hex}
```
另有 `BareZynq7000-Release`（`-O2`）——**两个 preset 共用同一个 `build/` 目录**，切换时要重新 configure；
上板实测跑的是 Release 档。构建末尾会自动校验 BootROM 头。

烧写走 VSCode task **"Flash (J-Link QSPI XIP)"**：向
`"C:\Program Files\SEGGER\JLink_V956\JLink.exe" -device XC7Z010 -if JTAG -speed 8000 -jtagconf 6,1 -autoconnect 1`
管道喂 `loadfile <镜像.hex>`（pickString 可选 app 或 tests 镜像）→ `w4 0xF8000008 0xDF0D`（SLCR 解锁）
→ `w4 0xF8000200 0x1`（PSS_RST_CTRL 软 POR，脚本化重启）→ `Sleep 200` → `qc`。
旧的 J-Flash 工程与 `scripts/jflash/` 已删（`JFlashSPI_CL.exe -open` 在本板报
"Failed to auto-detect SPI Flash"，勿再踩）。

烧写的硬性规矩（实测定案，见项目记忆）：flash loader 装进低 OCM，会覆盖 `.fast`/`.data`/`.bss`，
因此**烧写后必须重启**——上面的管线已用软 POR 完成之；手动场景可断电，或 J-Link
`h` → `wreg cpsr 0x1D3 / wreg pc 0xFC000900` → `g`（reset_handler 自带缓存/MMU 清理，等于
软冷启动；`r` 命令不能真复位，也绝不能从 halt 直接 `g` 恢复运行）。
烧写失败（exit 1）会把板子留在坏状态，继续重试无效，先断电再来。
stop-mode 下读无映射/无响应的地址会卡死核，只有断电能解。

PL 比特流：`source/board/bitstream.bin`（Vivado 原生 .bin）经
`python tools/convert_bitstream.py <in.bin> <out.c> <out.h>` 生成 `board_fpga_bitstream.c/h`
（默认 ZBLZ：raw LZMA1 + CRC32，~19.6:1；`--codec rle` 退回 ZBSP：RLE + CRC32，~9.9:1），
VSCode task 名 "Convert PL bitstream"。脚本内部负责字节序转换，不要手工交换。
解码器在 `source/middleware/lzma/`（公有领域 LZMA SDK + 流式封装，`lzma_stream.c/h`）。

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
  寄存器与位域名保持 UG585 原名（`zynq_soc.h`，改名会丢掉"查手册即所得"）。
- **公共 API 的第一个词必须是模块名**（2026-09-30 用户定）：同一个头文件里的接口只能有一个模块词，
  调用点不看头文件也能知道它属于哪一层。当前的模块词表：

  | 模块词 | 头文件 | 说明 |
  |---|---|---|
  | `mio_` | `zynq_mio.h` | PS 引脚域：MIO pad/pinmux（SLCR，ch.6，pad 0..53）+ GPIO 控制器（ch.14，pin 0..117 含 EMIO），一个模块两半（pin n == pad n, n<54） |
  | `uart_` | `zynq_uart.h` | Cadence UART |
  | `usb_` | `zynq_usb.h` | USB0 控制器原语（ULPI viewport 寄存器读）+ CherryUSB dcache 钩子；板级接线在 `board_usb.c` |
  | `dma_` | `zynq_dma.h` | PS DMAC（PL330）微码式内存搬运，轮询完成 |
  | `cache_` | `zynq_cache.h` | L1/PL310 缓存维护（DMA 一致性：中央互连双层 / ACP 单层）|
  | `clock_` | `zynq_clock.h` | 时钟树读回（`clock_freqs_get`）+ PL 时钟路由（`clock_fclk_config` / `clock_fclk_enable` / `clock_fclk_freq_get`） |
  | `irq_` | `zynq_irq.h` | GIC 分发 |
  | `tick_` | `zynq_tick.h` | TTC0 毫秒 tick |
  | `board_` | `board.h` | 板级 bring-up 与外设接线（board.c：console/LED/PL；board_usb.c：USB0 接线 + CherryUSB 钩子 `usb_dc_low_level_init`，名字是 middleware 契约） |
  | `fpga_` | `board_fpga.h`、`board_fpga_bitstream.h` | PL 配置与控制面 / PL 镜像数据 |
  | `soc_` | `zynq_soc.c` | SoC 启动（MMU/L2/时钟/QSPI） |
  | `coproc_` | `zynq_ppc.h` | proto_coproc 协议协处理器访问（PL AXI4-Lite IP @ 0x4000_0000，SPI/SWD/JTAG 移位时序） |
  | `swd_` | `zynq_swd.h` | SWD 访问层（CMSIS-DAP 风格，经 proto_coproc 移位；PPC 侧程序 `swd_dap.s`） |

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

