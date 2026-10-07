# BareZynq7000 项目长期约定

## 项目性质
XC7Z010 (Zynq-7000, Cortex-A9 双核，只用 CPU0) 裸机工程，QSPI **XIP** 运行，无 DDR、无 OS、无 FSBL。
工具链 `${sourceDir}/toolchain/arm-none-eabi-gcc`（GCC 15.3.1）；构建 `cmake --preset BareZynq7000-Debug` + `cmake --build build`（Ninja，Debug，-Og，`-Wall -Wextra -Wshadow -Wpedantic`，未开 -Werror）。
编译选项：`-mcpu=cortex-a9 -marm -mfloat-abi=hard -mfpu=neon`。
当前 `app` 是 **USB CDC ACM 双向吞吐测试**（`main.c`，非真回环：OUT 为 sink 只计数、IN 为 source 发固定图案）：CherryUSB 设备栈 + Zynq USB0 (ChipIdea usb2 2.20a) + 外接 USB3300 ULPI PHY (MIO28..39, 复位 MIO47 高有效)，实测双向 ~30-31 MB/s（4096B 缓冲）。

## 构建分层（CMake）
`source/CMakeLists.txt` 顺序：`soc → middleware → driver → board → app`。
`soc`/`middleware`/`driver`/`board` 均 `INTERFACE` 库（只传源文件+包含路径，不产生归档），只有 `app` 是真可执行目标。`board` 额外 `target_link` `soc`+`driver`+`lzma`；`app` 链 `board`+`driver`+`cherryusb`。
两个 preset `BareZynq7000-Debug/Release` 共用同一 `build/` 目录，切换要重新 configure；上板实测跑 Release。

## 目录与模块（2026-10-04 现状）
```
source/app/      app 目标：main.c + newlib 桩（syscalls.c / sysmem.c）
source/board/    board.c/h（板级 bring-up / 引脚宏 BOARD_*）、board_fpga.c/h
                 （PL PCAP 配置+控制面，手写）、board_fpga_bitstream.c/h
                 （PL 镜像，convert_bitstream.py 生成物）、bitstream.bin
source/driver/   zynq_mio（MIO pad/pinmux + GPIO，mio_）、zynq_uart（uart_）、
                 zynq_usb（USB0 底层接线，usb_）、zynq_dma（PL330，dma_）、
                 zynq_cache（L1/PL310 维护，cache_）、zynq_ppc（proto_coproc，coproc_）、
                 zynq_swd（SWD 访问层，swd_）
source/soc/      zynq_startup.s、zynq_soc.c（MMU/L2/时钟/QSPI 启动，soc_）、
                 zynq_clock（clock_）、zynq_irq（irq_）、zynq_tick（tick_）、
                 zynq_boot_header、zynq_soc.h（全树寄存器头）、zynq_cmsis.h、
                 zynq_common.h、cmsis/（Core(A) 子集）
source/middleware/ cherryusb（vendored v1.6.1：core + cdc + port/chipidea）、
                 lzma（公有领域 LZMA SDK + lzma_stream 流式封装，ZBLZ 解码）
doc/             code_notes.md（"为什么这么写"长文）+ UG585/UG1283 原文（PDF 被 .gitignore 排除）
tools/           convert_bitstream.py（bitstream.bin→board_fpga_bitstream.c/h，默认 ZBLZ=LZMA1+CRC32，--codec rle 退回 ZBSP）、test_cdc_speed.py
```
- `tests/` 目录（dma_test/ppc_stress/swd_test 独立镜像）**不在工作树**，在 stash `SWD测试`（README 描述的是该理想态，当前只构建 `app`）。
- `board_fpga_bitstream.c` 缺失时 `source/board/CMakeLists.txt` FATAL_ERROR；header guard 由输出文件名派生。

## 命名与 API 规矩（用户多轮拍板，勿回退）
- **linux/posix 风格**：小写下划线、一个功能一个函数。不要 STM32 LL/HAL 大写前缀，不要句柄+初始化结构体。
- **公共 API 第一个词 = 模块名 = 文件名里的词**：一个头文件只能有一个模块词。文件名与模块词对不上时**改文件名**（如 `zynq_gpio.c`→并入 `zynq_mio.c`、`zynq_system.c`→`zynq_soc.c`）。
- 模块词表：`mio_`(zynq_mio) / `uart_`(zynq_uart) / `usb_`(zynq_usb) / `dma_`(zynq_dma) / `cache_`(zynq_cache) / `clock_`(zynq_clock) / `irq_`(zynq_irq) / `tick_`(zynq_tick) / `board_`(board) / `fpga_`(board_fpga/bitstream) / `soc_`(zynq_soc) / `coproc_`(zynq_ppc) / `swd_`(zynq_swd)。
- 词表：`pl_*` 结果枚举 `enum pl_status`（`PL_OK`/`PL_ERR_*`/`PL_NO_IMAGE`）；PCAP 内部步骤一律 `pcap_*`；PL 控制面 `fpga_*`。数量宏 `*_COUNT`；带单位量写进名字（`tick_ms`/`ref_hz`/`word_count`）；寄存器值参数跟寄存器同名（`uart_set_baud_gen`）；轮询上限数强序总线周期（`*_POLL_LIMIT`）。`DEVCFG_*` 只留给寄存器宏。
- 分层：寄存器级原子访问 `static inline` 放头文件（不阻塞、不查参，bank 索引 `&0x3` 防越界）；会失败的配置函数放 `.c` 返回 0/-1；阻塞收发/打印放 `.c`。
- 注释：源码只留"给人看"短注（一行 `@brief`+必要 `@param/@return`，硬约束一行 `@note`）；背景/取舍/排雷长文一律 `doc/code_notes.md`。`.c` 保持 Doxygen 骨架，每文件带 `@file`。`zynq_soc.h` 寄存器字段注释与 vendored CMSIS 不动（UG585 原文对照）。
- **中断处理函数一律不放 `.fast`**。

## 关键工程约束（踩过坑，勿重犯）
- **启动顺序**（2026-10-07 收官后）：`mmu_init → mmu_and_caches_enable(含 l2_cache_enable) → sys_clock_init → qspi_reconfig`。重配窗口（端口死区）CPU 必须在 `.fast`(OCM)；窗口外端口始终良好。**`xip_warm_cache` 已删除**——"重配后端口不可靠"经 qspi_probe 五轮上板测量**证伪**：那是回环档 MIO8 反馈死（全零）、违规配置（回环仅 BAUD=000 有效；QSPI_REF 必须 > CPU_1x）、以及**真值用错文件**（烧录用 app.hex，间隙=0xFF 擦除态，而 crc32(app.bin) 间隙=0x00）三重假象。教训：负面"实测"也要审证据链；地面真值本身先证真。
- **QSPI 出货配置**：REF = IO PLL/5 = 200MHz（> CPU_1x=111MHz）+ BAUD /8 = SCLK 25MHz + **LPBK=0 直接采样**（CR=`0x800238D1`）。>40MHz 需 MIO8 反馈钟（板上未接线、pinmux 输入态），回环路径无反馈即死（读全零）→ 回环禁用；25MHz 是免改板的规格内最快档（100MHz 直接采样实测可读但超 FQSPICLK2 规格）。go-live（全缓存逐出后纯端口运行）已上板通过；**收编版冷启动 + FPGA 配置（pl_status=0）亦已上板验证（2026-10-07 终审）**。
- **L2 投机预取必须关**（PL310 AUX_CNT bit28/29 IPFE/DPFE + ACTLR bit1），否则重配几微秒预取踩死 flash→Prefetch Abort。只留需求式行填充。
- **qspi_probe 诊断模块已删除**（定案后拆脚手架，2026-10-07）：连同非缓存别名 `0xFB200000`、`soc_qspi_boot_cfg` 快照、tools/crc32.py 一并移除，完整实现见 git `a7ab2e1`。若将来需再测端口：重建非缓存别名（防污染行）+ .fast 重配窗口 + IRQ 屏蔽即可，安全设计见 code_notes "qspi_probe" 节。**地面真值教训保留：烧录物是 app.hex（间隙 0xFF），对 crc32 必须渲染 hex 而非 bin**。
- **`.fast` 段 VMA 从 0x00000000 起址**：段内函数地址为 0，被带 NULL 检查的 `irq_attach` 静默拒收（tick 死过）。
- **中断全 IRQ 不用 FIQ**：`irq_init()` 写 `GICInterface->CTLR=0x7`（FIQEn=0）；SPI 必须 `GIC_SetTarget(id,1)`。**FP 风险**：GCC `interrupt` 属性不保存 VFP/NEON，`target("general-regs-only")` 必要但不充分（只管本函数体，消不掉 -Wattributes 故 zynq_irq.c 压 pragma）；彻底方案是 `-mgeneral-regs-only`（体积不变，代价禁用浮点/printf %f），**未落地**。
- **J-Link/JFlash 规矩**：不用 SLCR `PSS_RST_CTRL` 软 POR（JTAG 永久失联，须断电）；`r` 不能真复位，软重启=`h`→`SetPC 0xFC000900`→`g`；**每次 JFlash 烧写后必须断电重启**（RAM loader 覆盖 .fast/.data/.bss）；stop-mode 读无映射/无响应地址会卡死核（只有断电能解）；烧写失败 exit1 先断电再试。
- **PL PCAP**（UG585 ch.6 11 步，纯轮询）：源地址 **64B 对齐**即可（DMA 64B burst）；`pcap_start()` 置 `SLCR->PCAP_CLK_CTRL|=1`+清 `CTRL[PCFG_POR_CNT_4K]`(bit29)；完成同时写 `INT_STS[13]` 与 `STATUS[DMA_DONE_CNT]`(29:28)；`MCTRL[PCFG_POR_B]`=0→`PL_ERR_NO_POWER`；DMA 命令 `SRC_LEN=DEST_LEN=bytes/4`、`DEST_ADDR=0xFFFFFFFF`。Zynq 无 APB 可写 PCAP FIFO，配置只能经 DevC DMA。
- **`FPGA_RST_CTRL`(0xF8000240) 复位值 0x01F33F0F**：仅 bit0-3 可用；不清则 PCAP DONE 照断言但 GP0 访问挂死。`board_init()` 序：`fpga_load()`→`clock_fclk_config`→`fpga_reset_release_all()`。
- **缓存一致性**（zynq_cache）：中央互连主设备（USB/ENET/SD/DMA）不嗅探 PL310→`cache_clean`/`cache_invalidate` **L1+L2 双层**；ACP 主设备（DevC/PCAP）硬件保证 L2 一致→`cache_clean_acp` 只清 L1。顺序坑：L1 循环→DSB→L2 循环→CACHE_SYNC→DSB（否则 L2 清到旧行，DMA 读旧数据）。
- 当前 PL 设计（MioLink，`C:\WorkSpace\fpga\project_1` 之外）：**proto_coproc @ 0x4000_0000**（PL AXI4-Lite IP，BRAM 跑 32 位程序，PL gpio 移位 SPI/SWD/JTAG 时序）；FCLK0=IO PLL/10=100MHz、FCLK1=IO PLL/20=50MHz（复位全接 FCLK_RESET0_N）。

## 比特流资产链
`bitstream.bin`（Vivado 原生 .bin）→ `convert_bitstream.py` → `board_fpga_bitstream.c/h`（ZBLZ=raw LZMA1+CRC32，~19.6:1，app.bin 247K→156K；`--codec rle` 退回 ZBSP RLE ~9.9:1）。运行时读首 4 字节 magic 选 LZMA/RLE 路径，解码进 64B 对齐 staging buffer 再 PCAP。**字节序脚本内部处理，勿手工交换**（大端 .bin 直接喂症状是 DMA 照推但 PL 全忽略、几秒后只报 NO_DONE）。换比特流：复制 Vivado `impl_1/design_1_wrapper.bin` 到 `bitstream.bin` 再跑脚本。

## 板级 / 运行时现状
- LED = MIO24（成功 500ms 慢闪/失败 100ms 快闪）；UART0 = MIO11 TX/MIO10 RX，115200 8N1 轮询（newlib printf，`__io_putchar`→`uart_putc`）。
- tick = TTC0 counter0 interval（SPI 42）1ms，全 IRQ；console 先于 `board_init()`（失败全程可见）。
- 控制台 printf（newlib）：`--specs=nano.specs`；`_sbrk` 用链接脚本 `__heap_start__/__heap_end__`（堆扩到 4KB，否则 printf malloc 失败退无缓冲）；`_isatty=1`→行缓冲。

## git / 未提交状态（2026-10-04）
- HEAD = `46b24df`。README 已是结构化重写版。
- 未提交改动（WIP）：`main.c` 移除 DMA 三相自检、改为纯 CDC 回环；`board.h` 抽出 `BOARD_*` 引脚宏 + `board_console_init/board_led_init/board_led_toggle`；`scripts/jflash/jflash_zynq7000.jflash` 删除（改用 `scripts/jflash/flash_app.jlink` 管线）；`zynq_swd.c/h` **新增未跟踪**（SWD 访问层，配合 zynq_ppc）。
- `tests/` 仍在 stash `SWD测试`（dma_test/ppc_stress/swd_test）。
- 体积基线：app.bin（含 LZMA 比特流）约 156KB；改动验证须同版比特流比，或注释-only 看 md5 不变。
- 上板验证：USB CDC 双向 ~30MB/s 已实测；DMA 自检/PL 分层/驱动重构部分只过编译，未全量上板复测。
