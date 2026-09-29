#ifndef ZYNQ_CMSIS_H
#define ZYNQ_CMSIS_H

/*
 * Device adaptation header for the vendored CMSIS-Core(A) headers in
 * source/cmsis (ARM.CMSIS 6.3.0, tag). It provides everything core_ca.h
 * expects from the device: the core/CPU configuration defines, the GIC and
 * L2C-310 base addresses and the interrupt ID enum (UG585 Table 7-3,
 * 16 SGI + 16 PPI + 64 SPI). Including it brings in the GIC_xxx, L2C_xxx
 * and L1C_xxx function families plus the GICDistributor, GICInterface and
 * L2C_310 register sets, replacing the hand-written GICD/GICC/L2C
 * definitions that used to live in zynq_soc_ip.h (single definition of
 * each peripheral).
 */

#include <stdint.h>

#define __CORTEX_A    (9U)
#define __CA_REV      (0x0000U)
#define __FPU_PRESENT (1U) /* VFPv3-D16; enabled by zynq_startup.s */
#define __GIC_PRESENT (1U)
#define __TIM_PRESENT (0U) /* Zynq-7000 has no architected generic timer */

/* PL310 present: zynq_system.c l2_enable() configures it via the CMSIS
   register set; the tag/DATA RAM latency registers (0x108/0x10C) are not
   members of the CMSIS L2C_310_TypeDef and are written by offset. */
#define __L2C_PRESENT (1U)

/* Private peripheral block (UG585 Table 4-2, PS address view). */
#define GIC_DISTRIBUTOR_BASE (0xF8F01000UL)
#define GIC_INTERFACE_BASE   (0xF8F00100UL)
#define L2C_310_BASE         (0xF8F02000UL)

/* Interrupt IDs 0..95 (UG585 Table 7-3). The zynq_irq driver keeps taking
   plain integer IDs; cast to IRQn_Type when calling GIC_* functions. */
typedef enum IRQn {
    /* Software generated interrupts */
    SGI0_IRQn = 0,
    SGI1_IRQn = 1,
    SGI2_IRQn = 2,
    SGI3_IRQn = 3,
    SGI4_IRQn = 4,
    SGI5_IRQn = 5,
    SGI6_IRQn = 6,
    SGI7_IRQn = 7,
    SGI8_IRQn = 8,
    SGI9_IRQn = 9,
    SGI10_IRQn = 10,
    SGI11_IRQn = 11,
    SGI12_IRQn = 12,
    SGI13_IRQn = 13,
    SGI14_IRQn = 14,
    SGI15_IRQn = 15,

    /* Private peripheral interrupts */
    GlobalTimer_IRQn = 27, /* SCU global timer        */
    FIQ_Fabric_IRQn = 28,  /* FIQ from the PL fabric  */
    PrivTimer_IRQn = 29,   /* CPU private timer       */
    PrivWdt_IRQn = 30,     /* CPU private watchdog    */

    /* Shared peripheral interrupts */
    CoreParity0_IRQn = 32,
    CoreParity1_IRQn = 33,
    L2Cache_IRQn = 34,
    OCM_IRQn = 35,
    ECC_IRQn = 36,
    Pmu0_IRQn = 37,
    Pmu1_IRQn = 38,
    SysMon_IRQn = 39,
    DVC_IRQn = 40,
    SysWdt_IRQn = 41,
    Ttc0Timer0_IRQn = 42,
    Ttc0Timer1_IRQn = 43,
    Ttc0Timer2_IRQn = 44,
    Dma0Abort_IRQn = 45,
    Dma0Ch0_IRQn = 46,
    Dma0Ch1_IRQn = 47,
    Dma0Ch2_IRQn = 48,
    Dma0Ch3_IRQn = 49,
    Smc_IRQn = 50,
    Qspi_IRQn = 51,
    Gpio_IRQn = 52,
    Usb0_IRQn = 53,
    Gem0_IRQn = 54,
    Gem0Wake_IRQn = 55,
    Sdio0_IRQn = 56,
    I2c0_IRQn = 57,
    Spi0_IRQn = 58,
    Uart0_IRQn = 59,
    Can0_IRQn = 60,
    PlIrq0_IRQn = 61,
    PlIrq1_IRQn = 62,
    PlIrq2_IRQn = 63,
    PlIrq3_IRQn = 64,
    PlIrq4_IRQn = 65,
    PlIrq5_IRQn = 66,
    PlIrq6_IRQn = 67,
    PlIrq7_IRQn = 68,
    Ttc1Timer0_IRQn = 69,
    Ttc1Timer1_IRQn = 70,
    Ttc1Timer2_IRQn = 71,
    Dma0Ch4_IRQn = 72,
    Dma0Ch5_IRQn = 73,
    Dma0Ch6_IRQn = 74,
    Dma0Ch7_IRQn = 75,
    Usb1_IRQn = 76,
    Gem1_IRQn = 77,
    Gem1Wake_IRQn = 78,
    Sdio1_IRQn = 79,
    I2c1_IRQn = 80,
    Spi1_IRQn = 81,
    Uart1_IRQn = 82,
    Can1_IRQn = 83,
    PlIrq8_IRQn = 84,
    PlIrq9_IRQn = 85,
    PlIrq10_IRQn = 86,
    PlIrq11_IRQn = 87,
    PlIrq12_IRQn = 88,
    PlIrq13_IRQn = 89,
    PlIrq14_IRQn = 90,
    PlIrq15_IRQn = 91,
    ScuParity_IRQn = 92 /* 93..95 reserved (UG585 Table 7-3) */
} IRQn_Type;

#include "core_ca.h"

#endif /* ZYNQ_CMSIS_H */
