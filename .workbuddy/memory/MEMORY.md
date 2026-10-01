# BareZynq7000 项目长期约定

## 项目性质
XC7Z010 (Zynq-7000, Cortex-A9 双核，只用 CPU0) 裸机工程，QSPI **XIP** 运行，无 DDR、无 OS、无 FSBL。
工具链 `${sourceDir}/toolchain/arm-none-eabi-gcc`（GCC 15.3.1）；构建 `cmake --preset BareZynq7000-Debug` + `cmake --build build`（Ninja，Debug，-Og，`-Wall -Wextra -Wshadow -Wpedantic`，未开 -Werror）。
编译选项：`-mcpu=cortex-a9 -marm -mfloat-abi=hard -mfpu=neon`。

## 目录与文件现状（2026-09-30）
```
source/app/     app 目标：main.c + newlib 桩（syscalls.c / sysmem.c）
source/board/   board.c/h（板级 bring-up）、board_fpga.c/h（PL 代码，手写）、
                board_fpga_bitstream.c/h（PL 镜像，生成物）、bitstream.bin
source/driver/  zynq_mio.c/h（MIO pad/pinmux + GPIO 控制器，模块词 mio_）、zynq_uart.c/h（**只留 PS 外设**）
source/soc/     zynq_startup.s、zynq_soc.c、zynq_clock.c/h、zynq_irq.c/h、zynq_tick.c/h、
                zynq_boot_header.c、zynq_soc_ip.h、zynq_cmsis.h、zynq_common.h、cmsis/（Core(A) 子集）
doc/            code_notes.md（源码"为什么这么写"的归档）+ UG585/UG1283 原文（PDF 被 .gitignore 排除）
```
- 前三者在 CMake 里都是 `INTERFACE` 库（只传源文件与包含路径，不产生归档），只有 `app` 是真目标。
- 改名沿革：驱动 `driver_*.c` → `zynq_*.c`（09-29）；`zynq_system.c` → **`zynq_soc.c`**（API 是 `soc_init`）；
  `zynq_gpio.c/h` 并入 **`zynq_mio.c/h`**（09-30，模块定名 mio）。旧名一律以现名为准。
- `tools/gen_mio_macros.py` 已删除（MIO 选择宏改为手写常量块，见 zynq_mio.h）。

## 命名与 API 规矩（用户多轮拍板，勿回退）
- **linux/posix 风格**：小写下划线、一个功能一个函数。**不要** STM32 LL/HAL 的大写前缀（`LL_GPIO_SetOutputPin`），
  也不要句柄+初始化结构体。
- **公共 API 第一个词 = 模块名，且模块词 = 文件名里的那个词**：一个头文件只能有一个模块词，
  调用点不看头文件就知道它属于哪一层。文件名与模块词对不上时**改文件名**，不要拆出第二个词。
  | 模块词 | 文件 |
  |---|---|
  | `mio_` | `zynq_mio.h`（PS 引脚域：pad 半 SLCR ch.6 mux/pull-up/slew，pad 0..53；pin 半 GPIO 控制器 ch.14，pin 0..117 含 EMIO；pin n == pad n, n<54） |
  | `uart_` `clock_` `irq_` `tick_` | `zynq_uart.h` / `zynq_clock.h` / `zynq_irq.h` / `zynq_tick.h` |
  | `board_` `fpga_` `soc_` | `board.h` / `board_fpga.h`+`board_fpga_bitstream.h` / `zynq_soc.c` |
- **词表**：PL 层用 `pl_*`（结果枚举 `enum pl_status`：`PL_OK` / `PL_ERR_*` / `PL_NO_IMAGE`，**不再叫 devcfg_status**），
  PCAP 传输内部步骤一律 `pcap_*`（`pcap_open → pcap_start → pcap_send_block → pcap_wait_pl_done`、`pcap_ack_dma_done`，
  不用 `begin()/finish()` 这种泛名）；PL 控制面仍是 `fpga_*`（`fpga_reset_*` / `fpga_lvl_*` / `fpga_is_*`）。
  数量宏一律 `*_COUNT`（`MIO_PIN_COUNT` 118 / `MIO_PAD_COUNT` 54 / `FPGA_RESET_LINE_COUNT` 4 / `IRQ_ID_COUNT` 96）；
  带单位的量写进名字（`tick_ms` 原 jiffies、`ref_hz`、`word_count`、`byte_offset`）；
  寄存器值参数跟寄存器同名（`uart_set_baud_gen(uart, baudgen)`、`find_baud_divisors(ref_hz, baud, baudgen, bauddiv)`）；
  轮询用 `busy_wait_loops` / `wait_pl_init(ready)` / `DMA_DONE_POLL_LIMIT`。
  `DEVCFG_*` 前缀只留给寄存器宏。
- 文件内 `static` 辅助函数不加模块词，但模块内词汇要统一（FCLK 相关一律 `fclk`）。
- 分层：寄存器级原子操作 `static inline` 放头文件（不阻塞、不做参数检查，bank 索引 `&0x3` 防越界踩踏）；
  会失败的配置函数放 `.c` 返回 0/-1；阻塞收发与打印放 `.c`。

## 代码风格与注释
- **注释写给人看**：一句话 `@brief` + 必要 `@param/@return`，硬约束一行 `@note`；
  背景/取舍/排雷长文一律放 **`doc/code_notes.md`**（与本 memory 互补：code_notes 记"为什么这么写"，memory 记"当时怎么踩的"）。
  `.c` 函数注释保持 Doxygen 骨架（`@brief`/`@param`/`@return`），每文件带 `@file`。
  `zynq_soc_ip.h` 的寄存器字段注释与 vendored CMSIS 不动（UG585 原文对照）；生成物头部由 `convert_bitstream.py` 产出。
- `zynq_soc_ip.h` 结构体成员注释放 `;` 之后（`TYPE NAME; /* 注释 */`），全文件统一。
- 改 C 代码后至少完整 cmake 构建验证；涉及启动路径的改动需用户上板实测确认。
- 坑：C 里 `x &= (cond) ? A : B;` 安全，但 `x &= ~(cond) ? A : B;` **是错的**（`~` 只作用于条件）。取反要先算掩码。
  GCC 新版对 `static const char hex[16] = "...";` 报 `-Wunterminated-string-initialization`，写 `hex[]` 不带长度。
- Bash 工具的 heredoc 会吃一层反斜杠（`\n` 变真换行），批量改注释/精确匹配一律先 Write 落 .py 脚本再执行，匹配失败即报 MISS 终止。

## CMSIS 范围（2026-09-28 用户明确）
**只要 CMSIS-Core(A)，不要 CMSIS-Driver 框架。** vendored 子集仅用于 intrinsics（`__DSB/__ISB/__DMB`）、
CP15 访问器、GIC API（0xF8F01000/0xF8F00100）、PL310 L2C API、L1C/MMU 辅助。
`source/soc/zynq_cmsis.h` 是唯一设备适配头（设备宏 + IRQn_Type），必须在 include `core_ca.h` 之前定义。
**禁止**引入 `Driver_*.h`、RTOS2、RTE_Components。完整副本在 `C:\WorkSpace\CMSIS_6-6.3.0`（A-profile 头文件在
`CMSIS/Core/Include/a-profile/`，**不是** `Core/A/Include`），用于核对（结论：逐字节一致，零本地改动；
上游 6.3.0 的 `cmsis_version.h` 里 Core(A) 版本宏写 6.1.0 是组件版本滞后，非错误）。

## 硬件与启动的硬约束（踩过坑，勿重犯）
- QSPI 重配窗口 CPU 必须在 `.fast`(OCM) 或有缓存兜底，禁止 MMU/cache 关闭时裸取指 flash。
  顺序固定：`mmu_init → mmu_and_caches_enable(含 l2_cache_enable) → xip_warm_cache → sys_clock_init → qspi_reconfig`。
- **L2 投机预取必须关**（AUX_CNT bit28/29 IPFE/DPFE + ACTLR bit1），否则重配那几微秒预取踩死 flash → Prefetch Abort。
  只保留需求式行填充。
- MMU 页表在 OCM3 (0xFFFF0000) `non-cache` 段；OCR 地址常量表见 `zynq_soc_ip.h`。
- **`.fast` 段 VMA 从 0x00000000 起址**：放进 `.fast` 的函数**地址就是 0**，取地址传给带 NULL 检查的注册接口会被静默拒收
  （`tick_irq_handler` 就这么被 `irq_attach` 拒过）。**中断处理函数一律不放 `.fast`**，只有 QSPI 重配窗口那类必须 OCM 执行的代码才放。
- **OCM_CFG=0x18 是本板正常值**，不要把 SWAP 位当损坏证据；读 flash 内容前先查**当前** map 的符号地址
  （printf 版 .rodata 变大后 `fpga_img` 挪过地址，拿旧地址读出 "bram" 字符串曾误判成 flash 损坏）。

## 中断（2026-09-29 起全部走 IRQ，不用 FIQ）
- `irq_init()` 写 `GICInterface->CTLR = 0x7`（EnableS|EnableNS|AckCtl，**FIQEn=0**，与 Xilinx `XScuGic` 一致），
  CPSR 用 `__enable_irq()` 解 IRQ、FIQ 保持屏蔽。源 = Group 0 + 优先级 + 使能，**SPI 还要 `GIC_SetTarget(id, 1)`**（CPU0），否则永不转发。
- 入口 `irq_handler`（`interrupt("IRQ")`）→ ID 分发 → EOI；1023 不做 EOI。`irq_attach(id, fn)` **必须检查返回值**；先绑定再使能。
- 计数/定时：TTC0 counter0（channel-1 寄存器组 +0x000）→ **SPI 42 = `Ttc0Timer0_IRQn`**；三计数器 = SPI 42/43/44。
  TTC 中断寄存器 **clear-on-read**（读即清，不是 W1C）。ICDICFR2 复位值 0x55555555 → SPI 默认高电平有效，无需写 ICFGR。
- 1ms tick 走 TTC0 counter0 interval 中断（`zynq_tick.c`），`tick_mdelay` 自旋在 `tick_ms` 上。
- **FP 风险（有官方依据）**：`zynq_startup.s` 在 reset 里开 CPACR(CP10/11) + `FPEXC.EN=1`，VFP/NEON 全程可用；
  而 ARM ARM (DDI0406) 明确 VFP/NEON 状态异常进入时**硬件不保存**、IRQ 模式下不 banked，GCC 的 `interrupt` 属性
  **不保存 FP 寄存器**（手册通篇未承诺；GCC PR target/94743 即为此发出 `-Wattributes` 警告，还指出编译器插入的
  memcpy 等 helper 也可能破坏 FP 状态）。所以：
  - `target("general-regs-only")` **必要但不充分**——只约束 handler 本体，管不到 `irq_handlers[id]()` 的间接调用与 helper；
    对当前代码生成零影响（汇编逐字节相同），价值是护栏（函数体出现 float 直接报错）。
  - 该 attribute **消不掉**那条警告（PR target/115263：警告判断的是命令行选项），故 `zynq_irq.c` 压 `-Wattributes` 的 pragma 仍必需。
  - GCC 的 interrupt 属性只保存它**实际用到**的通用寄存器（当前 push {r0-r5, ip, lr}），不是 r0-r12。
  - 彻底方案是命令行 `-mgeneral-regs-only`（实测全工程可零警告构建、体积不变，代价是禁用浮点/printf %f）。**未落地，等用户定。**

## J-Link / JFlash 排雷（2026-09-29 实测）
1. 不要用 SLCR `PSS_RST_CTRL` 软 POR（`w4 0xF8000008 0xDF0D; w4 0xF8000244 0x1`）——JTAG 永久无法 halt，必须断电。
2. `r`（NORMAL reset）在这套 J-Link V13 + XC7Z010 上**不能真正复位 CPU**；软重启法：`h` → `SetPC 0xFC000900`(reset_handler) → `g`
   （startup 支持调试器热复位，会重初始化 .data/.bss）。JFlash InitStep 的 reset 只做 halt。
3. **stop-mode 读是向停机核注入 load**：读未映射 VA → 注入 translation fault，恢复后固件死在 abort 循环；
   读无响应的 GP0 地址 → 注入 load 永不完成 → 核卡死、连 halt 都失败，**只有断电能解**。探测前先确认 MMU 有映射且从设备活着。
4. CPU 运行中读 RAM 走物理地址，看不到 write-back cache 里的新值（读出冻结的 tick 是正常假象）→ **要看真值先 `h`**。
5. **JFlash RAM loader 与应用 RAM 重叠**：loader 装进低 OCM，覆盖 .fast/.data/.bss/`irq_handlers[]`。
   挪 RAMAddr 无效（JFlash 只接受 0x20/0x20000 这类窗口），窗口顶又撞系统栈顶。**规矩：每次 JFlash 烧写后必须断电重启
   （或至少 SetPC reset_handler 全量重启），绝不能从 halt 直接 `g` 恢复。** 烧写失败(exit 1)会把板子留在坏状态，继续重试没用，先断电。

## PL：控制面 API（全走 `board_fpga.h`，禁止裸写 SLCR/devcfg）
- 复位（参数 line = FCLKRESETN[0..3]）：`fpga_reset_assert/release/is_asserted/pulse/state/set/assert_all/release_all`。
  `FPGA_RST_CTRL`(0xF8000240，复位值 0x01F33F0F)：**仅 bit0-3 可用**，其余 reserved；写 1 = 断言复位（引脚拉低）。
  软件复位顺序（UG585 ch.16 EMIO 示例）：写 0 → 写 1 → 写 0。
- 电平转换（UG585 ch.2）：`fpga_lvl_set/get/off/ps_to_pl/all`（0x0 / 0xA / 0xF）。
- 状态：`fpga_is_powered`(`MCTRL[PCFG_POR_B]`) / `fpga_is_ready`(`STATUS[PCFG_INIT]`) / `fpga_is_done`(`INT_STS[PCFG_DONE]`)。
- 配置：`fpga_config_clear()`（PROG_B 脉冲清 PL）、`fpga_load(img,len)`（**成功后仍保持复位**，需调用者给时钟再释放）。
- **PL 复位必须释放**：复位值 0xF 使四根 FCLK_RESETn 全有效，块设计里 SmartConnect/外设的 `ext_reset_in` 接 FCLK_RESET0_N。
  不清零则 PCAP DONE 照样断言（看似成功）但 GP0 访问无响应/挂死。`board_init()` 在 `clock_fclk_config` **之后**用
  `fpga_reset_release_all()` 清（与 FSBL 同序）。
- PL 时钟仍在 `zynq_clock.h` 的 `clock_fclk_*()`，故意不做 `fpga_` 包装（避免两套 API）。
- 同一寄存器只有一处实现：`pcap_open` 用 `fpga_lvl_ps_to_pl()`、`pcap_wait_pl_done` 用 `fpga_lvl_all()`、`pcap_start` 用 `fpga_config_clear()`。

## PL：PCAP 配置要点（UG585 ch.6 示例 11 步）
- 纯轮询、无 GIC/tick 依赖；分 `pcap_start() → 传输 → pcap_wait_pl_done()` 三段。CPU 只轮询 `INT_STS[DMA_DONE]` 与 `[PCFG_DONE]`。
- DMA 命令格式（FSBL / U-Boot zynqpl.c 同款）：`SRC_LEN = DEST_LEN = bytes/4`，`DEST_ADDR = 0xFFFFFFFF`（路由到 PCAP）。
- 4KB 规则的正确理解：只需源地址 **64 字节对齐**（DMA 内部发 64B burst，4KB 是 64 的整数倍），不必拆块。
- 必备动作：`SLCR->PCAP_CLK_CTRL |= 1`（放 `pcap_start()` 里以支持重配置）、`CTRL[PCFG_POR_CNT_4K]`(bit29) 清 0（走完整 64k POR）、
  DMA 完成 ack 要同时写 `INT_STS[13]` 与 `STATUS[DMA_DONE_CNT]`(29:28)；`MCTRL[PCFG_POR_B]`(bit8)=0 直接报 `PL_ERR_NO_POWER`。
- 硬件事实：Zynq-7000 **没有 APB 可写的 PCAP 数据 FIFO**，配置数据只能经 DevC DMA 引擎。
- 参考实现：`C:\WorkSpace\emPowerZynq\BSP\SEGGER\XC7Z007S_emPowerZynq\Setup\BSP_FPGA.c`。

## 比特流资产链
`source/board/bitstream.bin`（Vivado `write_bitstream -bin` 原生 .bin，首 128 字节内有 `AA 99 55 66`）
--`python tools/convert_bitstream.py <in.bin> <out.c> <out.h>`--> `board_fpga_bitstream.c/h`
（VSCode task "Convert PL bitstream"）。**字节序由脚本内部处理，不要手工交换。**
产物是 **ZBSP 打包格式**：16 字节 LE 头（`magic="ZBSP"`/`raw_len`/`packed_len`/`crc32`）+ RLE 载荷
（`0`=结束、`1..0x3F`=字面 run、`0x40..0x7F`=字节重复、`0x80..`=零 run）。运行时由首 4 字节 magic 自动选路径：
`ZBSP` → RLE 解码进 4KB `dma_stage_buf`（64B 对齐、cacheable OCM，每轮 clean L1 后发一条 DMA，先内存校验头再校验 sync/CRC，
全程不碰硬件直到 `pcap_start()` 成功）；否则 → 一条 DMA 搬整帧。
`board_fpga_bitstream.c` 缺失时 `source/board/CMakeLists.txt` 会 FATAL_ERROR。header guard 由输出文件名派生。
**当前 `fpga_img[]` 81420 字节 = SmartConnect + axi_bram_ctrl @ 0x4000_0000（8 KB）**；`bitstream.bin` 与
`C:\WorkSpace\fpga\project_1` 的 `impl_1/design_1_wrapper.bin` md5 一致（19d8569c…），`.bd` 映射只有 BRAM @0x40000000 range 8K。
（注：commit `1eaef37` 的描述写成了 "AXI GPIO @ 0x41200000"，那是更早一版设计的残留描述，**以实际 BRAM 设计为准**。
早前的 AXI GPIO 版已被替换，别再把 81420 B 认成那一版。）
换比特流：从 Vivado `impl_1/design_1_wrapper.bin` 复制到 `source/board/bitstream.bin` 再跑转换脚本。

## 板级 / 运行时现状（2026-09-30）
- LED = MIO 24（成功 500ms 慢闪 / 失败 100ms 快闪）；UART0 = MIO11 TX / MIO10 RX，115200 8N1 轮询。
- tick = TTC0 counter0 interval（SPI 42）1ms，全 IRQ 投递；FCLK0 = IO PLL/10 = 100MHz。
- `board_init()`：`fpga_load()` → `clock_fclk_config(FCLK0, FCLK_SRC_IO, 10, 1, true)` → `fpga_reset_release_all()`（先时钟后释放复位）。
- `main()`：打印 `board_pl_status()`；PL_OK 时跑 `bram_test()`（0x4000_0000 起 8KB 写读回，图案 + 其取反两遍）；之后循环打印 tick。

## 控制台 printf（2026-09-30 迁移到 newlib printf）
- 链接 `--specs=nano.specs`（gcc-arm-toolchain.cmake）；`main.c` 强定义 `__io_putchar/__io_getchar`（→ `uart_putc/uart_getc(UART0)`），
  `syscalls.c` 的弱 `_write/_read` 循环调它们；`_isatty=1` → 行缓冲，`\n` 结尾即刷出。
- **`_sbrk` 已重写**（sysmem.c）：用链接脚本的 `__heap_start__/__heap_end__` 做界（CubeMX 原版公式与本布局不符，会撞异常栈）。
  堆在 flash_xip.ld 里从 1KB 扩到 4KB。
- 用法：`uint32_t` 打印前 cast `(unsigned)` 配 `%u`/`%08X`；printf 首次调用会 malloc（失败则退化为无缓冲，仍可用）。

## 体积基线（比较必须同版比特流）
`app.bin` = 固件 + 比特流，换比特流后会变。当前基线（BRAM 比特流 + printf 版）：
**RAM 静态 6608 B、FLASH 102616 B**（printf 迁移前是 6084 / 98396）。判断"改动有没有引入开销"要跟同一版 bitstream 的基线比，
不要跨版本比绝对值。注释-only 改动的验证方式是 `app.bin` md5 不变（本轮基线 `ec082011d18ba7a06cc20079ae06c422`）。

## 待办 / 未验证
- 09-30 的改动（mio 改名、zynq_soc 改名、printf 迁移、doc/code_notes.md）**尚未提交**，工作区 30 个文件 dirty；最新提交 `e8bd440`。
- 驱动与 PL 分层的**上板验证仍未做**（只过编译）；BRAM 读写与 printf 已实测通过（09-30）。
