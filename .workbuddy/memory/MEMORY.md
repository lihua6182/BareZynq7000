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
- **J-Link 复位/停机观测三坑（2026-09-29 实测）**：
  1. `r`（NORMAL reset）在这套 J-Link V13 + XC7Z010 上**不能真正复位 CPU**（寄存器快照跨"复位"完全一致）；无重烧时的软重启法：`h` → `SetPC 0xFC000900`(reset_handler) → `g`——startup 注释明确支持调试器热复位，会重初始化 .data/.bss。JFlash 的 InitStep reset 只做 halt。
  2. **stop-mode 读是向停机核注入 load**：读未映射 VA（如 0xFFFC0238）→ 注入指令 translation fault → J-Link 恢复后 abort 生效、固件死在 abort 循环；读无响应的 GP0 地址（PL 从设备缺失/复位中）→ 注入 load 永不完成 → 核卡死在该指令上、连 halt 都失败，**只有断电能解**。探测未知地址前先确认 MMU 里有映射且从设备活着。
  3. CPU 运行中读 RAM 走物理地址，write-back cache 里的最新值看不到（jiffies 读出冻结的垃圾是正常假象）；**要看真值先 `h`**。
- **JFlash RAM loader 压扁固件 RAM（2026-09-29 踩坑）**：JFlash 工程原 `RAMAddr=0x20/RAMSize=0x20000`，每次烧写 SEGGER flash loader（THUMB 程序）装进低 OCM，正好覆盖 .data/.bss/`handlers[]`。烧写后若 CPU 未经 reset_handler 恢复（RAM 不重初始化），GIC/TTC 里残留的 tick 使能 + 垃圾 `handlers[42]`（loader 代码字节，实测 0xF04F2300=`mov.w r3,#0`）→ 野跳转 → prefetch abort。已改 `RAMAddr=0x20000/RAMSize=0x10000`（0x20000-0x30000，所有栈之上）。烧写后仍应断电重启一次最稳。
- **`.fast` 段 VMA 从 0x00000000 起址**（链接脚本里它是 RAM 段第一个段）：
  放进 `.fast` 的函数**地址就是 0**。凡是"取地址传给注册接口、接口带 NULL 检查"的地方，
  绝不能收 `.fast` 里的函数（2026-09-29 就因此让 `tick_irq` 被 `irq_attach` 静默拒绑）。
  中断处理函数一律不要放 `.fast`；只有 QSPI 重配窗口那类必须 OCM 执行的代码才放。

## 中断（2026-09-29 起全部走 IRQ，不用 FIQ）
- 投递配置：`irq_init()` 写 `GICInterface->CTLR = 0x7`（EnableS|EnableNS|AckCtl，**FIQEn=0**，
  与 Xilinx `XScuGic` CPUInitialize 一致），CPSR 用 `__enable_irq()` 解 IRQ、FIQ 保持屏蔽。
  即：源 = Group 0 + 优先级 + 使能，SPI 还要 `GIC_SetTarget(id, 1)`（CPU0），否则 SPI 永不转发。
- 入口：`irq_handler`（`interrupt("IRQ")`，需保存 r0-r12），ID 分发 → EOI；1023 不做 EOI。
- 绑定：`irq_attach(id, fn)` **必须检查返回值**；先绑定再使能，避免电平中断风暴。
- **`interrupt` 属性与 FP（官方依据 + GCC 15.3.1 实测）**：
  - **GCC 手册（ARM Function Attributes）原文**：`interrupt` = "generates function entry and exit
    sequences suitable for use in an interrupt handler"，**通篇不提 FP 寄存器**，没做任何保存承诺。
    `general-regs-only` 属性（须写成 `target("general-regs-only")`）= "no floating-point or Advanced
    SIMD registers should be used ... If the function explicitly uses floating-point code, then the
    compiler gives an error"，行为等同 `-mgeneral-regs-only`（手册："will not impose any restrictions
    on the assembler" → **管不住内联汇编/手写汇编**）。
  - **GCC PR target/94743（r11-1732, 2020）** 是那条 `-Wattributes` 警告的出处，标题即
    "IRQ handler doesn't save scratch VFP registers"。commit 原文要点：
    "The interrupt attribute does not guarantee that the FP registers are saved"；handler 调用其它
    函数时"we'd likely need to save all of them, for lack of knowledge of which registers they
    actually clobber"；**编译器插入的 helper 调用（例如某些 multilib 用 FP 寄存器加速的 memcpy）
    同样会偷偷破坏 FP 状态**。结论：GCC 选择不自动保存，只警告并建议用命令行 `-mgeneral-regs-only`。
  - **GCC PR target/115263（2024）**：`target("general-regs-only")` **消不掉**该警告
    （警告检查的是命令行选项 `TARGET_VFP_BASE` 一路的判断），所以压警告的 pragma 仍必需。
  - **ARM ARM (DDI0406)**：VFP/NEON 状态（D0-D31、FPSCR、FPEXC）全局共享、异常进入时硬件不保存，
    上下文切换必须由软件保存恢复；DDI0406 的 VFP support code 章节还专门讨论了
    "IRQ handler itself might use the VFP coprocessor" 的情形。
  - **本工程风险真实存在**：`zynq_startup.s` 已在 reset 里开 CPACR(CP10/11) 并写 `FPEXC.EN=1`，
    即 VFP/NEON 全程可用。UG585 也确认 A9 带 NEON MPE + SP/DP FPU。
  - 推论：`target("general-regs-only")` **必要但不充分**——它只约束 handler 本体，管不到
    `handlers[id]()` 间接调用的函数和编译器插入的 memcpy 等 helper；要覆盖这些必须用命令行
    `-mgeneral-regs-only`（至少覆盖 ISR 所在的编译单元）。
  - GCC 15.3.1 在本工程的实测：
  - 任何 `interrupt` 函数都会发 `-Wattributes`："FP registers might be clobbered despite
    'interrupt' attribute: compile with '-mgeneral-regs-only'" —— **加不加 `target("general-regs-only")`
    都一样发**，只有整个编译单元加 `-mgeneral-regs-only` 才消除。所以 `zynq_irq.c` 里压制
    `-Wattributes` 的 pragma 目前是必需的（工程未开 -Werror，纯粹为输出干净）。
  - `target("general-regs-only")` 对当前 `irq_handler` 的代码生成**零影响**（汇编逐字节相同），
    它的价值是护栏：函数体内出现 float 会直接编译报错（`argument of type 'float' not permitted
    with '-mgeneral-regs-only'`），而不是静默破坏被中断者的 VFP 状态。
  - GCC 的 interrupt 属性只保存它**实际用到**的通用寄存器（当前 push {r0-r5, ip, lr}），
    不是"保存 r0-r12"；VFP/NEON 状态它一概不管（IRQ 模式下不 banked）。
  - 整个工程加 `-mgeneral-regs-only` 实测（独立构建目录，已清理）：构建通过、零警告、
    `irq_handler` 汇编逐字节不变、`app.bin` 体积不变。它能覆盖**间接调用的用户 handler**
    （真正的 FP 破坏风险点），代价是全工程禁用浮点/printf %f。**未落地，等用户定。**
- 计数/定时：TTC0 计数器 0（channel-1 寄存器组，+0x000）→ **SPI 42 = `Ttc0Timer0_IRQn`**；
  TTC0 三计数器 = SPI 42/43/44。TTC 中断寄存器 **clear-on-read**（读 ISR 即清，不是 W1C）。
  ICDICFR2 复位值 0x55555555 → SPI 默认高电平有效，与 TTC 匹配，无需写 ICFGR。
- 1ms tick 走 TTC0 counter0 interval 中断（`zynq_tick.c`），`tick_mdelay` 自旋在 `jiffies` 上。

## PL 控制面 API（2026-09-29 收口；文件现为 `source/board/board_fpga.c/.h`）
- **所有对 PL 的操作都走 `board_fpga.h`**，不要再裸写 SLCR/devcfg。
  复位 `fpga_reset_assert/release/is_asserted/pulse/state/set/assert_all/release_all`（参数 n = FCLKRESETN[3:0]）；
  电平转换 `fpga_lvl_set/get/off/ps_to_pl/all`（UG585 ch.2 的 0x0 / 0xA / 0xF）；
  状态 `fpga_is_powered`(`MCTRL[PCFG_POR_B]`) / `fpga_is_ready`(`STATUS[PCFG_INIT]`) / `fpga_is_done`(`INT_STS[PCFG_DONE]`)；
  配置 `fpga_config_clear()`（PROG_B 脉冲清 PL）、`fpga_load(img,len)`（devcfg_init + 断言全部复位 + devcfg_load；**成功后仍保持复位**，需调用者给时钟再释放）。
- `FPGA_RST_CTRL`(0xF8000240，复位值 0x01F33F0F)：**仅 bit0-3 可用**（FCLKRESETN[3:0]），其余全 reserved；寄存器写 1 = 断言复位（引脚拉低）。UG585 ch.16 EMIO 示例的软件复位顺序：写 0 → 写 1 → 写 0。
- **架构分层（2026-09-29 用户定）**：`board_fpga.c/h` = PL 代码（PCAP 传输 `devcfg_init/devcfg_load` 已降为 static，
  对外只有 `fpga_*`），`board_fpga_bitstream.c/h` = PL 镜像数据（`fpga_img[]` / `FPGA_IMG_LEN`，由
  `tools/convert_bitstream.py` 生成）。换比特流只重新生成数据对。**driver/ 只留 PS 外设**（gpio、uart）。
- 同一寄存器只有一处实现：`devcfg_init` 用 `fpga_lvl_ps_to_pl()`、`finish` 用 `fpga_lvl_all()`、
  `begin` 用 `fpga_config_clear()`。
- PL 时钟**仍在** `zynq_clock.h` 的 `fclk_*()`，故意不做 fpga_ 前缀包装（避免两套 API）。
- 生成脚本 `tools/convert_bitstream.py` 的 header guard 现在**由输出文件名派生**（`BOARD_FPGA_BITSTREAM_H`），
  VSCode task "Convert PL bitstream" 的输出路径已同步；`source/board/CMakeLists.txt` 的 FATAL_ERROR 检查
  也改查 `board_fpga_bitstream.c`。
- 上板顺序：`fpga_load()` → `fclk_config()` → `fpga_reset_release_all()`（先时钟后释放复位）。

## PL 配置（devcfg/PCAP）
驱动：`source/driver/zynq_devcfg.c/.h`（2026-09-29 从 soc 移入 driver：纯轮询、无 GIC/tick 依赖，board 是唯一消费者），按 UG585 ch.6 PCAP 示例 11 步实现。分 `begin() → 传输 → finish()` 三段，
**PL 复位必须释放（2026-09-29 踩坑）**：`SLCR.FPGA_RST_CTRL`(0xF8000240) 复位值 0xF、四根 FCLK_RESETn 全有效，块设计里 SmartConnect/外设的 ext_reset_in 接 FCLK_RESET0_N。不清零则 PCAP DONE 照样断言（看似配置成功）但 GP0 上任何访问都无响应——`board_init()` 末尾用 `fpga_reset_release_all()` 清（fclk_config 之后，与 FSBL 同序）。
CPU 只轮询 `INT_STS[DMA_DONE]` 与 `[PCFG_DONE]`。DMA 命令格式（FSBL / U-Boot zynqpl.c 同款）：
`SRC_LEN = DEST_LEN = bytes/4`，`DEST_ADDR = 0xFFFFFFFF`（路由到 PCAP）。
对 4KB 规则的正确理解：只需源地址 **64 字节对齐**（对齐是"不跨 4KB 边界"的手段，DMA 内部发 64B burst，4KB 是 64 的整数倍）。
必备动作：`SLCR->PCAP_CLK_CTRL |= 1`（PCAP 时钟，放在 `begin()` 里以支持重复配置）、`CTRL[PCFG_POR_CNT_4K]`(bit29) 清 0（走完整 64k POR）、
DMA 完成 ack 要同时写 `INT_STS[13]` 与 `STATUS[DMA_DONE_CNT]`(29:28)、可用 `MCTRL[PCFG_POR_B]`(bit8) 判断 PL 是否供电（未供电直接报 `DEVCFG_ERR_PL_NO_POWER`）。
参考实现：`C:\WorkSpace\emPowerZynq\BSP\SEGGER\XC7Z007S_emPowerZynq\Setup\BSP_FPGA.c`（`BSP_FPGA_Init`/`_SendFPGAConfigData`）。
电平转换按 UG585 ch.2 走 0x0A →（配置）→ 0x0F（SEGGER 直接用 0x0F，不跟）。
**两条载入路径**（由首 4 字节 magic 自动选择）：
- `ZBSP`（0x5A425350）→ `load_packed()`：RLE 解码进 4KB 的 `stage_buf`（64B 对齐，cacheable OCM），
  每轮 clean L1 D-cache 后发一条 DMA；先在纯内存里校验头，再校验 sync word 与 CRC-32，全程不碰硬件直到 `begin()` 成功。
- 否则 → `load_direct()`：一条 DMA 搬完整帧（源须 64B 对齐、长度 4 的倍数）。
  **2026-09-29 起字节序由脚本内部处理**：`bitstream.bin` 直接用 Vivado `write_bitstream -bin` 的**原生 .bin**（首 128 字节内 `AA 99 55 66`），convert_bitstream.py 自动字节交换到 PCAP 序；不要再手工交换。
硬件事实：Zynq-7000 没有 APB 可写的 PCAP 数据 FIFO，配置数据只能经 DevC DMA 引擎，"手动写"= CPU 分块排队+轮询。
每次 DMA 传输必须 64 字节对齐且不跨 4KB 边界。main.c 里 LED 500ms 慢闪=PL 配置成功，100ms 快闪=失败（查 `pl_status`）。

### 比特流资产链（当前形态，2026-09-29 确认；旧的 objcopy `.bitstream` 段方案已废弃）
`source/board/bitstream.bin` --(`python tools/convert_bitstream.py <in.bin> <out.c> <out.h>`)--> `source/board/board_fpga.c/h`
（VSCode task 名 "Convert PL bitstream"）。产物是 **ZBSP 打包格式**：16 字节 LE 头
（`magic="ZBSP"` / `raw_len` / `packed_len` / `crc32`）+ RLE 载荷（SEGGER `BSP_FPGA.c` 同款编码：
`0`=结束、`1..0x3F`=字面 run、`0x40..0x7F`=字节重复、`0x80..`=零 run）。
当前 `fpga_img[]` **81420 字节**（2026-09-29 晚换的新设计：SmartConnect + **AXI GPIO @ 0x41200000**；
更早一版是 axi_bram_ctrl @ 0x4000_0000 的 64800 字节，已弃）。`board_fpga.c` 缺失时 `source/board/CMakeLists.txt` 会 FATAL_ERROR。
`app.bin` 体积 = 固件约 17 KB + 比特流，**换比特流后会变**（2026-09-29 晚：98376 B）；判断"改动有没有引入开销"
要跟同一次 bitstream 下的基线比，不要跨版本比绝对值。
`board_init()` 里 `devcfg_init()` 后，仅当 `FPGA_IMG_LEN != 0` 才 `devcfg_load()`，
随后 `fclk_config(FCLK0, FCLK_SRC_IO, 10, 1, true)` 给 PL 供 100 MHz，最后清 `FPGA_RST_CTRL` 释放 PL 复位。
Vivado 工程在 `C:\WorkSpace\fpga\project_1`，换外设后从 `impl_1/design_1_wrapper.bin` 复制到 `source/board/bitstream.bin` 再跑转换脚本。

## 代码风格
- **.c 文件的函数注释一律 Doxygen 风格**：`/** ... */` + `@brief` / `@param` / `@return`，
  长背景说明放在 `@brief` 后的空行下面，硬约束用 `@note`；每个 .c 顶部有 `/** @file x.c @brief ... */`。
  static 内部函数同样注释。头文件注释风格暂未统一（仍是 `/* */` 块注释）。
  生成物 `board_fpga.c/h` 的头部由 `tools/convert_bitstream.py` 产出，改头部要改脚本再重新生成。
- `zynq_soc_ip.h` 结构体成员注释放 `;` 之后（`TYPE NAME; /* 注释 */`），全文件统一。
- 改 C 代码后至少用 `-fsyntax-only` 或完整 cmake 构建验证；涉及启动路径的改动需用户上板实测确认。

## 目录与文件现状（2026-09-29 晚）
```
source/app/     app 目标：main.c + newlib 桩（syscalls.c / sysmem.c）
source/board/   board.c/h（板级初始化）、board_fpga.c/h（PL：PCAP 传输 + 控制面，手写）、
                board_fpga_bitstream.c/h（PL 镜像，生成物）、bitstream.bin
source/driver/  zynq_gpio.c/h、zynq_uart.c/h（**只有 PS 外设**；PL 相关已全部归入 board）
source/soc/     zynq_startup.s、zynq_system.c、zynq_clock.c/h、zynq_irq.c/h、zynq_tick.c/h、
                zynq_boot_header.c、zynq_soc_ip.h、zynq_cmsis.h、zynq_common.h、cmsis/（Core(A) 子集）
```
**注意：驱动文件名 2026-09-29 已从 `driver_*.c` 改为 `zynq_*.c`**（`driver_gpio`→`zynq_gpio`、
`driver_uart`→`zynq_uart`），与 soc/ 命名一致；旧名在记忆和文档里出现过，一律以新名为准。

## 板级 / 运行时现状（2026-09-29 晚）
LED = MIO 24（成功 500ms 慢闪 / 失败 100ms 快闪）；UART0 = MIO11 TX / MIO10 RX，115200 8N1 轮询；
tick = TTC0 counter0 interval 中断（SPI 42）1ms，全 IRQ 投递；FCLK0 = IO PLL/10 = 100MHz；
`board_init()` 末尾清 `SLCR.FPGA_RST_CTRL`（复位值 0xF，不清则 PL 逻辑—当前是 AXI GPIO @0x41200000—永不出复位，
PCAP DONE 照样断言但 GP0 访问挂死）。`main()` 打印 `pl_status` 后循环打印 tick 值。

## 外设驱动 API 风格（2026-09-29 用户明确）
**linux/posix 风格，小写下划线，一个功能一个函数。** 不要用 STM32 LL/ HAL 那种大写前缀
（`LL_GPIO_SetOutputPin`）或句柄+初始化结构体。
- GPIO 对标内核 gpio consumer API：`gpio_direction_output(pin, val)` / `gpio_direction_input(pin)` /
  `gpio_set_value(pin, val)` / `gpio_get_value(pin)` / `gpio_toggle(pin)`；另有 `gpio_set_high/set_low`、
  `gpio_get_output`（读输出锁存，区别于 `gpio_get_value` 读 pad）、`gpio_enable_driver/disable_driver`（OEN）、
  `gpio_enable_clock/disable_clock`（APER_CLK_CTRL bit22）。中断常量用内核 `GPIO_IRQ_TYPE_*`
  （LEVEL_LOW/HIGH、EDGE_FALLING/RISING/BOTH）→ 一个 `gpio_set_irq_type(pin, type)` 落到 INT_TYPE/POLARITY/ANY 三个寄存器；
  配套 `gpio_enable_irq/disable_irq/clear_irq(INT_STAT 是 W1C)/irq_pending/irq_enabled`。
- MIO pinmux 单独一组（属 SLCR 不属 GPIO 外设）：`mio_set_function/pull_up/receiver/slew_fast|slow`。
- UART 对标 posix：`uart_putc/puts/write/getc/try_getc`，配置 `uart_set_baud/databits/parity/stopbits`，
  时钟 `uart_enable_clock/disable_clock`，中断 `uart_irq_status/mask/enable/disable/clear`。
- **分层**：寄存器级原子操作 `static inline` 放头文件（不阻塞、不做参数检查，bank 索引 `&0x3` 防越界踩踏）；
  会失败的配置函数放 `.c` 返回 0/-1；阻塞收发与打印放 `.c`。inline 版不引入函数调用开销——
  实测全套改完后 `app.bin` 体积与改前完全一致（当时基线 81756 B；换比特流后基线变成 98376 B，
  比较时必须用同一版比特流）。
- 踩过的坑：C 里 `x &= (cond) ? A : B;` 是安全的（条件运算符优先级高于赋值），但
  `x &= ~(cond) ? A : B;` **是错的**——`~` 只作用于条件，两个分支恒为真。取反请用局部函数/变量先算掩码。
- GCC 新版对 `static const char hex[16] = "0123456789ABCDEF";` 报 `-Wunterminated-string-initialization`，
  写 `hex[]` 不带长度。
