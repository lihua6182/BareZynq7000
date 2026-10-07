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

`mmu_init → mmu_and_caches_enable(内含 l2_cache_enable) → sys_clock_init →
qspi_reconfig`。CPU 全程从 flash 取指：重配 PLL 与 QSPI 的窗口**从 OCM (.fast)**
执行（窗口内端口是死的）；窗口外端口始终是好的（重配前 BootROM 配置、重配后
25MHz 直接采样，均经实测）。页表放高 OCM（0xFFFF0000）非缓存段：CA9 页表
walker 与 D-cache 不一致。

### xip_warm_cache —— 已删除（2026-10-07 收官）

预热曾为"重配后端口不可靠"的 workaround，经 qspi_probe 五轮上板测量后**证伪并
删除**。最终结论：**端口在任何合法配置下都是好的**，"不可靠"是三重假象叠加：

1. **回环档全零**：SCLK > FQSPICLK2（~40MHz）必须用反馈时钟，而内部回环的反馈
   信号也经由 MIO8 焊盘（UG585 ch.12）——本板 MIO8 未接线、`MIO_PIN_08=0x601`
   为输入态 → 回环采样钟死 → 全零。100MHz golden（USE_LPBK）即死于此。
2. **违规配置不可信**：回环仅 BAUD_RATE_DIV=000 有效（UG585 16591）；'q' 扫描
   div>0 的 96 点全违规。Clock Ratio Restriction 要求 QSPI_REF_CLK > CPU_1x
   （16817）：BootROM 复刻 REF=33.33MHz < CPU_1x=111MHz，运行期必败
   （BootROM 时代 CPU 未提速，天然满足）。
3. **真值用错文件**：烧录用 **app.hex**（ihex 段间间隙无记录，保持擦除态 0xFF），
   而地面真值起初取 `crc32(app.bin)`（binary 间隙填 0x00）——两者差 2,148 字节
   （间隙 0x8FC/4B + 0xB7A0/2144B），造成"所有合法配置读数全错"的假象。
   以 0xFF 渲染 app.hex 后与板载读数**全幅 + 逐块完全一致**。

实测矩阵：LPBK=0 直接采样在 781kHz~62.5MHz（REF 200/250 × BAUD /4~/256，含
REF=33MHz 比值违规对照）全部读数正确；100MHz 直接采样也正确（但超 FQSPICLK2，
不在规格内）；125MHz 真时序崩溃。**go-live 终审通过**：全缓存逐出 + ICIALLU 后
系统纯靠 25MHz 直接采样端口运行，cdc 续行——无预热 XIP 上板实证。

收编（已落地）：`qspi_reconfig` = REF IO PLL/5=200MHz（>CPU_1x）+ BAUD /8 =
SCLK 25MHz + LPBK=0；`xip_warm_cache` 删除；启动顺序收敛为上述四步。
**教训**：①负面"实测"也要审证据链——"预热存在 ⇒ 运行期读走 L2 ⇒ 端口无流量"
是循环论证；②地面真值本身要先证真（烧录物是 hex 不是 bin）；③高速档（>40MHz）
要提速必须改板（MIO8 反馈），软件无解。

### qspi_probe（端口实测扫描，定案后已删除）

诊断模块完成使命后于 2026-10-07 拆除（含非缓存别名 `0xFB200000`、
`soc_qspi_boot_cfg` 快照、tools/crc32.py），完整实现见 git 历史
（`a7ab2e1`）。本节保留其安全设计，备将来类似测量参考。

扫描时主循环 UART 按 `q` 触发 `probe_sweep()`。每个配置一个窗口：
**关 IRQ → (.fast) 重配 → 非缓存全镜像 CRC → (.fast) 恢复 golden →
开 IRQ → 打印**。在运行期戳一个可能不可靠的端口之所以安全，靠四条：
IRQ 屏蔽使 ISR 不会在窗口内取指 flash；重配序列在 OCM（.fast）执行；
CRC 循环代码在首次（golden 配置下）全量预跑时已进 L1I/L2，而**非缓存
数据读不产生缓存分配**，窗口内代码行不可能被逐出；所以坏配置的结果是
CRC 不符，而不是跑飞——恢复 golden 也从 OCM 做。

扫描矩阵：div 0..3（SCLK = 200MHz ref 的 /2 /4 /8 /16）× DLY 0..31，
USE_LPBK 固定 1（MIO8 焊盘环回板上未接线，无可选项）。div>0 时内部回环
是否生效 UG585 App.A 语焉不详，靠扫出来的结果说话。判定用 zlib 多项式
0xEDB88320，与主机 `zlib.crc32` 逐字节一致（字读小端拆 4 字节）。

注意 `probe_apply_config` 是 `qspi_reconfig` 的**有意重复**：启动路径在
测量结论出来前一行不动，重复比参数化重构安全。结论落地后两份合一。

### 首轮扫描结果（2026-10-07 上板）——颠覆性

主机真值 `zlib.crc32(app.bin) = A1AC2F94`（span 156,372 B）。板上结果：

- **div=0（100MHz，现行配置）全部 32 个 DLY：`48773398` = crc32(全零填充)**
  ——单拍读每个字都返回 0x00000000。"==" 全是自相一致的垃圾，不是"全通过"。
- div=1/2/3（50/25/12.5MHz）：`C6D954A6`，96 个配置完全一致的**确定性错误**
  （非随机噪声）；与全零/全 FF/位反/字节反/半字交换/整流 ±1 位移/nibble 交换
  六种简单变换均不匹配——疑似 quad 通道级错乱（内部回环在 div>0 失效，
  MIO8 焊盘环回板上未接线）。
- **扫描空间内无任何可用配置**。单拍层面 100MHz 端口是全灭，不是"边际不可靠"。

推论链重写：①"重配后读不可靠"的旧结论方向正确但程度被低估——是**全灭**；
②运行期"稀疏读无碍"实为"几乎零流量"：L2 512KB 对 156KB 镜像 + OCM 数据
压力极小，镜像行实际从不逐出，100MHz 端口自建板以来可能从未成功服务过
任何访问；③删预热后 FPGA 配置失败 ≈ **突发读同样坏**的直接证据（'b' 命令
验证）；④**唯一已知良配置是 BootROM 配置**（预热内容正确性由 LZMA/ZBLZ
CRC 间接证明），已加 soc 层快照（`soc_qspi_boot_cfg`，soc_init 入口捕获）
+ 'c' 命令读取，下一步用 probe 复刻该配置测 CRC。

命令：'q' 单拍扫描、'b' 突发扫描（缓存窗+L1D/L2 逐出，逐行打印；若 golden
突发路径坏，恢复后的 printf 代码本身要从死端口重新取指——**LED 停闪即判决**，
断电重启）、'c' BootROM 配置快照、'r' BootROM 复刻+BAUD 爬升。

### 突发判决与 BootROM 配置（2026-10-07 第二轮上板）

'b' 在 div=0 行之前挂死 → **100MHz 突发读同样全灭**（恢复路径 printf 需从
死端口取指）。上午 FPGA 失败机理闭环：单拍、突发双灭，预热掩盖的是全端口失效。

'c' 捕获的 BootROM 配置：LQSPI_CLK_CTRL=`0x2821`（SRCSEL=ARM PLL、DIV=40）、
CR=`0x800238C9`（BAUD /4）、LQSPI_CR=`0x8000016B`（与现行同）、LPBK=`0x20`
（回环开、**DLY=0**）。关键巧合：ARM PLL 旁路（BootROM 时）与 1333MHz
（sys_clock_init 后）经 /40 都得 ref=33.33MHz → SCLK≈8.33MHz——该寄存器值
在 PLL 初始化前后语义一致。突发+单拍在该配置下均由启动过程间接证明
（预热内容过 ZBLZ CRC）。'r' 原样复刻并做 BAUD 爬升（16.7/8.3/4.2/2.1MHz），
最快匹配构建期 crc32（tools/crc32.py，POST_BUILD 自动打印）的档即收编候选。

### 'r' 复刻失败与三条违规归因（2026-10-07 第三轮上板）

'r' 结果：BootROM 原样复刻在运行期**也读错**（`CC7FB459`，且在 8.33/4.17/2.08MHz
三频点逐位一致；经全部 256 种字节填充验证**非常量填充**）；div=0（16.7MHz）=全零。
即：**端口从未在合法配置下被测试过**，既往失败各有独立的违规原因（UG585 ch.12）：

1. golden 全零：SCLK > FQSPICLK2（~40MHz）必须用反馈时钟，且内部回环同样要求
   **MIO8 pinmux 为反馈输出并能自由翻转**（"feedback signal is received from the
   internal input from the I/O so MIO pin 8 needs to be programmed"）；本板 MIO8
   未接线、软件未编程 → 回环采样时钟死 → 全零。
2. 'q' div>0 错乱：回环仅在 BAUD_RATE_DIV=000 时有效（ch.12 Configure Controller
   "ensure qspi.Config_reg[BAUD_RATE_DIV] is set to 0b00"），div>0 带 USE_LPBK=1
   全是违规配置，读数不可信。
3. 'r' 复刻错乱：**Clock Ratio Restriction——QSPI_REF_CLK 必须大于 CPU_1x**。
   运行时 CPU_1x = CPU/(4+2·CLK_621_TRUE) ≈ 111MHz（667/6）；BootROM REF=33.33MHz
   严重违反；BootROM 时代 CPU 尚未提速，比值天然满足。这解释了"同一寄存器值
   上电时正确、运行期错乱"。

纠错：SLCR LQSPI_CLK_CTRL 的 SRCSEL 编码为 0x=IO / 10=ARM / 11=DDR（zynq_clock.h），
`0x2821` = ARM PLL/40 = 33.33MHz，旁路与否同值（早前分析正确）。

'n' 合法配置扫描（LPBK=0 直接采样，<40MHz 无需反馈时钟）：REF∈{200,250}MHz
（均 >CPU_1x）× BAUD /2~/256 全覆盖 + BootROM 时钟树 LPBK=0 对照点（区分比值
违规与 USE_LPBK 之罪）。任一行匹配构建期真值即为可收编配置——届时 `qspi_reconfig`
收编该配置、删 `xip_warm_cache`、`soc_init` 移入 `.fast`，全量复测后根治完成。

### 'n' 收敛：损坏与端口配置无关（2026-10-07 第四轮上板）

'n' 全部合法配置 + 比值违规对照点返回**同一个** CRC：781kHz~62.5MHz（80 倍频率
范围）、REF 200/250MHz、比值满足与违反——逐位一致。频率无关性证明**损坏不在
QSPI 端口**，而是配置无关的系统性变换（读路径或 flash 内容本身）。边界行为全部
符合手册：125MHz 为真时序崩溃（另一 CRC 值），回环档全零（反馈钟死；
`mio_pin_08=00000601`：TRI_ENABLE=1、复选全 0，实证 MIO8 非反馈功能）。
`cpu_1x=111110000Hz`（667/6，CLK_621_TRUE=1）实证比值约束：合法下限 REF>111MHz。

主机侧 40+ 种变换鉴定（字节/半字/nibble/位反转、词/块置换、块内旋转、流移位、
地址位 stuck-0/1）对实测 CRC 无一命中——不是规整变换，须直接看字节：

- **'m' 逐词失配扫描**：在合法配置（REF200/BAUD /8/LPBK=0，25MHz）下，同一地址
  分别经别名（过端口）与缓存窗（warm-L2 命中 = 启动时读出的正确数据，ZBLZ CRC
  背书——板上自含真值，无需主机抓数）读全镜像，先跑 cached 参考 CRC（纯 warm
  命中）再扫描，只报前 24 处 `off/alias/cached` + 总失配数。
- **'e' go-live 终审**：应用合法配置 → 逐出 L1D+L2（全 XIP 窗）+ ICIALLU → 缓存
  窗全镜像 CRC → **不恢复 golden**：cdc 报告续行即"XIP 无预热"实证（根治完成态）；
  LED 停闪即判决。go-live 后 `probe_frozen` 锁死其余 probe 按键（恢复 golden
  必挂）。

### L2 投机预取必须关

PL310 AUX_CNT bit28/29（IPFE/DPFE）与 ACTLR bit1 开着时，预取器会在 QSPI 重配的
几微秒里往已死的 flash 发读 → Prefetch Abort 风暴（上板验证过）。只保留需求式行填充，
100 MHz XIP 才稳。TAG/DATA RAM 延迟寄存器（0x108/0x10C）不在 CMSIS 结构体里，按偏移写。

### qspi_reconfig 目标状态（2026-10-07 收编后）

QSPI ref = IO PLL/5 = 200 MHz（> CPU_1x=111MHz，ch.12 比值约束）、SCLK = ref/8 =
25 MHz（CR=`0x800238D1`，BAUD_RATE_DIV=010）、LPBK_DLY_ADJ=0 **直接焊盘采样**。
>40MHz 需 MIO8 反馈时钟（板上未接线、`MIO_PIN_08=0x601` 输入态），内部回环路径
无反馈钟即死（全零），故回环一律禁用；25MHz 是无需改板的规格内最快档（100MHz
直接采样实测可读但超 FQSPICLK2 规格，不采用）。退出 linear 模式前必须排空 FIFO
（ch.12），`LQSPI_RST_CTRL` 脉冲会清两个复位域。

---

## zynq_cache（L1/PL310 缓存维护）

### 两个 DMA 域，维护深度不同

- **中央互连主设备**（USB/ENET/SD/DMA 控制器）：不嗅探 PL310，CPU 写的数据必须
  用 `cache_clean/flush` 双层清写真正落到 OCM/DDR；读回方向 `cache_invalidate`
  双层丢弃（丢弃语义，只给 DMA 专用缓冲）。
- **ACP 主设备**（DevC/PCAP）：硬件在 SCU/ACP 处与 L2 一致，`cache_clean_acp`
  只清 L1 即可——board_fpga 的 staging buffer 一直就是这个语义（原
  clean_l1_dcache）。

### L1→L2 的顺序坑（GLM5.3-flash 踩过）

ARMv7-A 上 L1 clean（by MVA）指令返回 ≠ 数据已落到 L2。逐行交错执行
"L1 clean → L2 clean PA" 没有屏障，L2 的维护寄存器写（device 写）可以先于
L1 推来的数据执行，L2 清到旧行 → DMA 读到旧数据。正确序列 =
**L1 循环 → DSB → L2 循环 → CACHE_SYNC → DSB**（Linux `__dma_map_area` →
`outer_*_range` 同款）。这 bug 用常数 'U' 测速图案是测不出来的（旧=新），
变化数据（描述符/真实载荷）才会偶发出错。

行宽 L1 = PL310 = 32B（与 `CONFIG_USB_ALIGN_SIZE=32` 同源）。CherryUSB 的
`usb_dcache_*` 钩子保留在 zynq_usb.c——那是 middleware 的名字契约，
只做到 `cache_*` 的适配。

---

## board_fpga（PL 配置/PCAP）

### 4KB 规则的正确理解

DevC DMA 的"不跨 4KB 页"由**源地址 64 字节对齐**保证（DMA 发 64B burst，4KB 是 64
的倍数）——所以一条覆盖整个镜像的 DMA 命令是合法的，只要镜像 64B 对齐。
ZBSP 路径的 staging buffer 固定 64B 对齐、每块不跨页。

### ZBLZ/ZBSP 打包与字节序

- 打包：16 字节 LE 头（magic/raw_len/packed_len/crc32）+ 压缩载荷。默认 **ZBLZ**（raw
  LZMA1，lc=0 lp=0 pb=0、8KB 字典、带结束标记，本设计实测 19.6:1）；`--codec rle` 退回
  **ZBSP**（SEGGER `BSP_FPGA.c` 同款 RLE，9.9:1，兜底/调试用）。magic 兼任路径选择：
  bitgen 镜像以 0xFFFFFFFF 填充开头。
- **为什么是 LZMA 不是哈夫曼/RLE 调优**：对当前镜像，RLE 载荷的 69% 是字面数据字节
  （非零帧内容），RLE 类完全无能为力；纯字节哈夫曼理论下限只有 173KB（1.21x）。剩余
  冗余是"相同帧模式跨镜像重复"，只有 LZ 匹配+熵编码能吃掉：deflate 126KB（1.66x）、
  LZMA 106KB（1.97x）。实测数据见 build/compress_study*.py（研究脚本，gitignore 内）。
- **编码器↔解码器契约**：`convert_bitstream.py` 的 `LZMA_FILTERS` 与
  `lzma_stream.c` 的 `lzma_props`（`00 00 20 00 00`）+ `LZMA_STREAM_DICT_BYTES` 必须
  同步改。字典故意只用 8KB：7-series 帧重复是局部的，实测 8KB 与 8MB 字典比率只差
  0.2%，而固件侧静态内存（32KB 窗 + 5.5KB probs + 4KB stage）就定了。
- **滑动窗口为什么合法**：SDK 的 Dictionary Interface 官方支持"用掉 dic 数据后手动回退
  dicPos"；distance 校验由 `checkDicSize = prop.dicSize`（8KB）封顶（LzmaDec.c:519），
  所以窗口滑走时保留最后 8KB（= 最长匹配距离）即永远够。窗口 32KB > 字典 8KB，每轮
  滑出 24KB。probs 数量 1984+0x300<<lc=2752 个，经一次性静态 arena 分配器交给
  `LzmaDec_AllocateProbs`，尺寸错误会在运行时变 `LZMA_STREAM_MEM`（响亮失败）。
- **验证方式**：无主机 C 编译器，装了 ziglang（`python -m pip install ziglang`）后用
  `zig cc` 把**真实中间件源码**（lzma_stream.c + vendored LzmaDec.c）编成主机程序，
  跑**真实发布载荷**（从 board_fpga_bitstream.c 提取）：字节级等于参考 PCAP 镜像、
  CRC 一致、截断/损坏载荷正确拒绝、同一静态区二次解码正确（对应多次 fpga_load）。
  测试脚手架在 build/lzma_host_test/（gitignore 内）。板侧 zblz_sink/zblz_flush 是
  RLE 路径的同构搬运（整字 chunk、满 4KB 一条 DMA），未被主机测试覆盖，首次上板
  重点看这条。
- 字节序：PCAP 消费**小端字序**，sync word `0xAA995566` 在字节流里必须是 `66 55 99 AA`
  （内核 zynq-fpga 的 "byte swapped .bin"）。喂进 bitgen 原生大端 .bin 的症状很阴险：
  DMA 照推 2MB、PL 全忽略，几秒后只报一个泛泛的 NO_DONE。
  `convert_bitstream.py` 已自动换序，不要手工交换。

### FPGA_RST_CTRL 复位值事故

`SLCR.FPGA_RST_CTRL`(0xF8000240) 复位值 `0x01F33F0F`：四根 FCLKRESETn 全在复位里，
且寄存器只有 bit0-3 可用。不清零的表象是"配置成功"（PCAP DONE 照样断言），但 GP0
上任何访问无响应——2026-09-29 上板排雷的主坑。FSBL 在配完 PL 后清整个寄存器，
本工程同序：`fpga_load() → clock_fclk_config() → fpga_reset_release_all()`。

### main 的调用顺序（2026-10-02 起：console 先于 PL）

`board_init()` 原来排在 main 最前（PL 加载在静默期完成，失败要等控制台起来
才看到 pl_status）。现挪到 `uart_init` 之后、前加一行 printf——加载/失败
全程可见。依据：board 路径无 console 依赖，console 也不依赖 PL，双向独立；
tick 届时已在跑，1ms 中断只给 PCAP 轮询限值（强序总线周期数）添可忽略抖动。
soc_init 内部五步死顺序与此无关，未动。

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
- `zynq_soc.h` / `source/soc/cmsis/`：寄存器名与 vendored CMSIS 保持上游原样，
  改名会丢"查手册即所得"。
