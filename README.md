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
5. `zynq_system_init()`按UG585第25章的PLL更新序列把时钟配置到UG585表25-3（6:2:1模式）示例1的状态：CPU 667MHz、外设总线CPU_1x 111MHz、QSPI参考200MHz，Flash实际SCLK=100MHz（W25Q128JV四线0x6B读，Config_reg波特率÷2 + LPBK_DLY_ADJ内部回环采样DLY=00 + MIO8/9反馈时钟环回路由）。MMU开启后、QSPI重配前，`xip_warm_cache()`把整个XIP镜像读入统一的L2缓存——实测这块板经刚重配的QSPI线性口的读不可靠，预热保证重配窗口内零flash取指。QSPI参数切换在OCM的`.fast`段中按UG585第12章流程执行：退出linear模式→排空FIFO→改时钟并脉冲复位两个控制器复位域→重编程控制器寄存器→重新进入linear模式。时钟参数：晶振频率（`ZYNQ_PS_CLK_VALUE`）在`source/soc/zynq_common.h`中配置，是全工程唯一硬编码的频率；其余所有时钟频率（PLL/CPU/外设参考时钟等）在运行时由`zynq_clock`驱动（`zynq_clock_freqs_get()`）从SLCR寄存器读出。
