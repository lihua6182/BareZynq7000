# 代码长注释归档（code_notes.md）

源码注释只保留"给人看"的短注释：一行说清做什么、参数是谁，硬约束用一行 `@note`。
背景、取舍、排雷记录搬到这里（2026-09-30 起）。按模块组织；与 `.workbuddy/memory/`
的运行日志互补：这里放"为什么这么写"，那里放"当时怎么踩的"。

---

## zynq_mio（MIO pad + GPIO 控制器）

### 模块沿革与命名（四轮收敛）

1. 最初 `driver_gpio/driver_uart`；2026-09-29 改 `zynq_gpio/zynq_uart`。
2. 命名规矩定稿：公共 API 第一个词 = 模块名，模块词 = 文件名。
3. MIO pad/pinmux（属 SLCR）与 GPIO 控制器（ch.14）曾拆成 `zynq_gpio` + `zynq_mio` 两对。
4. 最终（用户拍板）：**合为一个模块 `mio`**，文件 `zynq_mio.c/h`，全部 API `mio_` 前缀；
   原 `gpio_*` 全部改名，寄存器访问器 `gpio_reg_*` → `mio_reg_*`。
   宏同步：`GPIO_PIN_COUNT`(118)→`MIO_PIN_COUNT`，原 54 计数让名为 `MIO_PAD_COUNT`，
   `GPIO_IRQ_TYPE_*`→`MIO_IRQ_TYPE_*`，`GPIO_MIO24_GPIO`→`MIO24_GPIO`，`GPIO_FN_*`→`MIO_FN_*`。
   **两套号码**：pad 0..53（mux/pull-up/slew）与 pin 0..117（GPIO 控制器，含 EMIO），
   pin n == pad n（n<54）。pad 函数形参叫 `pad`，pin 函数形参叫 `pin`。

### 命名风格对标

linux/posix 风格，小写下划线，一个功能一个函数，不用句柄+初始化结构体。
pin 半的名字对标内核 gpio consumer API（`gpiod_direction_output/gpiod_set_value`），
irq 常量对标内核 `IRQ_TYPE_*`；功能选择宏是 `MIO<pad>_<功能>` 形式（如 `MIO24_GPIO`、`MIO11_UART0`），
手写常量块维护在 `zynq_mio.h`，数据源是内核 `pinctrl-zynq.c` 的分组表（对 UG585 Table 2-4 交叉核对过）。
（早期曾由 `tools/gen_mio_macros.py` 生成，该脚本已删除。）

### MASK_DATA 写路径

输出只经 MASK_DATA_x_yL 寄存器写：高 16 位是"不要碰"掩码、低 16 位是数据，
所以单 pin 更新不会打扰同 bank 其它 pin。`DATA_x`(0x040) 是输出锁存（写了什么），
`DATA_x_RO`(0x060) 是 pad 实际电平——两者语义不同，`mio_get_output`/`mio_get_value` 分别对应。

### bank 掩码防跑飞

`MIO_BANK(pin) = (pin>>5)&3` 故意掩码：越界 pin 号只会写错到"别的 GPIO 寄存器"，
不会逃出 GPIO 地址块踩到其它外设。头文件里的 static inline 访问器因此不做参数检查
（调用者保证 `pin < MIO_PIN_COUNT`）；会失败的配置函数放 .c 里返回 0/-1。

### DIRM 与 OEN

`mio_direction_output` 设 DIRM=1 的同时必须开 OEN：只有 DIRM 的"输出"不驱动 pad
（UG585 ch.14）。另外该函数**先写电平再切方向**——反序会把输出锁存里的旧值瞬间
打到 pad 上，示波器上能看到毛刺。

---

## zynq_uart（Cadence UART）

- CR[5:2] 每个方向是一对 one-hot 位（EN/DIS）：写 1 一个会清掉另一个，所以
  enable/disable 系列都是"先掩后或"。
- 波特率搜索：`baud = ref / (BAUDGEN × (BAUDDIV+1))`，BAUDDIV 从 4 起扫（<4 非法），
  BAUDGEN 四舍五入，保留绝对误差最小的一对。改波特率只动两个除数寄存器，运行中调用安全。

---

## zynq_clock（时钟树）

全部频率运行时从 SLCR 读回重算（`clock_freqs_get`），全工程唯一硬编码频率是晶振
`PS_CLK_HZ`（zynq_common.h）——好处是 `sys_clock_init()` 重配 PLL 之后读数依然正确。
未解码项：CAN MIO 时钟、debug trace EMIO 时钟（DBG_CLK_CTRL[6]）、看门狗源（SWDT_CLK_SEL），
外部来源或不可判定。FCLK 的源 mux 不是无毛刺切换，`clock_fclk_config` 在解锁窗口内
先关时钟再改；分频器可以在线改。

---

## zynq_irq（GIC）

- 投递路线：所有源 Group 0 + `CTLR.FIQEn=0`（与 Xilinx XScuGic 的组合一致）→ 全走 IRQ，
  FIQ 输入保持复位屏蔽。CPSR.I 解除、CPSR.F 不动。
- `irq_init()` 先脉冲 `A9_CPU_RST_CTRL.PERI_RST`：JTAG 重载只复位 CPU，不复位 GIC，
  不清会把上一次运行的使能状态带进来。
- `irq_enable()` 对 SPI 必须写 target：ITARGETSR 复位为 0（无 CPU），不 targeted 的 SPI
  会 pend 但永不转发（对照 XScuGic_DistInit 给每个 SPI 盖 CPU0）。PPI/SGI 寄存器按 CPU
  banked，无需 target。
- 入口 `irq_handler`：IAR/EOIR 成对 mandatory，否则 SPSR 恢复后异常立刻重入；
  spurious ID 1023 越界检查失败、不 EOIR（IHI0048）。
- XIP 相关：分发代码在 flash 里，启动期 QSPI 重配完成前 IRQ 保持屏蔽；运行期要退出
  QSPI linear 模式的代码必须先 `__disable_irq()`，否则取指停摆。
- `target("general-regs-only")` + pragma 压 `-Wattributes`：GCC 的 `interrupt` 属性不保存
  VFP/NEON 寄存器，该属性只挡编译期警告不改变代码生成（详见 memory 的长考据）。

---

## zynq_tick（TTC0 毫秒 tick）

- TTC 中断寄存器 **clear-on-read**（不是 W1C）：handler 里读一次即清电平源，
  随后分发器 EOI 才能真正结束。
- **handler 不能放 `.fast` 段**：该段 VMA 从 0 起址，段内第一个函数地址就是 0，
  `irq_attach()` 的 NULL 检查会静默拒绑——tick 就这么死过一次（2026-09-29）。

---

## zynq_soc（启动：MMU/L2/时钟/QSPI）

### 启动顺序为什么是死的

`mmu_init → mmu_and_caches_enable(内含 l2_cache_enable) → xip_warm_cache →
sys_clock_init → qspi_reconfig`。CPU 全程从 flash 取指：必须先让缓存盖住 flash 窗口、
把镜像预热进 L2，然后**从 OCM (.fast)** 重配 PLL 与 QSPI。MMU/缓存关着做时钟、
或直接从 flash 做时钟，都会挂总线。页表放高 OCM（0xFFFF0000）非缓存段：CA9 页表
walker 与 D-cache 不一致。

### xip_warm_cache

重配后的 QSPI 线性口在这块板上读不可靠（实测），而重配后的第一次取指必然 miss
（启动代码从未进过缓存）——所以趁 QSPI 还是 BootROM 配置时把整个镜像当数据读一遍，
灌进 L2，重配窗口内的取指全部由 L2 供给。跨度 = `_sidata + .data 大小`
（QSPI 里可取指内容的末尾）；若以后再往 flash 加 `AT>FLASH` 段要跟着扩。

### L2 投机预取必须关

PL310 AUX_CNT bit28/29（IPFE/DPFE）与 ACTLR bit1 开着时，预取器会在 QSPI 重配的
几微秒里往已死的 flash 发读 → Prefetch Abort 风暴（上板验证过）。只保留需求式行填充，
100 MHz XIP 才稳。TAG/DATA RAM 延迟寄存器（0x108/0x10C）不在 CMSIS 结构体里，按偏移写。

### qspi_reconfig 目标状态

QSPI ref = IO PLL/5 = 200 MHz、SCLK = 100 MHz、LPBK_DLY_ADJ[USE_LPBK]=1 内部回环采样
（DLY 保持复位值；MIO8/9 不复用、板上未接线，SCLK_FB 焊盘环回路径关闭）；退出 linear
模式前必须排空 FIFO（ch.12），`LQSPI_RST_CTRL` 脉冲会清两个复位域。

---

## board_fpga（PL 配置/PCAP）

### 4KB 规则的正确理解

DevC DMA 的"不跨 4KB 页"由**源地址 64 字节对齐**保证（DMA 发 64B burst，4KB 是 64
的倍数）——所以一条覆盖整个镜像的 DMA 命令是合法的，只要镜像 64B 对齐。
ZBSP 路径的 staging buffer 固定 64B 对齐、每块不跨页。

### ZBSP 打包与字节序

- 打包：16 字节 LE 头（magic "ZBSP"/raw_len/packed_len/crc32）+ SEGGER `BSP_FPGA.c`
  同款 RLE（`0`=结束、`1..0x3F` 字面、`0x40..0x7F` 字节重复、`0x80..` 零 run），
  7-series 镜像大多是空帧，~25:1。magic 兼任路径选择：bitgen 镜像以 0xFFFFFFFF 填充开头。
- 字节序：PCAP 消费**小端字序**，sync word `0xAA995566` 在字节流里必须是 `66 55 99 AA`
  （内核 zynq-fpga 的 "byte swapped .bin"）。喂进 bitgen 原生大端 .bin 的症状很阴险：
  DMA 照推 2MB、PL 全忽略，几秒后只报一个泛泛的 NO_DONE。
  `convert_bitstream.py` 已自动换序，不要手工交换。

### FPGA_RST_CTRL 复位值事故

`SLCR.FPGA_RST_CTRL`(0xF8000240) 复位值 `0x01F33F0F`：四根 FCLKRESETn 全在复位里，
且寄存器只有 bit0-3 可用。不清零的表象是"配置成功"（PCAP DONE 照样断言），但 GP0
上任何访问无响应——2026-09-29 上板排雷的主坑。FSBL 在配完 PL 后清整个寄存器，
本工程同序：`fpga_load() → clock_fclk_config() → fpga_reset_release_all()`。

### 轮询上限的量纲

`PL_INIT_POLL_LIMIT`/`DMA_DONE_POLL_LIMIT` 数的是强序总线周期（一次 devcfg 读 ~100 ns），
不是墙钟——tick 和 GIC 都还不存在。~1s 给 PL 上电/PROG_B 握手，~10s 是每条 DMA 的保险。

### DMA 命令寄存器顺序

`DMA_SRC_ADDR → DMA_DEST_ADDR(=0xFFFFFFFF 路由到 PCAP) → DMA_SRC_LEN → DSB →
DMA_DEST_LEN`：写 DEST_LEN 才入队并启动。完成 ack 要同时写 `INT_STS[13]` 与
`STATUS[DMA_DONE_CNT]`（均 W1C）。传输级错误位集合见 `DMA_ERROR_MASK`（UG585 步骤 10）。

---

## 其它

- `syscalls.c`：CubeMX 模板；`_write/_read` 循环弱符号 `__io_putchar/__io_getchar`，
  main.c 里强定义绑到 UART0。`_isatty=1` → newlib 行缓冲，`\n` 即刷出。
  `_sbrk` 用链接脚本的 `__heap_start__/__heap_end__`，不按 CubeMX 的
  `_end/_estack/_Min_Stack_Size` 公式（与本布局不符，会撞异常栈）。
- `zynq_boot_header.c`：结构体字段注释是 UG585 ch.6 原文对照，别精简；校验和由
  `scripts/cmake/function_validate_boot_header.cmake` 在构建末尾核对。
- `sysmem.c` 的堆从 1 KB 扩到 4 KB（flash_xip.ld）：newlib stdio 首次 `printf` 会 malloc 缓冲区，
  1 KB 时分配失败会退化成无缓冲（功能仍在，但每次写都直接走 UART 轮询）。
- `main.c` 的 `bram_test()` 只在 `board_pl_status() == PL_OK` 后跑：GP0 上没有存活从设备时，
  访问**不是返回垃圾而是挂死总线**（无响应不应答），所以必须先确认 PL 配好再碰它。
- `zynq_soc_ip.h` / `source/soc/cmsis/`：寄存器名与 vendored CMSIS 保持上游原样，
  改名会丢"查手册即所得"。
