#ifndef ZYNQ_MIO_H
#define ZYNQ_MIO_H

#include "zynq_soc.h"

#define MIO_PIN_COUNT (118U)

#define MIO_IRQ_TYPE_LEVEL_LOW    (0U)
#define MIO_IRQ_TYPE_LEVEL_HIGH   (1U)
#define MIO_IRQ_TYPE_EDGE_FALLING (2U)
#define MIO_IRQ_TYPE_EDGE_RISING  (3U)
#define MIO_IRQ_TYPE_EDGE_BOTH    (4U)

#define MIO_PAD_COUNT (54U)

#define MIO_BANK0_LVL MIO_PIN_OPT_IO_CMOS_3V3
#define MIO_BANK1_LVL MIO_PIN_OPT_IO_CMOS_3V3

#define MIO_FN_GPIO     (0UL)
#define MIO_FN_QSPI     (MIO_PIN_OPT_LVL01_MUX)
#define MIO_FN_ETH      (MIO_PIN_OPT_LVL01_MUX)
#define MIO_FN_USB      (MIO_PIN_OPT_LVL11_MUX)
#define MIO_FN_SMC_NOR  (MIO_PIN_OPT_LVL21_MUX)
#define MIO_FN_SMC_CS1  (MIO_PIN_OPT_LVL22_MUX)
#define MIO_FN_SMC_NAND (MIO_PIN_OPT_LVL22_MUX)
#define MIO_FN_SDIO_PC  (MIO_PIN_OPT_LVL23_MUX)
#define MIO_FN_CAN      (MIO_PIN_OPT_LVL31_MUX)
#define MIO_FN_I2C      (MIO_PIN_OPT_LVL32_MUX)
#define MIO_FN_SWDT     (MIO_PIN_OPT_LVL33_MUX)
#define MIO_FN_SDIO     (MIO_PIN_OPT_LVL34_MUX)
#define MIO_FN_MDIO0    (MIO_PIN_OPT_LVL34_MUX)
#define MIO_FN_MDIO1    (MIO_PIN_OPT_LVL35_MUX)
#define MIO_FN_SPI      (MIO_PIN_OPT_LVL35_MUX)
#define MIO_FN_TTC      (MIO_PIN_OPT_LVL36_MUX)
#define MIO_FN_UART     (MIO_PIN_OPT_LVL37_MUX)

#define MIO0_GPIO     MIO_FN_GPIO
#define MIO0_QSPI_CS1 MIO_FN_QSPI
#define MIO0_SMC_NOR  MIO_FN_SMC_NOR
#define MIO0_SMC_NAND MIO_FN_SMC_NAND
#define MIO0_SDIO0_PC MIO_FN_SDIO_PC

#define MIO1_GPIO           MIO_FN_GPIO
#define MIO1_QSPI0          MIO_FN_QSPI
#define MIO1_SMC_NOR_ADDR25 MIO_FN_SMC_NOR
#define MIO1_SMC_NOR_CS1    MIO_FN_SMC_CS1
#define MIO1_SDIO1_PC       MIO_FN_SDIO_PC

#define MIO2_GPIO     MIO_FN_GPIO
#define MIO2_QSPI0    MIO_FN_QSPI
#define MIO2_SMC_NAND MIO_FN_SMC_NAND
#define MIO2_SDIO0_PC MIO_FN_SDIO_PC

#define MIO3_GPIO     MIO_FN_GPIO
#define MIO3_QSPI0    MIO_FN_QSPI
#define MIO3_SMC_NOR  MIO_FN_SMC_NOR
#define MIO3_SMC_NAND MIO_FN_SMC_NAND
#define MIO3_SDIO1_PC MIO_FN_SDIO_PC

#define MIO4_GPIO     MIO_FN_GPIO
#define MIO4_QSPI0    MIO_FN_QSPI
#define MIO4_SMC_NOR  MIO_FN_SMC_NOR
#define MIO4_SMC_NAND MIO_FN_SMC_NAND
#define MIO4_SDIO0_PC MIO_FN_SDIO_PC

#define MIO5_GPIO     MIO_FN_GPIO
#define MIO5_QSPI0    MIO_FN_QSPI
#define MIO5_SMC_NOR  MIO_FN_SMC_NOR
#define MIO5_SMC_NAND MIO_FN_SMC_NAND
#define MIO5_SDIO1_PC MIO_FN_SDIO_PC

#define MIO6_GPIO     MIO_FN_GPIO
#define MIO6_QSPI0    MIO_FN_QSPI
#define MIO6_SMC_NOR  MIO_FN_SMC_NOR
#define MIO6_SMC_NAND MIO_FN_SMC_NAND
#define MIO6_SDIO0_PC MIO_FN_SDIO_PC

#define MIO7_GPIO     MIO_FN_GPIO
#define MIO7_SMC_NOR  MIO_FN_SMC_NOR
#define MIO7_SMC_NAND MIO_FN_SMC_NAND
#define MIO7_SDIO1_PC MIO_FN_SDIO_PC

#define MIO8_GPIO       MIO_FN_GPIO
#define MIO8_QSPI_FBCLK MIO_FN_QSPI
#define MIO8_SMC_NOR    MIO_FN_SMC_NOR
#define MIO8_SMC_NAND   MIO_FN_SMC_NAND
#define MIO8_SDIO0_PC   MIO_FN_SDIO_PC
#define MIO8_CAN1       MIO_FN_CAN
#define MIO8_UART1      MIO_FN_UART
#define MIO8_I2C1       MIO_FN_I2C

#define MIO9_GPIO     MIO_FN_GPIO
#define MIO9_QSPI1    MIO_FN_QSPI
#define MIO9_SMC_NOR  MIO_FN_SMC_NOR
#define MIO9_SMC_NAND MIO_FN_SMC_NAND
#define MIO9_SDIO1_PC MIO_FN_SDIO_PC
#define MIO9_CAN1     MIO_FN_CAN
#define MIO9_UART1    MIO_FN_UART
#define MIO9_I2C1     MIO_FN_I2C

#define MIO10_GPIO     MIO_FN_GPIO
#define MIO10_QSPI1    MIO_FN_QSPI
#define MIO10_SMC_NOR  MIO_FN_SMC_NOR
#define MIO10_SMC_NAND MIO_FN_SMC_NAND
#define MIO10_SDIO0_PC MIO_FN_SDIO_PC
#define MIO10_SDIO1    MIO_FN_SDIO
#define MIO10_CAN0     MIO_FN_CAN
#define MIO10_UART0    MIO_FN_UART
#define MIO10_I2C0     MIO_FN_I2C
#define MIO10_SPI1     MIO_FN_SPI

#define MIO11_GPIO     MIO_FN_GPIO
#define MIO11_QSPI1    MIO_FN_QSPI
#define MIO11_SMC_NOR  MIO_FN_SMC_NOR
#define MIO11_SMC_NAND MIO_FN_SMC_NAND
#define MIO11_SDIO1_PC MIO_FN_SDIO_PC
#define MIO11_SDIO1    MIO_FN_SDIO
#define MIO11_CAN0     MIO_FN_CAN
#define MIO11_UART0    MIO_FN_UART
#define MIO11_I2C0     MIO_FN_I2C
#define MIO11_SPI1     MIO_FN_SPI

#define MIO12_GPIO     MIO_FN_GPIO
#define MIO12_QSPI1    MIO_FN_QSPI
#define MIO12_SMC_NAND MIO_FN_SMC_NAND
#define MIO12_SDIO0_PC MIO_FN_SDIO_PC
#define MIO12_SDIO1    MIO_FN_SDIO
#define MIO12_CAN1     MIO_FN_CAN
#define MIO12_UART1    MIO_FN_UART
#define MIO12_I2C1     MIO_FN_I2C
#define MIO12_SPI1     MIO_FN_SPI

#define MIO13_GPIO     MIO_FN_GPIO
#define MIO13_QSPI1    MIO_FN_QSPI
#define MIO13_SMC_NOR  MIO_FN_SMC_NOR
#define MIO13_SMC_NAND MIO_FN_SMC_NAND
#define MIO13_SDIO1_PC MIO_FN_SDIO_PC
#define MIO13_SDIO1    MIO_FN_SDIO
#define MIO13_CAN1     MIO_FN_CAN
#define MIO13_UART1    MIO_FN_UART
#define MIO13_I2C1     MIO_FN_I2C
#define MIO13_SPI1     MIO_FN_SPI

#define MIO14_GPIO     MIO_FN_GPIO
#define MIO14_SMC_NAND MIO_FN_SMC_NAND
#define MIO14_SDIO0_PC MIO_FN_SDIO_PC
#define MIO14_SDIO1    MIO_FN_SDIO
#define MIO14_CAN0     MIO_FN_CAN
#define MIO14_UART0    MIO_FN_UART
#define MIO14_I2C0     MIO_FN_I2C
#define MIO14_SWDT0    MIO_FN_SWDT
#define MIO14_SPI1     MIO_FN_SPI

#define MIO15_GPIO     MIO_FN_GPIO
#define MIO15_SMC_NOR  MIO_FN_SMC_NOR
#define MIO15_SDIO1_PC MIO_FN_SDIO_PC
#define MIO15_SDIO1    MIO_FN_SDIO
#define MIO15_CAN0     MIO_FN_CAN
#define MIO15_UART0    MIO_FN_UART
#define MIO15_I2C0     MIO_FN_I2C
#define MIO15_SWDT0    MIO_FN_SWDT
#define MIO15_SPI1     MIO_FN_SPI

#define MIO16_GPIO     MIO_FN_GPIO
#define MIO16_ETH0     MIO_FN_ETH
#define MIO16_SMC_NOR  MIO_FN_SMC_NOR
#define MIO16_SMC_NAND MIO_FN_SMC_NAND
#define MIO16_SDIO0_PC MIO_FN_SDIO_PC
#define MIO16_SDIO0    MIO_FN_SDIO
#define MIO16_CAN1     MIO_FN_CAN
#define MIO16_UART1    MIO_FN_UART
#define MIO16_I2C1     MIO_FN_I2C
#define MIO16_TTC1     MIO_FN_TTC
#define MIO16_SPI0     MIO_FN_SPI

#define MIO17_GPIO     MIO_FN_GPIO
#define MIO17_ETH0     MIO_FN_ETH
#define MIO17_SMC_NOR  MIO_FN_SMC_NOR
#define MIO17_SMC_NAND MIO_FN_SMC_NAND
#define MIO17_SDIO1_PC MIO_FN_SDIO_PC
#define MIO17_SDIO0    MIO_FN_SDIO
#define MIO17_CAN1     MIO_FN_CAN
#define MIO17_UART1    MIO_FN_UART
#define MIO17_I2C1     MIO_FN_I2C
#define MIO17_TTC1     MIO_FN_TTC
#define MIO17_SPI0     MIO_FN_SPI

#define MIO18_GPIO     MIO_FN_GPIO
#define MIO18_ETH0     MIO_FN_ETH
#define MIO18_SMC_NOR  MIO_FN_SMC_NOR
#define MIO18_SMC_NAND MIO_FN_SMC_NAND
#define MIO18_SDIO0_PC MIO_FN_SDIO_PC
#define MIO18_SDIO0    MIO_FN_SDIO
#define MIO18_CAN0     MIO_FN_CAN
#define MIO18_UART0    MIO_FN_UART
#define MIO18_I2C0     MIO_FN_I2C
#define MIO18_TTC0     MIO_FN_TTC
#define MIO18_SPI0     MIO_FN_SPI

#define MIO19_GPIO     MIO_FN_GPIO
#define MIO19_ETH0     MIO_FN_ETH
#define MIO19_SMC_NOR  MIO_FN_SMC_NOR
#define MIO19_SMC_NAND MIO_FN_SMC_NAND
#define MIO19_SDIO1_PC MIO_FN_SDIO_PC
#define MIO19_SDIO0    MIO_FN_SDIO
#define MIO19_CAN0     MIO_FN_CAN
#define MIO19_UART0    MIO_FN_UART
#define MIO19_I2C0     MIO_FN_I2C
#define MIO19_TTC0     MIO_FN_TTC
#define MIO19_SPI0     MIO_FN_SPI

#define MIO20_GPIO     MIO_FN_GPIO
#define MIO20_ETH0     MIO_FN_ETH
#define MIO20_SMC_NOR  MIO_FN_SMC_NOR
#define MIO20_SMC_NAND MIO_FN_SMC_NAND
#define MIO20_SDIO0_PC MIO_FN_SDIO_PC
#define MIO20_SDIO0    MIO_FN_SDIO
#define MIO20_CAN1     MIO_FN_CAN
#define MIO20_UART1    MIO_FN_UART
#define MIO20_I2C1     MIO_FN_I2C
#define MIO20_SPI0     MIO_FN_SPI

#define MIO21_GPIO     MIO_FN_GPIO
#define MIO21_ETH0     MIO_FN_ETH
#define MIO21_SMC_NOR  MIO_FN_SMC_NOR
#define MIO21_SMC_NAND MIO_FN_SMC_NAND
#define MIO21_SDIO1_PC MIO_FN_SDIO_PC
#define MIO21_SDIO0    MIO_FN_SDIO
#define MIO21_CAN1     MIO_FN_CAN
#define MIO21_UART1    MIO_FN_UART
#define MIO21_I2C1     MIO_FN_I2C
#define MIO21_SPI0     MIO_FN_SPI

#define MIO22_GPIO     MIO_FN_GPIO
#define MIO22_ETH0     MIO_FN_ETH
#define MIO22_SMC_NOR  MIO_FN_SMC_NOR
#define MIO22_SMC_NAND MIO_FN_SMC_NAND
#define MIO22_SDIO0_PC MIO_FN_SDIO_PC
#define MIO22_SDIO1    MIO_FN_SDIO
#define MIO22_CAN0     MIO_FN_CAN
#define MIO22_UART0    MIO_FN_UART
#define MIO22_I2C0     MIO_FN_I2C
#define MIO22_SPI1     MIO_FN_SPI

#define MIO23_GPIO     MIO_FN_GPIO
#define MIO23_ETH0     MIO_FN_ETH
#define MIO23_SMC_NOR  MIO_FN_SMC_NOR
#define MIO23_SMC_NAND MIO_FN_SMC_NAND
#define MIO23_SDIO1_PC MIO_FN_SDIO_PC
#define MIO23_SDIO1    MIO_FN_SDIO
#define MIO23_CAN0     MIO_FN_CAN
#define MIO23_UART0    MIO_FN_UART
#define MIO23_I2C0     MIO_FN_I2C
#define MIO23_SPI1     MIO_FN_SPI

#define MIO24_GPIO     MIO_FN_GPIO
#define MIO24_ETH0     MIO_FN_ETH
#define MIO24_SMC_NOR  MIO_FN_SMC_NOR
#define MIO24_SDIO0_PC MIO_FN_SDIO_PC
#define MIO24_SDIO1    MIO_FN_SDIO
#define MIO24_CAN1     MIO_FN_CAN
#define MIO24_UART1    MIO_FN_UART
#define MIO24_I2C1     MIO_FN_I2C
#define MIO24_SPI1     MIO_FN_SPI

#define MIO25_GPIO     MIO_FN_GPIO
#define MIO25_ETH0     MIO_FN_ETH
#define MIO25_SMC_NOR  MIO_FN_SMC_NOR
#define MIO25_SDIO1_PC MIO_FN_SDIO_PC
#define MIO25_SDIO1    MIO_FN_SDIO
#define MIO25_CAN1     MIO_FN_CAN
#define MIO25_UART1    MIO_FN_UART
#define MIO25_I2C1     MIO_FN_I2C
#define MIO25_SPI1     MIO_FN_SPI

#define MIO26_GPIO     MIO_FN_GPIO
#define MIO26_ETH0     MIO_FN_ETH
#define MIO26_SMC_NOR  MIO_FN_SMC_NOR
#define MIO26_SDIO0_PC MIO_FN_SDIO_PC
#define MIO26_SDIO1    MIO_FN_SDIO
#define MIO26_CAN0     MIO_FN_CAN
#define MIO26_UART0    MIO_FN_UART
#define MIO26_I2C0     MIO_FN_I2C
#define MIO26_SWDT0    MIO_FN_SWDT
#define MIO26_SPI1     MIO_FN_SPI

#define MIO27_GPIO     MIO_FN_GPIO
#define MIO27_ETH0     MIO_FN_ETH
#define MIO27_SMC_NOR  MIO_FN_SMC_NOR
#define MIO27_SDIO1_PC MIO_FN_SDIO_PC
#define MIO27_SDIO1    MIO_FN_SDIO
#define MIO27_CAN0     MIO_FN_CAN
#define MIO27_UART0    MIO_FN_UART
#define MIO27_I2C0     MIO_FN_I2C
#define MIO27_SWDT0    MIO_FN_SWDT
#define MIO27_SPI1     MIO_FN_SPI

#define MIO28_GPIO     MIO_FN_GPIO
#define MIO28_ETH1     MIO_FN_ETH
#define MIO28_USB0     MIO_FN_USB
#define MIO28_SMC_NOR  MIO_FN_SMC_NOR
#define MIO28_SDIO0_PC MIO_FN_SDIO_PC
#define MIO28_SDIO0    MIO_FN_SDIO
#define MIO28_CAN1     MIO_FN_CAN
#define MIO28_UART1    MIO_FN_UART
#define MIO28_I2C1     MIO_FN_I2C
#define MIO28_TTC1     MIO_FN_TTC
#define MIO28_SPI0     MIO_FN_SPI

#define MIO29_GPIO     MIO_FN_GPIO
#define MIO29_ETH1     MIO_FN_ETH
#define MIO29_USB0     MIO_FN_USB
#define MIO29_SMC_NOR  MIO_FN_SMC_NOR
#define MIO29_SDIO1_PC MIO_FN_SDIO_PC
#define MIO29_SDIO0    MIO_FN_SDIO
#define MIO29_CAN1     MIO_FN_CAN
#define MIO29_UART1    MIO_FN_UART
#define MIO29_I2C1     MIO_FN_I2C
#define MIO29_TTC1     MIO_FN_TTC
#define MIO29_SPI0     MIO_FN_SPI

#define MIO30_GPIO     MIO_FN_GPIO
#define MIO30_ETH1     MIO_FN_ETH
#define MIO30_USB0     MIO_FN_USB
#define MIO30_SMC_NOR  MIO_FN_SMC_NOR
#define MIO30_SDIO0_PC MIO_FN_SDIO_PC
#define MIO30_SDIO0    MIO_FN_SDIO
#define MIO30_CAN0     MIO_FN_CAN
#define MIO30_UART0    MIO_FN_UART
#define MIO30_I2C0     MIO_FN_I2C
#define MIO30_TTC0     MIO_FN_TTC
#define MIO30_SPI0     MIO_FN_SPI

#define MIO31_GPIO     MIO_FN_GPIO
#define MIO31_ETH1     MIO_FN_ETH
#define MIO31_USB0     MIO_FN_USB
#define MIO31_SMC_NOR  MIO_FN_SMC_NOR
#define MIO31_SDIO1_PC MIO_FN_SDIO_PC
#define MIO31_SDIO0    MIO_FN_SDIO
#define MIO31_CAN0     MIO_FN_CAN
#define MIO31_UART0    MIO_FN_UART
#define MIO31_I2C0     MIO_FN_I2C
#define MIO31_TTC0     MIO_FN_TTC
#define MIO31_SPI0     MIO_FN_SPI

#define MIO32_GPIO     MIO_FN_GPIO
#define MIO32_ETH1     MIO_FN_ETH
#define MIO32_USB0     MIO_FN_USB
#define MIO32_SMC_NOR  MIO_FN_SMC_NOR
#define MIO32_SDIO0_PC MIO_FN_SDIO_PC
#define MIO32_SDIO0    MIO_FN_SDIO
#define MIO32_CAN1     MIO_FN_CAN
#define MIO32_UART1    MIO_FN_UART
#define MIO32_I2C1     MIO_FN_I2C
#define MIO32_SPI0     MIO_FN_SPI

#define MIO33_GPIO     MIO_FN_GPIO
#define MIO33_ETH1     MIO_FN_ETH
#define MIO33_USB0     MIO_FN_USB
#define MIO33_SMC_NOR  MIO_FN_SMC_NOR
#define MIO33_SDIO1_PC MIO_FN_SDIO_PC
#define MIO33_SDIO0    MIO_FN_SDIO
#define MIO33_CAN1     MIO_FN_CAN
#define MIO33_UART1    MIO_FN_UART
#define MIO33_I2C1     MIO_FN_I2C
#define MIO33_SPI0     MIO_FN_SPI

#define MIO34_GPIO     MIO_FN_GPIO
#define MIO34_ETH1     MIO_FN_ETH
#define MIO34_USB0     MIO_FN_USB
#define MIO34_SMC_NOR  MIO_FN_SMC_NOR
#define MIO34_SDIO0_PC MIO_FN_SDIO_PC
#define MIO34_SDIO1    MIO_FN_SDIO
#define MIO34_CAN0     MIO_FN_CAN
#define MIO34_UART0    MIO_FN_UART
#define MIO34_I2C0     MIO_FN_I2C
#define MIO34_SPI1     MIO_FN_SPI

#define MIO35_GPIO     MIO_FN_GPIO
#define MIO35_ETH1     MIO_FN_ETH
#define MIO35_USB0     MIO_FN_USB
#define MIO35_SMC_NOR  MIO_FN_SMC_NOR
#define MIO35_SDIO1_PC MIO_FN_SDIO_PC
#define MIO35_SDIO1    MIO_FN_SDIO
#define MIO35_CAN0     MIO_FN_CAN
#define MIO35_UART0    MIO_FN_UART
#define MIO35_I2C0     MIO_FN_I2C
#define MIO35_SPI1     MIO_FN_SPI

#define MIO36_GPIO     MIO_FN_GPIO
#define MIO36_ETH1     MIO_FN_ETH
#define MIO36_USB0     MIO_FN_USB
#define MIO36_SMC_NOR  MIO_FN_SMC_NOR
#define MIO36_SDIO0_PC MIO_FN_SDIO_PC
#define MIO36_SDIO1    MIO_FN_SDIO
#define MIO36_CAN1     MIO_FN_CAN
#define MIO36_UART1    MIO_FN_UART
#define MIO36_I2C1     MIO_FN_I2C
#define MIO36_SPI1     MIO_FN_SPI

#define MIO37_GPIO     MIO_FN_GPIO
#define MIO37_ETH1     MIO_FN_ETH
#define MIO37_USB0     MIO_FN_USB
#define MIO37_SMC_NOR  MIO_FN_SMC_NOR
#define MIO37_SDIO1_PC MIO_FN_SDIO_PC
#define MIO37_SDIO1    MIO_FN_SDIO
#define MIO37_CAN1     MIO_FN_CAN
#define MIO37_UART1    MIO_FN_UART
#define MIO37_I2C1     MIO_FN_I2C
#define MIO37_SPI1     MIO_FN_SPI

#define MIO38_GPIO     MIO_FN_GPIO
#define MIO38_ETH1     MIO_FN_ETH
#define MIO38_USB0     MIO_FN_USB
#define MIO38_SMC_NOR  MIO_FN_SMC_NOR
#define MIO38_SDIO0_PC MIO_FN_SDIO_PC
#define MIO38_SDIO1    MIO_FN_SDIO
#define MIO38_CAN0     MIO_FN_CAN
#define MIO38_UART0    MIO_FN_UART
#define MIO38_I2C0     MIO_FN_I2C
#define MIO38_SWDT0    MIO_FN_SWDT
#define MIO38_SPI1     MIO_FN_SPI

#define MIO39_GPIO     MIO_FN_GPIO
#define MIO39_ETH1     MIO_FN_ETH
#define MIO39_USB0     MIO_FN_USB
#define MIO39_SMC_NOR  MIO_FN_SMC_NOR
#define MIO39_SDIO1_PC MIO_FN_SDIO_PC
#define MIO39_SDIO1    MIO_FN_SDIO
#define MIO39_CAN0     MIO_FN_CAN
#define MIO39_UART0    MIO_FN_UART
#define MIO39_I2C0     MIO_FN_I2C
#define MIO39_SWDT0    MIO_FN_SWDT
#define MIO39_SPI1     MIO_FN_SPI

#define MIO40_GPIO     MIO_FN_GPIO
#define MIO40_USB1     MIO_FN_USB
#define MIO40_SDIO0_PC MIO_FN_SDIO_PC
#define MIO40_SDIO0    MIO_FN_SDIO
#define MIO40_CAN1     MIO_FN_CAN
#define MIO40_UART1    MIO_FN_UART
#define MIO40_I2C1     MIO_FN_I2C
#define MIO40_TTC1     MIO_FN_TTC
#define MIO40_SPI0     MIO_FN_SPI

#define MIO41_GPIO     MIO_FN_GPIO
#define MIO41_USB1     MIO_FN_USB
#define MIO41_SDIO1_PC MIO_FN_SDIO_PC
#define MIO41_SDIO0    MIO_FN_SDIO
#define MIO41_CAN1     MIO_FN_CAN
#define MIO41_UART1    MIO_FN_UART
#define MIO41_I2C1     MIO_FN_I2C
#define MIO41_TTC1     MIO_FN_TTC
#define MIO41_SPI0     MIO_FN_SPI

#define MIO42_GPIO     MIO_FN_GPIO
#define MIO42_USB1     MIO_FN_USB
#define MIO42_SDIO0_PC MIO_FN_SDIO_PC
#define MIO42_SDIO0    MIO_FN_SDIO
#define MIO42_CAN0     MIO_FN_CAN
#define MIO42_UART0    MIO_FN_UART
#define MIO42_I2C0     MIO_FN_I2C
#define MIO42_TTC0     MIO_FN_TTC
#define MIO42_SPI0     MIO_FN_SPI

#define MIO43_GPIO     MIO_FN_GPIO
#define MIO43_USB1     MIO_FN_USB
#define MIO43_SDIO1_PC MIO_FN_SDIO_PC
#define MIO43_SDIO0    MIO_FN_SDIO
#define MIO43_CAN0     MIO_FN_CAN
#define MIO43_UART0    MIO_FN_UART
#define MIO43_I2C0     MIO_FN_I2C
#define MIO43_TTC0     MIO_FN_TTC
#define MIO43_SPI0     MIO_FN_SPI

#define MIO44_GPIO     MIO_FN_GPIO
#define MIO44_USB1     MIO_FN_USB
#define MIO44_SDIO0_PC MIO_FN_SDIO_PC
#define MIO44_SDIO0    MIO_FN_SDIO
#define MIO44_CAN1     MIO_FN_CAN
#define MIO44_UART1    MIO_FN_UART
#define MIO44_I2C1     MIO_FN_I2C
#define MIO44_SPI0     MIO_FN_SPI

#define MIO45_GPIO     MIO_FN_GPIO
#define MIO45_USB1     MIO_FN_USB
#define MIO45_SDIO1_PC MIO_FN_SDIO_PC
#define MIO45_SDIO0    MIO_FN_SDIO
#define MIO45_CAN1     MIO_FN_CAN
#define MIO45_UART1    MIO_FN_UART
#define MIO45_I2C1     MIO_FN_I2C
#define MIO45_SPI0     MIO_FN_SPI

#define MIO46_GPIO     MIO_FN_GPIO
#define MIO46_USB1     MIO_FN_USB
#define MIO46_SDIO0_PC MIO_FN_SDIO_PC
#define MIO46_SDIO1    MIO_FN_SDIO
#define MIO46_CAN0     MIO_FN_CAN
#define MIO46_UART0    MIO_FN_UART
#define MIO46_I2C0     MIO_FN_I2C
#define MIO46_SPI1     MIO_FN_SPI

#define MIO47_GPIO     MIO_FN_GPIO
#define MIO47_USB1     MIO_FN_USB
#define MIO47_SDIO1_PC MIO_FN_SDIO_PC
#define MIO47_SDIO1    MIO_FN_SDIO
#define MIO47_CAN0     MIO_FN_CAN
#define MIO47_UART0    MIO_FN_UART
#define MIO47_I2C0     MIO_FN_I2C
#define MIO47_SPI1     MIO_FN_SPI

#define MIO48_GPIO     MIO_FN_GPIO
#define MIO48_USB1     MIO_FN_USB
#define MIO48_SDIO0_PC MIO_FN_SDIO_PC
#define MIO48_SDIO1    MIO_FN_SDIO
#define MIO48_CAN1     MIO_FN_CAN
#define MIO48_UART1    MIO_FN_UART
#define MIO48_I2C1     MIO_FN_I2C
#define MIO48_SPI1     MIO_FN_SPI

#define MIO49_GPIO     MIO_FN_GPIO
#define MIO49_USB1     MIO_FN_USB
#define MIO49_SDIO1_PC MIO_FN_SDIO_PC
#define MIO49_SDIO1    MIO_FN_SDIO
#define MIO49_CAN1     MIO_FN_CAN
#define MIO49_UART1    MIO_FN_UART
#define MIO49_I2C1     MIO_FN_I2C
#define MIO49_SPI1     MIO_FN_SPI

#define MIO50_GPIO     MIO_FN_GPIO
#define MIO50_USB1     MIO_FN_USB
#define MIO50_SDIO0_PC MIO_FN_SDIO_PC
#define MIO50_SDIO1    MIO_FN_SDIO
#define MIO50_CAN0     MIO_FN_CAN
#define MIO50_UART0    MIO_FN_UART
#define MIO50_I2C0     MIO_FN_I2C
#define MIO50_SWDT0    MIO_FN_SWDT
#define MIO50_SPI1     MIO_FN_SPI

#define MIO51_GPIO     MIO_FN_GPIO
#define MIO51_USB1     MIO_FN_USB
#define MIO51_SDIO1_PC MIO_FN_SDIO_PC
#define MIO51_SDIO1    MIO_FN_SDIO
#define MIO51_CAN0     MIO_FN_CAN
#define MIO51_UART0    MIO_FN_UART
#define MIO51_I2C0     MIO_FN_I2C
#define MIO51_SWDT0    MIO_FN_SWDT
#define MIO51_SPI1     MIO_FN_SPI

#define MIO52_GPIO     MIO_FN_GPIO
#define MIO52_MDIO0    MIO_FN_MDIO0
#define MIO52_MDIO1    MIO_FN_MDIO1
#define MIO52_SDIO0_PC MIO_FN_SDIO_PC
#define MIO52_CAN1     MIO_FN_CAN
#define MIO52_UART1    MIO_FN_UART
#define MIO52_I2C1     MIO_FN_I2C
#define MIO52_SWDT0    MIO_FN_SWDT

#define MIO53_GPIO     MIO_FN_GPIO
#define MIO53_MDIO0    MIO_FN_MDIO0
#define MIO53_MDIO1    MIO_FN_MDIO1
#define MIO53_SDIO1_PC MIO_FN_SDIO_PC
#define MIO53_CAN1     MIO_FN_CAN
#define MIO53_UART1    MIO_FN_UART
#define MIO53_I2C1     MIO_FN_I2C
#define MIO53_SWDT0    MIO_FN_SWDT

static inline volatile uint32_t *mio_reg_at(uint32_t byte_offset)
{
    return &((volatile uint32_t *)(uintptr_t)gpio)[byte_offset / 4UL];
}

static inline volatile uint32_t *mio_reg_bank(uint32_t byte_offset, uint32_t bank)
{
    return mio_reg_at(byte_offset + (0x040UL * bank));
}

static inline volatile uint32_t *mio_reg_mdata(uint32_t bank, uint32_t half)
{
    return mio_reg_at((0x008UL * bank) + (0x004UL * half));
}

static inline volatile uint32_t *mio_reg_data(uint32_t bank)
{
    return mio_reg_at(0x040UL + (0x004UL * bank));
}

static inline volatile uint32_t *mio_reg_data_ro(uint32_t bank)
{
    return mio_reg_at(0x060UL + (0x004UL * bank));
}

static inline volatile uint32_t *mio_reg_dirm(uint32_t bank)
{
    return mio_reg_bank(0x204UL, bank);
}
static inline volatile uint32_t *mio_reg_oen(uint32_t bank)
{
    return mio_reg_bank(0x208UL, bank);
}
static inline volatile uint32_t *mio_reg_int_mask(uint32_t bank)
{
    return mio_reg_bank(0x20CUL, bank);
}
static inline volatile uint32_t *mio_reg_int_en(uint32_t bank)
{
    return mio_reg_bank(0x210UL, bank);
}
static inline volatile uint32_t *mio_reg_int_dis(uint32_t bank)
{
    return mio_reg_bank(0x214UL, bank);
}
static inline volatile uint32_t *mio_reg_int_stat(uint32_t bank)
{
    return mio_reg_bank(0x218UL, bank);
}
static inline volatile uint32_t *mio_reg_int_type(uint32_t bank)
{
    return mio_reg_bank(0x21CUL, bank);
}
static inline volatile uint32_t *mio_reg_int_pol(uint32_t bank)
{
    return mio_reg_bank(0x220UL, bank);
}
static inline volatile uint32_t *mio_reg_int_any(uint32_t bank)
{
    return mio_reg_bank(0x224UL, bank);
}

#define MIO_BANK(pin) (((pin) < MIO_PAD_COUNT) ? ((pin) >> 5UL) : (2UL + (((pin) - (uint32_t)MIO_PAD_COUNT) >> 5UL)))
#define MIO_BANK_BIT(pin) (((pin) < MIO_PAD_COUNT) ? ((pin) & 0x1FUL) : (((pin) - (uint32_t)MIO_PAD_COUNT) & 0x1FUL))
#define MIO_BIT(pin)      (1UL << MIO_BANK_BIT(pin))

#define MIO_MDATA_HALF(pin) ((MIO_BANK_BIT(pin) >> 4UL) & 0x1UL)
#define MIO_MDATA_BIT(pin)  (1UL << (MIO_BANK_BIT(pin) & 0xFUL))

void mio_enable_clock(void);

void mio_disable_clock(void);

int mio_direction_output(uint32_t pin, int value);

int mio_direction_input(uint32_t pin);

static inline int mio_is_output(uint32_t pin)
{
    return ((*mio_reg_dirm(MIO_BANK(pin)) & MIO_BIT(pin)) != 0U) ? 1 : 0;
}

static inline int mio_is_driver_enabled(uint32_t pin)
{
    return ((*mio_reg_oen(MIO_BANK(pin)) & MIO_BIT(pin)) != 0U) ? 1 : 0;
}

static inline void mio_enable_driver(uint32_t pin)
{
    *mio_reg_oen(MIO_BANK(pin)) |= MIO_BIT(pin);
}

static inline void mio_disable_driver(uint32_t pin)
{
    *mio_reg_oen(MIO_BANK(pin)) &= ~MIO_BIT(pin);
}

static inline void mio_set_value(uint32_t pin, int value)
{
    uint32_t bit = MIO_MDATA_BIT(pin);

    *mio_reg_mdata(MIO_BANK(pin), MIO_MDATA_HALF(pin)) = ((0xFFFFUL ^ bit) << 16) | ((value != 0) ? bit : 0UL);
}

static inline void mio_set_high(uint32_t pin)
{
    uint32_t bit = MIO_MDATA_BIT(pin);

    *mio_reg_mdata(MIO_BANK(pin), MIO_MDATA_HALF(pin)) = ((0xFFFFUL ^ bit) << 16) | bit;
}

static inline void mio_set_low(uint32_t pin)
{
    uint32_t bit = MIO_MDATA_BIT(pin);

    *mio_reg_mdata(MIO_BANK(pin), MIO_MDATA_HALF(pin)) = ((0xFFFFUL ^ bit) << 16);
}

static inline void mio_toggle(uint32_t pin)
{
    uint32_t bank = MIO_BANK(pin);
    uint32_t bit = MIO_MDATA_BIT(pin);
    uint32_t cur = (*mio_reg_data(bank) >> MIO_BANK_BIT(pin)) & 1UL;

    *mio_reg_mdata(bank, MIO_MDATA_HALF(pin)) = ((0xFFFFUL ^ bit) << 16) | ((cur != 0U) ? 0UL : bit);
}

static inline int mio_get_output_latch(uint32_t pin)
{
    return ((*mio_reg_data(MIO_BANK(pin)) & MIO_BIT(pin)) != 0U) ? 1 : 0;
}

static inline int mio_get_value(uint32_t pin)
{
    return (int)((*mio_reg_data_ro(MIO_BANK(pin)) >> MIO_BANK_BIT(pin)) & 1UL);
}

static inline void mio_enable_irq(uint32_t pin)
{
    *mio_reg_int_en(MIO_BANK(pin)) = MIO_BIT(pin);
}

static inline void mio_disable_irq(uint32_t pin)
{
    *mio_reg_int_dis(MIO_BANK(pin)) = MIO_BIT(pin);
}

static inline int mio_irq_enabled(uint32_t pin)
{
    return ((*mio_reg_int_mask(MIO_BANK(pin)) & MIO_BIT(pin)) != 0U) ? 1 : 0;
}

static inline int mio_irq_pending(uint32_t pin)
{
    return ((*mio_reg_int_stat(MIO_BANK(pin)) & MIO_BIT(pin)) != 0U) ? 1 : 0;
}

static inline void mio_clear_irq(uint32_t pin)
{
    *mio_reg_int_stat(MIO_BANK(pin)) = MIO_BIT(pin);
}

void mio_set_irq_type(uint32_t pin, uint32_t type);

int mio_set_function(uint32_t pad, uint32_t fn);

int mio_enable_pull_up(uint32_t pad);
int mio_disable_pull_up(uint32_t pad);

int mio_enable_receiver(uint32_t pad);
int mio_disable_receiver(uint32_t pad);

int mio_set_slew_fast(uint32_t pad);
int mio_set_slew_slow(uint32_t pad);

#endif
