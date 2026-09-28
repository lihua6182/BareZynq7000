#ifndef ZYNQ_SOC_IP_H
#define ZYNQ_SOC_IP_H

#include "zynq_common.h"

#define DDR_BASE        (0x00000000UL) /* DDR (size per device, up to 1 GB) */
#define OCM_LOW_BASE    (0x00000000UL) /* OCM low (when mapped to 0x0)      */
#define PL_BASE         (0x40000000UL) /* PL slave registers (GP AXI)       */
#define IOU_PERIPH_BASE (0xE0000000UL) /* IOU peripherals (UART/SPI/...)    */
#define LQSPI_BASE      (0xFC000000UL) /* Linear QSPI (XIP) memory          */
#define OCM_HIGH_BASE   (0xFFFC0000UL) /* OCM high (when mapped to top)     */

#define SLCR_BASE    (0XF8000000UL)
#define DDRC_BASE    (0XF8006000UL)
#define DEVCFG_BASE  (0XF8007000UL)
#define DMAC_BASE    (0XF8003000UL)
#define DMAC_NS_BASE (0XF8004000UL)
#define GEM0_BASE    (0XE000B000UL)
#define GEM1_BASE    (0XE000C000UL)
#define GPIO_BASE    (0XE000A000UL)
#define I2C0_BASE    (0XE0004000UL)
#define I2C1_BASE    (0XE0005000UL)
#define QSPI_BASE    (0XE000D000UL)
#define SDIO0_BASE   (0XE0100000UL)
#define SDIO1_BASE   (0XE0101000UL)
#define SPI0_BASE    (0XE0006000UL)
#define SPI1_BASE    (0XE0007000UL)
#define SWDT_BASE    (0XF8005000UL)
#define TTC0_BASE    (0XF8001000UL)
#define TTC1_BASE    (0XF8002000UL)
#define UART0_BASE   (0XE0000000UL)
#define UART1_BASE   (0XE0001000UL)
#define CAN0_BASE    (0XE0008000UL)
#define CAN1_BASE    (0XE0009000UL)
#define USB0_BASE    (0XE0002000UL)
#define USB1_BASE    (0XE0003000UL)
#define OCM_BASE     (0XF800C000UL)
#define SMC_BASE     (0XE1000000UL)
#define AFI0_BASE    (0XF8008000UL)
#define AFI1_BASE    (0XF8009000UL)
#define AFI2_BASE    (0XF800A000UL)
#define AFI3_BASE    (0XF800B000UL)
#define MPCORE_BASE  (0xF8F00000UL)
#define XADC_BASE    (0XF8007100UL)

typedef struct {                           /* System Level Control Registers (SLCR) */
    __IO uint32_t SCL;                     /* 0x000  */
    __O uint32_t LOCK;                     /* 0x004 SLCR lock register */
    __O uint32_t UNLOCK;                   /* 0x008 SLCR unlock register */
    __I uint32_t LOCKSTA;                  /* 0x00C  */
    uint32_t RESERVED0[60];                /* Reserved, 0x010-0x0FF */
    __IO uint32_t ARM_PLL_CTRL;            /* 0x100 ARM PLL Control */
    __IO uint32_t DDR_PLL_CTRL;            /* 0x104 DDR PLL Control */
    __IO uint32_t IO_PLL_CTRL;             /* 0x108 IO PLL Control */
    __I uint32_t PLL_STATUS;               /* 0x10C PLL Status */
    __IO uint32_t ARM_PLL_CFG;             /* 0x110 ARM PLL Configuration */
    __IO uint32_t DDR_PLL_CFG;             /* 0x114 DDR PLL Configuration */
    __IO uint32_t IO_PLL_CFG;              /* 0x118 IO PLL Configuration */
    uint32_t RESERVED1;                    /* Reserved, 0x11C */
    __IO uint32_t ARM_CLK_CTRL;            /* 0x120 CPU Clock Control */
    __IO uint32_t DDR_CLK_CTRL;            /* 0x124 DDR Clock Control */
    __IO uint32_t DCI_CLK_CTRL;            /* 0x128 DCI clock control */
    __IO uint32_t APER_CLK_CTRL;           /* 0x12C AMBA peripheral clock control */
    __IO uint32_t USB0_CLK_CTRL;           /* 0x130 USB 0 ULPI Clock Control */
    __IO uint32_t USB1_CLK_CTRL;           /* 0x134 USB 1 ULPI Clock Control */
    __IO uint32_t GEM0_RCLK_CTRL;          /* 0x138 GigE 0 Rx Clock and Rx Signals Select */
    __IO uint32_t GEM1_RCLK_CTRL;          /* 0x13C GbE controller 1 clock control */
    __IO uint32_t GEM0_CLK_CTRL;           /* 0x140 GigE 0 Ref Clock Control */
    __IO uint32_t GEM1_CLK_CTRL;           /* 0x144  */
    __IO uint32_t SMC_CLK_CTRL;            /* 0x148 SMC clock control */
    __IO uint32_t LQSPI_CLK_CTRL;          /* 0x14C QSPI clock control */
    __IO uint32_t SDIO_CLK_CTRL;           /* 0x150 SDIO clock control */
    __IO uint32_t UART_CLK_CTRL;           /* 0x154 UART clock control */
    __IO uint32_t SPI_CLK_CTRL;            /* 0x158 SPI1 clock control */
    __IO uint32_t CAN_CLK_CTRL;            /* 0x15C CAN clock control */
    __IO uint32_t CAN_MIOCLK_CTRL;         /* 0x160  */
    __IO uint32_t DBG_CLK_CTRL;            /* 0x164 Debug clock control */
    __IO uint32_t PCAP_CLK_CTRL;           /* 0x168 PCAP clock control */
    __IO uint32_t TOPSW_CLK_CTRL;          /* 0x16C  */
    __IO uint32_t FPGA0_CLK_CTRL;          /* 0x170 PL Clock 0 Output control */
    __IO uint32_t FPGA0_THR_CTRL;          /* 0x174  */
    __IO uint32_t FPGA0_THR_CNT;           /* 0x178  */
    __I uint32_t FPGA0_THR_STA;            /* 0x17C  */
    __IO uint32_t FPGA1_CLK_CTRL;          /* 0x180 PL Clock 1 Output control */
    __IO uint32_t FPGA1_THR_CTRL;          /* 0x184  */
    __IO uint32_t FPGA1_THR_CNT;           /* 0x188  */
    __I uint32_t FPGA1_THR_STA;            /* 0x18C  */
    __IO uint32_t FPGA2_CLK_CTRL;          /* 0x190 PL Clock 2 output control */
    __IO uint32_t FPGA2_THR_CTRL;          /* 0x194  */
    __IO uint32_t FPGA2_THR_CNT;           /* 0x198  */
    __I uint32_t FPGA2_THR_STA;            /* 0x19C  */
    __IO uint32_t FPGA3_CLK_CTRL;          /* 0x1A0 PL Clock 3 output control */
    __IO uint32_t FPGA3_THR_CTRL;          /* 0x1A4  */
    __IO uint32_t FPGA3_THR_CNT;           /* 0x1A8  */
    __I uint32_t FPGA3_THR_STA;            /* 0x1AC  */
    uint32_t RESERVED2[5];                 /* Reserved, 0x1B0-0x1C3 */
    __IO uint32_t CLK_621_TRUE;            /* 0x1C4 CPU Clock Ratio Mode select */
    uint32_t RESERVED3[14];                /* Reserved, 0x1C8-0x1FF */
    __IO uint32_t PSS_RST_CTRL;            /* 0x200  */
    __IO uint32_t DDR_RST_CTRL;            /* 0x204  */
    __IO uint32_t TOPSW_RST_CTRL;          /* 0x208  */
    __IO uint32_t DMAC_RST_CTRL;           /* 0x20C DMAC Software Reset Control */
    __IO uint32_t USB_RST_CTRL;            /* 0x210 USB Software Reset Control */
    __IO uint32_t GEM_RST_CTRL;            /* 0x214  */
    __IO uint32_t SDIO_RST_CTRL;           /* 0x218  */
    __IO uint32_t SPI_RST_CTRL;            /* 0x21C  */
    __IO uint32_t CAN_RST_CTRL;            /* 0x220  */
    __IO uint32_t I2C_RST_CTRL;            /* 0x224  */
    __IO uint32_t UART_RST_CTRL;           /* 0x228  */
    __IO uint32_t GPIO_RST_CTRL;           /* 0x22C  */
    __IO uint32_t LQSPI_RST_CTRL;          /* 0x230  */
    __IO uint32_t SMC_RST_CTRL;            /* 0x234  */
    __IO uint32_t OCM_RST_CTRL;            /* 0x238  */
    uint32_t RESERVED4;                    /* Reserved, 0x23C */
    __IO uint32_t FPGA_RST_CTRL;           /* 0x240  */
    __IO uint32_t A9_CPU_RST_CTRL;         /* 0x244  */
    uint32_t RESERVED5;                    /* Reserved, 0x248 */
    __IO uint32_t RS_AWDT_CTRL;            /* 0x24C  */
    uint32_t RESERVED6[2];                 /* Reserved, 0x250-0x257 */
    __IO uint32_t REBOOT_STATUS;           /* 0x258  */
    __IO uint32_t BOOT_MODE;               /* 0x25C  (mixed access) */
    uint32_t RESERVED7[40];                /* Reserved, 0x260-0x2FF */
    __IO uint32_t APU_CTRL;                /* 0x300  */
    __IO uint32_t WDT_CLK_SEL;             /* 0x304 System watchdog clock select */
    uint32_t RESERVED8[78];                /* Reserved, 0x308-0x43F */
    __IO uint32_t TZ_DMA_NS;               /* 0x440 DMAC TrustZone Config */
    __IO uint32_t TZ_DMA_IRQ_NS;           /* 0x444 DMAC TrustZone Config for Interrupts */
    __IO uint32_t TZ_DMA_PERIPH_NS;        /* 0x448 DMAC TrustZone Config for Peripherals */
    uint32_t RESERVED9[57];                /* Reserved, 0x44C-0x52F */
    __I uint32_t PSS_IDCODE;               /* 0x530  */
    uint32_t RESERVED10[51];               /* Reserved, 0x534-0x5FF */
    __IO uint32_t DDR_URGENT;              /* 0x600 DDR Urgent control */
    uint32_t RESERVED11[2];                /* Reserved, 0x604-0x60B */
    __IO uint32_t DDR_CAL_START;           /* 0x60C  (mixed access) */
    uint32_t RESERVED12;                   /* Reserved, 0x610 */
    __IO uint32_t DDR_REF_START;           /* 0x614  (mixed access) */
    __IO uint32_t DDR_CMD_STA;             /* 0x618  (mixed access) */
    __IO uint32_t DDR_URGENT_SEL;          /* 0x61C DDR Urgent select */
    __IO uint32_t DDR_DFI_STATUS;          /* 0x620  (mixed access) */
    uint32_t RESERVED13[55];               /* Reserved, 0x624-0x6FF */
    __IO uint32_t MIO_PIN_00;              /* 0x700 MIO Pin 0 Control */
    __IO uint32_t MIO_PIN_01;              /* 0x704 MIO Pin 1 Control */
    __IO uint32_t MIO_PIN_02;              /* 0x708 MIO Pin 2 Control */
    __IO uint32_t MIO_PIN_03;              /* 0x70C MIO Pin 3 Control */
    __IO uint32_t MIO_PIN_04;              /* 0x710 MIO Pin 4 Control */
    __IO uint32_t MIO_PIN_05;              /* 0x714 MIO Pin 5 Control */
    __IO uint32_t MIO_PIN_06;              /* 0x718 MIO Pin 6 Control */
    __IO uint32_t MIO_PIN_07;              /* 0x71C MIO Pin 7 Control */
    __IO uint32_t MIO_PIN_08;              /* 0x720 MIO Pin 8 Control */
    __IO uint32_t MIO_PIN_09;              /* 0x724 MIO Pin 9 Control */
    __IO uint32_t MIO_PIN_10;              /* 0x728 MIO Pin 10 Control */
    __IO uint32_t MIO_PIN_11;              /* 0x72C MIO Pin 11 Control */
    __IO uint32_t MIO_PIN_12;              /* 0x730 MIO Pin 12 Control */
    __IO uint32_t MIO_PIN_13;              /* 0x734 MIO Pin 13 Control */
    __IO uint32_t MIO_PIN_14;              /* 0x738 MIO Pin 14 Control */
    __IO uint32_t MIO_PIN_15;              /* 0x73C MIO Pin 15 Control */
    __IO uint32_t MIO_PIN_16;              /* 0x740 MIO Pin 16 Control */
    __IO uint32_t MIO_PIN_17;              /* 0x744 MIO Pin 17 Control */
    __IO uint32_t MIO_PIN_18;              /* 0x748 MIO Pin 18 Control */
    __IO uint32_t MIO_PIN_19;              /* 0x74C MIO Pin 19 Control */
    __IO uint32_t MIO_PIN_20;              /* 0x750 MIO Pin 20 Control */
    __IO uint32_t MIO_PIN_21;              /* 0x754 MIO Pin 21 Control */
    __IO uint32_t MIO_PIN_22;              /* 0x758 MIO Pin 22 Control */
    __IO uint32_t MIO_PIN_23;              /* 0x75C MIO Pin 23 Control */
    __IO uint32_t MIO_PIN_24;              /* 0x760 MIO Pin 24 Control */
    __IO uint32_t MIO_PIN_25;              /* 0x764 MIO Pin 25 Control */
    __IO uint32_t MIO_PIN_26;              /* 0x768 MIO Pin 26 Control */
    __IO uint32_t MIO_PIN_27;              /* 0x76C MIO Pin 27 Control */
    __IO uint32_t MIO_PIN_28;              /* 0x770 MIO Pin 28 Control */
    __IO uint32_t MIO_PIN_29;              /* 0x774 MIO Pin 29 Control */
    __IO uint32_t MIO_PIN_30;              /* 0x778 MIO Pin 30 Control */
    __IO uint32_t MIO_PIN_31;              /* 0x77C MIO Pin 31 Control */
    __IO uint32_t MIO_PIN_32;              /* 0x780 MIO Pin 32 Control */
    __IO uint32_t MIO_PIN_33;              /* 0x784 MIO Pin 33 Control */
    __IO uint32_t MIO_PIN_34;              /* 0x788 MIO Pin 34 Control */
    __IO uint32_t MIO_PIN_35;              /* 0x78C MIO Pin 35 Control */
    __IO uint32_t MIO_PIN_36;              /* 0x790 MIO Pin 36 Control */
    __IO uint32_t MIO_PIN_37;              /* 0x794 MIO Pin 37 Control */
    __IO uint32_t MIO_PIN_38;              /* 0x798 MIO Pin 38 Control */
    __IO uint32_t MIO_PIN_39;              /* 0x79C MIO Pin 39 Control */
    __IO uint32_t MIO_PIN_40;              /* 0x7A0 MIO Pin 40 Control */
    __IO uint32_t MIO_PIN_41;              /* 0x7A4 MIO Pin 41 Control */
    __IO uint32_t MIO_PIN_42;              /* 0x7A8 MIO Pin 42 Control */
    __IO uint32_t MIO_PIN_43;              /* 0x7AC MIO Pin 43 Control */
    __IO uint32_t MIO_PIN_44;              /* 0x7B0 MIO Pin 44 Control */
    __IO uint32_t MIO_PIN_45;              /* 0x7B4 MIO Pin 45 Control */
    __IO uint32_t MIO_PIN_46;              /* 0x7B8 MIO Pin 46 Control */
    __IO uint32_t MIO_PIN_47;              /* 0x7BC MIO Pin 47 Control */
    __IO uint32_t MIO_PIN_48;              /* 0x7C0 MIO Pin 48 Control */
    __IO uint32_t MIO_PIN_49;              /* 0x7C4 MIO Pin 49 Control */
    __IO uint32_t MIO_PIN_50;              /* 0x7C8 MIO Pin 50 Control */
    __IO uint32_t MIO_PIN_51;              /* 0x7CC MIO Pin 51 Control */
    __IO uint32_t MIO_PIN_52;              /* 0x7D0 MIO Pin 52 Control */
    __IO uint32_t MIO_PIN_53;              /* 0x7D4 MIO Pin 53 Control */
    uint32_t RESERVED14[11];               /* Reserved, 0x7D8-0x803 */
    __IO uint32_t MIO_LOOPBACK;            /* 0x804  */
    uint32_t RESERVED15;                   /* Reserved, 0x808 */
    __IO uint32_t MIO_MST_TRI0;            /* 0x80C  */
    __IO uint32_t MIO_MST_TRI1;            /* 0x810  */
    uint32_t RESERVED16[7];                /* Reserved, 0x814-0x82F */
    __IO uint32_t SD0_WP_CD_SEL;           /* 0x830  */
    __IO uint32_t SD1_WP_CD_SEL;           /* 0x834  */
    uint32_t RESERVED17[50];               /* Reserved, 0x838-0x8FF */
    __IO uint32_t LVL_SHFTR_EN;            /* 0x900 Enable for level shifters between PS and PL */
    uint32_t RESERVED18[3];                /* Reserved, 0x904-0x90F */
    __IO uint32_t OCM_CFG;                 /* 0x910 OCM Address Mapping */
    uint32_t RESERVED19[66];               /* Reserved, 0x914-0xA1B */
    __IO uint32_t L2C_RAM;                 /* 0xA1C (Reserved); required for L2 Cache config. (see TRM,  */
    uint32_t RESERVED20[56];               /* Reserved, 0xA20-0xAFF */
    __IO uint32_t GPIOB_CTRL;              /* 0xB00  */
    __IO uint32_t GPIOB_CFG_CMOS18;        /* 0xB04  */
    __IO uint32_t GPIOB_CFG_CMOS25;        /* 0xB08  */
    __IO uint32_t GPIOB_CFG_CMOS33;        /* 0xB0C  */
    uint32_t RESERVED21;                   /* Reserved, 0xB10 */
    __IO uint32_t GPIOB_CFG_HSTL;          /* 0xB14  */
    __IO uint32_t GPIOB_DRVR_BIAS_CTRL;    /* 0xB18  (mixed access) */
    uint32_t RESERVED22[9];                /* Reserved, 0xB1C-0xB3F */
    __IO uint32_t DDRIOB_ADDR0;            /* 0xB40  */
    __IO uint32_t DDRIOB_ADDR1;            /* 0xB44 DDR IOB Config for BA[2:0], ODT, CS_B, WE_B, RAS_B a */
    __IO uint32_t DDRIOB_DATA0;            /* 0xB48 DDR IOB Config for Data 15:0 */
    __IO uint32_t DDRIOB_DATA1;            /* 0xB4C DDR IOB Config for Data 31:16 */
    __IO uint32_t DDRIOB_DIFF0;            /* 0xB50 DDR IOB Config for DQS 1:0 */
    __IO uint32_t DDRIOB_DIFF1;            /* 0xB54 DDR IOB Config for DQS 3:2 */
    __IO uint32_t DDRIOB_CLOCK;            /* 0xB58 DDR IOB Config for Clock Output */
    __IO uint32_t DDRIOB_DRIVE_SLEW_ADDR;  /* 0xB5C Drive and Slew controls for Address and Command pins */
    __IO uint32_t DDRIOB_DRIVE_SLEW_DATA;  /* 0xB60 Drive and Slew controls for DQ pins of the DDR Inter */
    __IO uint32_t DDRIOB_DRIVE_SLEW_DIFF;  /* 0xB64 Drive and Slew controls for DQS pins of the DDR Inte */
    __IO uint32_t DDRIOB_DRIVE_SLEW_CLOCK; /* 0xB68 Drive and Slew controls for Clock pins of the DDR In */
    __IO uint32_t DDRIOB_DDR_CTRL;         /* 0xB6C DDR IOB Buffer Control */
    __IO uint32_t DDRIOB_DCI_CTRL;         /* 0xB70 DDR IOB DCI Config */
    __IO uint32_t DDRIOB_DCI_STATUS;       /* 0xB74 DDR IO Buffer DCI Status (mixed access) */
} SLCR_Type;

typedef struct {                              /* DDR Memory Controller (DDRC) */
    __IO uint32_t DDRC_CTRL;                  /* 0x000 DDRC Control */
    __IO uint32_t TWO_RANK_CFG;               /* 0x004 Two Rank Configuration */
    __IO uint32_t HPR;                        /* 0x008 HPR Queue control */
    __IO uint32_t LPR;                        /* 0x00C LPR Queue control */
    __IO uint32_t WR_REG;                     /* 0x010 WR Queue control */
    __IO uint32_t DRAM_PARAM_REG0;            /* 0x014 DRAM Parameters 0 */
    __IO uint32_t DRAM_PARAM_REG1;            /* 0x018 DRAM Parameters 1 */
    __IO uint32_t DRAM_PARAM_REG2;            /* 0x01C DRAM Parameters 2 */
    __IO uint32_t DRAM_PARAM_REG3;            /* 0x020 DRAM Parameters 3 (mixed access) */
    __IO uint32_t DRAM_PARAM_REG4;            /* 0x024 DRAM Parameters 4 (mixed access) */
    __IO uint32_t DRAM_INIT_PARAM;            /* 0x028 DRAM Initialization Parameters */
    __IO uint32_t DRAM_EMR;                   /* 0x02C DRAM EMR2, EMR3 access */
    __IO uint32_t DRAM_EMR_MR;                /* 0x030 DRAM EMR, MR access */
    __IO uint32_t DRAM_BURST8_RDWR;           /* 0x034 DRAM Burst 8 read/write (mixed access) */
    __IO uint32_t DRAM_DISABLE_DQ;            /* 0x038 DRAM Disable DQDRAM_addr_map_bank (mixed access) */
    __IO uint32_t DRAM_ADDR_MAP_BANK;         /* 0x03C Row/Column address bits */
    __IO uint32_t DRAM_ADDR_MAP_COL;          /* 0x040 Column address bits */
    __IO uint32_t DRAM_ADDR_MAP_ROW;          /* 0x044 Select DRAM row address bits */
    __IO uint32_t DRAM_ODT;                   /* 0x048 DRAM ODT control */
    __IO uint32_t PHY_DBG;                    /* 0x04C PHY debug */
    __IO uint32_t PHY_CMD_TIMEOUT_RDDATA_CPT; /* 0x050 PHY command time out and read data capture FIFO (mixed access) */
    __I uint32_t MODE_STS;                    /* 0x054 Controller operation mode status */
    __IO uint32_t DLL_CALIB;                  /* 0x058 DLL calibration */
    __IO uint32_t ODT_DELAY_HOLD;             /* 0x05C ODT delay and ODT hold */
    __IO uint32_t CTRL_REG1;                  /* 0x060 Controller 1 (mixed access) */
    __IO uint32_t CTRL_REG2;                  /* 0x064 Controller 2 (mixed access) */
    __IO uint32_t CTRL_REG3;                  /* 0x068 Controller 3 */
    __IO uint32_t CTRL_REG4;                  /* 0x06C Controller 4 */
    uint32_t RESERVED1[2];                    /* Reserved, 0x070-0x077 */
    __IO uint32_t CTRL_REG5;                  /* 0x078 Controller register 5 (mixed access) */
    __IO uint32_t CTRL_REG6;                  /* 0x07C Controller register 6 (mixed access) */
    uint32_t RESERVED2[8];                    /* Reserved, 0x080-0x09F */
    __IO uint32_t CHE_REFRESH_TIMER01;        /* 0x0A0 CHE_REFRESH_TIMER01 */
    __IO uint32_t CHE_T_ZQ;                   /* 0x0A4 ZQ parameters */
    __IO uint32_t CHE_T_ZQ_SHORT_INTERVAL;    /* 0x0A8 Misc parameters */
    __IO uint32_t DEEP_PWRDWN;                /* 0x0AC Deep powerdown (LPDDR2) */
    __IO uint32_t REG_2C;                     /* 0x0B0 Training control (mixed access) */
    __IO uint32_t REG_2D;                     /* 0x0B4 Misc Debug */
    __IO uint32_t DFI_TIMING;                 /* 0x0B8 DFI timing */
    uint32_t RESERVED3[2];                    /* Reserved, 0x0BC-0x0C3 */
    __IO uint32_t CHE_ECC_CONTROL;            /* 0x0C4 ECC error clear */
    __IO uint32_t CHE_CORR_ECC_LOG;           /* 0x0C8 ECC error correction (mixed access) */
    __I uint32_t CHE_CORR_ECC_ADDR;           /* 0x0CC ECC error correction address log */
    __I uint32_t CHE_CORR_ECC_DATA_31_0;      /* 0x0D0 ECC error correction data log low */
    __I uint32_t CHE_CORR_ECC_DATA_63_32;     /* 0x0D4 ECC error correction data log mid */
    __I uint32_t CHE_CORR_ECC_DATA_71_64;     /* 0x0D8 ECC error correction data log high */
    __IO uint32_t CHE_UNCORR_ECC_LOG;         /* 0x0DC ECC unrecoverable error status */
    __I uint32_t CHE_UNCORR_ECC_ADDR;         /* 0x0E0 ECC unrecoverable error address */
    __I uint32_t CHE_UNCORR_ECC_DATA_31_0;    /* 0x0E4 ECC unrecoverable error data low */
    __I uint32_t CHE_UNCORR_ECC_DATA_63_32;   /* 0x0E8 ECC unrecoverable error data middle */
    __I uint32_t CHE_UNCORR_ECC_DATA_71_64;   /* 0x0EC ECC unrecoverable error data high */
    __IO uint32_t CHE_ECC_STATS;              /* 0x0F0 ECC error count */
    __IO uint32_t ECC_SCRUB;                  /* 0x0F4 ECC mode/scrub */
    __I uint32_t CHE_ECC_CORR_BIT_MASK_31_0;  /* 0x0F8 ECC data mask low */
    __I uint32_t CHE_ECC_CORR_BIT_MASK_63_32; /* 0x0FC ECC data mask high */
    uint32_t RESERVED4[5];                    /* Reserved, 0x100-0x113 */
    __IO uint32_t PHY_RCVR_ENABLE;            /* 0x114 Phy receiver enable register */
    __IO uint32_t PHY_CONFIG0;                /* 0x118 PHY configuration register for data slice 0. */
    __IO uint32_t PHY_CONFIG1;                /* 0x11C PHY configuration register for data slice 1. */
    __IO uint32_t PHY_CONFIG2;                /* 0x120 PHY configuration register for data slice 2. */
    __IO uint32_t PHY_CONFIG3;                /* 0x124 PHY configuration register for data slice 3. */
    uint32_t RESERVED5;                       /* Reserved, 0x128 */
    __IO uint32_t PHY_INIT_RATIO0;            /* 0x12C PHY init ratio register for data slice 0. */
    __IO uint32_t PHY_INIT_RATIO1;            /* 0x130 PHY init ratio register for data slice 1. */
    __IO uint32_t PHY_INIT_RATIO2;            /* 0x134 PHY init ratio register for data slice 2. */
    __IO uint32_t PHY_INIT_RATIO3;            /* 0x138 PHY init ratio register for data slice 3. */
    uint32_t RESERVED6;                       /* Reserved, 0x13C */
    __IO uint32_t PHY_RD_DQS_CFG0;            /* 0x140 PHY read DQS configuration register for data slice 0 */
    __IO uint32_t PHY_RD_DQS_CFG1;            /* 0x144 PHY read DQS configuration register for data slice 1 */
    __IO uint32_t PHY_RD_DQS_CFG2;            /* 0x148 PHY read DQS configuration register for data slice 2 */
    __IO uint32_t PHY_RD_DQS_CFG3;            /* 0x14C PHY read DQS configuration register for data slice 3 */
    uint32_t RESERVED7;                       /* Reserved, 0x150 */
    __IO uint32_t PHY_WR_DQS_CFG0;            /* 0x154 PHY write DQS configuration register for data slice  */
    __IO uint32_t PHY_WR_DQS_CFG1;            /* 0x158 PHY write DQS configuration register for data slice  */
    __IO uint32_t PHY_WR_DQS_CFG2;            /* 0x15C PHY write DQS configuration register for data slice  */
    __IO uint32_t PHY_WR_DQS_CFG3;            /* 0x160 PHY write DQS configuration register for data slice  */
    uint32_t RESERVED8;                       /* Reserved, 0x164 */
    __IO uint32_t PHY_WE_CFG0;                /* 0x168 PHY FIFO write enable configuration for data slice 0 */
    __IO uint32_t PHY_WE_CFG1;                /* 0x16C PHY FIFO write enable configuration for data slice 1 */
    __IO uint32_t PHY_WE_CFG2;                /* 0x170 PHY FIFO write enable configuration for data slice 2 */
    __IO uint32_t PHY_WE_CFG3;                /* 0x174 PHY FIFO write enable configuration for data slice 3 */
    uint32_t RESERVED9;                       /* Reserved, 0x178 */
    __IO uint32_t WR_DATA_SLV0;               /* 0x17C PHY write data slave ratio config for data slice 0. */
    __IO uint32_t WR_DATA_SLV1;               /* 0x180 PHY write data slave ratio config for data slice 1. */
    __IO uint32_t WR_DATA_SLV2;               /* 0x184 PHY write data slave ratio config for data slice 2. */
    __IO uint32_t WR_DATA_SLV3;               /* 0x188 PHY write data slave ratio config for data slice 3. */
    uint32_t RESERVED10;                      /* Reserved, 0x18C */
    __IO uint32_t REG_64;                     /* 0x190 Training control 2 */
    __IO uint32_t REG_65;                     /* 0x194 Training control 3 */
    uint32_t RESERVED11[3];                   /* Reserved, 0x198-0x1A3 */
    __I uint32_t REG69_6A0;                   /* 0x1A4 Training results for data slice 0. */
    __IO uint32_t REG69_6A1;                  /* 0x1A8 Training results for data slice 1. */
    uint32_t RESERVED12;                      /* Reserved, 0x1AC */
    __I uint32_t REG6C_6D2;                   /* 0x1B0 Training results for data slice 2. */
    __I uint32_t REG6C_6D3;                   /* 0x1B4 Training results for data slice 3. */
    __I uint32_t REG6E_710;                   /* 0x1B8 Training results (2) for data slice 0. */
    __IO uint32_t REG6E_711;                  /* 0x1BC Training results (2) for data slice 1. */
    __IO uint32_t REG6E_712;                  /* 0x1C0 Training results (2) for data slice 2. */
    __IO uint32_t REG6E_713;                  /* 0x1C4 Training results (2) for data slice 3. */
    uint32_t RESERVED13;                      /* Reserved, 0x1C8 */
    __I uint32_t PHY_DLL_STS0;                /* 0x1CC Slave DLL results for data slice 0. */
    __IO uint32_t PHY_DLL_STS1;               /* 0x1D0 Slave DLL results for data slice 1. */
    __IO uint32_t PHY_DLL_STS2;               /* 0x1D4 Slave DLL results for data slice 2. */
    __IO uint32_t PHY_DLL_STS3;               /* 0x1D8 Slave DLL results for data slice 3. */
    uint32_t RESERVED14;                      /* Reserved, 0x1DC */
    __IO uint32_t DLL_LOCK_STS;               /* 0x1E0 DLL Lock Status, read */
    __I uint32_t PHY_CTRL_STS;                /* 0x1E4 PHY Control status, read */
    __I uint32_t PHY_CTRL_STS_REG2;           /* 0x1E8 PHY Control status (2), read */
    uint32_t RESERVED15[5];                   /* Reserved, 0x1EC-0x1FF */
    __I uint32_t AXI_ID;                      /* 0x200 ID and revision information */
    __IO uint32_t PAGE_MASK;                  /* 0x204 Page mask */
    __IO uint32_t AXI_PRIORITY_WR_PORT0;      /* 0x208 AXI Priority control for write port 0. (mixed access) */
    uint32_t RESERVED16[3];                   /* Reserved, 0x20C-0x217 */
    __IO uint32_t AXI_PRIORITY_RD_PORT0;      /* 0x218 AXI Priority control for read port 0. (mixed access) */
    uint32_t RESERVED17[30];                  /* Reserved, 0x21C-0x293 */
    __IO uint32_t EXCL_ACCESS_CFG0;           /* 0x294 Exclusive access configuration for port 0. */
    __IO uint32_t EXCL_ACCESS_CFG1;           /* 0x298 Exclusive access configuration for port 1. */
    __IO uint32_t EXCL_ACCESS_CFG2;           /* 0x29C Exclusive access configuration for port 2. */
    __IO uint32_t EXCL_ACCESS_CFG3;           /* 0x2A0 Exclusive access configuration for port 3. */
    __I uint32_t MODE_REG_READ;               /* 0x2A4 Mode register read data */
    __IO uint32_t LPDDR_CTRL0;                /* 0x2A8 LPDDR2 Control 0 */
    __IO uint32_t LPDDR_CTRL1;                /* 0x2AC LPDDR2 Control 1 */
    __IO uint32_t LPDDR_CTRL2;                /* 0x2B0 LPDDR2 Control 2 */
    __IO uint32_t LPDDR_CTRL3;                /* 0x2B4 LPDDR2 Control 3 */
} DDRC_Type;

typedef struct {                   /* Device Configuration Interface (DevCfg) */
    __IO uint32_t CTRL;            /* 0x000 Control Register (mixed access) */
    __IO uint32_t LOCK;            /* 0x004 Lock Register (mixed access) */
    __IO uint32_t CFG;             /* 0x008 Configuration Register */
    __IO uint32_t INT_STS;         /* 0x00C Interrupt Status Register */
    __IO uint32_t INT_MASK;        /* 0x010 Interrupt Mask Register */
    __IO uint32_t STATUS;          /* 0x014 Status Register (mixed access) */
    __IO uint32_t DMA_SRC_ADDR;    /* 0x018 DMA Source Address Register */
    __IO uint32_t DMA_DEST_ADDR;   /* 0x01C DMA Destination Address Reg */
    __IO uint32_t DMA_SRC_LEN;     /* 0x020 DMA Source Transfer Length */
    __IO uint32_t DMA_DEST_LEN;    /* 0x024 DMA Destination Transfer */
    __O uint32_t ROM_SHADOW;       /* 0x028 DMA ROM Shadow Register */
    __IO uint32_t MULTIBOOT_ADDR;  /* 0x02C Multi BootAddress Pointer */
    __IO uint32_t SW_ID;           /* 0x030 Software ID Register */
    __IO uint32_t UNLOCK;          /* 0x034 Unlock Register */
    uint32_t RESERVED0[18];        /* Reserved, 0x038-0x07F */
    __IO uint32_t MCTRL;           /* 0x080 Miscellaneous Control Reg (mixed access) */
    uint32_t RESERVED1[31];        /* Reserved, 0x084-0x0FF */
    __IO uint32_t XADCIF_CFG;      /* 0x100  */
    __IO uint32_t XADCIF_INT_STS;  /* 0x104  */
    __IO uint32_t XADCIF_INT_MASK; /* 0x108  */
    __I uint32_t XADCIF_MSTS;      /* 0x10C  */
    __O uint32_t XADCIF_CMDFIFO;   /* 0x110  */
    __I uint32_t XADCIF_RDFIFO;    /* 0x114  */
    __IO uint32_t XADCIF_MCTL;     /* 0x118  */
} DEVCFG_Type;

typedef struct {               /* DMA Controller (PL330 DMAC) */
    __IO uint32_t DS;          /* 0x000 DMA Status Register (mixed access) */
    __IO uint32_t DPC;         /* 0x004 DMA Program Counter Rregister (mixed access) */
    uint32_t RESERVED0[6];     /* Reserved, 0x008-0x01F */
    __IO uint32_t INTEN;       /* 0x020 DMA Interrupt Enable Register (mixed access) */
    __IO uint32_t ES;          /* 0x024 DMA Event Status Register (mixed access) */
    __IO uint32_t INTMIS;      /* 0x028  (mixed access) */
    __IO uint32_t INTCLR;      /* 0x02C DMA Interrupt Clear Register (mixed access) */
    __IO uint32_t FSRD;        /* 0x030  (mixed access) */
    __IO uint32_t FSRC;        /* 0x034  (mixed access) */
    __IO uint32_t FTM;         /* 0x038 DMA Fault Type DMA Manager Register (mixed access) */
    uint32_t RESERVED1;        /* Reserved, 0x03C */
    __IO uint32_t FTC0;        /* 0x040 DMA Fault Type for DMA Channel 0 (mixed access) */
    __IO uint32_t FTR1;        /* 0x044  (mixed access) */
    __IO uint32_t FTR2;        /* 0x048  (mixed access) */
    __IO uint32_t FTR3;        /* 0x04C  (mixed access) */
    __IO uint32_t FTR4;        /* 0x050  (mixed access) */
    __IO uint32_t FTR5;        /* 0x054  (mixed access) */
    __IO uint32_t FTR6;        /* 0x058  (mixed access) */
    __IO uint32_t FTR7;        /* 0x05C  (mixed access) */
    uint32_t RESERVED2[40];    /* Reserved, 0x060-0x0FF */
    __IO uint32_t CS0;         /* 0x100 Channel Status for DMA Channel 0 (mixed access) */
    __IO uint32_t CPC0;        /* 0x104  (mixed access) */
    __IO uint32_t CSR1;        /* 0x108  (mixed access) */
    __IO uint32_t CPC1;        /* 0x10C  (mixed access) */
    __IO uint32_t CSR2;        /* 0x110  (mixed access) */
    __IO uint32_t CPC2;        /* 0x114  (mixed access) */
    __IO uint32_t CSR3;        /* 0x118  (mixed access) */
    __IO uint32_t CPC3;        /* 0x11C  (mixed access) */
    __IO uint32_t CSR4;        /* 0x120  (mixed access) */
    __IO uint32_t CPC4;        /* 0x124  (mixed access) */
    __IO uint32_t CSR5;        /* 0x128  (mixed access) */
    __IO uint32_t CPC5;        /* 0x12C  (mixed access) */
    __IO uint32_t CSR6;        /* 0x130  (mixed access) */
    __IO uint32_t CPC6;        /* 0x134  (mixed access) */
    __IO uint32_t CSR7;        /* 0x138  (mixed access) */
    __IO uint32_t CPC7;        /* 0x13C  (mixed access) */
    uint32_t RESERVED3[176];   /* Reserved, 0x140-0x3FF */
    __IO uint32_t SAR0;        /* 0x400  (mixed access) */
    __IO uint32_t DAR0;        /* 0x404  */
    __IO uint32_t CCR0;        /* 0x408  (mixed access) */
    __IO uint32_t LC0_0;       /* 0x40C Loop Counter 0 for DMA Channel 0 (mixed access) */
    __IO uint32_t LC1_0;       /* 0x410 Loop Counter 1 for DMA Channel 0 (mixed access) */
    uint32_t RESERVED4[3];     /* Reserved, 0x414-0x41F */
    __IO uint32_t SAR1;        /* 0x420  (mixed access) */
    __IO uint32_t DAR1;        /* 0x424  (mixed access) */
    __IO uint32_t CCR1;        /* 0x428  (mixed access) */
    __IO uint32_t LC0_1;       /* 0x42C  (mixed access) */
    __IO uint32_t LC1_1;       /* 0x430  (mixed access) */
    uint32_t RESERVED5[3];     /* Reserved, 0x434-0x43F */
    __IO uint32_t SAR2;        /* 0x440  (mixed access) */
    __IO uint32_t DAR2;        /* 0x444  (mixed access) */
    __IO uint32_t CCR2;        /* 0x448  (mixed access) */
    __IO uint32_t LC0_2;       /* 0x44C  (mixed access) */
    __IO uint32_t LC1_2;       /* 0x450  (mixed access) */
    uint32_t RESERVED6[3];     /* Reserved, 0x454-0x45F */
    __IO uint32_t SAR3;        /* 0x460  (mixed access) */
    __IO uint32_t DAR3;        /* 0x464  (mixed access) */
    __IO uint32_t CCR3;        /* 0x468  (mixed access) */
    __IO uint32_t LC0_3;       /* 0x46C  (mixed access) */
    __IO uint32_t LC1_3;       /* 0x470  (mixed access) */
    uint32_t RESERVED7[3];     /* Reserved, 0x474-0x47F */
    __IO uint32_t SAR4;        /* 0x480  (mixed access) */
    __IO uint32_t DAR4;        /* 0x484  (mixed access) */
    __IO uint32_t CCR4;        /* 0x488  (mixed access) */
    __IO uint32_t LC0_4;       /* 0x48C  (mixed access) */
    __IO uint32_t LC1_4;       /* 0x490  (mixed access) */
    uint32_t RESERVED8[3];     /* Reserved, 0x494-0x49F */
    __IO uint32_t SAR5;        /* 0x4A0  (mixed access) */
    __IO uint32_t DAR5;        /* 0x4A4  (mixed access) */
    __IO uint32_t CCR5;        /* 0x4A8  (mixed access) */
    __IO uint32_t LC0_5;       /* 0x4AC  (mixed access) */
    __IO uint32_t LC1_5;       /* 0x4B0  (mixed access) */
    uint32_t RESERVED9[3];     /* Reserved, 0x4B4-0x4BF */
    __IO uint32_t SAR6;        /* 0x4C0  (mixed access) */
    __IO uint32_t DAR6;        /* 0x4C4  (mixed access) */
    __IO uint32_t CCR6;        /* 0x4C8  (mixed access) */
    __IO uint32_t LC0_6;       /* 0x4CC  (mixed access) */
    __IO uint32_t LC1_6;       /* 0x4D0  (mixed access) */
    uint32_t RESERVED10[3];    /* Reserved, 0x4D4-0x4DF */
    __IO uint32_t SAR7;        /* 0x4E0  (mixed access) */
    __IO uint32_t DAR7;        /* 0x4E4  (mixed access) */
    __IO uint32_t CCR7;        /* 0x4E8  (mixed access) */
    __IO uint32_t LC0_7;       /* 0x4EC  (mixed access) */
    __IO uint32_t LC1_7;       /* 0x4F0  (mixed access) */
    uint32_t RESERVED11[515];  /* Reserved, 0x4F4-0xCFF */
    __IO uint32_t DBGSTATUS;   /* 0xD00 Debug Status Register (mixed access) */
    __IO uint32_t DBGCMD;      /* 0xD04 Debug Command Register (mixed access) */
    __IO uint32_t DBGINST0;    /* 0xD08 Debug Instruction 0 Register (mixed access) */
    __IO uint32_t DBGINST1;    /* 0xD0C Debug Instruction 1 Register (mixed access) */
    uint32_t RESERVED12[60];   /* Reserved, 0xD10-0xDFF */
    __IO uint32_t CR0;         /* 0xE00 Configuration Register 0 (mixed access) */
    __IO uint32_t CR1;         /* 0xE04 Configuration Register 1 (mixed access) */
    __IO uint32_t CR2;         /* 0xE08 Configuration Register 2 (mixed access) */
    __IO uint32_t CR3;         /* 0xE0C Configuration Register 3 (mixed access) */
    __IO uint32_t CR4;         /* 0xE10 Configuration Register 4 (mixed access) */
    __IO uint32_t CRDN;        /* 0xE14 Configuration Register Dn (mixed access) */
    uint32_t RESERVED13[26];   /* Reserved, 0xE18-0xE7F */
    __IO uint32_t WD;          /* 0xE80  (mixed access) */
    uint32_t RESERVED14[87];   /* Reserved, 0xE84-0xFDF */
    __IO uint32_t PERIPH_ID_0; /* 0xFE0  (mixed access) */
    __IO uint32_t PERIPH_ID_1; /* 0xFE4  (mixed access) */
    __IO uint32_t PERIPH_ID_2; /* 0xFE8  (mixed access) */
    __IO uint32_t PERIPH_ID_3; /* 0xFEC  (mixed access) */
    __IO uint32_t PCELL_ID_0;  /* 0xFF0  (mixed access) */
    __IO uint32_t PCELL_ID_1;  /* 0xFF4  (mixed access) */
    __IO uint32_t PCELL_ID_2;  /* 0xFF8  (mixed access) */
    __IO uint32_t PCELL_ID_3;  /* 0xFFC  (mixed access) */
} DMAC_Type;

typedef struct {                        /* Gigabit Ethernet Controller (GEM) */
    __IO uint32_t NWCTRL;               /* 0x000 Network Control reg (mixed access) */
    __IO uint32_t NWCFG;                /* 0x004 Network Config reg */
    __I uint32_t NWSR;                  /* 0x008 Network Status reg */
    uint32_t RESERVED0;                 /* Reserved, 0x00C */
    __IO uint32_t DMACR;                /* 0x010 DMA Control reg (mixed access) */
    __IO uint32_t TXSR;                 /* 0x014 TX Status reg (mixed access) */
    __IO uint32_t RXQBASE;              /* 0x018 RX Q Base address reg (mixed access) */
    __IO uint32_t TXQBASE;              /* 0x01C TX Q Base address reg (mixed access) */
    __IO uint32_t RXSR;                 /* 0x020 RX Status reg (mixed access) */
    __IO uint32_t ISR;                  /* 0x024 Interrupt Status reg (mixed access) */
    __O uint32_t IER;                   /* 0x028 Interrupt Enable reg */
    __O uint32_t IDR;                   /* 0x02C Interrupt Disable reg */
    __IO uint32_t IMR;                  /* 0x030 Interrupt Mask reg (mixed access) */
    __IO uint32_t PHYMNTNC;             /* 0x034 Phy Maintaince reg */
    __I uint32_t RXPAUSE;               /* 0x038 RX Pause Time reg */
    __IO uint32_t TXPAUSE;              /* 0x03C TX Pause Time reg */
    uint32_t RESERVED1[16];             /* Reserved, 0x040-0x07F */
    __IO uint32_t HASHL;                /* 0x080 Hash Low address reg */
    __IO uint32_t HASHH;                /* 0x084 Hash High address reg */
    __IO uint32_t LADDR1L;              /* 0x088 Specific1 addr low reg */
    __IO uint32_t LADDR1H;              /* 0x08C Specific1 addr high reg (mixed access) */
    __IO uint32_t LADDR2L;              /* 0x090 Specific2 addr low reg */
    __IO uint32_t LADDR2H;              /* 0x094 Specific2 addr high reg (mixed access) */
    __IO uint32_t LADDR3L;              /* 0x098 Specific3 addr low reg */
    __IO uint32_t LADDR3H;              /* 0x09C Specific3 addr high reg (mixed access) */
    __IO uint32_t LADDR4L;              /* 0x0A0 Specific4 addr low reg */
    __IO uint32_t LADDR4H;              /* 0x0A4 Specific4 addr high reg (mixed access) */
    __IO uint32_t MATCH1;               /* 0x0A8 Type ID1 Match reg (mixed access) */
    __IO uint32_t MATCH2;               /* 0x0AC Type ID2 Match reg (mixed access) */
    __IO uint32_t MATCH3;               /* 0x0B0 Type ID3 Match reg (mixed access) */
    __IO uint32_t MATCH4;               /* 0x0B4 Type ID4 Match reg (mixed access) */
    __IO uint32_t WAKE_ON_LAN;          /* 0x0B8  (mixed access) */
    __IO uint32_t STRETCH;              /* 0x0BC IPG Stretch reg (mixed access) */
    __IO uint32_t STACKED_VLAN;         /* 0x0C0  (mixed access) */
    __IO uint32_t TX_PFC_PAUSE;         /* 0x0C4  (mixed access) */
    __IO uint32_t SPEC_ADDR1_MASK_BOT;  /* 0x0C8  */
    __IO uint32_t SPEC_ADDR1_MASK_TOP;  /* 0x0CC  (mixed access) */
    uint32_t RESERVED2[11];             /* Reserved, 0x0D0-0x0FB */
    __I uint32_t MODULE_ID;             /* 0x0FC  */
    __I uint32_t OCTETS_TX_BOT;         /* 0x100  */
    __I uint32_t OCTETS_TX_TOP;         /* 0x104  */
    __I uint32_t FRAMES_TX;             /* 0x108  */
    __I uint32_t BROADCAST_FRAMES_TX;   /* 0x10C  */
    __IO uint32_t MULTI_FRAMES_TX;      /* 0x110  */
    __I uint32_t PAUSE_FRAMES_TX;       /* 0x114  */
    __I uint32_t FRAMES_64B_TX;         /* 0x118  */
    __I uint32_t FRAMES_65TO127B_TX;    /* 0x11C  */
    __I uint32_t FRAMES_128TO255B_TX;   /* 0x120  */
    __I uint32_t FRAMES_256TO511B_TX;   /* 0x124  */
    __I uint32_t FRAMES_512TO1023B_TX;  /* 0x128  */
    __I uint32_t FRAMES_1024TO1518B_TX; /* 0x12C  */
    uint32_t RESERVED3;                 /* Reserved, 0x130 */
    __I uint32_t TX_UNDER_RUNS;         /* 0x134  */
    __I uint32_t SINGLE_COLLISN_FRAMES; /* 0x138  */
    __I uint32_t MULTI_COLLISN_FRAMES;  /* 0x13C  */
    __I uint32_t EXCESSIVE_COLLISNS;    /* 0x140  */
    __I uint32_t LATE_COLLISNS;         /* 0x144  */
    __I uint32_t DEFERRED_TX_FRAMES;    /* 0x148  */
    __I uint32_t CARRIER_SENSE_ERRS;    /* 0x14C  */
    __I uint32_t OCTETS_RX_BOT;         /* 0x150  */
    __I uint32_t OCTETS_RX_TOP;         /* 0x154  */
    __IO uint32_t FRAMES_RX;            /* 0x158  */
    __I uint32_t BDCAST_FAMES_RX;       /* 0x15C  */
    __I uint32_t MULTI_FRAMES_RX;       /* 0x160  */
    __I uint32_t PAUSE_RX;              /* 0x164  */
    __I uint32_t FRAMES_64B_RX;         /* 0x168  */
    __I uint32_t FRAMES_65TO127B_RX;    /* 0x16C  */
    __I uint32_t FRAMES_128TO255B_RX;   /* 0x170  */
    __I uint32_t FRAMES_256TO511B_RX;   /* 0x174  */
    __I uint32_t FRAMES_512TO1023B_RX;  /* 0x178  */
    __I uint32_t FRAMES_1024TO1518B_RX; /* 0x17C  */
    uint32_t RESERVED4;                 /* Reserved, 0x180 */
    __I uint32_t UNDERSZ_RX;            /* 0x184  */
    __I uint32_t OVERSZ_RX;             /* 0x188  */
    __I uint32_t JAB_RX;                /* 0x18C  */
    __I uint32_t FCS_ERRORS;            /* 0x190  */
    __I uint32_t LENGTH_FIELD_ERRORS;   /* 0x194  */
    __I uint32_t RXSYMBCNT;             /* 0x198 Symbol Error Counter */
    __I uint32_t RXALIGNCNT;            /* 0x19C Alignment Error Counter */
    __I uint32_t RX_RESOURCE_ERRORS;    /* 0x1A0  */
    __I uint32_t RXORCNT;               /* 0x1A4 Receive Overrun Counter */
    __IO uint32_t IP_HDR_CSUM_ERRORS;   /* 0x1A8  */
    __I uint32_t TCP_CSUM_ERRORS;       /* 0x1AC  */
    __I uint32_t UDP_CSUM_ERRORS;       /* 0x1B0  */
    uint32_t RESERVED5[5];              /* Reserved, 0x1B4-0x1C7 */
    __IO uint32_t TIMER_STROBE_S;       /* 0x1C8  */
    __IO uint32_t TIMER_STROBE_NS;      /* 0x1CC  (mixed access) */
    __IO uint32_t PTP_SEC;              /* 0x1D0 1588 second counter */
    __IO uint32_t PTP_NANOSEC;          /* 0x1D4 1588 nanosecond counter (mixed access) */
    __IO uint32_t TIMER_ADJUST;         /* 0x1D8  (mixed access) */
    __IO uint32_t TIMER_INCR;           /* 0x1DC  (mixed access) */
    __I uint32_t PTP_TX_S;              /* 0x1E0  */
    __I uint32_t PTP_TX_NS;             /* 0x1E4  */
    __I uint32_t PTP_RX_S;              /* 0x1E8  */
    __I uint32_t PTP_RX_NS;             /* 0x1EC  */
    __I uint32_t PTP_PEER_TX_S;         /* 0x1F0  */
    __I uint32_t PTP_PEER_TX_NS;        /* 0x1F4  */
    __I uint32_t PTP_PEER_RX_S;         /* 0x1F8  */
    __I uint32_t PTP_PEER_RX_NS;        /* 0x1FC  */
    uint32_t RESERVED6[33];             /* Reserved, 0x200-0x283 */
    __I uint32_t DESIGN_CFG2;           /* 0x284  */
    __I uint32_t DESIGN_CFG3;           /* 0x288  */
    __I uint32_t DESIGN_CFG4;           /* 0x28C  */
    __I uint32_t DESIGN_CFG5;           /* 0x290  */
} GEM_Type;

typedef struct {                   /* General Purpose I/O (GPIO) */
    __IO uint32_t MASK_DATA_0_LSW; /* 0x000 Mask and Data Register LSW, WO (mixed access) */
    __IO uint32_t MASK_DATA_0_MSW; /* 0x004 Mask and Data Register MSW, WO (mixed access) */
    __IO uint32_t MASK_DATA_1_LSW; /* 0x008 Data/Mask Registers offset (mixed access) */
    __IO uint32_t MASK_DATA_1_MSW; /* 0x00C  (mixed access) */
    __IO uint32_t MASK_DATA_2_LSW; /* 0x010  (mixed access) */
    __IO uint32_t MASK_DATA_2_MSW; /* 0x014  (mixed access) */
    __IO uint32_t MASK_DATA_3_LSW; /* 0x018  (mixed access) */
    __IO uint32_t MASK_DATA_3_MSW; /* 0x01C  (mixed access) */
    uint32_t RESERVED0[8];         /* Reserved, 0x020-0x03F */
    __IO uint32_t DATA_0;          /* 0x040 Data Register, RW */
    __IO uint32_t DATA_1;          /* 0x044  */
    __IO uint32_t DATA_2;          /* 0x048  */
    __IO uint32_t DATA_3;          /* 0x04C  */
    uint32_t RESERVED1[4];         /* Reserved, 0x050-0x05F */
    __I uint32_t DATA_0_RO;        /* 0x060 Data Register - Input, RO */
    __I uint32_t DATA_1_RO;        /* 0x064  */
    __I uint32_t DATA_2_RO;        /* 0x068  */
    __I uint32_t DATA_3_RO;        /* 0x06C  */
    uint32_t RESERVED2[101];       /* Reserved, 0x070-0x203 */
    __IO uint32_t DIRM_0;          /* 0x204 Direction Mode Register, RW */
    __IO uint32_t OEN_0;           /* 0x208 Output Enable Register, RW */
    __I uint32_t INT_MASK_0;       /* 0x20C Interrupt Mask Register, RO */
    __O uint32_t INT_EN_0;         /* 0x210 Interrupt Enable Register, WO */
    __O uint32_t INT_DIS_0;        /* 0x214 Interrupt Disable Register, WO */
    __IO uint32_t INT_STAT_0;      /* 0x218 Interrupt Status Register, RO */
    __IO uint32_t INT_TYPE_0;      /* 0x21C Interrupt Type Register, RW */
    __IO uint32_t INT_POLARITY_0;  /* 0x220 Interrupt Polarity Register, RW */
    __IO uint32_t INT_ANY_0;       /* 0x224 Interrupt On Any Register, RW */
    uint32_t RESERVED3[7];         /* Reserved, 0x228-0x243 */
    __IO uint32_t DIRM_1;          /* 0x244  */
    __IO uint32_t OEN_1;           /* 0x248  */
    __I uint32_t INT_MASK_1;       /* 0x24C  */
    __O uint32_t INT_EN_1;         /* 0x250  */
    __O uint32_t INT_DIS_1;        /* 0x254  */
    __IO uint32_t INT_STAT_1;      /* 0x258  */
    __IO uint32_t INT_TYPE_1;      /* 0x25C  */
    __IO uint32_t INT_POLARITY_1;  /* 0x260  */
    __IO uint32_t INT_ANY_1;       /* 0x264  */
    uint32_t RESERVED4[7];         /* Reserved, 0x268-0x283 */
    __IO uint32_t DIRM_2;          /* 0x284  */
    __IO uint32_t OEN_2;           /* 0x288  */
    __I uint32_t INT_MASK_2;       /* 0x28C  */
    __O uint32_t INT_EN_2;         /* 0x290  */
    __O uint32_t INT_DIS_2;        /* 0x294  */
    __IO uint32_t INT_STAT_2;      /* 0x298  */
    __IO uint32_t INT_TYPE_2;      /* 0x29C  */
    __IO uint32_t INT_POLARITY_2;  /* 0x2A0  */
    __IO uint32_t INT_ANY_2;       /* 0x2A4  */
    uint32_t RESERVED5[7];         /* Reserved, 0x2A8-0x2C3 */
    __IO uint32_t DIRM_3;          /* 0x2C4  */
    __IO uint32_t OEN_3;           /* 0x2C8  */
    __I uint32_t INT_MASK_3;       /* 0x2CC  */
    __O uint32_t INT_EN_3;         /* 0x2D0  */
    __O uint32_t INT_DIS_3;        /* 0x2D4  */
    __IO uint32_t INT_STAT_3;      /* 0x2D8  */
    __IO uint32_t INT_TYPE_3;      /* 0x2DC  */
    __IO uint32_t INT_POLARITY_3;  /* 0x2E0  */
    __IO uint32_t INT_ANY_3;       /* 0x2E4  */
} GPIO_Type;

typedef struct {              /* I2C Controller */
    __IO uint32_t CR;         /* 0x000 32-bit Control (mixed access) */
    __I uint32_t SR;          /* 0x004 Status */
    __IO uint32_t ADDR;       /* 0x008 IIC Address (mixed access) */
    __IO uint32_t DATA;       /* 0x00C IIC FIFO Data (mixed access) */
    __IO uint32_t ISR;        /* 0x010 Interrupt Status (mixed access) */
    __IO uint32_t TRANS_SIZE; /* 0x014 Transfer Size */
    __IO uint32_t SLV_PAUSE;  /* 0x018 Slave monitor pause (mixed access) */
    __IO uint32_t TIME_OUT;   /* 0x01C Time Out */
    __I uint32_t IMR;         /* 0x020 Interrupt Enabled Mask */
    __IO uint32_t IER;        /* 0x024 Interrupt Enable (mixed access) */
    __IO uint32_t IDR;        /* 0x028 Interrupt Disable (mixed access) */
} I2C_Type;

typedef struct {                /* Quad-SPI Flash Controller */
    __IO uint32_t CR;           /* 0x000 QSPI configuration register (mixed access) */
    __IO uint32_t SR;           /* 0x004 QSPI interrupt status register (mixed access) */
    __IO uint32_t IER;          /* 0x008 Interrupt Enable register. (mixed access) */
    __IO uint32_t IDR;          /* 0x00C Interrupt disable register. (mixed access) */
    __I uint32_t IMR;           /* 0x010 Interrupt mask register */
    __IO uint32_t ER;           /* 0x014 SPI_Enable Register (mixed access) */
    __IO uint32_t DR;           /* 0x018 Delay Register */
    __O uint32_t TXD_00;        /* 0x01C Transmit Data Register. Keyhole addresses for the Tr */
    __I uint32_t RXD;           /* 0x020 Receive Data Register */
    __IO uint32_t SICR;         /* 0x024 Slave Idle Count Register (mixed access) */
    __IO uint32_t TXWR;         /* 0x028 TX_FIFO Threshold Register */
    __IO uint32_t RXWR;         /* 0x02C RX FIFO Threshold Register */
    __IO uint32_t GPIO;         /* 0x030 General Purpose Inputs and Outputs Register for the  */
    uint32_t RESERVED0;         /* Reserved, 0x034 */
    __IO uint32_t LPBK_DLY_ADJ; /* 0x038 Loopback Master Clock Delay Adjustment Register */
    uint32_t RESERVED1[17];     /* Reserved, 0x03C-0x07F */
    __O uint32_t TXD_01;        /* 0x080 Transmit Data Register. Keyhole addresses for the Tr */
    __O uint32_t TXD_10;        /* 0x084 Transmit Data Register. Keyhole addresses for the Tr */
    __O uint32_t TXD_11;        /* 0x088 Transmit Data Register. Keyhole addresses for the Tr */
    uint32_t RESERVED2[5];      /* Reserved, 0x08C-0x09F */
    __IO uint32_t LQSPI_CR;     /* 0x0A0 Configuration Register specifically for the Linear Q */
    __IO uint32_t LQSPI_SR;     /* 0x0A4 Status Register specifically for the Linear Quad-SPI */
    uint32_t RESERVED3[21];     /* Reserved, 0x0A8-0x0FB */
    __IO uint32_t MOD_ID;       /* 0x0FC Module Identification register */
} QSPI_Type;

typedef struct {                               /* SD/SDIO Controller (ARASAN) */
    __IO uint32_t ARGMT2_LO;                   /* 0x000 Argument2 Low Register */
    __IO uint32_t BLK_SIZE;                    /* 0x004 Block Size Register (mixed access) */
    __IO uint32_t ARGMT;                       /* 0x008 Argument Register */
    __IO uint32_t XFER_MODE;                   /* 0x00C Transfer Mode Register (mixed access) */
    __I uint32_t RESP0;                        /* 0x010 Response0 Register */
    __IO uint32_t RESP1;                       /* 0x014 Response1 Register */
    __IO uint32_t RESP2;                       /* 0x018 Response2 Register */
    __IO uint32_t RESP3;                       /* 0x01C Response3 Register */
    __IO uint32_t BUF_DAT_PORT;                /* 0x020 Buffer Data Port */
    __I uint32_t PRES_STATE;                   /* 0x024 Present State */
    __IO uint32_t HOST_CTRL1;                  /* 0x028 Host Control 1 */
    __IO uint32_t CLK_CTRL;                    /* 0x02C Clock Control */
    __IO uint32_t NORMAL_INTERRUPT_STAT;       /* 0x030  */
    __IO uint32_t NORM_INTR_STAT_EN;           /* 0x034 Normal/Error Interrupt Status Enable (16-bit regs at 0x34/0x36) */
    __IO uint32_t NORM_INTR_SIG_EN;            /* 0x038 Normal/Error Interrupt Signal Enable (16-bit regs at 0x38/0x3A) */
    __I uint32_t AUTO_CMD12_ERROR_STATUS;      /* 0x03C  */
    __I uint32_t CAPS;                         /* 0x040 Capabilities Register */
    uint32_t RESERVED1;                        /* Reserved, 0x044 */
    __I uint32_t MAXIMUM_CURRENT_CAPABILITIES; /* 0x048  */
    uint32_t RESERVED2[2];                     /* Reserved, 0x04C-0x053 */
    __IO uint32_t ADMA_ERROR_STATUS;           /* 0x054  (mixed access) */
    __IO uint32_t ADMA_SYSTEM_ADDRESS;         /* 0x058  */
    uint32_t RESERVED3;                        /* Reserved, 0x05C */
    __IO uint32_t PRE_VAL_1;                   /* 0x060 Preset Value Register */
    __O uint32_t PRE_VAL_2;                    /* 0x064 Preset Value Register */
    uint32_t RESERVED4[34];                    /* Reserved, 0x068-0x0EF */
    __IO uint32_t SPI_INTERRUPT_SUPPORT;       /* 0x0F0  */
} SDIO_Type;

typedef struct {              /* SPI Controller */
    __IO uint32_t CR;         /* 0x000 Configuration (mixed access) */
    __IO uint32_t SR;         /* 0x004 Interrupt Status (mixed access) */
    __IO uint32_t IER;        /* 0x008 Interrupt Enable (mixed access) */
    __IO uint32_t IDR;        /* 0x00C Interrupt Disable (mixed access) */
    __I uint32_t IMR;         /* 0x010 Interrupt Enabled Mask */
    __IO uint32_t ER;         /* 0x014 Enable/Disable Register (mixed access) */
    __IO uint32_t DR;         /* 0x018 Delay Register */
    __O uint32_t TXD;         /* 0x01C Data Transmit Register */
    __I uint32_t RXD;         /* 0x020 Data Receive Register */
    __IO uint32_t SICR;       /* 0x024 Slave Idle Count (mixed access) */
    __IO uint32_t TXWR;       /* 0x028 Transmit FIFO Watermark */
    __IO uint32_t RXWR;       /* 0x02C Receive FIFO Watermark */
    uint32_t RESERVED0[51];   /* Reserved, 0x030-0x0FB */
    __I uint32_t MOD_ID_REG0; /* 0x0FC  */
} SPI_Type;

typedef struct {           /* System Watchdog Timer */
    __IO uint32_t MODE;    /* 0x000  (mixed access) */
    __IO uint32_t CONTROL; /* 0x004  (mixed access) */
    __O uint32_t RESTART;  /* 0x008  */
    __I uint32_t STATUS;   /* 0x00C  */
} SWDT_Type;

typedef struct {                         /* Triple Timer Counter (3 x counters) */
    __IO uint32_t CLOCK_CONTROL_1;       /* 0x000 Clock Control Register */
    __IO uint32_t CLOCK_CONTROL_2;       /* 0x004  */
    __IO uint32_t CLOCK_CONTROL_3;       /* 0x008  */
    __IO uint32_t COUNTER_CONTROL_1;     /* 0x00C Counter Control Register */
    __IO uint32_t COUNTER_CONTROL_2;     /* 0x010  */
    __IO uint32_t COUNTER_CONTROL_3;     /* 0x014  */
    __I uint32_t COUNTER_VALUE_1;        /* 0x018 Current Counter Value */
    __I uint32_t COUNTER_VALUE_2;        /* 0x01C  */
    __I uint32_t COUNTER_VALUE_3;        /* 0x020  */
    __IO uint32_t INTERVAL_COUNTER_1;    /* 0x024 Interval Count Value */
    __IO uint32_t INTERVAL_COUNTER_2;    /* 0x028  */
    __IO uint32_t INTERVAL_COUNTER_3;    /* 0x02C  */
    __IO uint32_t MATCH_1_COUNTER_1;     /* 0x030 Match 1 value */
    __IO uint32_t MATCH_1_COUNTER_2;     /* 0x034  */
    __IO uint32_t MATCH_1_COUNTER_3;     /* 0x038  */
    __IO uint32_t MATCH_2_COUNTER_1;     /* 0x03C Match 2 value */
    __IO uint32_t MATCH_2_COUNTER_2;     /* 0x040  */
    __IO uint32_t MATCH_2_COUNTER_3;     /* 0x044  */
    __IO uint32_t MATCH_3_COUNTER_1;     /* 0x048 Match 3 value */
    __IO uint32_t MATCH_3_COUNTER_2;     /* 0x04C  */
    __IO uint32_t MATCH_3_COUNTER_3;     /* 0x050  */
    __IO uint32_t INTERRUPT_REGISTER_1;  /* 0x054 Interrupt Status Register (rc/w1c) */
    __IO uint32_t INTERRUPT_REGISTER_2;  /* 0x058  (rc/w1c) */
    __IO uint32_t INTERRUPT_REGISTER_3;  /* 0x05C  (rc/w1c) */
    __IO uint32_t INTERRUPT_ENABLE_1;    /* 0x060 Interrupt Enable Register */
    __IO uint32_t INTERRUPT_ENABLE_2;    /* 0x064  */
    __IO uint32_t INTERRUPT_ENABLE_3;    /* 0x068  */
    __IO uint32_t EVENT_CONTROL_TIMER_1; /* 0x06C  */
    __IO uint32_t EVENT_CONTROL_TIMER_2; /* 0x070  */
    __IO uint32_t EVENT_CONTROL_TIMER_3; /* 0x074  */
    __I uint32_t EVENT_REGISTER_1;       /* 0x078  */
    __I uint32_t EVENT_REGISTER_2;       /* 0x07C  */
    __I uint32_t EVENT_REGISTER_3;       /* 0x080  */
} TTC_Type;

typedef struct {           /* UART Controller (Cadence) */
    __IO uint32_t CR;      /* 0x000 Control Register [8:0] */
    __IO uint32_t MR;      /* 0x004 Mode Register [9:0] (mixed access) */
    __IO uint32_t IER;     /* 0x008 Interrupt Enable [12:0] (mixed access) */
    __IO uint32_t IDR;     /* 0x00C Interrupt Disable [12:0] (mixed access) */
    __I uint32_t IMR;      /* 0x010 Interrupt Mask [12:0] */
    __IO uint32_t ISR;     /* 0x014 Interrupt Status [12:0] */
    __IO uint32_t BAUDGEN; /* 0x018 Baud Rate Generator [15:0] (mixed access) */
    __IO uint32_t RXTOUT;  /* 0x01C RX Timeout [7:0] (mixed access) */
    __IO uint32_t RXWM;    /* 0x020 RX FIFO Trigger Level [5:0] (mixed access) */
    __IO uint32_t MODEMCR; /* 0x024 Modem Control [5:0] (mixed access) */
    __IO uint32_t MODEMSR; /* 0x028 Modem Status [8:0] (mixed access) */
    __I uint32_t SR;       /* 0x02C Channel Status [14:0] */
    __IO uint32_t FIFO;    /* 0x030 FIFO [7:0] (mixed access) */
    __IO uint32_t BAUDDIV; /* 0x034 Baud Rate Divider [7:0] (mixed access) */
    __IO uint32_t FLOWDEL; /* 0x038 Flow Delay [5:0] (mixed access) */
    uint32_t RESERVED0[2]; /* Reserved, 0x03C-0x043 */
    __IO uint32_t TXWM;    /* 0x044 TX FIFO Trigger Level [5:0] (mixed access) */
} UART_Type;

typedef struct {              /* CAN Controller */
    __IO uint32_t SRR;        /* 0x000 Software Reset Register */
    __IO uint32_t MSR;        /* 0x004 Mode Select Register */
    __IO uint32_t BRPR;       /* 0x008 Baud Rate Prescaler */
    __IO uint32_t BTR;        /* 0x00C Bit Timing Register */
    __I uint32_t ECR;         /* 0x010 Error Counter Register */
    __IO uint32_t ESR;        /* 0x014 Error Status Register (mixed access) */
    __IO uint32_t SR;         /* 0x018 Status Register (mixed access) */
    __IO uint32_t ISR;        /* 0x01C Interrupt Status Register (mixed access) */
    __IO uint32_t IER;        /* 0x020 Interrupt Enable Register */
    __IO uint32_t ICR;        /* 0x024 Interrupt Clear Register (mixed access) */
    __IO uint32_t TCR;        /* 0x028 Timestamp Control Register (mixed access) */
    __IO uint32_t WIR;        /* 0x02C Watermark Interrupt Reg */
    __O uint32_t TXFIFO_ID;   /* 0x030 TX FIFO ID */
    __IO uint32_t TXFIFO_DLC; /* 0x034 TX FIFO DLC */
    __IO uint32_t TXFIFO_DW1; /* 0x038 TX FIFO Data Word 1 */
    __IO uint32_t TXFIFO_DW2; /* 0x03C TX FIFO Data Word 2 */
    __O uint32_t TXHPB_ID;    /* 0x040 TX High Priority Buffer ID */
    __IO uint32_t TXHPB_DLC;  /* 0x044 TX High Priority Buffer DLC */
    __IO uint32_t TXHPB_DW1;  /* 0x048 TX High Priority Buf Data 1 */
    __IO uint32_t TXHPB_DW2;  /* 0x04C TX High Priority Buf Data Word 2 */
    __I uint32_t RXFIFO_ID;   /* 0x050 RX FIFO ID */
    __IO uint32_t RXFIFO_DLC; /* 0x054 RX FIFO DLC */
    __IO uint32_t RXFIFO_DW1; /* 0x058 RX FIFO Data Word 1 */
    __IO uint32_t RXFIFO_DW2; /* 0x05C RX FIFO Data Word 2 */
    __IO uint32_t AFR;        /* 0x060 Acceptance Filter Register */
    __IO uint32_t AFMR1;      /* 0x064 Acceptance Filter Mask 1 */
    __IO uint32_t AFIR1;      /* 0x068 Acceptance Filter ID 1 */
    __IO uint32_t AFMR2;      /* 0x06C Acceptance Filter Mask 2 */
    __IO uint32_t AFIR2;      /* 0x070 Acceptance Filter ID 2 */
    __IO uint32_t AFMR3;      /* 0x074 Acceptance Filter Mask 3 */
    __IO uint32_t AFIR3;      /* 0x078 Acceptance Filter ID 3 */
    __IO uint32_t AFMR4;      /* 0x07C Acceptance Filter Mask 4 */
    __IO uint32_t AFIR4;      /* 0x080 Acceptance Filter ID 4 */
} CAN_Type;

typedef struct {                                  /* USB 2.0 ULPI Controller */
    __I uint32_t ID;                              /* 0x000  */
    __IO uint32_t HWGENERAL;                      /* 0x004  */
    __I uint32_t HWHOST;                          /* 0x008  */
    __I uint32_t HWDEVICE;                        /* 0x00C  */
    __I uint32_t HWTXBUF;                         /* 0x010  */
    __I uint32_t HWRXBUF;                         /* 0x014  */
    uint32_t RESERVED0[26];                       /* Reserved, 0x018-0x07F */
    __IO uint32_t GPTIMER0LD;                     /* 0x080  */
    __IO uint32_t GPTIMER0CTRL;                   /* 0x084  (mixed access) */
    __IO uint32_t GPTIMER1LD;                     /* 0x088  */
    __IO uint32_t GPTIMER1CTRL;                   /* 0x08C  (mixed access) */
    __IO uint32_t SBUSCFG;                        /* 0x090  */
    uint32_t RESERVED1[27];                       /* Reserved, 0x094-0x0FF */
    __I uint32_t CAPLENGTH_HCIVERSION;            /* 0x100  */
    __I uint32_t HCSPARAMS;                       /* 0x104  */
    __I uint32_t HCCPARAMS;                       /* 0x108  */
    uint32_t RESERVED2[5];                        /* Reserved, 0x10C-0x11F */
    __I uint32_t DCIVERSION;                      /* 0x120  */
    __I uint32_t DCCPARAMS;                       /* 0x124  */
    uint32_t RESERVED3[6];                        /* Reserved, 0x128-0x13F */
    __IO uint32_t USBCMD;                         /* 0x140  (mixed access) */
    __IO uint32_t USBSTS;                         /* 0x144  (mixed access) */
    __IO uint32_t USBINTR;                        /* 0x148  (mixed access) */
    __IO uint32_t FRINDEX;                        /* 0x14C  */
    uint32_t RESERVED4;                           /* Reserved, 0x150 */
    __IO uint32_t PERIODICLISTBASE_DEVICEADDR;    /* 0x154  (mixed access) */
    __IO uint32_t ASYNCLISTADDR_ENDPOINTLISTADDR; /* 0x158  (mixed access) */
    __IO uint32_t TTCTRL;                         /* 0x15C  (mixed access) */
    __IO uint32_t BURSTSIZE;                      /* 0x160  */
    __IO uint32_t TXFILLTUNING;                   /* 0x164  (mixed access) */
    __IO uint32_t TXTTFILLTUNING;                 /* 0x168  (mixed access) */
    __IO uint32_t IC_USB;                         /* 0x16C  (mixed access) */
    __IO uint32_t ULPI_VIEWPORT;                  /* 0x170  (mixed access) */
    uint32_t RESERVED5;                           /* Reserved, 0x174 */
    __IO uint32_t ENDPTNAK;                       /* 0x178  */
    __IO uint32_t ENDPTNAKEN;                     /* 0x17C  */
    __I uint32_t CONFIGFLAG;                      /* 0x180  */
    __IO uint32_t PORTSC1;                        /* 0x184  (mixed access) */
    uint32_t RESERVED6[7];                        /* Reserved, 0x188-0x1A3 */
    __IO uint32_t OTGSC;                          /* 0x1A4  (mixed access) */
    __IO uint32_t USBMODE;                        /* 0x1A8  */
    __IO uint32_t ENDPTSETUPSTAT;                 /* 0x1AC  */
    __IO uint32_t ENDPTPRIME;                     /* 0x1B0  */
    __IO uint32_t ENDPTFLUSH;                     /* 0x1B4  */
    __IO uint32_t ENDPTSTAT;                      /* 0x1B8  */
    __IO uint32_t ENDPTCOMPLETE;                  /* 0x1BC  (rc/w1c) */
    __IO uint32_t ENDPTCTRL0;                     /* 0x1C0  (mixed access) */
    __IO uint32_t ENDPTCTRL1;                     /* 0x1C4  (mixed access) */
    __IO uint32_t ENDPTCTRL2;                     /* 0x1C8  */
    __IO uint32_t ENDPTCTRL3;                     /* 0x1CC  */
    __IO uint32_t ENDPTCTRL4;                     /* 0x1D0  */
    __IO uint32_t ENDPTCTRL5;                     /* 0x1D4  */
    __IO uint32_t ENDPTCTRL6;                     /* 0x1D8  */
    __IO uint32_t ENDPTCTRL7;                     /* 0x1DC  */
    __IO uint32_t ENDPTCTRL8;                     /* 0x1E0  */
    __IO uint32_t ENDPTCTRL9;                     /* 0x1E4  */
    __IO uint32_t ENDPTCTRL10;                    /* 0x1E8  */
    __IO uint32_t ENDPTCTRL11;                    /* 0x1EC  */
} USB_Type;

typedef struct {                         /* On-Chip Memory Control */
    __IO uint32_t OCM_PARITY_CTRL;       /* 0x000  (mixed access) */
    __IO uint32_t OCM_PARITY_ERRADDRESS; /* 0x004  (mixed access) */
    __IO uint32_t OCM_IRQ_STS;           /* 0x008  (mixed access) */
    __IO uint32_t OCM_CONTROL;           /* 0x00C  (mixed access) */
} OCM_Type;

typedef struct {                     /* Static Memory Controller (PL353 SMC) */
    __I uint32_t MEMC_STATUS;        /* 0x000  */
    __I uint32_t MEMIF_CFG;          /* 0x004  */
    __O uint32_t MEMC_CFG_SET;       /* 0x008  */
    __O uint32_t MEMC_CFG_CLR;       /* 0x00C  */
    __O uint32_t DIRECT_CMD;         /* 0x010  */
    __O uint32_t SET_CYCLES;         /* 0x014  */
    __IO uint32_t SET_OPMODE;        /* 0x018  (mixed access) */
    uint32_t RESERVED0;              /* Reserved, 0x01C */
    __IO uint32_t REFRESH_PERIOD_0;  /* 0x020  */
    __IO uint32_t REFRESH_PERIOD_1;  /* 0x024  */
    uint32_t RESERVED1[54];          /* Reserved, 0x028-0x0FF */
    __I uint32_t SRAM_CYCLES0_0;     /* 0x100  */
    __I uint32_t OPMODE0_0;          /* 0x104  */
    uint32_t RESERVED2[6];           /* Reserved, 0x108-0x11F */
    __I uint32_t SRAM_CYCLES0_1;     /* 0x120  */
    __I uint32_t OPMODE0_1;          /* 0x124  */
    uint32_t RESERVED3[22];          /* Reserved, 0x128-0x17F */
    __I uint32_t NAND_CYCLES1_0;     /* 0x180  */
    __I uint32_t OPMODE1_0;          /* 0x184  */
    uint32_t RESERVED4[30];          /* Reserved, 0x188-0x1FF */
    __I uint32_t USER_STATUS;        /* 0x200  */
    __O uint32_t USER_CONFIG;        /* 0x204  */
    uint32_t RESERVED5[126];         /* Reserved, 0x208-0x3FF */
    __I uint32_t ECC_STATUS_1;       /* 0x400  */
    __IO uint32_t ECC_MEMCFG_1;      /* 0x404  */
    __IO uint32_t ECC_MEMCOMMAND1_1; /* 0x408  */
    __IO uint32_t ECC_MEMCOMMAND2_1; /* 0x40C  */
    __I uint32_t ECC_ADDR0_1;        /* 0x410  */
    __IO uint32_t ECC_ADDR1_1;       /* 0x414  */
    __I uint32_t ECC_VALUE0_1;       /* 0x418  */
    __I uint32_t ECC_VALUE1_1;       /* 0x41C  */
    __I uint32_t ECC_VALUE2_1;       /* 0x420  */
    __I uint32_t ECC_VALUE3_1;       /* 0x424  */
} SMC_Type;

typedef struct {                         /* AXI High-Performance Port (AFI) */
    __IO uint32_t AFI_RDCHAN_CTRL;       /* 0x000  */
    __IO uint32_t AFI_RDCHAN_ISSUINGCAP; /* 0x004  */
    __IO uint32_t AFI_RDQOS;             /* 0x008  */
    __IO uint32_t AFI_RDDATAFIFO_LEVEL;  /* 0x00C  */
    __IO uint32_t AFI_RDDEBUG;           /* 0x010  */
    __IO uint32_t AFI_WRCHAN_CTRL;       /* 0x014 AXI HP0 write channel control */
    __IO uint32_t AFI_WRCHAN_ISSUINGCAP; /* 0x018 AXI HP0 write channel issuing capability */
    __IO uint32_t AFI_WRQOS;             /* 0x01C AXI HP0 write channel Quality of Service */
    __IO uint32_t AFI_WRDATAFIFO_LEVEL;  /* 0x020 AXI HP0 write data FIFO level */
    __IO uint32_t AFI_WRDEBUG;           /* 0x024  */
} AFI_Type;

/*
 * PS-XADC interface (UG585 ch. 30). Only these seven registers exist at
 * 0xF8007100-0xF8007118; everything above is reserved. The XADC DRP
 * registers (CFR0/SEQ/ATR/..., addressed by the XADC_* macros further
 * below) are NOT memory mapped here: they are reached by pushing DRP
 * addresses through CMDFIFO and reading results from RDFIFO.
 */
typedef struct {            /* XADC / ADC On-Chip (12-bit ADC) */
    __IO uint32_t CFG;      /* 0x000 Configuration Register */
    __IO uint32_t INT_STS;  /* 0x004 Interrupt Status Register */
    __IO uint32_t INT_MASK; /* 0x008 Interrupt Mask Register */
    __I uint32_t MSTS;      /* 0x00C Misc status register */
    __O uint32_t CMDFIFO;   /* 0x010 Command FIFO Register (wo) */
    __I uint32_t RDFIFO;    /* 0x014 Read FIFO Register (ro) */
    __IO uint32_t MCTL;     /* 0x018 Misc control register */
} XADC_Type;

typedef struct {                        /* Cortex-A9 MPCore Private Peripheral Block */
    __IO uint32_t SCU_CONTROL;          /* 0x000 SCU Control Register */
    __I uint32_t SCU_CONFIG;            /* 0x004 SCU Configuration Register */
    __IO uint32_t SCU_CPU_POWER_STATUS; /* 0x008 SCU CPU Power Status Register */
    __O uint32_t SCU_INVALIDATE;        /* 0x00C SCU Invalidate All Registers in Secure State */
    uint32_t RESERVED0[12];             /* Reserved, 0x010-0x03F */
    __IO uint32_t SCU_FILTERING_START;  /* 0x040 SCU Filtering Start Address Register */
    __IO uint32_t SCU_FILTERING_END;    /* 0x044 SCU Filtering End Address Register */
    uint32_t RESERVED1[2];              /* Reserved, 0x048-0x04F */
    __IO uint32_t SCU_ACCESS_CONTROL;   /* 0x050 SCU Access Control Register (SAC) */
    __IO uint32_t SCU_NONSECURE_ACCESS; /* 0x054 SCU Non-secure Access Control Register (SNSAC) */
    uint32_t RESERVED2[106];            /* Reserved, 0x058-0x1FF */
    __IO uint32_t GT_COUNTER_LO;        /* 0x200 Global Timer Counter Register (lower 32 bits) */
    __IO uint32_t GT_COUNTER_HI;        /* 0x204 Global Timer Counter Register (upper 32 bits) */
    __IO uint32_t GT_CONTROL;           /* 0x208 Global Timer Control Register */
    __I uint32_t GT_ISR;                /* 0x20C Global Timer Interrupt Status Register */
    __IO uint32_t GT_COMPARATOR_LO;     /* 0x210 Global Timer Comparator Value (lower) */
    __IO uint32_t GT_COMPARATOR_HI;     /* 0x214 Global Timer Comparator Value (upper) */
    __IO uint32_t GT_AUTO_INCREMENT;    /* 0x218 Global Timer Auto-increment Register */
    uint32_t RESERVED3[249];            /* Reserved, 0x21C-0x5FF */
    __IO uint32_t PTIM_LOAD;            /* 0x600 CPU Private Timer Load Register */
    __I uint32_t PTIM_COUNTER;          /* 0x604 CPU Private Timer Counter Register */
    __IO uint32_t PTIM_CONTROL;         /* 0x608 CPU Private Timer Control Register */
    __IO uint32_t PTIM_ISR;             /* 0x60C CPU Private Timer Interrupt Status Register */
    uint32_t RESERVED4[4];              /* Reserved, 0x610-0x61F */
    __IO uint32_t WD_LOAD;              /* 0x620 CPU Private Watchdog Load Register */
    __I uint32_t WD_COUNTER;            /* 0x624 CPU Private Watchdog Counter Register */
    __IO uint32_t WD_CONTROL;           /* 0x628 CPU Private Watchdog Control Register */
    __IO uint32_t WD_ISR;               /* 0x62C CPU Private Watchdog Interrupt Status Register */
} MPCORE_Type;

#define SLCR    ((SLCR_Type*)SLCR_BASE)
#define DDRC    ((DDRC_Type*)DDRC_BASE)
#define DEVCFG  ((DEVCFG_Type*)DEVCFG_BASE)
#define DMAC    ((DMAC_Type*)DMAC_BASE)
#define DMAC_NS ((DMAC_Type*)DMAC_NS_BASE)
#define GEM0    ((GEM_Type*)GEM0_BASE)
#define GEM1    ((GEM_Type*)GEM1_BASE)
#define GPIO    ((GPIO_Type*)GPIO_BASE)
#define I2C0    ((I2C_Type*)I2C0_BASE)
#define I2C1    ((I2C_Type*)I2C1_BASE)
#define QSPI    ((QSPI_Type*)QSPI_BASE)
#define SDIO0   ((SDIO_Type*)SDIO0_BASE)
#define SDIO1   ((SDIO_Type*)SDIO1_BASE)
#define SPI0    ((SPI_Type*)SPI0_BASE)
#define SPI1    ((SPI_Type*)SPI1_BASE)
#define SWDT    ((SWDT_Type*)SWDT_BASE)
#define TTC0    ((TTC_Type*)TTC0_BASE)
#define TTC1    ((TTC_Type*)TTC1_BASE)
#define UART0   ((UART_Type*)UART0_BASE)
#define UART1   ((UART_Type*)UART1_BASE)
#define CAN0    ((CAN_Type*)CAN0_BASE)
#define CAN1    ((CAN_Type*)CAN1_BASE)
#define USB0    ((USB_Type*)USB0_BASE)
#define USB1    ((USB_Type*)USB1_BASE)
#define OCM     ((OCM_Type*)OCM_BASE)
#define SMC     ((SMC_Type*)SMC_BASE)
#define AFI0    ((AFI_Type*)AFI0_BASE)
#define AFI1    ((AFI_Type*)AFI1_BASE)
#define AFI2    ((AFI_Type*)AFI2_BASE)
#define AFI3    ((AFI_Type*)AFI3_BASE)
#define MPCORE  ((MPCORE_Type*)MPCORE_BASE)

#define XADC ((XADC_Type*)XADC_BASE)

/*****************  SLCR keys  *****************/
#define SLCR_LOCK_KEY   (0x767BU) /* Key to lock SLCR writes   */
#define SLCR_UNLOCK_KEY (0xDF0DU) /* Key to unlock SLCR writes */

/*****************  SLCR MIO pin options (MIO_PIN_xx registers)  ***************/
#define MIO_PIN_OPT_DIS_RCVR       (1UL << 13) /* Disable receiver            */
#define MIO_PIN_OPT_ENA_PULLUP     (1UL << 12) /* Enable weak pull-up         */
#define MIO_PIN_OPT_IO_HSTL        (4UL << 9)  /* HSTL input/output (3.3V)    */
#define MIO_PIN_OPT_IO_CMOS_3V3    (3UL << 9)  /* LVCMOS 3.3V                 */
#define MIO_PIN_OPT_IO_CMOS_2V5    (2UL << 9)  /* LVCMOS 2.5V                 */
#define MIO_PIN_OPT_IO_CMOS_1V8    (1UL << 9)  /* LVCMOS 1.8V                 */
#define MIO_PIN_OPT_ENA_SPEED_FAST (1UL << 8)  /* Fast slew rate              */
#define MIO_PIN_OPT_LVL37_MUX      (7UL << 5)  /* L3 mux select MIO 7         */
#define MIO_PIN_OPT_LVL36_MUX      (6UL << 5)  /* L3 mux select MIO 6         */
#define MIO_PIN_OPT_LVL35_MUX      (5UL << 5)  /* L3 mux select MIO 5         */
#define MIO_PIN_OPT_LVL34_MUX      (4UL << 5)  /* L3 mux select MIO 4         */
#define MIO_PIN_OPT_LVL33_MUX      (3UL << 5)  /* L3 mux select MIO 3         */
#define MIO_PIN_OPT_LVL32_MUX      (2UL << 5)  /* L3 mux select MIO 2         */
#define MIO_PIN_OPT_LVL31_MUX      (1UL << 5)  /* L3 mux select MIO 1         */
#define MIO_PIN_OPT_LVL30_MUX      (0UL << 5)  /* L3 mux select MIO 0         */
#define MIO_PIN_OPT_LVL23_MUX      (3UL << 3)  /* L2 mux select MIO 3         */
#define MIO_PIN_OPT_LVL22_MUX      (2UL << 3)  /* L2 mux select MIO 2         */
#define MIO_PIN_OPT_LVL21_MUX      (1UL << 3)  /* L2 mux select MIO 1         */
#define MIO_PIN_OPT_LVL20_MUX      (0UL << 3)  /* L2 mux select MIO 0         */
#define MIO_PIN_OPT_LVL12_MUX      (1UL << 2)  /* L1 mux select MIO 2         */
#define MIO_PIN_OPT_LVL11_MUX      (1UL << 2)  /* L1 mux select MIO 1         */
#define MIO_PIN_OPT_LVL10_MUX      (0UL << 2)  /* L1 mux select MIO 0         */
#define MIO_PIN_OPT_LVL01_MUX      (1UL << 1)  /* L0 mux select MIO 1         */
#define MIO_PIN_OPT_LVL00_MUX      (0UL << 1)  /* L0 mux select MIO 0         */

/*****************  Bit definitions for UART  *****************/
#define UART_CR_RXRST      (0x1UL)  /* RX logic reset */
#define UART_CR_TXRST      (0x2UL)  /* TX logic reset */
#define UART_CR_RX_EN      (0x4UL)  /* RX enabled */
#define UART_CR_RX_DIS     (0x8UL)  /* RX disabled. */
#define UART_CR_TX_EN      (0x10UL) /* TX enabled */
#define UART_CR_TX_DIS     (0x20UL) /* TX disabled. */
#define UART_CR_EN_DIS_Pos (2U)
#define UART_CR_EN_DIS_Msk (0x3CUL)  /* Enable/disable Mask */
#define UART_CR_TORST      (0x40UL)  /* RX timeout counter restart */
#define UART_CR_STARTBRK   (0x80UL)  /* Set break */
#define UART_CR_STOPBRK    (0x100UL) /* Stop transmission of break */

#define UART_MR_CHARLEN_8_BIT    (0x0UL) /* 8 bits data */
#define UART_MR_CHMODE_NORM      (0x0UL) /* Normal mode */
#define UART_MR_PARITY_EVEN      (0x0UL) /* Even parity mode */
#define UART_MR_STOPMODE_1_BIT   (0x0UL) /* 1 stop bit */
#define UART_MR_CLKSEL           (0x1UL) /* Input clock selection */
#define UART_MR_CHARLEN_7_BIT    (0x4UL) /* 7 bits data */
#define UART_MR_CHARLEN_6_BIT    (0x6UL) /* 6 bits data */
#define UART_MR_CHARLEN_Pos      (1U)
#define UART_MR_CHARLEN_Msk      (0x6UL)  /* Data length mask */
#define UART_MR_PARITY_ODD       (0x8UL)  /* Odd parity mode */
#define UART_MR_PARITY_SPACE     (0x10UL) /* Space parity mode */
#define UART_MR_PARITY_MARK      (0x18UL) /* Mark parity mode */
#define UART_MR_PARITY_NONE      (0x20UL) /* No parity mode */
#define UART_MR_PARITY_Pos       (3U)
#define UART_MR_PARITY_Msk       (0x38UL)  /* Parity mask */
#define UART_MR_STOPMODE_1_5_BIT (0x40UL)  /* 1.5 stop bits */
#define UART_MR_STOPMODE_2_BIT   (0x80UL)  /* 2 stop bits */
#define UART_MR_STOPMODE_Msk     (0xC0UL)  /* NBSTOP field mask, bits 7:6 */
#define UART_MR_CHMODE_ECHO      (0x100UL) /* Auto echo mode */
#define UART_MR_CHMODE_L_LOOP    (0x200UL) /* Local loopback mode */
#define UART_MR_CHMODE_Pos       (8U)
#define UART_MR_CHMODE_Msk       (0x300UL) /* Mode mask */
#define UART_MR_CHMODE_R_LOOP    (0x300UL) /* Remote loopback mode */
#define UART_MR_CCLK             (0x400UL) /* Input clock selection */

#define UART_IXR_RXOVR    (0x1UL)    /* RX FIFO trigger interrupt. */
#define UART_IXR_RXEMPTY  (0x2UL)    /* RX FIFO empty interrupt. */
#define UART_IXR_RXFULL   (0x4UL)    /* RX FIFO full interrupt. */
#define UART_IXR_TXEMPTY  (0x8UL)    /* TX FIFO empty interrupt. */
#define UART_IXR_TXFULL   (0x10UL)   /* TX FIFO full interrupt. */
#define UART_IXR_OVER     (0x20UL)   /* Overrun error interrupt */
#define UART_IXR_FRAMING  (0x40UL)   /* Framing error interrupt */
#define UART_IXR_PARITY   (0x80UL)   /* Parity error interrupt */
#define UART_IXR_TOUT     (0x100UL)  /* Timeout error interrupt */
#define UART_IXR_DMS      (0x200UL)  /* Modem status change interrupt */
#define UART_IXR_TTRIG    (0x400UL)  /* Tx Trig interrupt */
#define UART_IXR_TNFUL    (0x800UL)  /* Tx FIFO Nearly Full interrupt */
#define UART_IXR_TOVR     (0x1000UL) /* Tx FIFO Overflow interrupt */
#define UART_IXR_RBRK     (0x2000UL) /* Rx FIFO break detect interrupt */
#define UART_IXR_MASK_Pos (0U)
#define UART_IXR_MASK_Msk (0x3FFFUL) /* Valid bit mask */

#define UART_BAUDGEN_DISABLE   (0x0UL)   /* Disable clock */
#define UART_BAUDGEN_RESET_VAL (0x28BUL) /* Reset value */
#define UART_BAUDGEN_MASK_Pos  (0U)
#define UART_BAUDGEN_MASK_Msk  (0xFFFFUL) /* Valid bits mask */

#define UART_BAUDDIV_RESET_VAL (0xFUL) /* Reset value */
#define UART_BAUDDIV_MASK_Pos  (0U)
#define UART_BAUDDIV_MASK_Msk  (0xFFUL) /* 8 bit baud divider mask */

#define UART_RXTOUT_DISABLE  (0x0UL) /* Disable time out */
#define UART_RXTOUT_MASK_Pos (0U)
#define UART_RXTOUT_MASK_Msk (0xFFUL) /* Valid bits mask */

#define UART_RXWM_DISABLE   (0x0UL)  /* Disable RX trigger interrupt */
#define UART_RXWM_RESET_VAL (0x20UL) /* Reset value */
#define UART_RXWM_MASK_Pos  (0U)
#define UART_RXWM_MASK_Msk  (0x3FUL) /* Valid bits mask */

#define UART_TXWM_RESET_VAL (0x20UL) /* Reset value */
#define UART_TXWM_MASK_Pos  (0U)
#define UART_TXWM_MASK_Msk  (0x3FUL) /* Valid bits mask */

#define UART_MODEMCR_DTR (0x1UL)  /* Data terminal ready */
#define UART_MODEMCR_RTS (0x2UL)  /* Request to send */
#define UART_MODEMCR_FCM (0x20UL) /* Flow control mode */

#define UART_MODEMSR_DCTS (0x1UL)   /* Change of CTS */
#define UART_MODEMSR_DDSR (0x2UL)   /* Change of DSR */
#define UART_MODEMSR_TERI (0x4UL)   /* Trailing Edge Ring Indicator */
#define UART_MODEMSR_DDCD (0x8UL)   /* Delta DCD indicator */
#define UART_MODEMSR_CTS  (0x10UL)  /* Complement of CTS input */
#define UART_MODEMSR_DSR  (0x20UL)  /* Complement of DSR input */
#define UART_MODEMSR_RI   (0x40UL)  /* Complement of RI input */
#define UART_MODEMSR_DCD  (0x80UL)  /* Complement of DCD input */
#define UART_MODEMSR_FCMS (0x100UL) /* Flow control mode (FCMS) */

#define UART_SR_RXOVR   (0x1UL)    /* RX FIFO fill over trigger */
#define UART_SR_RXEMPTY (0x2UL)    /* RX FIFO empty */
#define UART_SR_RXFULL  (0x4UL)    /* RX FIFO full */
#define UART_SR_TXEMPTY (0x8UL)    /* TX FIFO empty */
#define UART_SR_TXFULL  (0x10UL)   /* TX FIFO full */
#define UART_SR_RACTIVE (0x400UL)  /* RX active */
#define UART_SR_TACTIVE (0x800UL)  /* TX active */
#define UART_SR_FLOWDEL (0x1000UL) /* RX FIFO fill over flow delay */
#define UART_SR_TTRIG   (0x2000UL) /* TX FIFO Trigger Status */
#define UART_SR_TNFUL   (0x4000UL) /* TX FIFO Nearly Full Status */

#define UART_RXBS_BYTE0_PARE (0x1UL) /* Byte0 Parity Error */
#define UART_RXBS_BYTE0_FRME (0x2UL) /* Byte0 Frame Error */
#define UART_RXBS_BYTE0_BRKE (0x4UL) /* Byte0 Break Error */
#define UART_RXBS_MASK_Pos   (0U)
#define UART_RXBS_MASK_Msk   (0x7UL)   /* 3 bit RX byte status mask */
#define UART_RXBS_BYTE1_PARE (0x8UL)   /* Byte1 Parity Error */
#define UART_RXBS_BYTE1_FRME (0x10UL)  /* Byte1 Frame Error */
#define UART_RXBS_BYTE1_BRKE (0x20UL)  /* Byte1 Break Error */
#define UART_RXBS_BYTE2_PARE (0x40UL)  /* Byte2 Parity Error */
#define UART_RXBS_BYTE2_FRME (0x80UL)  /* Byte2 Frame Error */
#define UART_RXBS_BYTE2_BRKE (0x100UL) /* Byte2 Break Error */
#define UART_RXBS_BYTE3_PARE (0x200UL) /* Byte3 Parity Error */
#define UART_RXBS_BYTE3_FRME (0x400UL) /* Byte3 Frame Error */
#define UART_RXBS_BYTE3_BRKE (0x800UL) /* Byte3 Break Error */

/*****************  Bit definitions for GPIO  *****************/
#define GPIO_INTTYPE_BANK1_RESET (0x3FFFFFUL)   /* Resets specific to Zynq */
#define GPIO_INTTYPE_BANK0_RESET (0xFFFFFFFFUL) /* Resets specific to Zynq */
#define GPIO_INTTYPE_BANK2_RESET (0xFFFFFFFFUL) /* Resets specific to Zynq */
#define GPIO_INTTYPE_BANK3_RESET (0xFFFFFFFFUL) /* Reset common to both platforms */
#define GPIO_INTTYPE_BANK4_RESET (0xFFFFFFFFUL) /* Resets specific to Zynq Ultrascale+ MP */
#define GPIO_INTTYPE_BANK5_RESET (0xFFFFFFFFUL) /* Resets specific to Zynq Ultrascale+ MP */

#define GPIO_PS_GPIO_BASEADDR (0xFF0B0000UL) /* Flag for Base Address for PS_GPIO in Versal */

#define GPIO_ZERO (0x0UL) /* Flag for 0 Value */

#define GPIO_ONE (0x1UL) /* Flag for 1 Value */

#define GPIO_TWO (0x2UL) /* Flag for 2 Value */

#define GPIO_THREE (0x3UL) /* Flag for 3 Value */

#define GPIO_FOUR (0x4UL) /* Flag for 4 Value */

#define GPIO_SIX (0x6UL) /* Flag for 6 Value */

/*****************  Bit definitions for QSPI  *****************/
#define QSPI_CR_MSTREN_Pos    (0U)
#define QSPI_CR_MSTREN_Msk    (0x1UL) /* Master Mode Enable */
#define QSPI_CR_CPOL_Pos      (1U)
#define QSPI_CR_CPOL_Msk      (0x2UL) /* Polarity Configuration */
#define QSPI_CR_CPHA_Pos      (2U)
#define QSPI_CR_CPHA_Msk      (0x4UL) /* Phase Configuration */
#define QSPI_CR_PRESC_MAXIMUM (0x7UL) /* Prescaler maximum value */
#define QSPI_CR_PRESC_Pos     (3U)
#define QSPI_CR_PRESC_Msk     (0x38UL) /* Prescaler Setting */
#define QSPI_CR_REF_CLK_Pos   (8U)
#define QSPI_CR_REF_CLK_Msk   (0x100UL) /* Ref clk bit - should be 0 */
#define QSPI_CR_SSCTRL_Pos    (10U)
#define QSPI_CR_SSCTRL_Msk    (0x400UL) /* Slave Select Decode */
#define QSPI_CR_SSFORCE_Pos   (14U)
#define QSPI_CR_SSFORCE_Msk   (0x4000UL) /* Force Slave Select */
#define QSPI_CR_MANSTRT_Pos   (16U)
#define QSPI_CR_MANSTRT_Msk   (0x10000UL) /* Manual Transmission Start */
#define QSPI_CR_HOLD_B_Pos    (19U)
#define QSPI_CR_HOLD_B_Msk    (0x80000UL) /* HOLD_B Pin Drive Enable */
#define QSPI_CR_ENDIAN_Pos    (26U)
#define QSPI_CR_ENDIAN_Msk    (0x4000000UL) /* Tx/Rx FIFO endianness */
#define QSPI_CR_IFMODE_Pos    (31U)
#define QSPI_CR_IFMODE_Msk    (0x80000000UL) /* Flash mem interface mode */

#define QSPI_IXR_RXOVR_Pos    (0U)
#define QSPI_IXR_RXOVR_Msk    (0x1UL) /* QSPI Rx FIFO Overrun */
#define QSPI_IXR_TXOW_Pos     (2U)
#define QSPI_IXR_TXOW_Msk     (0x4UL) /* QSPI Tx FIFO Overwater */
#define QSPI_IXR_TXFULL_Pos   (3U)
#define QSPI_IXR_TXFULL_Msk   (0x8UL) /* QSPI Tx FIFO Full */
#define QSPI_IXR_RXNEMPTY_Pos (4U)
#define QSPI_IXR_RXNEMPTY_Msk (0x10UL) /* QSPI Rx FIFO Not Empty */
#define QSPI_IXR_RXFULL_Pos   (5U)
#define QSPI_IXR_RXFULL_Msk   (0x20UL) /* QSPI Rx FIFO Full */
#define QSPI_IXR_TXUF_Pos     (6U)
#define QSPI_IXR_TXUF_Msk     (0x40UL) /* QSPI Tx FIFO Underflow */
#define QSPI_IXR_DISABLE_ALL  (0x7DUL) /* Disable all interrupts */

#define QSPI_ISR_RESET_STATE (0x4UL) /* Default to tx/rx empty */

#define QSPI_ER_ENABLE_Pos (0U)
#define QSPI_ER_ENABLE_Msk (0x1UL) /* QSPI Enable Bit Mask */

#define QSPI_DR_INIT_Pos  (0U)
#define QSPI_DR_INIT_Msk  (0xFFUL) /* Delay Initially mask */
#define QSPI_DR_AFTER_Pos (8U)
#define QSPI_DR_AFTER_Msk (0xFF00UL) /* Delay After Transfers mask */

#define QSPI_SICR_MASK_Pos (0U)
#define QSPI_SICR_MASK_Msk (0xFFUL) /* Slave Idle Count Mask */

#define QSPI_TXWR_MASK_Pos (0U)
#define QSPI_TXWR_MASK_Msk (0x3FUL) /* Transmit Watermark Mask */

#define QSPI_RXWR_MASK_Pos (0U)
#define QSPI_RXWR_MASK_Msk (0x3FUL) /* Receive Watermark Mask */

#define QSPI_FIFO_DEPTH (0x3FUL) /* FIFO depth (words) */

#define QSPI_LQSPI_CR_INST_Pos    (0U)
#define QSPI_LQSPI_CR_INST_Msk    (0xFFUL) /* Read instr code */
#define QSPI_LQSPI_CR_MODE_ON_Pos (24U)
#define QSPI_LQSPI_CR_MODE_ON_Msk (0x1000000UL) /* Mode on */
#define QSPI_LQSPI_CR_MODE_EN_Pos (25U)
#define QSPI_LQSPI_CR_MODE_EN_Msk (0x2000000UL) /* Enable mode bits */
#define QSPI_LQSPI_CR_U_PAGE_Pos  (28U)
#define QSPI_LQSPI_CR_U_PAGE_Msk  (0x10000000UL) /* Upper memory page */
#define QSPI_LQSPI_CR_SEP_BUS_Pos (29U)
#define QSPI_LQSPI_CR_SEP_BUS_Msk (0x20000000UL) /* Separate memory bus */
#define QSPI_LQSPI_CR_TWO_MEM_Pos (30U)
#define QSPI_LQSPI_CR_TWO_MEM_Msk (0x40000000UL) /* Both memories or one */
#define QSPI_LQSPI_CR_LINEAR_Pos  (31U)
#define QSPI_LQSPI_CR_LINEAR_Msk  (0x80000000UL) /* LQSPI mode enable */
#define QSPI_LQSPI_CR_RST_STATE   (0x8000016BUL) /* Default CR value */

#define QSPI_LPBK_DLY_ADJ_USE_LPBK_Pos (5U)
#define QSPI_LPBK_DLY_ADJ_USE_LPBK_Msk (0x20UL) /* Loopback Bit */

#define QSPI_DUMMY_TX_DATA (0xFFFFFFFFUL) /*  */

/*****************  Bit definitions for I2C  *****************/
#define I2C_CR_SLVMON_Pos   (5U)
#define I2C_CR_SLVMON_Msk   (0x20UL) /* Slave monitor mode */
#define I2C_CR_CLR_FIFO_Pos (6U)
#define I2C_CR_CLR_FIFO_Msk (0x40UL) /* Clear FIFO, auto clears */
#define I2C_CR_DIV_B_Pos    (8U)
#define I2C_CR_DIV_B_Msk    (0x3F00UL) /* Clock Divisor B */
#define I2C_CR_DIV_A_Pos    (14U)
#define I2C_CR_DIV_A_Msk    (0xC000UL) /* Clock Divisor A */

#define I2C_DIV_A_MAX (0x4UL) /* Maximum value of Divisor A */

#define I2C_SR_RXRW_Pos  (3U)
#define I2C_SR_RXRW_Msk  (0x8UL) /* Receive read/write Mask */
#define I2C_SR_RXDV_Pos  (5U)
#define I2C_SR_RXDV_Msk  (0x20UL) /* Receiver Data Valid Mask */
#define I2C_SR_TXDV_Pos  (6U)
#define I2C_SR_TXDV_Msk  (0x40UL) /* Transmit Data Valid Mask */
#define I2C_SR_RXOVF_Pos (7U)
#define I2C_SR_RXOVF_Msk (0x80UL) /* Receiver Overflow Mask */
#define I2C_SR_BA_Pos    (8U)
#define I2C_SR_BA_Msk    (0x100UL) /* Bus Active Mask */

#define I2C_ADDR_MASK_Pos (0U)
#define I2C_ADDR_MASK_Msk (0x3FFUL) /* IIC Address Mask */

#define I2C_DATA_INTR_DEPTH (0xEUL) /* Number of bytes at DATA intr */
#define I2C_DATA_MASK_Pos   (0U)
#define I2C_DATA_MASK_Msk   (0xFFUL) /* IIC Data Mask */

#define I2C_IXR_DATA_Pos     (1U)
#define I2C_IXR_DATA_Msk     (0x2UL) /* Data Interrupt mask */
#define I2C_IXR_NACK_Pos     (2U)
#define I2C_IXR_NACK_Msk     (0x4UL)   /* NACK Interrupt mask */
#define I2C_IXR_ALL_INTR_Msk (0x2FFUL) /* All ISR Mask (non-contiguous) */
#define I2C_IXR_DEFAULT_Msk  (0x2FFUL) /* Default ISR Mask (non-contiguous) */

#define I2C_TRANS_SIZE_Pos (0U)
#define I2C_TRANS_SIZE_Msk (0x3FUL) /* IIC Transfer Size Mask */

#define I2C_FIFO_DEPTH (0x10UL) /* Number of bytes in the FIFO */

#define I2C_SLV_PAUSE_Pos (0U)
#define I2C_SLV_PAUSE_Msk (0xFUL) /* Slave monitor pause mask */

#define I2C_TIME_OUT_Pos (0U)
#define I2C_TIME_OUT_Msk (0xFFUL) /* IIC Time Out mask */

#define I2C_TO_RESET_VALUE (0xFFUL) /* IIC Time Out reset value */

#define I2C_POLL_DEFAULT_TIMEOUT_VAL (0x3E8UL) /* Timeout in us */

/*****************  Bit definitions for SPI  *****************/
#define SPI_CR_MSTREN_Pos     (0U)
#define SPI_CR_MSTREN_Msk     (0x1UL) /* Master Mode Enable */
#define SPI_CR_CPOL_Pos       (1U)
#define SPI_CR_CPOL_Msk       (0x2UL) /* Polarity Configuration */
#define SPI_CR_CPHA_Pos       (2U)
#define SPI_CR_CPHA_Msk       (0x4UL) /* Phase Configuration */
#define SPI_CR_PRESC_MAXIMUM  (0x7UL) /* Prescaler maximum value */
#define SPI_CR_SSCTRL_MAXIMUM (0xFUL) /* Slave Select maximum value */
#define SPI_CR_PRESC_Pos      (3U)
#define SPI_CR_PRESC_Msk      (0x38UL) /* Prescaler Setting */
#define SPI_CR_SSDECEN_Pos    (9U)
#define SPI_CR_SSDECEN_Msk    (0x200UL) /* Slave Select Decode Enable */
#define SPI_CR_SSCTRL_Pos     (10U)
#define SPI_CR_SSCTRL_Msk     (0x3C00UL) /* Slave Select Decode */
#define SPI_CR_SSFORCE_Pos    (14U)
#define SPI_CR_SSFORCE_Msk    (0x4000UL) /* Force Slave Select */
#define SPI_CR_MANSTRT_Pos    (16U)
#define SPI_CR_MANSTRT_Msk    (0x10000UL) /* Manual Transmission Start */
#define SPI_CR_RESET_STATE    (0x20000UL) /* Mode Fail Generation Enable */

#define SPI_IXR_RXOVR_Pos    (0U)
#define SPI_IXR_RXOVR_Msk    (0x1UL) /* Rx FIFO Overrun */
#define SPI_IXR_MODF_Pos     (1U)
#define SPI_IXR_MODF_Msk     (0x2UL) /* Mode Fault */
#define SPI_IXR_TXOW_Pos     (2U)
#define SPI_IXR_TXOW_Msk     (0x4UL) /* Tx FIFO Overwater */
#define SPI_IXR_TXFULL_Pos   (3U)
#define SPI_IXR_TXFULL_Msk   (0x8UL) /* Tx FIFO Full */
#define SPI_IXR_RXNEMPTY_Pos (4U)
#define SPI_IXR_RXNEMPTY_Msk (0x10UL) /* Rx FIFO Not Empty */
#define SPI_IXR_RXFULL_Pos   (5U)
#define SPI_IXR_RXFULL_Msk   (0x20UL) /* Rx FIFO Full */
#define SPI_IXR_TXUF_Pos     (6U)
#define SPI_IXR_TXUF_Msk     (0x40UL) /* Tx FIFO Underflow */

#define SPI_ER_ENABLE_Pos (0U)
#define SPI_ER_ENABLE_Msk (0x1UL) /* SPI Enable Bit Mask */

#define SPI_DR_INIT_Pos  (0U)
#define SPI_DR_INIT_Msk  (0xFFUL) /* Delay Initially mask */
#define SPI_DR_AFTER_Pos (8U)
#define SPI_DR_AFTER_Msk (0xFF00UL) /* Delay After Transfers mask */
#define SPI_DR_BTWN_Pos  (16U)
#define SPI_DR_BTWN_Msk  (0xFF0000UL) /* Delay Between Transfers	mask */

#define SPI_SICR_MASK_Pos (0U)
#define SPI_SICR_MASK_Msk (0xFFUL) /* Slave Idle Count Mask */

#define SPI_TXWR_MASK_Pos (0U)
#define SPI_TXWR_MASK_Msk (0x7FUL) /* Transmit Watermark Mask */

#define SPI_RXWR_MASK_Pos (0U)
#define SPI_RXWR_MASK_Msk (0x7FUL) /* Receive Watermark Mask */

#define SPI_FIFO_DEPTH (0x80UL) /* FIFO depth of Tx and Rx */

/*****************  Bit definitions for GEM  *****************/
#define GEM_MAX_TYPE_ID   (0x4UL)  /* Maxmum number of type id supported */
#define GEM_MAX_HASH_BITS (0x40UL) /* Maximum value for hash bits. 2**6 */
#define GEM_MAX_RXBD      (0x80UL) /* Size of RX buffer descriptor queues */
#define GEM_MAX_TXBD      (0x80UL) /* Size of TX buffer descriptor queues */

#define GEM_SEND (0x1UL) /* send direction */

#define GEM_RECV (0x2UL) /* receive direction */

#define GEM_RX_SCR_BYPASS_SIZE (0x1UL)    /*  */
#define GEM_RX_SYNC_RESET_SIZE (0x1UL)    /*  */
#define GEM_RX_BUF_SIZE_JUMBO  (0x2800UL) /*  */

#define GEM_SCREEN_TYPE2_REG0 (0x540UL) /* Screening Type2 Reg0 * */

#define GEM_ENABLE_HS_MAC_SIZE (0x1UL) /*  */

#define GEM_NWCTRL_LOOPEN_Pos  (1U)
#define GEM_NWCTRL_LOOPEN_Msk  (0x2UL) /* local loopback */
#define GEM_NWCTRL_RXEN_Pos    (2U)
#define GEM_NWCTRL_RXEN_Msk    (0x4UL) /* Enable receive */
#define GEM_NWCTRL_TXEN_Pos    (3U)
#define GEM_NWCTRL_TXEN_Msk    (0x8UL) /* Enable transmit */
#define GEM_NWCTRL_MDEN_Pos    (4U)
#define GEM_NWCTRL_MDEN_Msk    (0x10UL) /* Enable MDIO port */
#define GEM_NWCTRL_STARTTX_Pos (9U)
#define GEM_NWCTRL_STARTTX_Msk (0x200UL) /* Start tx (tx_go) */
#define GEM_NWCTRL_PAUSETX_Pos (11U)
#define GEM_NWCTRL_PAUSETX_Msk (0x800UL) /* Transmit pause frame */

#define GEM_NWCFG_100_Pos         (0U)
#define GEM_NWCFG_100_Msk         (0x1UL) /* 100 Mbps */
#define GEM_NWCFG_FDEN_Pos        (1U)
#define GEM_NWCFG_FDEN_Msk        (0x2UL) /* full duplex */
#define GEM_NWCFG_JUMBO_Pos       (3U)
#define GEM_NWCFG_JUMBO_Msk       (0x8UL) /* Jumbo frames */
#define GEM_NWCFG_COPYALLEN_Pos   (4U)
#define GEM_NWCFG_COPYALLEN_Msk   (0x10UL) /* Copy all frames */
#define GEM_NWCFG_MDC_SHIFT_Msk   (0x12UL) /* shift bits for MDC (non-contiguous) */
#define GEM_NWCFG_XTADDMACHEN_Pos (9U)
#define GEM_NWCFG_XTADDMACHEN_Msk (0x200UL) /*  */
#define GEM_NWCFG_1000_Pos        (10U)
#define GEM_NWCFG_1000_Msk        (0x400UL) /* 1000 Mbps */
#define GEM_NWCFG_PCSSEL_Pos      (11U)
#define GEM_NWCFG_PCSSEL_Msk      (0x800UL) /* PCS Select */
#define GEM_NWCFG_RETRYTESTEN_Pos (12U)
#define GEM_NWCFG_RETRYTESTEN_Msk (0x1000UL) /* Retry test */
#define GEM_NWCFG_PAUSEEN_Pos     (13U)
#define GEM_NWCFG_PAUSEEN_Msk     (0x2000UL) /* Enable pause RX */
#define GEM_NWCFG_RXOFFS_Pos      (14U)
#define GEM_NWCFG_RXOFFS_Msk      (0xC000UL) /* RX buffer offset */
#define GEM_NWCFG_LENERRDSCRD_Pos (16U)
#define GEM_NWCFG_LENERRDSCRD_Msk (0x10000UL) /*  */
#define GEM_NWCFG_RESET_Pos       (19U)
#define GEM_NWCFG_RESET_Msk       (0x80000UL) /* reset value */
#define GEM_NWCFG_MDCCLKDIV_Pos   (18U)
#define GEM_NWCFG_MDCCLKDIV_Msk   (0x1C0000UL) /* MDC Mask PCLK divisor */
#define GEM_NWCFG_DWIDTH_64_Pos   (21U)
#define GEM_NWCFG_DWIDTH_64_Msk   (0x200000UL) /* 64 bit Data bus width */
#define GEM_NWCFG_DWIDTH_128_Pos  (22U)
#define GEM_NWCFG_DWIDTH_128_Msk  (0x400000UL) /* 128 bit Data bus width */
#define GEM_NWCFG_HDRXEN_Pos      (25U)
#define GEM_NWCFG_HDRXEN_Msk      (0x2000000UL) /* RX half duplex */
#define GEM_NWCFG_SGMIIEN_Pos     (27U)
#define GEM_NWCFG_SGMIIEN_Msk     (0x8000000UL) /* SGMII Enable */
#define GEM_NWCFG_IPDSTRETCH_Pos  (28U)
#define GEM_NWCFG_IPDSTRETCH_Msk  (0x10000000UL) /* enable transmit IPG */

#define GEM_NWSR_MDIO_Pos     (1U)
#define GEM_NWSR_MDIO_Msk     (0x2UL) /* Status of mdio_in */
#define GEM_NWSR_MDIOIDLE_Pos (2U)
#define GEM_NWSR_MDIOIDLE_Msk (0x4UL) /* PHY management idle */

#define GEM_DMACR_SINGLE_AHB_BURST (0x1UL)  /* single AHB bursts */
#define GEM_DMACR_INCR4_AHB_BURST  (0x4UL)  /* 4 bytes AHB bursts */
#define GEM_DMACR_INCR8_AHB_BURST  (0x8UL)  /* 8 bytes AHB bursts */
#define GEM_DMACR_INCR16_AHB_BURST (0x10UL) /* 16 bytes AHB bursts */
#define GEM_DMACR_BLENGTH_Pos      (0U)
#define GEM_DMACR_BLENGTH_Msk      (0x1FUL) /* buffer burst length */
#define GEM_DMACR_ENDIAN_Pos       (7U)
#define GEM_DMACR_ENDIAN_Msk       (0x80UL) /* endian configuration */
#define GEM_DMACR_RXSIZE_Pos       (8U)
#define GEM_DMACR_RXSIZE_Msk       (0x300UL) /* RX buffer memory size */
#define GEM_DMACR_TXSIZE_Pos       (10U)
#define GEM_DMACR_TXSIZE_Msk       (0x400UL) /* TX buffer memory size */
#define GEM_DMACR_RXEXTEND_Pos     (28U)
#define GEM_DMACR_RXEXTEND_Msk     (0x10000000UL) /* Rx Extended desc mode */
#define GEM_DMACR_TXEXTEND_Pos     (29U)
#define GEM_DMACR_TXEXTEND_Msk     (0x20000000UL) /* Tx Extended desc mode */
#define GEM_DMACR_ADDR_WIDTH_64    (0x40000000UL) /* 64 bit address bus */

#define GEM_TXSR_USEDREAD_Pos (0U)
#define GEM_TXSR_USEDREAD_Msk (0x1UL) /* TX buffer used bit set */
#define GEM_TXSR_FRAMERX_Pos  (1U)
#define GEM_TXSR_FRAMERX_Msk  (0x2UL) /* Collision tx frame */
#define GEM_TXSR_RXOVR_Pos    (2U)
#define GEM_TXSR_RXOVR_Msk    (0x4UL) /* Retry limit exceeded */
#define GEM_TXSR_TXGO_Pos     (3U)
#define GEM_TXSR_TXGO_Msk     (0x8UL) /* Status of go flag */
#define GEM_TXSR_TXCOMPL_Pos  (5U)
#define GEM_TXSR_TXCOMPL_Msk  (0x20UL) /* Transmit completed OK */
#define GEM_TXSR_URUN_Pos     (6U)
#define GEM_TXSR_URUN_Msk     (0x40UL) /* Transmit underrun */
#define GEM_TXSR_HRESPNOK_Pos (8U)
#define GEM_TXSR_HRESPNOK_Msk (0x100UL) /* Transmit hresp not OK */

#define GEM_RXSR_BUFFNA_Pos   (0U)
#define GEM_RXSR_BUFFNA_Msk   (0x1UL) /* RX buffer used bit set */
#define GEM_RXSR_FRAMERX_Pos  (1U)
#define GEM_RXSR_FRAMERX_Msk  (0x2UL) /* Frame received OK */
#define GEM_RXSR_RXOVR_Pos    (2U)
#define GEM_RXSR_RXOVR_Msk    (0x4UL) /* Receive overrun */
#define GEM_RXSR_HRESPNOK_Pos (3U)
#define GEM_RXSR_HRESPNOK_Msk (0x8UL) /* Receive hresp not OK */

#define GEM_SR_ALL_Pos (0U)
#define GEM_SR_ALL_Msk (0xFFFFFFFFUL) /* Mask for full register */

#define GEM_HS_MAC_SPEED_SIZE (0x3UL) /* Auto-negotiation */

#define GEM_PCS_CON_AUTO_NEG_Pos (12U)
#define GEM_PCS_CON_AUTO_NEG_Msk (0x1000UL) /* Auto-negotiation */

#define GEM_LINK_STATUS_SIZE (0x1UL) /*  */

#define GEM_USX_SPEED_SIZE (0x3UL) /*  */

#define GEM_SERDES_RATE_SIZE (0x2UL) /*  */

#define GEM_TX_EN_SIZE         (0x1UL) /*  */
#define GEM_TX_SCR_BYPASS_SIZE (0x1UL) /*  */

#define GEM_SIGNAL_OK_SIZE (0x1UL) /*  */

#define GEM_BLOCK_LOCK_SIZE (0x1UL) /*  */

#define GEM_NO_PCS_SIZE (0x1UL) /* NO PCS bit size */

#define GEM_HIGH_SPEED_SIZE (0x1UL) /* High Speed bit size */

#define GEM_INTQ1SR_RXCOMPL_Pos (1U)
#define GEM_INTQ1SR_RXCOMPL_Msk (0x2UL) /* Receive completed OK */
#define GEM_INTQ1SR_TXERR_Pos   (6U)
#define GEM_INTQ1SR_TXERR_Msk   (0x40UL) /* Transmit AMBA Error */
#define GEM_INTQ1SR_TXCOMPL_Pos (7U)
#define GEM_INTQ1SR_TXCOMPL_Msk (0x80UL) /* Transmit completed OK */

#define GEM_IXR_MGMNT_Pos      (0U)
#define GEM_IXR_MGMNT_Msk      (0x1UL) /* PHY management complete */
#define GEM_IXR_FRAMERX_Pos    (1U)
#define GEM_IXR_FRAMERX_Msk    (0x2UL) /* Frame received ok */
#define GEM_IXR_RXUSED_Pos     (2U)
#define GEM_IXR_RXUSED_Msk     (0x4UL) /* Rx buffer used bit read */
#define GEM_IXR_TXUSED_Pos     (3U)
#define GEM_IXR_TXUSED_Msk     (0x8UL) /* Tx buffer used bit read */
#define GEM_IXR_URUN_Pos       (4U)
#define GEM_IXR_URUN_Msk       (0x10UL) /* Transmit underrun */
#define GEM_IXR_RETRY_Pos      (5U)
#define GEM_IXR_RETRY_Msk      (0x20UL) /* Retry limit exceeded */
#define GEM_IXR_TXCOMPL_Pos    (7U)
#define GEM_IXR_TXCOMPL_Msk    (0x80UL) /* Frame transmitted ok */
#define GEM_IXR_USXGMII_Pos    (8U)
#define GEM_IXR_USXGMII_Msk    (0x100UL) /* USXGMII Interrupt */
#define GEM_IXR_RXOVR_Pos      (10U)
#define GEM_IXR_RXOVR_Msk      (0x400UL) /* Receive overrun occurred */
#define GEM_IXR_HRESPNOK_Pos   (11U)
#define GEM_IXR_HRESPNOK_Msk   (0x800UL) /* hresp not ok */
#define GEM_IXR_PAUSENZERO_Pos (12U)
#define GEM_IXR_PAUSENZERO_Msk (0x1000UL) /* Pause frame received */
#define GEM_IXR_PAUSETX_Pos    (14U)
#define GEM_IXR_PAUSETX_Msk    (0x4000UL) /* Pause frame transmitted */
#define GEM_IXR_ALL_Pos        (0U)
#define GEM_IXR_ALL_Msk        (0x7FFFUL) /* Everything! */
#define GEM_IXR_PTPDRRX_Pos    (18U)
#define GEM_IXR_PTPDRRX_Msk    (0x40000UL) /* PTP Delay_req RXed */
#define GEM_IXR_PTPSRX_Pos     (19U)
#define GEM_IXR_PTPSRX_Msk     (0x80000UL) /* PTP Sync RXed */
#define GEM_IXR_PTPDRTX_Pos    (20U)
#define GEM_IXR_PTPDRTX_Msk    (0x100000UL) /* PTP Delay_req TXed */
#define GEM_IXR_PTPSTX_Pos     (21U)
#define GEM_IXR_PTPSTX_Msk     (0x200000UL) /* PTP Sync TXed */
#define GEM_IXR_PTPPDRRX_Pos   (22U)
#define GEM_IXR_PTPPDRRX_Msk   (0x400000UL) /* PTP Pdelay_req RXed */
#define GEM_IXR_PTPPSRX_Pos    (23U)
#define GEM_IXR_PTPPSRX_Msk    (0x800000UL) /* PTP Pdelay_resp RXed */
#define GEM_IXR_PTPPDRTX_Pos   (24U)
#define GEM_IXR_PTPPDRTX_Msk   (0x1000000UL) /* PTP Pdelay_req TXed */
#define GEM_IXR_PTPPSTX_Pos    (25U)
#define GEM_IXR_PTPPSTX_Msk    (0x2000000UL) /* PTP Pdelay_resp TXed */

#define GEM_PHYMNTNC_PREG_SHFT_MSK (0x12UL) /* Shift bits for PHREG */
#define GEM_PHYMNTNC_PHAD_SHFT_MSK (0x17UL) /* Shift bits for PHYAD */
#define GEM_PHYMNTNC_DATA_Pos      (0U)
#define GEM_PHYMNTNC_DATA_Msk      (0xFFFUL) /* data bits */
#define GEM_PHYMNTNC_REG_Pos       (18U)
#define GEM_PHYMNTNC_REG_Msk       (0x7C0000UL) /* register bits */
#define GEM_PHYMNTNC_ADDR_Pos      (23U)
#define GEM_PHYMNTNC_ADDR_Msk      (0xF800000UL) /* Address bits */
#define GEM_PHYMNTNC_OP_W_Pos      (28U)
#define GEM_PHYMNTNC_OP_W_Msk      (0x10000000UL) /* write operation */
#define GEM_PHYMNTNC_OP_R_Pos      (29U)
#define GEM_PHYMNTNC_OP_R_Msk      (0x20000000UL) /* read operation */
#define GEM_PHYMNTNC_OP_Msk        (0x40020000UL) /* operation mask bits (non-contiguous) */

#define GEM_RXWM_LOW_SHFT_MSK (0x10UL) /* Shift for RXWM low */
#define GEM_RXWM_HIGH_Pos     (0U)
#define GEM_RXWM_HIGH_Msk     (0xFFFFUL) /* RXWM high mask */
#define GEM_RXWM_LOW_Pos      (16U)
#define GEM_RXWM_LOW_Msk      (0xFFFF0000UL) /* RXWM low mask */

#define GEM_CMPA_MASK_Pos   (13U)
#define GEM_CMPA_MASK_Msk   (0x3E000UL) /* Compare reg number */
#define GEM_CMPA_ENABLE_Pos (18U)
#define GEM_CMPA_ENABLE_Msk (0x40000UL) /* Enable Compare A */

#define GEM_TXBUF_LEN_Pos   (0U)
#define GEM_TXBUF_LEN_Msk   (0x3FFFUL) /* Mask for length field */
#define GEM_TXBUF_LAST_Pos  (15U)
#define GEM_TXBUF_LAST_Msk  (0x8000UL) /* Last buffer */
#define GEM_TXBUF_NOCRC_Pos (16U)
#define GEM_TXBUF_NOCRC_Msk (0x10000UL) /* No CRC */
#define GEM_TXBUF_TCP_Pos   (26U)
#define GEM_TXBUF_TCP_Msk   (0x4000000UL) /* Late collision. */
#define GEM_TXBUF_EXH_Pos   (27U)
#define GEM_TXBUF_EXH_Msk   (0x8000000UL) /* Buffers exhausted */
#define GEM_TXBUF_URUN_Pos  (28U)
#define GEM_TXBUF_URUN_Msk  (0x10000000UL) /* Transmit underrun occurred */
#define GEM_TXBUF_RETRY_Pos (29U)
#define GEM_TXBUF_RETRY_Msk (0x20000000UL) /* Retry limit exceeded */
#define GEM_TXBUF_WRAP_Pos  (30U)
#define GEM_TXBUF_WRAP_Msk  (0x40000000UL) /* Wrap bit, last descriptor */
#define GEM_TXBUF_USED_Pos  (31U)
#define GEM_TXBUF_USED_Msk  (0x80000000UL) /* Used bit. */

#define GEM_RXBUF_NEW_Pos       (0U)
#define GEM_RXBUF_NEW_Msk       (0x1UL) /* Used bit.. */
#define GEM_RXBUF_WRAP_Pos      (1U)
#define GEM_RXBUF_WRAP_Msk      (0x2UL) /* Wrap bit, last BD */
#define GEM_RXBUF_LEN_Pos       (0U)
#define GEM_RXBUF_LEN_Msk       (0x1FFFUL) /* Mask for length field */
#define GEM_RXBUF_LEN_JUMBO_Pos (0U)
#define GEM_RXBUF_LEN_JUMBO_Msk (0x3FFFUL) /* Mask for jumbo length */
#define GEM_RXBUF_SOF_Pos       (14U)
#define GEM_RXBUF_SOF_Msk       (0x4000UL) /* Start of frame. */
#define GEM_RXBUF_EOF_Pos       (15U)
#define GEM_RXBUF_EOF_Msk       (0x8000UL) /* End of frame. */
#define GEM_RXBUF_CFI_Pos       (16U)
#define GEM_RXBUF_CFI_Msk       (0x10000UL) /* CFI frame */
#define GEM_RXBUF_VPRI_Pos      (17U)
#define GEM_RXBUF_VPRI_Msk      (0xE0000UL) /* Vlan priority */
#define GEM_RXBUF_PRI_Pos       (20U)
#define GEM_RXBUF_PRI_Msk       (0x100000UL) /* Priority tagged */
#define GEM_RXBUF_VLAN_Pos      (21U)
#define GEM_RXBUF_VLAN_Msk      (0x200000UL) /* VLAN tagged */
#define GEM_RXBUF_IDMATCH_Pos   (22U)
#define GEM_RXBUF_IDMATCH_Msk   (0xC00000UL) /* ID matched mask */
#define GEM_RXBUF_IDFOUND_Pos   (24U)
#define GEM_RXBUF_IDFOUND_Msk   (0x1000000UL) /* Type ID matched */
#define GEM_RXBUF_EXH_Pos       (27U)
#define GEM_RXBUF_EXH_Msk       (0x8000000UL) /* buffer exhausted */
#define GEM_RXBUF_UNIHASH_Pos   (29U)
#define GEM_RXBUF_UNIHASH_Msk   (0x20000000UL) /* Unicast hashed frame */
#define GEM_RXBUF_MULTIHASH_Pos (30U)
#define GEM_RXBUF_MULTIHASH_Msk (0x40000000UL) /* Multicast hashed frame */
#define GEM_RXBUF_BCAST_Pos     (31U)
#define GEM_RXBUF_BCAST_Msk     (0x80000000UL) /* Broadcast frame */
#define GEM_RXBUF_ADD_Pos       (2U)
#define GEM_RXBUF_ADD_Msk       (0xFFFFFFFCUL) /* Mask for address */

/*****************  Bit definitions for SDIO  *****************/
#define SDIO_HC_DMA_SDMA_Msk     (0x0UL) /* SDMA Mode (non-contiguous) */
#define SDIO_HC_SPEC_V1          (0x0UL) /* HC spec version 1 */
#define SDIO_HC_LED_Pos          (0U)
#define SDIO_HC_LED_Msk          (0x1UL) /* LED Control */
#define SDIO_HC_SPEC_V2          (0x1UL) /* HC spec version 2 */
#define SDIO_HC_BUS_WIDTH_4      (0x2UL) /*  */
#define SDIO_HC_SPEC_V3          (0x2UL) /* HC spec version 3 */
#define SDIO_HC_WIDTH_Pos        (1U)
#define SDIO_HC_WIDTH_Msk        (0x2UL) /* Bus width */
#define SDIO_HC_SPEED_Pos        (2U)
#define SDIO_HC_SPEED_Msk        (0x4UL) /* High Speed */
#define SDIO_HC_DMA_ADMA1_Pos    (3U)
#define SDIO_HC_DMA_ADMA1_Msk    (0x8UL) /* ADMA1 Mode */
#define SDIO_HC_DMA_ADMA2_32_Pos (4U)
#define SDIO_HC_DMA_ADMA2_32_Msk (0x10UL) /* ADMA2 Mode - 32 bit */
#define SDIO_HC_DMA_ADMA2_64_Pos (3U)
#define SDIO_HC_DMA_ADMA2_64_Msk (0x18UL) /* ADMA2 Mode - 64 bit */
#define SDIO_HC_DMA_Pos          (3U)
#define SDIO_HC_DMA_Msk          (0x18UL) /* DMA Mode Select */
#define SDIO_HC_EXT_BUS_WIDTH    (0x20UL) /* Bus width - 8 bit */
#define SDIO_HC_CARD_DET_TL_Pos  (6U)
#define SDIO_HC_CARD_DET_TL_Msk  (0x40UL) /* Card Detect Tst Lvl */
#define SDIO_HC_CARD_DET_SD_Pos  (7U)
#define SDIO_HC_CARD_DET_SD_Msk  (0x80UL) /* Card Detect Sig Det */

#define SDIO_PC_BUS_PWR_Pos      (0U)
#define SDIO_PC_BUS_PWR_Msk      (0x1UL) /* Bus Power Control */
#define SDIO_PC_BUS_VSEL_1V8_Msk (0xAUL) /* Bus Voltage 1.8V (non-contiguous) */
#define SDIO_PC_BUS_VSEL_3V0_Pos (2U)
#define SDIO_PC_BUS_VSEL_3V0_Msk (0xCUL) /* Bus Voltage 3.0V */
#define SDIO_PC_BUS_VSEL_3V3_Pos (1U)
#define SDIO_PC_BUS_VSEL_3V3_Msk (0xEUL) /* Bus Voltage 3.3V */
#define SDIO_PC_BUS_VSEL_Pos     (1U)
#define SDIO_PC_BUS_VSEL_Msk     (0xEUL) /* Bus Voltage Select */
#define SDIO_PC_EMMC_HW_RST_Pos  (4U)
#define SDIO_PC_EMMC_HW_RST_Msk  (0x10UL) /* HW reset for eMMC */

#define SDIO_BGC_STP_REQ_Pos     (0U)
#define SDIO_BGC_STP_REQ_Msk     (0x1UL) /* Block Gap Stop Req */
#define SDIO_BGC_CNT_REQ_Pos     (1U)
#define SDIO_BGC_CNT_REQ_Msk     (0x2UL) /* Block Gap Cont Req */
#define SDIO_BGC_RWC_Pos         (2U)
#define SDIO_BGC_RWC_Msk         (0x4UL) /* Block Gap Rd Wait */
#define SDIO_BGC_INTR_Pos        (3U)
#define SDIO_BGC_INTR_Msk        (0x8UL) /* Block Gap Intr */
#define SDIO_BGC_SPI_MODE_Pos    (4U)
#define SDIO_BGC_SPI_MODE_Msk    (0x10UL) /* Block Gap SPI Mode */
#define SDIO_BGC_BOOT_EN_Pos     (5U)
#define SDIO_BGC_BOOT_EN_Msk     (0x20UL) /* Block Gap Boot Enb */
#define SDIO_BGC_ALT_BOOT_EN_Pos (6U)
#define SDIO_BGC_ALT_BOOT_EN_Msk (0x40UL) /* Block Gap Alt BootEn */
#define SDIO_BGC_BOOT_ACK_Pos    (7U)
#define SDIO_BGC_BOOT_ACK_Msk    (0x80UL) /* Block Gap Boot Ack */

#define SDIO_WC_WUP_ON_INTR_Pos  (0U)
#define SDIO_WC_WUP_ON_INTR_Msk  (0x1UL) /* Wakeup Card Intr */
#define SDIO_WC_WUP_ON_INSRT_Pos (1U)
#define SDIO_WC_WUP_ON_INSRT_Msk (0x2UL) /* Wakeup Card Insert */
#define SDIO_WC_WUP_ON_REM_Pos   (2U)
#define SDIO_WC_WUP_ON_REM_Msk   (0x4UL) /* Wakeup Card Removal */

#define SDIO_CC_SDCLK_FREQ_BASE_Msk    (0x0UL) /* Base clock (non-contiguous) */
#define SDIO_CC_INT_CLK_EN_Pos         (0U)
#define SDIO_CC_INT_CLK_EN_Msk         (0x1UL) /* INT clk enable */
#define SDIO_CC_INT_CLK_STABLE_Pos     (1U)
#define SDIO_CC_INT_CLK_STABLE_Msk     (0x2UL) /* INT clk stable */
#define SDIO_CC_SDCLK_FREQ_SEL_EXT_Pos (0U)
#define SDIO_CC_SDCLK_FREQ_SEL_EXT_Msk (0x3UL) /* SD clk freq sel upper */
#define SDIO_CC_SD_CLK_EN_Pos          (2U)
#define SDIO_CC_SD_CLK_EN_Msk          (0x4UL) /* SD clk enable */
#define SDIO_CC_MAX_NUM_OF_DIV         (0x9UL) /* Max number of Clock dividers */
#define SDIO_CC_SD_CLK_GEN_SEL_Pos     (5U)
#define SDIO_CC_SD_CLK_GEN_SEL_Msk     (0x20UL) /* SD clk gen selection */
#define SDIO_CC_SDCLK_FREQ_SEL_Pos     (0U)
#define SDIO_CC_SDCLK_FREQ_SEL_Msk     (0xFFUL)  /* SD clk freq sel */
#define SDIO_CC_MAX_DIV_CNT            (0x100UL) /* Max divider count */
#define SDIO_CC_SDCLK_FREQ_D2_Pos      (8U)
#define SDIO_CC_SDCLK_FREQ_D2_Msk      (0x100UL) /* Divider 2 */
#define SDIO_CC_SDCLK_FREQ_D4_Pos      (9U)
#define SDIO_CC_SDCLK_FREQ_D4_Msk      (0x200UL) /* Divider 4 */
#define SDIO_CC_SDCLK_FREQ_D8_Pos      (10U)
#define SDIO_CC_SDCLK_FREQ_D8_Msk      (0x400UL) /* Divider 8 */
#define SDIO_CC_EXT_MAX_DIV_CNT        (0x7FEUL) /* Max extended divider count */
#define SDIO_CC_SDCLK_FREQ_D16_Pos     (11U)
#define SDIO_CC_SDCLK_FREQ_D16_Msk     (0x800UL) /* Divider 16 */
#define SDIO_CC_SDCLK_FREQ_D32_Pos     (12U)
#define SDIO_CC_SDCLK_FREQ_D32_Msk     (0x1000UL) /* Divider 32 */
#define SDIO_CC_SDCLK_FREQ_D64_Pos     (13U)
#define SDIO_CC_SDCLK_FREQ_D64_Msk     (0x2000UL) /* Divider 64 */
#define SDIO_CC_SDCLK_FREQ_D128_Pos    (14U)
#define SDIO_CC_SDCLK_FREQ_D128_Msk    (0x4000UL) /* Divider 128 */
#define SDIO_CC_SDCLK_FREQ_D256_Pos    (15U)
#define SDIO_CC_SDCLK_FREQ_D256_Msk    (0x8000UL) /* Divider 256 */

#define SDIO_TC_CNTR_VAL_Pos (0U)
#define SDIO_TC_CNTR_VAL_Msk (0xFUL) /* Data timeout counter */

#define SDIO_SWRST_ALL_Pos      (0U)
#define SDIO_SWRST_ALL_Msk      (0x1UL) /* Software Reset All */
#define SDIO_SWRST_CMD_LINE_Pos (1U)
#define SDIO_SWRST_CMD_LINE_Msk (0x2UL) /* Software reset for CMD line */
#define SDIO_SWRST_DAT_LINE_Pos (2U)
#define SDIO_SWRST_DAT_LINE_Msk (0x4UL) /* Software reset for DAT line */

#define SDIO_INTR_CC_Pos             (0U)
#define SDIO_INTR_CC_Msk             (0x1UL) /* Command Complete */
#define SDIO_INTR_ERR_CCRC_Pos       (1U)
#define SDIO_INTR_ERR_CCRC_Msk       (0x2UL) /* Command CRC Error */
#define SDIO_INTR_TC_Pos             (1U)
#define SDIO_INTR_TC_Msk             (0x2UL) /* Transfer Complete */
#define SDIO_INTR_BGE_Pos            (2U)
#define SDIO_INTR_BGE_Msk            (0x4UL) /* Block Gap Event */
#define SDIO_INTR_DMA_Pos            (3U)
#define SDIO_INTR_DMA_Msk            (0x8UL) /* DMA Interrupt */
#define SDIO_INTR_ERR_CI_Pos         (3U)
#define SDIO_INTR_ERR_CI_Msk         (0x8UL) /* Command Index Error */
#define SDIO_INTR_BWR_Pos            (4U)
#define SDIO_INTR_BWR_Msk            (0x10UL) /* Buffer Write Ready */
#define SDIO_INTR_ERR_DT_Pos         (4U)
#define SDIO_INTR_ERR_DT_Msk         (0x10UL) /* Data Timeout Error */
#define SDIO_INTR_BRR_Pos            (5U)
#define SDIO_INTR_BRR_Msk            (0x20UL) /* Buffer Read Ready */
#define SDIO_INTR_ERR_DCRC_Pos       (5U)
#define SDIO_INTR_ERR_DCRC_Msk       (0x20UL) /* Data CRC Error */
#define SDIO_INTR_CARD_INSRT_Pos     (6U)
#define SDIO_INTR_CARD_INSRT_Msk     (0x40UL) /* Card Insert */
#define SDIO_INTR_ERR_DEB_Pos        (6U)
#define SDIO_INTR_ERR_DEB_Msk        (0x40UL) /* Data End Bit Error */
#define SDIO_INTR_CARD_REM_Pos       (7U)
#define SDIO_INTR_CARD_REM_Msk       (0x80UL) /* Card Remove */
#define SDIO_INTR_ERR_CUR_LMT_Pos    (7U)
#define SDIO_INTR_ERR_CUR_LMT_Msk    (0x80UL) /* Current Limit Error */
#define SDIO_INTR_CARD_Pos           (8U)
#define SDIO_INTR_CARD_Msk           (0x100UL) /* Card Interrupt */
#define SDIO_INTR_ERR_AUTO_CMD12_Pos (8U)
#define SDIO_INTR_ERR_AUTO_CMD12_Msk (0x100UL) /* Auto CMD12 Error */
#define SDIO_INTR_ERR_ADMA_Pos       (9U)
#define SDIO_INTR_ERR_ADMA_Msk       (0x200UL) /* ADMA Error */
#define SDIO_INTR_INT_A_Pos          (9U)
#define SDIO_INTR_INT_A_Msk          (0x200UL) /* INT A Interrupt */
#define SDIO_INTR_INT_B_Pos          (10U)
#define SDIO_INTR_INT_B_Msk          (0x400UL) /* INT B Interrupt */
#define SDIO_INTR_INT_C_Pos          (11U)
#define SDIO_INTR_INT_C_Msk          (0x800UL) /* INT C Interrupt */
#define SDIO_INTR_ERR_TR_Pos         (12U)
#define SDIO_INTR_ERR_TR_Msk         (0x1000UL) /* Tuning Error */
#define SDIO_INTR_RE_TUNING_Pos      (12U)
#define SDIO_INTR_RE_TUNING_Msk      (0x1000UL) /* Re-Tuning Interrupt */
#define SDIO_INTR_ERR_Pos            (15U)
#define SDIO_INTR_ERR_Msk            (0x8000UL) /* Error Interrupt */

#define SDIO_NORM_INTR_ALL_Pos (0U)
#define SDIO_NORM_INTR_ALL_Msk (0xFFFFUL) /*  */

#define SDIO_ERROR_INTR_ALL_Msk (0xF3FFUL) /* Mask for error bits (non-contiguous) */

#define SDIO_BLK_SIZE_1024 (0x400UL) /*  */
#define SDIO_BLK_SIZE_2048 (0x800UL) /*  */
#define SDIO_BLK_SIZE_Pos  (0U)
#define SDIO_BLK_SIZE_Msk  (0xFFFUL) /* Transfer Block Size */

#define SDIO_SDMA_BUFF_SIZE_Pos (12U)
#define SDIO_SDMA_BUFF_SIZE_Msk (0x7000UL) /* Host SDMA Buffer Size */

#define SDIO_TM_DMA_EN_Pos        (0U)
#define SDIO_TM_DMA_EN_Msk        (0x1UL) /* DMA Enable */
#define SDIO_TM_BLK_CNT_EN_Pos    (1U)
#define SDIO_TM_BLK_CNT_EN_Msk    (0x2UL) /* Block Count Enable */
#define SDIO_TM_AUTO_CMD12_EN_Pos (2U)
#define SDIO_TM_AUTO_CMD12_EN_Msk (0x4UL) /* Auto CMD12 Enable */

#define SDIO_CMD_RESP_NONE_Msk    (0x0UL) /* No Response (non-contiguous) */
#define SDIO_CMD_SYS_DEF          (0x0UL) /*  */
#define SDIO_CMD_TYPE_NORM_Msk    (0x0UL) /* CMD Type - Normal (non-contiguous) */
#define SDIO_CMD_RESP_L136_Pos    (0U)
#define SDIO_CMD_RESP_L136_Msk    (0x1UL) /* Response length 138 */
#define SDIO_CMD_SYS_eC           (0x1UL) /*  */
#define SDIO_CMD_RESP_L48_Pos     (1U)
#define SDIO_CMD_RESP_L48_Msk     (0x2UL) /* Response length 48 */
#define SDIO_CMD_SYS_OTP          (0x3UL) /*  */
#define SDIO_CMD_SYS_ASSD         (0x4UL) /*  */
#define SDIO_CMD_SYS_VEND         (0x5UL) /*  */
#define SDIO_CMD_TYPE_SUSPEND_Pos (6U)
#define SDIO_CMD_TYPE_SUSPEND_Msk (0x40UL) /* CMD Type - Suspend */
#define SDIO_CMD_TYPE_RESUME_Pos  (7U)
#define SDIO_CMD_TYPE_RESUME_Msk  (0x80UL) /* CMD Type - Resume */
#define SDIO_CMD_TYPE_ABORT_Pos   (6U)
#define SDIO_CMD_TYPE_ABORT_Msk   (0xC0UL) /* CMD Type - Abort */
#define SDIO_CMD_TYPE_Pos         (6U)
#define SDIO_CMD_TYPE_Msk         (0xC0UL) /* Command Type */

#define SDIO_DAT_PRESENT_SEL_Pos (5U)
#define SDIO_DAT_PRESENT_SEL_Msk (0x20UL) /* Data Present Select */

#define SDIO_AUTO_CMD_CRC_Pos (2U)
#define SDIO_AUTO_CMD_CRC_Msk (0x4UL) /* Auto CMD CRC Error */
#define SDIO_AUTO_CMD_IND_Pos (4U)
#define SDIO_AUTO_CMD_IND_Msk (0x10UL) /* Auto CMD Index Error */

#define SDIO_HC2_DRV_STR_B_Msk       (0x0UL) /* Driver Strength B (non-contiguous) */
#define SDIO_HC2_UHS_MODE_SDR12_Msk  (0x0UL) /* SDR12 UHS Mode (non-contiguous) */
#define SDIO_HC2_UHS_MODE_SDR25_Pos  (0U)
#define SDIO_HC2_UHS_MODE_SDR25_Msk  (0x1UL) /* SDR25 UHS Mode */
#define SDIO_HC2_UHS_MODE_SDR50_Pos  (1U)
#define SDIO_HC2_UHS_MODE_SDR50_Msk  (0x2UL) /* SDR50 UHS Mode */
#define SDIO_HC2_UHS_MODE_SDR104_Pos (0U)
#define SDIO_HC2_UHS_MODE_SDR104_Msk (0x3UL) /* SDR104 UHS Mode */
#define SDIO_HC2_UHS_MODE_DDR50_Pos  (2U)
#define SDIO_HC2_UHS_MODE_DDR50_Msk  (0x4UL) /* DDR50 UHS Mode */
#define SDIO_HC2_HS400_Msk           (0x5UL) /* HS400 Mode (non-contiguous) */
#define SDIO_HC2_UHS_MODE_Pos        (0U)
#define SDIO_HC2_UHS_MODE_Msk        (0x7UL) /* UHS Mode select bits */
#define SDIO_HC2_1V8_EN_Pos          (3U)
#define SDIO_HC2_1V8_EN_Msk          (0x8UL) /* 1.8V Signal Enable */
#define SDIO_HC2_DRV_STR_A_Pos       (4U)
#define SDIO_HC2_DRV_STR_A_Msk       (0x10UL) /* Driver Strength A */
#define SDIO_HC2_DRV_STR_C_Pos       (5U)
#define SDIO_HC2_DRV_STR_C_Msk       (0x20UL) /* Driver Strength C */
#define SDIO_HC2_DRV_STR_D_Pos       (4U)
#define SDIO_HC2_DRV_STR_D_Msk       (0x30UL) /* Driver Strength D */
#define SDIO_HC2_EXEC_TNG_Pos        (6U)
#define SDIO_HC2_EXEC_TNG_Msk        (0x40UL) /* Execute Tuning */
#define SDIO_HC2_PRE_VAL_EN_Pos      (15U)
#define SDIO_HC2_PRE_VAL_EN_Msk      (0x8000UL) /* Preset Value Enable */

#define SDIO_CAP_MAX_BLK_LEN_512B_Msk (0x0UL) /* Max block 512 bytes (non-contiguous) */
#define SDIO_CAP_MAX_BL_LN_1024_Pos   (16U)
#define SDIO_CAP_MAX_BL_LN_1024_Msk   (0x10000UL) /* Max block 1024 bytes */
#define SDIO_CAP_MAX_BL_LN_2048_Pos   (17U)
#define SDIO_CAP_MAX_BL_LN_2048_Msk   (0x20000UL) /* Max block 2048 bytes */
#define SDIO_CAP_MAX_BLK_LEN_Pos      (16U)
#define SDIO_CAP_MAX_BLK_LEN_Msk      (0x30000UL) /* Max block length */
#define SDIO_CAP_MAX_BL_LN_4096_Pos   (16U)
#define SDIO_CAP_MAX_BL_LN_4096_Msk   (0x30000UL) /* Max block 4096 bytes */
#define SDIO_CAP_EXT_MEDIA_BUS_Pos    (18U)
#define SDIO_CAP_EXT_MEDIA_BUS_Msk    (0x40000UL) /* Extended media bus */
#define SDIO_CAP_ADMA2_Pos            (19U)
#define SDIO_CAP_ADMA2_Msk            (0x80000UL) /* ADMA2 support */
#define SDIO_CAP_HIGH_SPEED_Pos       (21U)
#define SDIO_CAP_HIGH_SPEED_Msk       (0x200000UL) /* High speed support */
#define SDIO_CAP_SDMA_Pos             (22U)
#define SDIO_CAP_SDMA_Msk             (0x400000UL) /* SDMA support */
#define SDIO_CAP_VOLT_3V3_Pos         (24U)
#define SDIO_CAP_VOLT_3V3_Msk         (0x1000000UL) /* 3.3V support */
#define SDIO_CAP_VOLT_3V0_Pos         (25U)
#define SDIO_CAP_VOLT_3V0_Msk         (0x2000000UL) /* 3.0V support */
#define SDIO_CAP_VOLT_1V8_Pos         (26U)
#define SDIO_CAP_VOLT_1V8_Msk         (0x4000000UL) /* 1.8V support */
#define SDIO_CAP_SPI_MODE_Pos         (29U)
#define SDIO_CAP_SPI_MODE_Msk         (0x20000000UL) /* SPI mode */
#define SDIO_CAP_SPI_BLOCK_MODE_Pos   (30U)
#define SDIO_CAP_SPI_BLOCK_MODE_Msk   (0x40000000UL) /* SPI block mode */

#define SDIO_CAPS_REM_CARD      (0x0UL)        /* Removable Slot */
#define SDIO_CAPS_EMB_SLOT      (0x40000000UL) /* Embedded Slot */
#define SDIO_CAPS_SHR_BUS       (0x80000000UL) /* Shared Bus Slot */
#define SDIO_CAPS_SLOT_TYPE_Pos (30U)
#define SDIO_CAPS_SLOT_TYPE_Msk (0xC0000000UL) /* Slot Type */

#define SDIO_ECAPS_RE_TNG_MODE1_Msk (0x0UL) /* Re-tuning mode 1 (non-contiguous) */
#define SDIO_ECAPS_SDR50_Pos        (0U)
#define SDIO_ECAPS_SDR50_Msk        (0x1UL) /* SDR50 Mode support */
#define SDIO_ECAPS_SDR104_Pos       (1U)
#define SDIO_ECAPS_SDR104_Msk       (0x2UL) /* SDR104 Mode support */
#define SDIO_ECAPS_DDR50_Pos        (2U)
#define SDIO_ECAPS_DDR50_Msk        (0x4UL) /* DDR50 Mode support */
#define SDIO_ECAPS_DRV_TYPE_A_Pos   (4U)
#define SDIO_ECAPS_DRV_TYPE_A_Msk   (0x10UL) /* DriverType A support */
#define SDIO_ECAPS_DRV_TYPE_C_Pos   (5U)
#define SDIO_ECAPS_DRV_TYPE_C_Msk   (0x20UL) /* DriverType C support */
#define SDIO_ECAPS_DRV_TYPE_D_Pos   (6U)
#define SDIO_ECAPS_DRV_TYPE_D_Msk   (0x40UL) /* DriverType D support */
#define SDIO_ECAPS_RE_TNG_MODE2_Pos (14U)
#define SDIO_ECAPS_RE_TNG_MODE2_Msk (0x4000UL) /* Re-tuning mode 2 */
#define SDIO_ECAPS_RE_TNG_MODE3_Pos (15U)
#define SDIO_ECAPS_RE_TNG_MODE3_Msk (0x8000UL) /* Re-tuning mode 3 */
#define SDIO_ECAPS_SPI_MODE_Pos     (24U)
#define SDIO_ECAPS_SPI_MODE_Msk     (0x1000000UL) /* SPI mode */
#define SDIO_ECAPS_SPI_BLK_MODE_Pos (25U)
#define SDIO_ECAPS_SPI_BLK_MODE_Msk (0x2000000UL) /* SPI block mode */

#define SDIO_PSR_INHIBIT_CMD_Pos   (0U)
#define SDIO_PSR_INHIBIT_CMD_Msk   (0x1UL) /* Command inhibit - CMD */
#define SDIO_PSR_INHIBIT_DAT_Pos   (1U)
#define SDIO_PSR_INHIBIT_DAT_Msk   (0x2UL) /* Command Inhibit - DAT */
#define SDIO_PSR_DAT_ACTIVE_Pos    (2U)
#define SDIO_PSR_DAT_ACTIVE_Msk    (0x4UL) /* DAT line active */
#define SDIO_PSR_RE_TUNING_REQ_Pos (3U)
#define SDIO_PSR_RE_TUNING_REQ_Msk (0x8UL) /* Re-tuning request */
#define SDIO_PSR_WR_ACTIVE_Pos     (8U)
#define SDIO_PSR_WR_ACTIVE_Msk     (0x100UL) /* Write transfer active */
#define SDIO_PSR_RD_ACTIVE_Pos     (9U)
#define SDIO_PSR_RD_ACTIVE_Msk     (0x200UL) /* Read transfer active */
#define SDIO_PSR_BUFF_WR_EN_Pos    (10U)
#define SDIO_PSR_BUFF_WR_EN_Msk    (0x400UL) /* Buffer write enable */
#define SDIO_PSR_BUFF_RD_EN_Pos    (11U)
#define SDIO_PSR_BUFF_RD_EN_Msk    (0x800UL) /* Buffer read enable */
#define SDIO_PSR_CARD_INSRT_Pos    (16U)
#define SDIO_PSR_CARD_INSRT_Msk    (0x10000UL) /* Card inserted */
#define SDIO_PSR_CARD_STABLE_Pos   (17U)
#define SDIO_PSR_CARD_STABLE_Msk   (0x20000UL) /* Card state stable */
#define SDIO_PSR_CARD_DPL_Pos      (18U)
#define SDIO_PSR_CARD_DPL_Msk      (0x40000UL) /* Card detect pin level */
#define SDIO_PSR_DAT30_SG_LVL_Pos  (20U)
#define SDIO_PSR_DAT30_SG_LVL_Msk  (0xF00000UL) /* Data 3:0 signal lvl */
#define SDIO_PSR_CMD_SG_LVL_Pos    (24U)
#define SDIO_PSR_CMD_SG_LVL_Msk    (0x1000000UL) /* Cmd Line signal lvl */
#define SDIO_PSR_DAT74_SG_LVL_Pos  (25U)
#define SDIO_PSR_DAT74_SG_LVL_Msk  (0x1E000000UL) /* Data 7:4 signal lvl */

#define SDIO_FE_INTR_ERR_CCRC_Pos     (1U)
#define SDIO_FE_INTR_ERR_CCRC_Msk     (0x2UL) /* Command CRC Error */
#define SDIO_FE_AUTO_CMD_CRC_Pos      (2U)
#define SDIO_FE_AUTO_CMD_CRC_Msk      (0x4UL) /* Auto CMD CRC Error */
#define SDIO_FE_INTR_ERR_CI_Pos       (3U)
#define SDIO_FE_INTR_ERR_CI_Msk       (0x8UL) /* Command Index Error */
#define SDIO_FE_AUTO_CMD_IND_Pos      (4U)
#define SDIO_FE_AUTO_CMD_IND_Msk      (0x10UL) /* Auto CMD Index Error */
#define SDIO_FE_INTR_ERR_DT_Pos       (4U)
#define SDIO_FE_INTR_ERR_DT_Msk       (0x10UL) /* Data Timeout Error */
#define SDIO_FE_INTR_ERR_DCRC_Pos     (5U)
#define SDIO_FE_INTR_ERR_DCRC_Msk     (0x20UL) /* Data CRC Error */
#define SDIO_FE_INTR_ERR_DEB_Pos      (6U)
#define SDIO_FE_INTR_ERR_DEB_Msk      (0x40UL) /* Data End Bit Error */
#define SDIO_FE_INTR_ERR_CUR_LMT_Pos  (7U)
#define SDIO_FE_INTR_ERR_CUR_LMT_Msk  (0x80UL) /* Current Limit Error */
#define SDIO_FE_INTR_ERR_AUTO_CMD_Pos (8U)
#define SDIO_FE_INTR_ERR_AUTO_CMD_Msk (0x100UL) /* Auto CMD Error */
#define SDIO_FE_INTR_ERR_ADMA_Pos     (9U)
#define SDIO_FE_INTR_ERR_ADMA_Msk     (0x200UL) /* ADMA Error */
#define SDIO_FE_INTR_ERR_TR_Pos       (12U)
#define SDIO_FE_INTR_ERR_TR_Msk       (0x1000UL) /* Target Response */

#define SDIO_ADMA_ERR_STATE_Pos (0U)
#define SDIO_ADMA_ERR_STATE_Msk (0x3UL) /* ADMA Error State */

#define SDIO_PHYREG1_ITAP_EN_Pos     (0U)
#define SDIO_PHYREG1_ITAP_EN_Msk     (0x1UL)  /* Input Tap Enable Mask */
#define SDIO_PHYREG1_ITAP_DLY_Msk    (0x2EUL) /* Input Tap Delay Mask (non-contiguous) */
#define SDIO_PHYREG1_ITAP_CHGWIN_Pos (6U)
#define SDIO_PHYREG1_ITAP_CHGWIN_Msk (0x40UL) /* Input Tap Change Window */
#define SDIO_PHYREG1_OTAP_EN_Pos     (8U)
#define SDIO_PHYREG1_OTAP_EN_Msk     (0x100UL) /* Output Tap Enable Mask */
#define SDIO_PHYREG1_OTAP_DLY_Pos    (12U)
#define SDIO_PHYREG1_OTAP_DLY_Msk    (0xF000UL) /* Output Tap Delay Mask */
#define SDIO_PHYREG1_STROBE_SEL_Pos  (16U)
#define SDIO_PHYREG1_STROBE_SEL_Msk  (0xFF0000UL) /* STROBE Sel Mask */

#define SDIO_PHYREG2_DLL_EN_Pos       (0U)
#define SDIO_PHYREG2_DLL_EN_Msk       (0x1UL) /* DLL enable */
#define SDIO_PHYREG2_DLL_RDY_Pos      (1U)
#define SDIO_PHYREG2_DLL_RDY_Msk      (0x2UL) /* DLL Ready */
#define SDIO_PHYREG2_TRIM_ICP_DEF_VAL (0x8UL) /* DLL TRIM ICP default value */
#define SDIO_PHYREG2_FREQ_SEL_Pos     (4U)
#define SDIO_PHYREG2_FREQ_SEL_Msk     (0x70UL) /* Freq selection */
#define SDIO_PHYREG2_TRIM_ICP_Pos     (8U)
#define SDIO_PHYREG2_TRIM_ICP_Msk     (0xF00UL) /* DLL TRIM ICP */
#define SDIO_PHYREG2_DLYTX_SEL_Pos    (16U)
#define SDIO_PHYREG2_DLYTX_SEL_Msk    (0x10000UL) /* Delay Chain based Tx Clk */
#define SDIO_PHYREG2_DLYRX_SEL_Pos    (17U)
#define SDIO_PHYREG2_DLYRX_SEL_Msk    (0x20000UL) /* Delay Chain based Rx Clk */

#define SDIO_FREQ_SEL_170MHZ_200MHz (0x0UL) /* Macro to select between 170MHz to 200MHz */
#define SDIO_FREQ_SEL_140MHZ_169MHz (0x1UL) /* Macro to select between 140MHz to 169MHz */
#define SDIO_FREQ_SEL_110MHZ_139MHz (0x2UL) /* Macro to select between 110MHz to 139MHz */
#define SDIO_FREQ_SEL_80MHZ_109MHz  (0x3UL) /* Macro to select between 80MHz to 109MHz */
#define SDIO_FREQ_SEL_50MHZ_79MHz   (0x4UL) /* Macro to select between 50MHz to 79MHz */

#define SDIO_APP_CMD_PREFIX (0x8000UL) /*  */

#define SDIO_CIC_CHK_PATTERN (0xAAUL) /*  */

#define SDIO_OCR_LOW_VOL  (0x80UL)     /*  */
#define SDIO_OCR_HIGH_VOL (0xFF8000UL) /*  */

#define SDIO_SCR_SEC_SUPP_NONE (0x0UL) /*  */
#define SDIO_SCR_SPEC3_2V0     (0x0UL) /*  */
#define SDIO_SCR_SPEC_1V0      (0x0UL) /*  */
#define SDIO_SCR_BLKCNT        (0x1UL) /* SCR Block Count */
#define SDIO_SCR_CMD_SUPP_Pos  (0U)
#define SDIO_SCR_CMD_SUPP_Msk  (0x3UL) /*  */
#define SDIO_SCR_BLKSIZE       (0x8UL) /* SCR Block Size */
#define SDIO_SCR_REG_LEN       (0x8UL) /*  */

#define SDIO_SWITCH_FUNC_CHECK     (0x0UL)        /*  */
#define SDIO_SWITCH_CMD_BLKCNT     (0x1UL)        /* Blk cnt for SWITCH cmd */
#define SDIO_SWITCH_CMD_BLKSIZE    (0x40UL)       /* Blk sz for SWITCH cmd */
#define SDIO_SWITCH_SD_RESP_LEN    (0x40UL)       /*  */
#define SDIO_SWITCH_CMD_HS_GET     (0xFFFFF0UL)   /* SWITCH cmd to Get HS */
#define SDIO_SWITCH_CMD_SDR12_SET  (0x80FFFFF0UL) /* SWITCH cmd to Set SDR12 */
#define SDIO_SWITCH_CMD_HS_SET     (0x80FFFFF1UL) /* SWITCH cmd to Set HS */
#define SDIO_SWITCH_CMD_SDR25_SET  (0x80FFFFF1UL) /* SWITCH cmd to Set SDR25 */
#define SDIO_SWITCH_CMD_SDR50_SET  (0x80FFFFF2UL) /* SWITCH cmd to Set SDR50 */
#define SDIO_SWITCH_CMD_SDR104_SET (0x80FFFFF3UL) /* SWITCH cmd to Set SDR104 */
#define SDIO_SWITCH_CMD_DDR50_SET  (0x80FFFFF4UL) /* SWITCH cmd to Set DDR50 */

#define SDIO_MODE_FUNC_GRP1 (0x1UL) /*  */
#define SDIO_MODE_FUNC_GRP2 (0x2UL) /*  */
#define SDIO_MODE_FUNC_GRP3 (0x3UL) /*  */
#define SDIO_MODE_FUNC_GRP4 (0x4UL) /*  */
#define SDIO_MODE_FUNC_GRP5 (0x5UL) /*  */
#define SDIO_MODE_FUNC_GRP6 (0x6UL) /*  */

#define SDIO_FUNC_GRP_DEF_VAL     (0xFUL)      /*  */
#define SDIO_FUNC_ALL_GRP_DEF_VAL (0xFFFFFFUL) /*  */

#define SDIO_ACC_MODE_DEF_SDR12 (0x0UL) /*  */
#define SDIO_ACC_MODE_HS_SDR25  (0x1UL) /*  */
#define SDIO_ACC_MODE_SDR50     (0x2UL) /*  */
#define SDIO_ACC_MODE_SDR104    (0x3UL) /*  */
#define SDIO_ACC_MODE_DDR50     (0x4UL) /*  */

#define SDIO_DRV_TYPE_B (0x0UL) /*  */
#define SDIO_DRV_TYPE_A (0x1UL) /*  */
#define SDIO_DRV_TYPE_C (0x2UL) /*  */
#define SDIO_DRV_TYPE_D (0x3UL) /*  */

#define SDIO_CUR_LIM_200 (0x0UL) /*  */
#define SDIO_CUR_LIM_400 (0x1UL) /*  */
#define SDIO_CUR_LIM_600 (0x2UL) /*  */
#define SDIO_CUR_LIM_800 (0x3UL) /*  */

#define SDIO_EXT_CSD_CMD_SET     (0x0UL)   /*  */
#define SDIO_EXT_CSD_CMD_BLKCNT  (0x1UL)   /* Blk Cnt for EXT CSD */
#define SDIO_EXT_CSD_SET_BITS    (0x1UL)   /*  */
#define SDIO_EXT_CSD_CLR_BITS    (0x2UL)   /*  */
#define SDIO_EXT_CSD_WRITE_BYTE  (0x3UL)   /*  */
#define SDIO_EXT_CSD_CMD_BLKSIZE (0x200UL) /* Blk Sz for EXT CSD */
#define SDIO_EXT_CSD_SIZE        (0x200UL) /*  */

#define SDIO_MMC_DELAY_FOR_SWITCH (0x3E8UL)     /*  */
#define SDIO_MMC_DDR_MAX_CLK      (0x3197500UL) /* Max clk for MMC DDR52 mode */
#define SDIO_MMC_HSD_MAX_CLK      (0x3197500UL) /* Max clk for MMC HSD mode */
#define SDIO_MMC_HS200_MAX_CLK    (0xBEBC200UL) /* Max clk for MMC HS200 mode */

#define SDIO_74CLK_DELAY (0xB90UL) /*  */

#define SDIO_100CLK_DELAY (0xFA0UL) /*  */

#define SDIO_INIT_DELAY (0x2710UL) /*  */

#define SDIO_CARD_STATE_IDLE (0x0UL)    /*  */
#define SDIO_CARD_SD         (0x1UL)    /* SD card */
#define SDIO_CARD_STATE_RDY  (0x1UL)    /*  */
#define SDIO_CARD_MMC        (0x2UL)    /* MMC card */
#define SDIO_CARD_STATE_IDEN (0x2UL)    /*  */
#define SDIO_CARD_SDIO       (0x3UL)    /* SDIO card */
#define SDIO_CARD_STATE_STBY (0x3UL)    /*  */
#define SDIO_CARD_SDCOMBO    (0x4UL)    /* SDCOMBO card */
#define SDIO_CARD_STATE_TRAN (0x4UL)    /*  */
#define SDIO_CARD_STATE_DATA (0x5UL)    /*  */
#define SDIO_CARD_STATE_RCV  (0x6UL)    /*  */
#define SDIO_CARD_STATE_PROG (0x7UL)    /*  */
#define SDIO_CARD_STATE_DIS  (0x8UL)    /*  */
#define SDIO_CARD_STATE_BTST (0x9UL)    /*  */
#define SDIO_CARD_STATE_SLP  (0xAUL)    /*  */
#define SDIO_CARD_DEF_ADDR   (0x1234UL) /* Card Default address */

#define SDIO_CHIP_EMMC (0x5UL) /* EMMC chip */

#define SDIO_DESC_MAX_LENGTH (0x10000UL) /*  */

#define SDIO_CLK_400_KHZ (0x61A80UL)   /* 400 KHZ */
#define SDIO_CLK_50_MHZ  (0x2FAF080UL) /* 50 MHZ */
#define SDIO_CLK_52_MHZ  (0x3197500UL) /* 52 MHZ */

#define SDIO_SD_STATUS_BLKCNT  (0x1UL)       /* Blk Cnt for SD status */
#define SDIO_SD_VER_1_0        (0x1UL)       /* SD ver 1 */
#define SDIO_SD_VER_2_0        (0x2UL)       /* SD ver 2 */
#define SDIO_SD_STATUS_BLKSIZE (0x40UL)      /* Blk Sz for SD status */
#define SDIO_SD_SDR12_MAX_CLK  (0x17D7840UL) /* Max clk for SDR12 mode */
#define SDIO_SD_DDR50_MAX_CLK  (0x2FAF080UL) /* Max clk for DDR50 mode */
#define SDIO_SD_SDR25_MAX_CLK  (0x2FAF080UL) /* Max clk for SDR25 mode */
#define SDIO_SD_SDR50_MAX_CLK  (0x5F5E100UL) /* Max clk for SDR50 mode */
#define SDIO_SD_INPUT_MAX_CLK  (0xA6E49C0UL) /*  */
#define SDIO_SD_SDR104_MAX_CLK (0xC65D400UL) /* Max clk for SDR104 mode */

#define SDIO_1_BIT_WIDTH (0x1UL) /* Bus Width 1 */

#define SDIO_4_BIT_WIDTH (0x2UL) /* Bus Width 4 */

#define SDIO_8_BIT_WIDTH (0x3UL) /* Bus Width 8 */

#define SDIO_UHS_SPEED_MODE_SDR12  (0x0UL)       /* SDR12 mode */
#define SDIO_UHS_SPEED_MODE_SDR25  (0x1UL)       /* SDR25 mode */
#define SDIO_UHS_SPEED_MODE_SDR50  (0x2UL)       /* SDR50 mode */
#define SDIO_UHS_SPEED_MODE_SDR104 (0x3UL)       /* SDR104 mode */
#define SDIO_UHS_SPEED_MODE_DDR50  (0x4UL)       /* DDR50 mode */
#define SDIO_UHS_SDR12_MAX_CLK     (0x17D7840UL) /* Max clk for SDR12 mode */
#define SDIO_UHS_DDR50_MAX_CLK     (0x2FAF080UL) /* Max clk for DDR50 mode */
#define SDIO_UHS_SDR25_MAX_CLK     (0x2FAF080UL) /* Max clk for SDR25 mode */
#define SDIO_UHS_SDR50_MAX_CLK     (0x5F5E100UL) /* Max clk for SDR50 mode */
#define SDIO_UHS_SDR104_MAX_CLK    (0xC65D400UL) /* Max clk for SDR104 mode */

#define SDIO_HIGH_SPEED_MODE    (0x5UL)       /* High Speed mode */
#define SDIO_HIGH_SPEED_MAX_CLK (0x2FAF080UL) /* Max clk for HS mode */

#define SDIO_DEFAULT_SPEED_MODE (0x6UL) /* Default speed mode */

#define SDIO_HS200_MODE (0x7UL) /* eMMC HS200 mode */

#define SDIO_HS400_MODE (0x8UL) /* eMMC HS400 mode */

#define SDIO_DDR52_MODE (0x4UL) /* eMMC DDR52 mode */

#define SDIO_TUNING_CMD_BLKCNT  (0x1UL)  /* Blk Cnt for Tuning cmd */
#define SDIO_TUNING_CMD_BLKSIZE (0x40UL) /* Blk Sz for Tuning cmd */

#define SDIO_SLOT_REM (0x0UL) /* Removable slot */
#define SDIO_SLOT_EMB (0x1UL) /* Embedded slot */

#define SDIO_WIDTH_4 (0x4UL) /* Bus width 4 */
#define SDIO_WIDTH_8 (0x8UL) /* Bus width 8 */

#define SDIO_SLOTTYPE_SDADIR (0x4UL) /* SD3.0 Auto Direction */

/*****************  Bit definitions for CAN  *****************/
#define CAN_SRR_SRST_Pos (0U)
#define CAN_SRR_SRST_Msk (0x1UL) /* Reset */
#define CAN_SRR_CEN_Pos  (1U)
#define CAN_SRR_CEN_Msk  (0x2UL) /* Can Enable */

#define CAN_MSR_SLEEP_Pos (0U)
#define CAN_MSR_SLEEP_Msk (0x1UL) /* Sleep Mode Select */
#define CAN_MSR_LBACK_Pos (1U)
#define CAN_MSR_LBACK_Msk (0x2UL) /* Loop Back Mode Select */
#define CAN_MSR_SNOOP_Pos (2U)
#define CAN_MSR_SNOOP_Msk (0x4UL) /* Snoop Mode Select */

#define CAN_BRPR_BRP_Pos (0U)
#define CAN_BRPR_BRP_Msk (0xFFUL) /* Baud Rate Prescaler */

#define CAN_BTR_TS1_Pos (0U)
#define CAN_BTR_TS1_Msk (0xFUL) /* Time Segment 1 */
#define CAN_BTR_TS2_Pos (4U)
#define CAN_BTR_TS2_Msk (0x70UL) /* Time Segment 2 */
#define CAN_BTR_SJW_Pos (7U)
#define CAN_BTR_SJW_Msk (0x180UL) /* Synchronization Jump Width */

#define CAN_ECR_TEC_Pos (0U)
#define CAN_ECR_TEC_Msk (0xFFUL) /* Transmit Error Counter */
#define CAN_ECR_REC_Pos (8U)
#define CAN_ECR_REC_Msk (0xFF00UL) /* Receive Error Counter */

#define CAN_ESR_CRCER_Pos (0U)
#define CAN_ESR_CRCER_Msk (0x1UL) /* CRC Error */
#define CAN_ESR_FMER_Pos  (1U)
#define CAN_ESR_FMER_Msk  (0x2UL) /* Form Error */
#define CAN_ESR_STER_Pos  (2U)
#define CAN_ESR_STER_Msk  (0x4UL) /* Stuff Error */
#define CAN_ESR_BERR_Pos  (3U)
#define CAN_ESR_BERR_Msk  (0x8UL) /* Bit Error */
#define CAN_ESR_ACKER_Pos (4U)
#define CAN_ESR_ACKER_Msk (0x10UL) /* ACK Error */

#define CAN_SR_CONFIG_Pos (0U)
#define CAN_SR_CONFIG_Msk (0x1UL) /* Configuration Mode */
#define CAN_SR_LBACK_Pos  (1U)
#define CAN_SR_LBACK_Msk  (0x2UL) /* Loop Back Mode */
#define CAN_SR_SLEEP_Pos  (2U)
#define CAN_SR_SLEEP_Msk  (0x4UL) /* Sleep Mode */
#define CAN_SR_NORMAL_Pos (3U)
#define CAN_SR_NORMAL_Msk (0x8UL) /* Normal Mode */
#define CAN_SR_BIDLE_Pos  (4U)
#define CAN_SR_BIDLE_Msk  (0x10UL) /* Bus Idle */
#define CAN_SR_BBSY_Pos   (5U)
#define CAN_SR_BBSY_Msk   (0x20UL) /* Bus Busy */
#define CAN_SR_ERRWRN_Pos (6U)
#define CAN_SR_ERRWRN_Msk (0x40UL) /* Error Warning */
#define CAN_SR_ESTAT_Pos  (7U)
#define CAN_SR_ESTAT_Msk  (0x180UL) /* Error Status */
#define CAN_SR_TXBFLL_Pos (9U)
#define CAN_SR_TXBFLL_Msk (0x200UL) /* TX High Priority Buffer full */
#define CAN_SR_TXFLL_Pos  (10U)
#define CAN_SR_TXFLL_Msk  (0x400UL) /* TX FIFO is full */
#define CAN_SR_ACFBSY_Pos (11U)
#define CAN_SR_ACFBSY_Msk (0x800UL) /* Acceptance Filter busy */
#define CAN_SR_SNOOP_Pos  (12U)
#define CAN_SR_SNOOP_Msk  (0x1000UL) /* Snoop Mask */

#define CAN_IXR_ARBLST_Pos   (0U)
#define CAN_IXR_ARBLST_Msk   (0x1UL) /* Arbitration Lost Interrupt */
#define CAN_IXR_TXOK_Pos     (1U)
#define CAN_IXR_TXOK_Msk     (0x2UL) /* TX Successful Interrupt */
#define CAN_IXR_TXFLL_Pos    (2U)
#define CAN_IXR_TXFLL_Msk    (0x4UL) /* TX FIFO Full Interrupt */
#define CAN_IXR_TXBFLL_Pos   (3U)
#define CAN_IXR_TXBFLL_Msk   (0x8UL) /* TX High Priority Buf Full */
#define CAN_IXR_RXOK_Pos     (4U)
#define CAN_IXR_RXOK_Msk     (0x10UL) /* New Message Received Intr */
#define CAN_IXR_RXUFLW_Pos   (5U)
#define CAN_IXR_RXUFLW_Msk   (0x20UL) /* RX FIFO Underflow Interrupt */
#define CAN_IXR_RXOFLW_Pos   (6U)
#define CAN_IXR_RXOFLW_Msk   (0x40UL) /* RX FIFO Overflow Interrupt */
#define CAN_IXR_RXNEMP_Pos   (7U)
#define CAN_IXR_RXNEMP_Msk   (0x80UL) /* RX FIFO Not Empty Interrupt */
#define CAN_IXR_ERROR_Pos    (8U)
#define CAN_IXR_ERROR_Msk    (0x100UL) /* Error Interrupt */
#define CAN_IXR_BSOFF_Pos    (9U)
#define CAN_IXR_BSOFF_Msk    (0x200UL) /* Bus Off Interrupt */
#define CAN_IXR_SLP_Pos      (10U)
#define CAN_IXR_SLP_Msk      (0x400UL) /* Sleep Interrupt */
#define CAN_IXR_WKUP_Pos     (11U)
#define CAN_IXR_WKUP_Msk     (0x800UL) /* Wake up Interrupt */
#define CAN_IXR_RXFWMFLL_Pos (12U)
#define CAN_IXR_RXFWMFLL_Msk (0x1000UL) /* Rx FIFO Watermark Full */
#define CAN_IXR_TXFWMEMP_Pos (13U)
#define CAN_IXR_TXFWMEMP_Msk (0x2000UL) /* Tx Fifo Watermark Empty */
#define CAN_IXR_TXFEMP_Pos   (14U)
#define CAN_IXR_TXFEMP_Msk   (0x4000UL) /* Tx Fifo Empty Interrupt */

#define CAN_TCR_CTS_Pos (0U)
#define CAN_TCR_CTS_Msk (0x1UL) /* Clear Timestamp counter mask */

#define CAN_WIR_FW_Pos (0U)
#define CAN_WIR_FW_Msk (0x3FUL) /* Rx Full Threshold mask */
#define CAN_WIR_EW_Pos (8U)
#define CAN_WIR_EW_Msk (0x3F00UL) /* Tx Empty Threshold mask */

#define CAN_IDR_RTR_Pos (0U)
#define CAN_IDR_RTR_Msk (0x1UL) /* Remote TX Request */
#define CAN_IDR_ID2_Pos (1U)
#define CAN_IDR_ID2_Msk (0x7FFFEUL) /* Extended Message Ident */
#define CAN_IDR_IDE_Pos (19U)
#define CAN_IDR_IDE_Msk (0x80000UL) /* Identifier Extension */
#define CAN_IDR_SRR_Pos (20U)
#define CAN_IDR_SRR_Msk (0x100000UL) /* Substitute Remote TX Req */
#define CAN_IDR_ID1_Pos (21U)
#define CAN_IDR_ID1_Msk (0xFFE00000UL) /* Standard Messg Identifier */

#define CAN_DLCR_TIMESTAMP_Pos (0U)
#define CAN_DLCR_TIMESTAMP_Msk (0xFFFFUL) /* Timestamp Mask (Rx only) */
#define CAN_DLCR_DLC_Pos       (28U)
#define CAN_DLCR_DLC_Msk       (0xF0000000UL) /* Data Length Code */

#define CAN_DW1R_DB3_Pos (0U)
#define CAN_DW1R_DB3_Msk (0xFFUL) /* Data Byte 3 */
#define CAN_DW1R_DB2_Pos (8U)
#define CAN_DW1R_DB2_Msk (0xFF00UL) /* Data Byte 2 */
#define CAN_DW1R_DB1_Pos (16U)
#define CAN_DW1R_DB1_Msk (0xFF0000UL) /* Data Byte 1 */
#define CAN_DW1R_DB0_Pos (24U)
#define CAN_DW1R_DB0_Msk (0xFF000000UL) /* Data Byte 0 */

#define CAN_DW2R_DB7_Pos (0U)
#define CAN_DW2R_DB7_Msk (0xFFUL) /* Data Byte 7 */
#define CAN_DW2R_DB6_Pos (8U)
#define CAN_DW2R_DB6_Msk (0xFF00UL) /* Data Byte 6 */
#define CAN_DW2R_DB5_Pos (16U)
#define CAN_DW2R_DB5_Msk (0xFF0000UL) /* Data Byte 5 */
#define CAN_DW2R_DB4_Pos (24U)
#define CAN_DW2R_DB4_Msk (0xFF000000UL) /* Data Byte 4 */

#define CAN_AFR_UAF1_Pos (0U)
#define CAN_AFR_UAF1_Msk (0x1UL) /* Use Acceptance Filter No.1 */
#define CAN_AFR_UAF2_Pos (1U)
#define CAN_AFR_UAF2_Msk (0x2UL) /* Use Acceptance Filter No.2 */
#define CAN_AFR_UAF3_Pos (2U)
#define CAN_AFR_UAF3_Msk (0x4UL) /* Use Acceptance Filter No.3 */
#define CAN_AFR_UAF4_Pos (3U)
#define CAN_AFR_UAF4_Msk (0x8UL) /* Use Acceptance Filter No.4 */

/*****************  Bit definitions for TTC  *****************/
#define TTC_CLK_CNTRL_PS_EN_Pos    (0U)
#define TTC_CLK_CNTRL_PS_EN_Msk    (0x1UL)  /* Prescale enable */
#define TTC_CLK_CNTRL_PS_DISABLE   (0x10UL) /* Prescale disable */
#define TTC_CLK_CNTRL_PS_VAL_Pos   (1U)
#define TTC_CLK_CNTRL_PS_VAL_Msk   (0x1EUL) /* Prescale value */
#define TTC_CLK_CNTRL_SRC_Pos      (5U)
#define TTC_CLK_CNTRL_SRC_Msk      (0x20UL) /* Clock source */
#define TTC_CLK_CNTRL_EXT_EDGE_Pos (6U)
#define TTC_CLK_CNTRL_EXT_EDGE_Msk (0x40UL) /* External Clock edge */

#define TTC_CNT_CNTRL_DIS_Pos      (0U)
#define TTC_CNT_CNTRL_DIS_Msk      (0x1UL) /* Disable the counter */
#define TTC_CNT_CNTRL_INT_Pos      (1U)
#define TTC_CNT_CNTRL_INT_Msk      (0x2UL) /* Interval mode */
#define TTC_CNT_CNTRL_DECR_Pos     (2U)
#define TTC_CNT_CNTRL_DECR_Msk     (0x4UL) /* Decrement mode */
#define TTC_CNT_CNTRL_MATCH_Pos    (3U)
#define TTC_CNT_CNTRL_MATCH_Msk    (0x8UL) /* Match mode */
#define TTC_CNT_CNTRL_RST_Pos      (4U)
#define TTC_CNT_CNTRL_RST_Msk      (0x10UL) /* Reset counter */
#define TTC_CNT_CNTRL_EN_WAVE_Pos  (5U)
#define TTC_CNT_CNTRL_EN_WAVE_Msk  (0x20UL) /* Enable waveform */
#define TTC_CNT_CNTRL_RESET_VALUE  (0x21UL) /* Reset value */
#define TTC_CNT_CNTRL_POL_WAVE_Pos (6U)
#define TTC_CNT_CNTRL_POL_WAVE_Msk (0x40UL) /* Waveform polarity */

#define TTC_COUNT_VALUE_Pos (0U)
#define TTC_COUNT_VALUE_Msk (0xFFFFFFFFUL) /* 32-bit counter value */

#define TTC_INTERVAL_VAL_Pos (0U)
#define TTC_INTERVAL_VAL_Msk (0xFFFFFFFFUL) /* 32-bit Interval value */

#define TTC_MATCH_MASK_Pos (0U)
#define TTC_MATCH_MASK_Msk (0xFFFFFFFFUL) /* 32-bit Match value */

#define TTC_NUM_MATCH_REG (0x3UL) /* Num of Match reg */

#define TTC_IXR_INTERVAL_Pos (0U)
#define TTC_IXR_INTERVAL_Msk (0x1UL) /* Interval Interrupt */
#define TTC_IXR_MATCH_0_Pos  (1U)
#define TTC_IXR_MATCH_0_Msk  (0x2UL) /* Match 1 Interrupt */
#define TTC_IXR_MATCH_1_Pos  (2U)
#define TTC_IXR_MATCH_1_Msk  (0x4UL) /* Match 2 Interrupt */
#define TTC_IXR_MATCH_2_Pos  (3U)
#define TTC_IXR_MATCH_2_Msk  (0x8UL) /* Match 3 Interrupt */
#define TTC_IXR_CNT_OVR_Pos  (4U)
#define TTC_IXR_CNT_OVR_Msk  (0x10UL) /* Counter Overflow */
#define TTC_IXR_ALL_Pos      (0U)
#define TTC_IXR_ALL_Msk      (0x1FUL) /* All valid Interrupts */

/*****************  Bit definitions for GIC  *****************/
#define GIC_NUM_OF_CORES_PER_CLUSTER (0x4UL) /*  */
#define GIC_NUM_INT_Pos              (0U)
#define GIC_NUM_INT_Msk              (0x1FUL) /* Number of Interrupt IDs */

#define GIC_EN_INT_Pos (0U)
#define GIC_EN_INT_Msk (0x1UL) /* Interrupt In Enable */

#define GIC_DOMAIN_MASK_Pos (10U)
#define GIC_DOMAIN_MASK_Msk (0x400UL) /* Number os Security domains */

#define GIC_CPU_NUM_Pos (5U)
#define GIC_CPU_NUM_Msk (0xE0UL) /* Number of CPU Interfaces */

#define GIC_REV_MASK_Pos (12U)
#define GIC_REV_MASK_Msk (0xFFF000UL) /* Revision Number */

#define GIC_IMPL_MASK_Pos (0U)
#define GIC_IMPL_MASK_Msk (0xFFFUL) /* Implementor */

#define GIC_SPI_CPU0_Pos (0U)
#define GIC_SPI_CPU0_Msk (0x1UL) /* CPU 0 Mask */
#define GIC_SPI_CPU1_Pos (1U)
#define GIC_SPI_CPU1_Msk (0x2UL) /* CPU 1 Mask */
#define GIC_SPI_CPU2_Pos (2U)
#define GIC_SPI_CPU2_Msk (0x4UL) /* CPU 2 Mask */
#define GIC_SPI_CPU3_Pos (3U)
#define GIC_SPI_CPU3_Msk (0x8UL) /* CPU 3 Mask */
#define GIC_SPI_CPU4_Pos (4U)
#define GIC_SPI_CPU4_Msk (0x10UL) /* CPU 4 Mask */
#define GIC_SPI_CPU5_Pos (5U)
#define GIC_SPI_CPU5_Msk (0x20UL) /* CPU 5 Mask */
#define GIC_SPI_CPU6_Pos (6U)
#define GIC_SPI_CPU6_Msk (0x40UL) /* CPU 6 Mask */
#define GIC_SPI_CPU7_Pos (7U)
#define GIC_SPI_CPU7_Msk (0x80UL) /* CPU 7 Mask */

#define GIC_INT_CFG_Pos (0U)
#define GIC_INT_CFG_Msk (0x3UL) /* Interrupt configuration Mask */

#define GIC_PPI_C00_Pos (0U)
#define GIC_PPI_C00_Msk (0x1UL) /* PPI Status */
#define GIC_PPI_C01_Pos (1U)
#define GIC_PPI_C01_Msk (0x2UL) /* PPI Status */
#define GIC_PPI_C02_Pos (2U)
#define GIC_PPI_C02_Msk (0x4UL) /* PPI Status */
#define GIC_PPI_C03_Pos (3U)
#define GIC_PPI_C03_Msk (0x8UL) /* PPI Status */
#define GIC_PPI_C04_Pos (4U)
#define GIC_PPI_C04_Msk (0x10UL) /* PPI Status */
#define GIC_PPI_C05_Pos (5U)
#define GIC_PPI_C05_Msk (0x20UL) /* PPI Status */
#define GIC_PPI_C06_Pos (6U)
#define GIC_PPI_C06_Msk (0x40UL) /* PPI Status */
#define GIC_PPI_C07_Pos (7U)
#define GIC_PPI_C07_Msk (0x80UL) /* PPI Status */
#define GIC_PPI_C08_Pos (8U)
#define GIC_PPI_C08_Msk (0x100UL) /* PPI Status */
#define GIC_PPI_C09_Pos (9U)
#define GIC_PPI_C09_Msk (0x200UL) /* PPI Status */
#define GIC_PPI_C10_Pos (10U)
#define GIC_PPI_C10_Msk (0x400UL) /* PPI Status */
#define GIC_PPI_C11_Pos (11U)
#define GIC_PPI_C11_Msk (0x800UL) /* PPI Status */
#define GIC_PPI_C12_Pos (12U)
#define GIC_PPI_C12_Msk (0x1000UL) /* PPI Status */
#define GIC_PPI_C13_Pos (13U)
#define GIC_PPI_C13_Msk (0x2000UL) /* PPI Status */
#define GIC_PPI_C14_Pos (14U)
#define GIC_PPI_C14_Msk (0x4000UL) /* PPI Status */
#define GIC_PPI_C15_Pos (15U)
#define GIC_PPI_C15_Msk (0x8000UL) /* PPI Status */

#define GIC_AHB_TIE_OFF_Pos (0U)
#define GIC_AHB_TIE_OFF_Msk (0x1UL) /* State of CFGBIGEND */

#define GIC_SFI_TRIG_SATT_Pos (15U)
#define GIC_SFI_TRIG_SATT_Msk (0x8000UL) /* 0= Use a secure interrupt */
#define GIC_SFI_TRIG_CPU_Pos  (16U)
#define GIC_SFI_TRIG_CPU_Msk  (0xFF0000UL)  /* CPU Target list */
#define GIC_SFI_SELFTRIG_Msk  (0x2010000UL) /*  (non-contiguous) */

#define GIC_CNTR_EN_S_Pos   (0U)
#define GIC_CNTR_EN_S_Msk   (0x1UL) /* Secure enable, 0=Disabled, 1=Enabled */
#define GIC_CNTR_EN_NS_Pos  (1U)
#define GIC_CNTR_EN_NS_Msk  (0x2UL) /* Non Secure enable */
#define GIC_CNTR_ACKCTL_Pos (2U)
#define GIC_CNTR_ACKCTL_Msk (0x4UL) /* Ack control for secure or non secure */

#define GIC_ACK_INTID_Pos (0U)
#define GIC_ACK_INTID_Msk (0x3FFUL) /* Interrupt ID */

#define GIC_CPUID_MASK_Pos (10U)
#define GIC_CPUID_MASK_Msk (0xC00UL) /* CPU ID */

#define GIC_EOI_INTID_Pos (0U)
#define GIC_EOI_INTID_Msk (0x3FFUL) /* Interrupt ID */

#define GIC_RUN_PRIORITY_Pos (0U)
#define GIC_RUN_PRIORITY_Msk (0xFFUL) /* Interrupt Priority */

#define GIC_PEND_INTID_Pos (0U)
#define GIC_PEND_INTID_Msk (0x3FFUL) /* Pending Interrupt ID */

#define GIC_RDIST_PWRR_RDPD_Pos             (0U)
#define GIC_RDIST_PWRR_RDPD_Msk             (0x1UL) /*  */
#define GIC_RDIST_WAKER_LOW_POWER_STATE_Pos (0U)
#define GIC_RDIST_WAKER_LOW_POWER_STATE_Msk (0x7UL)        /*  */
#define GIC_RDIST_START_ADDR                (0xF9080000UL) /*  */
#define GIC_RDIST_END_ADDR                  (0xF90B0000UL) /*  */

#define GIC_DEFAULT_SECURITY (0xFFFFFFFFUL) /*  */

/*****************  Bit definitions for XADC  *****************/
#define XADC_CFG_TCKRATE_Pos (8U)
#define XADC_CFG_TCKRATE_Msk (0x300UL) /* Clock freq control */
#define XADC_CFG_REDGE_Pos   (12U)
#define XADC_CFG_REDGE_Msk   (0x1000UL) /* Read Edge Mask */
#define XADC_CFG_WEDGE_Pos   (13U)
#define XADC_CFG_WEDGE_Msk   (0x2000UL) /* Write Edge Mask */
#define XADC_CFG_DFIFOTH_Pos (16U)
#define XADC_CFG_DFIFOTH_Msk (0xF0000UL) /* Data FIFO Threshold mask */
#define XADC_CFG_CFIFOTH_Pos (20U)
#define XADC_CFG_CFIFOTH_Msk (0xF00000UL) /* Command FIFO Threshold mask */
#define XADC_CFG_ENABLE_Pos  (31U)
#define XADC_CFG_ENABLE_Msk  (0x80000000UL) /* Enable access from PS mask */

#define XADC_INTX_ALM0_Pos      (0U)
#define XADC_INTX_ALM0_Msk      (0x1UL) /* Alarm 0 Mask */
#define XADC_INTX_ALM1_Pos      (1U)
#define XADC_INTX_ALM1_Msk      (0x2UL) /* Alarm 1 Mask */
#define XADC_INTX_ALM2_Pos      (2U)
#define XADC_INTX_ALM2_Msk      (0x4UL) /* Alarm 2 Mask */
#define XADC_INTX_ALM3_Pos      (3U)
#define XADC_INTX_ALM3_Msk      (0x8UL) /* Alarm 3 Mask */
#define XADC_INTX_ALM4_Pos      (4U)
#define XADC_INTX_ALM4_Msk      (0x10UL) /* Alarm 4 Mask */
#define XADC_INTX_ALM5_Pos      (5U)
#define XADC_INTX_ALM5_Msk      (0x20UL) /* Alarm 5 Mask */
#define XADC_INTX_ALM6_Pos      (6U)
#define XADC_INTX_ALM6_Msk      (0x40UL) /* Alarm 6 Mask */
#define XADC_INTX_ALM_ALL_Pos   (0U)
#define XADC_INTX_ALM_ALL_Msk   (0x7FUL) /* Alarm Signals Mask */
#define XADC_INTX_OT_Pos        (7U)
#define XADC_INTX_OT_Msk        (0x80UL) /* Over temperature Alarm Status */
#define XADC_INTX_DFIFO_GTH_Pos (8U)
#define XADC_INTX_DFIFO_GTH_Msk (0x100UL) /* Data FIFO greater than threshold */
#define XADC_INTX_CFIFO_LTH_Pos (9U)
#define XADC_INTX_CFIFO_LTH_Msk (0x200UL) /* CMD FIFO less than threshold */
#define XADC_INTX_ALL_Pos       (0U)
#define XADC_INTX_ALL_Msk       (0x3FFUL) /* Alarm Signals Mask */

#define XADC_MSTS_ALM_Pos       (0U)
#define XADC_MSTS_ALM_Msk       (0x7FUL) /* Alarms Mask */
#define XADC_MSTS_OT_Pos        (7U)
#define XADC_MSTS_OT_Msk        (0x80UL) /* Over Temperature Mask */
#define XADC_MSTS_DFIFOE_Pos    (8U)
#define XADC_MSTS_DFIFOE_Msk    (0x100UL) /* Data FIFO Empty Mask */
#define XADC_MSTS_DFIFOF_Pos    (9U)
#define XADC_MSTS_DFIFOF_Msk    (0x200UL) /* Data FIFO Full Mask */
#define XADC_MSTS_CFIFOE_Pos    (10U)
#define XADC_MSTS_CFIFOE_Msk    (0x400UL) /* Command FIFO Empty Mask */
#define XADC_MSTS_CFIFOF_Pos    (11U)
#define XADC_MSTS_CFIFOF_Msk    (0x800UL) /* Command FIFO Full Mask */
#define XADC_MSTS_DFIFO_LVL_Pos (12U)
#define XADC_MSTS_DFIFO_LVL_Msk (0xF000UL) /* Data FIFO Level Mask */
#define XADC_MSTS_CFIFO_LVL_Pos (16U)
#define XADC_MSTS_CFIFO_LVL_Msk (0xF0000UL) /* Command FIFO Level mask */

#define XADC_MCTL_FLUSH_Pos (0U)
#define XADC_MCTL_FLUSH_Msk (0x1UL) /* Flush the FIFOs */
#define XADC_MCTL_RESET_Pos (4U)
#define XADC_MCTL_RESET_Msk (0x10UL) /* Reset XADC */

#define XADC_CFR0_AVG1_Msk      (0x0UL) /* No Averaging (non-contiguous) */
#define XADC_CFR0_CHANNEL_Pos   (0U)
#define XADC_CFR0_CHANNEL_Msk   (0x1FUL) /* Channel number bit Mask */
#define XADC_CFR0_ACQ_Pos       (8U)
#define XADC_CFR0_ACQ_Msk       (0x100UL) /* Add acquisition by 6 ADCCLK */
#define XADC_CFR0_DU_Pos        (10U)
#define XADC_CFR0_DU_Msk        (0x400UL) /* Bipolar/Unipolar mode */
#define XADC_CFR0_MUX_Pos       (11U)
#define XADC_CFR0_MUX_Msk       (0x800UL) /* External Mask Enable */
#define XADC_CFR0_AVG16_Pos     (12U)
#define XADC_CFR0_AVG16_Msk     (0x1000UL) /* Average 16 samples */
#define XADC_CFR0_AVG64_Pos     (13U)
#define XADC_CFR0_AVG64_Msk     (0x2000UL) /* Average 64 samples */
#define XADC_CFR0_AVG256_Pos    (12U)
#define XADC_CFR0_AVG256_Msk    (0x3000UL) /* Average 256 samples */
#define XADC_CFR0_AVG_VALID_Pos (12U)
#define XADC_CFR0_AVG_VALID_Msk (0x3000UL) /* Averaging bit Mask */
#define XADC_CFR0_CAL_AVG_Pos   (15U)
#define XADC_CFR0_CAL_AVG_Msk   (0x8000UL) /* Averaging enable Mask */

#define XADC_CFR1_CAL_DISABLE_Msk        (0x0UL) /* No Calibration (non-contiguous) */
#define XADC_CFR1_SEQ_SAFEMODE_Msk       (0x0UL) /* Default Safe Mode (non-contiguous) */
#define XADC_CFR1_OT_Pos                 (0U)
#define XADC_CFR1_OT_Msk                 (0x1UL) /* Over Temperature Enable */
#define XADC_CFR1_ALM_TEMP_Pos           (1U)
#define XADC_CFR1_ALM_TEMP_Msk           (0x2UL) /* Alarm 0 - Temperature */
#define XADC_CFR1_ALM_VCCINT_Pos         (2U)
#define XADC_CFR1_ALM_VCCINT_Msk         (0x4UL) /* Alarm 1 - VCCINT Enable */
#define XADC_CFR1_ALM_VCCAUX_Pos         (3U)
#define XADC_CFR1_ALM_VCCAUX_Msk         (0x8UL) /* Alarm 2 - VCCAUX Enable */
#define XADC_CFR1_CAL_VALID_Pos          (4U)
#define XADC_CFR1_CAL_VALID_Msk          (0xF0UL) /* Valid Calibration Mask */
#define XADC_CFR1_ALM_VBRAM_Pos          (8U)
#define XADC_CFR1_ALM_VBRAM_Msk          (0x100UL) /* Alm 3 - VBRAM, 7 series */
#define XADC_CFR1_ALM_VCCPINT_Pos        (9U)
#define XADC_CFR1_ALM_VCCPINT_Msk        (0x200UL) /* Alm 4 - VCCPINT, Zynq */
#define XADC_CFR1_ALM_VCCPAUX_Pos        (10U)
#define XADC_CFR1_ALM_VCCPAUX_Msk        (0x400UL) /* Alm 5 - VCCPAUX, Zynq */
#define XADC_CFR1_ALM_VCCPDRO_Pos        (11U)
#define XADC_CFR1_ALM_VCCPDRO_Msk        (0x800UL) /* Alm 6 - VCCPDRO, Zynq */
#define XADC_CFR1_ALM_ALL_Msk            (0xF0FUL) /* Mask for all alarms (non-contiguous) */
#define XADC_CFR1_SEQ_ONEPASS_Pos        (12U)
#define XADC_CFR1_SEQ_ONEPASS_Msk        (0x1000UL) /* Onepass through Seq */
#define XADC_CFR1_SEQ_CONTINPASS_Pos     (13U)
#define XADC_CFR1_SEQ_CONTINPASS_Msk     (0x2000UL) /* Continuous Cycling Seq */
#define XADC_CFR1_SEQ_SINGCHAN_Pos       (12U)
#define XADC_CFR1_SEQ_SINGCHAN_Msk       (0x3000UL) /* Single channel - No Seq */
#define XADC_CFR1_SEQ_SIMUL_SAMPLING_Pos (14U)
#define XADC_CFR1_SEQ_SIMUL_SAMPLING_Msk (0x4000UL) /* Simulataneous Sampling Mask */
#define XADC_CFR1_SEQ_INDEPENDENT_Pos    (15U)
#define XADC_CFR1_SEQ_INDEPENDENT_Msk    (0x8000UL) /* Independent Mode */
#define XADC_CFR1_SEQ_VALID_Pos          (12U)
#define XADC_CFR1_SEQ_VALID_Msk          (0xF000UL) /* Sequence bit Mask */

#define XADC_CFR2_CD_MIN       (0x8UL) /* Minimum value of divisor */
#define XADC_CFR2_PD_ADC1_Pos  (5U)
#define XADC_CFR2_PD_ADC1_Msk  (0x20UL) /* Power Down ADC1 Mask */
#define XADC_CFR2_PD_Pos       (4U)
#define XADC_CFR2_PD_Msk       (0x30UL) /* Power Down Mask */
#define XADC_CFR2_PD_XADC_Pos  (4U)
#define XADC_CFR2_PD_XADC_Msk  (0x30UL) /* Power Down XADC Mask */
#define XADC_CFR2_CD_MAX       (0xFFUL) /* Maximum value of divisor */
#define XADC_CFR2_CD_VALID_Pos (8U)
#define XADC_CFR2_CD_VALID_Msk (0xFF00UL) /* Clock Divisor bit Mask */

#define XADC_SEQ_CH_CALIB   (0x1UL)        /* ADC Calibration Channel */
#define XADC_SEQ_CH_VCCPINT (0x20UL)       /* VCCPINT, Zynq Only */
#define XADC_SEQ_CH_VCCPAUX (0x40UL)       /* VCCPAUX, Zynq Only */
#define XADC_SEQ_CH_VCCPDRO (0x80UL)       /* VCCPDRO, Zynq Only */
#define XADC_SEQ_CH_TEMP    (0x100UL)      /* On Chip Temperature Channel */
#define XADC_SEQ_CH_VCCINT  (0x200UL)      /* VCCINT Channel */
#define XADC_SEQ_CH_VCCAUX  (0x400UL)      /* VCCAUX Channel */
#define XADC_SEQ_CH_VPVN    (0x800UL)      /* VP/VN analog inputs Channel */
#define XADC_SEQ_CH_VREFP   (0x1000UL)     /* VREFP Channel */
#define XADC_SEQ_CH_VREFN   (0x2000UL)     /* VREFN Channel */
#define XADC_SEQ_CH_VBRAM   (0x4000UL)     /* VBRAM Channel, 7 series */
#define XADC_SEQ_CH_AUX00   (0x10000UL)    /* 1st Aux Channel */
#define XADC_SEQ_CH_AUX01   (0x20000UL)    /* 2nd Aux Channel */
#define XADC_SEQ_CH_AUX02   (0x40000UL)    /* 3rd Aux Channel */
#define XADC_SEQ_CH_AUX03   (0x80000UL)    /* 4th Aux Channel */
#define XADC_SEQ_CH_AUX04   (0x100000UL)   /* 5th Aux Channel */
#define XADC_SEQ_CH_AUX05   (0x200000UL)   /* 6th Aux Channel */
#define XADC_SEQ_CH_AUX06   (0x400000UL)   /* 7th Aux Channel */
#define XADC_SEQ_CH_AUX07   (0x800000UL)   /* 8th Aux Channel */
#define XADC_SEQ_CH_AUX08   (0x1000000UL)  /* 9th Aux Channel */
#define XADC_SEQ_CH_AUX09   (0x2000000UL)  /* 10th Aux Channel */
#define XADC_SEQ_CH_AUX10   (0x4000000UL)  /* 11th Aux Channel */
#define XADC_SEQ_CH_AUX11   (0x8000000UL)  /* 12th Aux Channel */
#define XADC_SEQ_CH_AUX12   (0x10000000UL) /* 13th Aux Channel */
#define XADC_SEQ_CH_AUX13   (0x20000000UL) /* 14th Aux Channel */
#define XADC_SEQ_CH_AUX14   (0x40000000UL) /* 15th Aux Channel */
#define XADC_SEQ_CH_AUX15   (0x80000000UL) /* 16th Aux Channel */

#define XADC_SEQ00_CH_VALID_Msk (0x7FE1UL) /* Mask for the valid channels (non-contiguous) */

#define XADC_SEQ01_CH_VALID_Pos (0U)
#define XADC_SEQ01_CH_VALID_Msk (0xFFFFUL) /* Mask for the valid channels */

#define XADC_SEQ02_CH_VALID_Pos (5U)
#define XADC_SEQ02_CH_VALID_Msk (0x7FE0UL) /* Mask for the valid channels */

#define XADC_SEQ03_CH_VALID_Pos (0U)
#define XADC_SEQ03_CH_VALID_Msk (0xFFFFUL) /* Mask for the valid channels */

#define XADC_SEQ04_CH_VALID_Pos (11U)
#define XADC_SEQ04_CH_VALID_Msk (0x800UL) /* Mask for the valid channels */

#define XADC_SEQ05_CH_VALID_Pos (0U)
#define XADC_SEQ05_CH_VALID_Msk (0xFFFFUL) /* Mask for the valid channels */

#define XADC_SEQ06_CH_VALID_Pos (11U)
#define XADC_SEQ06_CH_VALID_Msk (0x800UL) /* Mask for the valid channels */

#define XADC_SEQ07_CH_VALID_Pos (0U)
#define XADC_SEQ07_CH_VALID_Msk (0xFFFFUL) /* Mask for the valid channels */

#define XADC_ATR_OT_UPPER_ENB_VAL (0x3UL) /* Value for OT enable */
#define XADC_ATR_OT_UPPER_ENB_Pos (0U)
#define XADC_ATR_OT_UPPER_ENB_Msk (0xFUL)   /* Mask for OT enable */
#define XADC_ATR_OT_UPPER_VAL_MAX (0xFFFUL) /* Max OT value */
#define XADC_ATR_OT_UPPER_VAL_Pos (4U)
#define XADC_ATR_OT_UPPER_VAL_Msk (0xFFF0UL) /* Mask for OT value */

#define XADC_JTAG_DATA_Pos      (0U)
#define XADC_JTAG_DATA_Msk      (0xFFFFUL) /* Mask for the Data */
#define XADC_JTAG_ADDR_Pos      (16U)
#define XADC_JTAG_ADDR_Msk      (0x3FF0000UL) /* Mask for the Addr */
#define XADC_JTAG_CMD_READ_Pos  (26U)
#define XADC_JTAG_CMD_READ_Msk  (0x4000000UL) /* Mask for CMD Read */
#define XADC_JTAG_CMD_WRITE_Pos (27U)
#define XADC_JTAG_CMD_WRITE_Msk (0x8000000UL) /* Mask for CMD Write */
#define XADC_JTAG_CMD_Pos       (26U)
#define XADC_JTAG_CMD_Msk       (0x3C000000UL) /* Mask for the Cmd */

/*****************  Bit definitions for DMAC  *****************/
#define DMAC_DS_DMA_STATUS_STOPPED (0x0UL) /* debug status busy mask */
#define DMAC_DS_DMA_STATUS         (0xFUL) /* DMA status mask */

#define DMAC_DBGSTATUS_BUSY (0x1UL) /* debug status busy mask */

#define DMAC_CR1_I_CACHE_LEN_Pos (0U)
#define DMAC_CR1_I_CACHE_LEN_Msk (0x7UL) /* i_cache_len mask */

#define DMAC_CHANNELS_PER_DEV (0x8UL) /*  */

#define DMAC_INTCLR_ALL_Pos (0U)
#define DMAC_INTCLR_ALL_Msk (0xFFUL) /*  */

/*****************  Bit definitions for DEVCFG  *****************/
#define DEVCFG_CTRL_DAP_EN_Pos          (0U)
#define DEVCFG_CTRL_DAP_EN_Msk          (0x7UL) /* DAP Enable Mask */
#define DEVCFG_CTRL_SEU_EN_Pos          (8U)
#define DEVCFG_CTRL_SEU_EN_Msk          (0x100UL) /* SEU Enable Mask */
#define DEVCFG_CTRL_PCFG_AES_EN_Pos     (9U)
#define DEVCFG_CTRL_PCFG_AES_EN_Msk     (0xE00UL) /* AES Enable Mask */
#define DEVCFG_CTRL_PCFG_AES_FUSE_Pos   (12U)
#define DEVCFG_CTRL_PCFG_AES_FUSE_Msk   (0x1000UL) /* AES key source */
#define DEVCFG_CTRL_USER_MODE_Pos       (15U)
#define DEVCFG_CTRL_USER_MODE_Msk       (0x8000UL) /* User Mode Mask */
#define DEVCFG_CTRL_JTAG_CHAIN_DIS_Pos  (23U)
#define DEVCFG_CTRL_JTAG_CHAIN_DIS_Msk  (0x800000UL) /* JTAG Chain Disable */
#define DEVCFG_CTRL_MULTIBOOT_EN_Pos    (24U)
#define DEVCFG_CTRL_MULTIBOOT_EN_Msk    (0x1000000UL) /* Multiboot Enable */
#define DEVCFG_CTRL_PCAP_RATE_EN_Pos    (25U)
#define DEVCFG_CTRL_PCAP_RATE_EN_Msk    (0x2000000UL) /* Quarter PCAP rate (AES) */
#define DEVCFG_CTRL_PCAP_MODE_Pos       (26U)
#define DEVCFG_CTRL_PCAP_MODE_Msk       (0x4000000UL) /* Enable PCAP */
#define DEVCFG_CTRL_PCAP_PR_Pos         (27U)
#define DEVCFG_CTRL_PCAP_PR_Msk         (0x8000000UL) /* Enable PCAP for PR */
#define DEVCFG_CTRL_PCFG_POR_CNT_4K_Pos (29U)
#define DEVCFG_CTRL_PCFG_POR_CNT_4K_Msk (0x20000000UL) /* Control PL POR timer */
#define DEVCFG_CTRL_PCFG_PROG_B_Pos     (30U)
#define DEVCFG_CTRL_PCFG_PROG_B_Msk     (0x40000000UL) /* PROG_B reset to the PL */
#define DEVCFG_CTRL_FORCE_RST_Pos       (31U)
#define DEVCFG_CTRL_FORCE_RST_Msk       (0x80000000UL) /* Secure lockdown */

#define DEVCFG_LOCK_SEU_Pos       (2U)
#define DEVCFG_LOCK_SEU_Msk       (0x4UL) /* Lock SEU_En update */
#define DEVCFG_LOCK_AES_EN_Pos    (3U)
#define DEVCFG_LOCK_AES_EN_Msk    (0x8UL) /* Lock AES_EN update */
#define DEVCFG_LOCK_AES_EFUSE_Pos (4U)
#define DEVCFG_LOCK_AES_EFUSE_Msk (0x10UL) /* Lock AES Efuse bit */

#define DEVCFG_IXR_PCFG_DONE_Pos     (2U)
#define DEVCFG_IXR_PCFG_DONE_Msk     (0x4UL) /* Done Signal  Mask */
#define DEVCFG_IXR_PCFG_CFG_RST_Pos  (3U)
#define DEVCFG_IXR_PCFG_CFG_RST_Msk  (0x8UL) /* FPGA Reset mask */
#define DEVCFG_IXR_PCFG_POR_B_Pos    (4U)
#define DEVCFG_IXR_PCFG_POR_B_Msk    (0x10UL) /* FPGA POR mask */
#define DEVCFG_IXR_PCFG_SEU_ERR_Pos  (5U)
#define DEVCFG_IXR_PCFG_SEU_ERR_Msk  (0x20UL) /* SEU Error mask */
#define DEVCFG_IXR_PCFG_HMAC_ERR_Pos (6U)
#define DEVCFG_IXR_PCFG_HMAC_ERR_Msk (0x40UL) /* HMAC error mask */
#define DEVCFG_IXR_DMA_DONE_Pos      (13U)
#define DEVCFG_IXR_DMA_DONE_Msk      (0x2000UL) /* DMA Command Done */
#define DEVCFG_IXR_P2D_LEN_ERR_Pos   (11U)
#define DEVCFG_IXR_P2D_LEN_ERR_Msk   (0x800UL) /* PCAP/DMA length mismatch */
#define DEVCFG_IXR_DMA_Q_OV_Pos      (14U)
#define DEVCFG_IXR_DMA_Q_OV_Msk      (0x4000UL) /* DMA command queue overflow */
#define DEVCFG_IXR_DMA_CMD_ERR_Pos   (15U)
#define DEVCFG_IXR_DMA_CMD_ERR_Msk   (0x8000UL) /* Illegal DMA command */
#define DEVCFG_IXR_RD_FIFO_LVL_Pos   (16U)
#define DEVCFG_IXR_RD_FIFO_LVL_Msk   (0x10000UL) /* Rx FIFO level */
#define DEVCFG_IXR_WR_FIFO_LVL_Pos   (17U)
#define DEVCFG_IXR_WR_FIFO_LVL_Msk   (0x20000UL) /* Tx FIFO level */
#define DEVCFG_IXR_RX_FIFO_OV_Pos    (18U)
#define DEVCFG_IXR_RX_FIFO_OV_Msk    (0x40000UL) /* Rx FIFO Overflow */
#define DEVCFG_IXR_AXI_RERR_Pos      (20U)
#define DEVCFG_IXR_AXI_RERR_Msk      (0x100000UL) /* AXI read response error */
#define DEVCFG_IXR_AXI_RTO_Pos       (21U)
#define DEVCFG_IXR_AXI_RTO_Msk       (0x200000UL) /* AXI read timeout */
#define DEVCFG_IXR_AXI_WERR_Pos      (22U)
#define DEVCFG_IXR_AXI_WERR_Msk      (0x400000UL) /* AXI write response error */
#define DEVCFG_IXR_AXI_WTO_Pos       (23U)
#define DEVCFG_IXR_AXI_WTO_Msk       (0x800000UL) /* AXI write timeout */
#define DEVCFG_IXR_ALL_Msk           (0xF7F8EFUL) /*  (non-contiguous) */
#define DEVCFG_IXR_PSS_GPWRDWN_B_Pos (29U)
#define DEVCFG_IXR_PSS_GPWRDWN_B_Msk (0x20000000UL) /* Global power down */

#define DEVCFG_STATUS_EFUSE_BBRAM_KEY_DISABLE_Pos (3U)
#define DEVCFG_STATUS_EFUSE_BBRAM_KEY_DISABLE_Msk (0x8UL) /*  */
#define DEVCFG_STATUS_PCFG_INIT_Pos               (4U)
#define DEVCFG_STATUS_PCFG_INIT_Msk               (0x10UL)  /* PL ready for PCAP data */
#define DEVCFG_STATUS_PSS_GPWRDWN_B               (0x200UL) /* Global power down */
#define DEVCFG_STATUS_DMA_DONE_CNT_Pos            (28U)
#define DEVCFG_STATUS_DMA_DONE_CNT_Msk            (0x30000000UL) /* Finished, unacknowledged DMA transfers */
#define DEVCFG_STATUS_TX_FIFO_LVL_Pos             (12U)
#define DEVCFG_STATUS_TX_FIFO_LVL_Msk             (0x7F000UL) /* Tx FIFO level */
#define DEVCFG_STATUS_RX_FIFO_LVL_Pos             (20U)
#define DEVCFG_STATUS_RX_FIFO_LVL_Msk             (0x1F00000UL) /* Rx FIFO level */
#define DEVCFG_STATUS_DMA_CMD_Q_E_Pos             (30U)
#define DEVCFG_STATUS_DMA_CMD_Q_E_Msk             (0x40000000UL) /* DMA command queue empty */
#define DEVCFG_STATUS_DMA_CMD_Q_F_Pos             (31U)
#define DEVCFG_STATUS_DMA_CMD_Q_F_Msk             (0x80000000UL) /* DMA command queue full */

#define DEVCFG_DMA_LEN_Pos         (0U)
#define DEVCFG_DMA_LEN_Msk         (0x7FFFFFFUL)  /* Length Mask */
#define DEVCFG_DMA_INVALID_ADDRESS (0xFFFFFFFFUL) /* Invalid DMA address */

#define DEVCFG_MCTRL_PCFG_POR_B_Pos      (8U)
#define DEVCFG_MCTRL_PCFG_POR_B_Msk      (0x100UL) /* PL power status: 1 = PL powered */
#define DEVCFG_MCTRL_PCAP_LPBK_Pos       (4U)
#define DEVCFG_MCTRL_PCAP_LPBK_Msk       (0x10UL) /* PCAP loopback mask */
#define DEVCFG_MCTRL_PCAP_PS_VERSION_Pos (28U)
#define DEVCFG_MCTRL_PCAP_PS_VERSION_Msk (0xF0000000UL) /* PS Version Mask */

#define DEVCFG_CFG_FIFO_QUARTER  (0x0UL) /* Quarter empty */
#define DEVCFG_CFG_FIFO_HALF     (0x1UL) /* Half empty */
#define DEVCFG_CFG_FIFO_3QUARTER (0x2UL) /* 3/4 empty */
#define DEVCFG_CFG_FIFO_EMPTY    (0x4UL) /* Empty */

#define DEVCFG_UNLOCK_DATA (0x757BDF0DUL) /* First APB access data */

#define DEVCFG_CONFIG_RESET_VALUE (0x508UL) /* Config reg reset value */

/*****************  Bit definitions for NAND  *****************/
#define NAND_REFRESH_PERIOD_0_Pos (0U)
#define NAND_REFRESH_PERIOD_0_Msk (0xFUL) /*  */
#define NAND_REFRESH_PERIOD_1_Pos (0U)
#define NAND_REFRESH_PERIOD_1_Msk (0xFUL) /*  */

#define NAND_OPMODE_MW_Pos            (0U)
#define NAND_OPMODE_MW_Msk            (0x3UL) /*  */
#define NAND_OPMODE_RD_SYNC_Pos       (2U)
#define NAND_OPMODE_RD_SYNC_Msk       (0x4UL) /*  */
#define NAND_OPMODE_RD_BL_Pos         (3U)
#define NAND_OPMODE_RD_BL_Msk         (0x38UL) /*  */
#define NAND_OPMODE_WR_SYNC_Pos       (6U)
#define NAND_OPMODE_WR_SYNC_Msk       (0x40UL) /*  */
#define NAND_OPMODE_WR_BL_Pos         (7U)
#define NAND_OPMODE_WR_BL_Msk         (0x380UL) /*  */
#define NAND_OPMODE_BAA_Pos           (10U)
#define NAND_OPMODE_BAA_Msk           (0x400UL) /*  */
#define NAND_OPMODE_ADV_Pos           (11U)
#define NAND_OPMODE_ADV_Msk           (0x800UL) /*  */
#define NAND_OPMODE_BLS_Pos           (12U)
#define NAND_OPMODE_BLS_Msk           (0x1000UL) /*  */
#define NAND_OPMODE_BURST_ALIGN_Pos   (13U)
#define NAND_OPMODE_BURST_ALIGN_Msk   (0xE000UL) /*  */
#define NAND_OPMODE_ADDRESS_Pos       (16U)
#define NAND_OPMODE_ADDRESS_Msk       (0xFF0000UL) /*  */
#define NAND_OPMODE_ADDRESS_MATCH_Pos (24U)
#define NAND_OPMODE_ADDRESS_MATCH_Msk (0xFF000000UL) /*  */

#define NAND_ECC_MEMCFG_PAGE_SIZE_Pos                    (0U)
#define NAND_ECC_MEMCFG_PAGE_SIZE_Msk                    (0x3UL) /*  */
#define NAND_ECC_MEMCFG_ECC_MODE_Pos                     (2U)
#define NAND_ECC_MEMCFG_ECC_MODE_Msk                     (0xCUL) /*  */
#define NAND_ECC_MEMCFG_ECC_READ_END_Pos                 (4U)
#define NAND_ECC_MEMCFG_ECC_READ_END_Msk                 (0x10UL) /*  */
#define NAND_ECC_MEMCFG_ECC_JUMP_Pos                     (5U)
#define NAND_ECC_MEMCFG_ECC_JUMP_Msk                     (0x60UL) /*  */
#define NAND_ECC_MEMCFG_IGNORE_ADD8_Pos                  (7U)
#define NAND_ECC_MEMCFG_IGNORE_ADD8_Msk                  (0x80UL) /*  */
#define NAND_ECC_MEMCOMMAND1_WR_CMD_Pos                  (0U)
#define NAND_ECC_MEMCOMMAND1_WR_CMD_Msk                  (0xFFUL) /*  */
#define NAND_ECC_MEMCOMMAND2_WR_COL_CHANGE_Pos           (0U)
#define NAND_ECC_MEMCOMMAND2_WR_COL_CHANGE_Msk           (0xFFUL) /*  */
#define NAND_ECC_MEMCFG_ECC_INT_PASS_Pos                 (8U)
#define NAND_ECC_MEMCFG_ECC_INT_PASS_Msk                 (0x100UL) /*  */
#define NAND_ECC_MEMCFG_ECC_INT_ABORT_Pos                (9U)
#define NAND_ECC_MEMCFG_ECC_INT_ABORT_Msk                (0x200UL) /*  */
#define NAND_ECC_MEMCFG_ECC_EXTRA_BLOCK_Pos              (10U)
#define NAND_ECC_MEMCFG_ECC_EXTRA_BLOCK_Msk              (0x400UL) /*  */
#define NAND_ECC_MEMCFG_ECC_EXTRA_BLOCK_SIZE_Pos         (11U)
#define NAND_ECC_MEMCFG_ECC_EXTRA_BLOCK_SIZE_Msk         (0x1800UL) /*  */
#define NAND_ECC_MEMCOMMAND1_RD_CMD_Pos                  (8U)
#define NAND_ECC_MEMCOMMAND1_RD_CMD_Msk                  (0xFF00UL) /*  */
#define NAND_ECC_MEMCOMMAND2_RD_COL_CHANGE_Pos           (8U)
#define NAND_ECC_MEMCOMMAND2_RD_COL_CHANGE_Msk           (0xFF00UL) /*  */
#define NAND_ECC_MEMCOMMAND1_RD_CMD_END_Pos              (16U)
#define NAND_ECC_MEMCOMMAND1_RD_CMD_END_Msk              (0xFF0000UL) /*  */
#define NAND_ECC_MEMCOMMAND2_RD_COL_CHANGE_END_Pos       (16U)
#define NAND_ECC_MEMCOMMAND2_RD_COL_CHANGE_END_Msk       (0xFF0000UL) /*  */
#define NAND_ECC_MEMCOMMAND2_RD_COL_CHANGE_END_VALID_Pos (16U)
#define NAND_ECC_MEMCOMMAND2_RD_COL_CHANGE_END_VALID_Msk (0xFF0000UL) /*  */
#define NAND_ECC_VALUE_Pos                               (0U)
#define NAND_ECC_VALUE_Msk                               (0xFFFFFFUL) /*  */
#define NAND_ECC_MEMCOMMAND1_RD_CMD_END_VALID_Pos        (24U)
#define NAND_ECC_MEMCOMMAND1_RD_CMD_END_VALID_Msk        (0x1000000UL) /*  */
#define NAND_ECC_VALUE_CORRECT_Pos                       (27U)
#define NAND_ECC_VALUE_CORRECT_Msk                       (0x8000000UL) /*  */
#define NAND_ECC_VALUE_FAIL_Pos                          (28U)
#define NAND_ECC_VALUE_FAIL_Msk                          (0x10000000UL) /*  */
#define NAND_ECC_VALUE_READ_Pos                          (29U)
#define NAND_ECC_VALUE_READ_Msk                          (0x20000000UL) /*  */
#define NAND_ECC_VALUE_VALID_Pos                         (30U)
#define NAND_ECC_VALUE_VALID_Msk                         (0x40000000UL) /*  */
#define NAND_ECC_VALUE_INT_Pos                           (31U)
#define NAND_ECC_VALUE_INT_Msk                           (0x80000000UL) /*  */

#define NAND_PERIPH_ID_PART_NUM_Pos    (0U)
#define NAND_PERIPH_ID_PART_NUM_Msk    (0xFFFUL) /*  */
#define NAND_PERIPH_ID_DESIGNER_ID_Pos (12U)
#define NAND_PERIPH_ID_DESIGNER_ID_Msk (0xFF000UL) /*  */
#define NAND_PERIPH_ID_REVISION_Pos    (20U)
#define NAND_PERIPH_ID_REVISION_Msk    (0xF00000UL) /*  */
#define NAND_PERIPH_ID_INTG_CFG_Pos    (24U)
#define NAND_PERIPH_ID_INTG_CFG_Msk    (0x1000000UL) /*  */

#define NAND_PCELL_ID_Pos (0U)
#define NAND_PCELL_ID_Msk (0xFFUL) /*  */

#endif /* ZYNQ7000_H */