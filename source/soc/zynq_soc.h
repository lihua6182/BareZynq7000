#ifndef ZYNQ_SOC_H
#define ZYNQ_SOC_H

#include <assert.h>
#include <stddef.h>

#include "zynq_common.h"

#define DDR_BASE        (0x00000000U)
#define OCM_LOW_BASE    (0x00000000U)
#define PL_BASE         (0x40000000U)
#define IOU_PERIPH_BASE (0xE0000000U)
#define LQSPI_BASE      (0xFC000000U)
#define OCM_HIGH_BASE   (0xFFFC0000U)

#define SLCR_BASE    (0xF8000000U)
#define DDRC_BASE    (0xF8006000U)
#define DEVCFG_BASE  (0xF8007000U)
#define DMAC_BASE    (0xF8003000U)
#define DMAC_NS_BASE (0xF8004000U)
#define GEM0_BASE    (0xE000B000U)
#define GEM1_BASE    (0xE000C000U)
#define GPIO_BASE    (0xE000A000U)
#define I2C0_BASE    (0xE0004000U)
#define I2C1_BASE    (0xE0005000U)
#define QSPI_BASE    (0xE000D000U)
#define SDIO0_BASE   (0xE0100000U)
#define SDIO1_BASE   (0xE0101000U)
#define SPI0_BASE    (0xE0006000U)
#define SPI1_BASE    (0xE0007000U)
#define SWDT_BASE    (0xF8005000U)
#define TTC0_BASE    (0xF8001000U)
#define TTC1_BASE    (0xF8002000U)
#define UART0_BASE   (0xE0000000U)
#define UART1_BASE   (0xE0001000U)
#define CAN0_BASE    (0xE0008000U)
#define CAN1_BASE    (0xE0009000U)
#define USB0_BASE    (0xE0002000U)
#define USB1_BASE    (0xE0003000U)
#define OCM_BASE     (0xF800C000U)
#define SMC_BASE     (0xE1000000U)
#define AFI0_BASE    (0xF8008000U)
#define AFI1_BASE    (0xF8009000U)
#define AFI2_BASE    (0xF800A000U)
#define AFI3_BASE    (0xF800B000U)
#define MPCORE_BASE  (0xF8F00000U)
#define GICD_BASE    (0xF8F01000U)
#define GICC_BASE    (0xF8F00100U)
#define L2C_BASE     (0xF8F02000U)
#define XADC_BASE    (0xF8007100U)

struct slcr
{
    volatile uint32_t scl;
    volatile uint32_t lock;
    volatile uint32_t unlock;
    volatile const uint32_t locksta;
    uint32_t reserved0[60];
    volatile uint32_t arm_pll_ctrl;
    volatile uint32_t ddr_pll_ctrl;
    volatile uint32_t io_pll_ctrl;
    volatile const uint32_t pll_status;
    volatile uint32_t arm_pll_cfg;
    volatile uint32_t ddr_pll_cfg;
    volatile uint32_t io_pll_cfg;
    uint32_t reserved1;
    volatile uint32_t arm_clk_ctrl;
    volatile uint32_t ddr_clk_ctrl;
    volatile uint32_t dci_clk_ctrl;
    volatile uint32_t aper_clk_ctrl;
    volatile uint32_t usb0_clk_ctrl;
    volatile uint32_t usb1_clk_ctrl;
    volatile uint32_t gem0_rclk_ctrl;
    volatile uint32_t gem1_rclk_ctrl;
    volatile uint32_t gem0_clk_ctrl;
    volatile uint32_t gem1_clk_ctrl;
    volatile uint32_t smc_clk_ctrl;
    volatile uint32_t lqspi_clk_ctrl;
    volatile uint32_t sdio_clk_ctrl;
    volatile uint32_t uart_clk_ctrl;
    volatile uint32_t spi_clk_ctrl;
    volatile uint32_t can_clk_ctrl;
    volatile uint32_t can_mioclk_ctrl;
    volatile uint32_t dbg_clk_ctrl;
    volatile uint32_t pcap_clk_ctrl;
    volatile uint32_t topsw_clk_ctrl;
    volatile uint32_t fpga0_clk_ctrl;
    volatile uint32_t fpga0_thr_ctrl;
    volatile uint32_t fpga0_thr_cnt;
    volatile const uint32_t fpga0_thr_sta;
    volatile uint32_t fpga1_clk_ctrl;
    volatile uint32_t fpga1_thr_ctrl;
    volatile uint32_t fpga1_thr_cnt;
    volatile const uint32_t fpga1_thr_sta;
    volatile uint32_t fpga2_clk_ctrl;
    volatile uint32_t fpga2_thr_ctrl;
    volatile uint32_t fpga2_thr_cnt;
    volatile const uint32_t fpga2_thr_sta;
    volatile uint32_t fpga3_clk_ctrl;
    volatile uint32_t fpga3_thr_ctrl;
    volatile uint32_t fpga3_thr_cnt;
    volatile const uint32_t fpga3_thr_sta;
    uint32_t reserved2[5];
    volatile uint32_t clk_621_true;
    uint32_t reserved3[14];
    volatile uint32_t pss_rst_ctrl;
    volatile uint32_t ddr_rst_ctrl;
    volatile uint32_t topsw_rst_ctrl;
    volatile uint32_t dmac_rst_ctrl;
    volatile uint32_t usb_rst_ctrl;
    volatile uint32_t gem_rst_ctrl;
    volatile uint32_t sdio_rst_ctrl;
    volatile uint32_t spi_rst_ctrl;
    volatile uint32_t can_rst_ctrl;
    volatile uint32_t i2c_rst_ctrl;
    volatile uint32_t uart_rst_ctrl;
    volatile uint32_t gpio_rst_ctrl;
    volatile uint32_t lqspi_rst_ctrl;
    volatile uint32_t smc_rst_ctrl;
    volatile uint32_t ocm_rst_ctrl;
    uint32_t reserved4;
    volatile uint32_t fpga_rst_ctrl;
    volatile uint32_t a9_cpu_rst_ctrl;
    uint32_t reserved5;
    volatile uint32_t rs_awdt_ctrl;
    uint32_t reserved6[2];
    volatile uint32_t reboot_status;
    volatile uint32_t boot_mode;
    uint32_t reserved7[40];
    volatile uint32_t apu_ctrl;
    volatile uint32_t wdt_clk_sel;
    uint32_t reserved8[78];
    volatile uint32_t tz_dma_ns;
    volatile uint32_t tz_dma_irq_ns;
    volatile uint32_t tz_dma_periph_ns;
    uint32_t reserved9[57];
    volatile const uint32_t pss_idcode;
    uint32_t reserved10[51];
    volatile uint32_t ddr_urgent;
    uint32_t reserved11[2];
    volatile uint32_t ddr_cal_start;
    uint32_t reserved12;
    volatile uint32_t ddr_ref_start;
    volatile uint32_t ddr_cmd_sta;
    volatile uint32_t ddr_urgent_sel;
    volatile uint32_t ddr_dfi_status;
    uint32_t reserved13[55];
    volatile uint32_t mio_pin_00;
    volatile uint32_t mio_pin_01;
    volatile uint32_t mio_pin_02;
    volatile uint32_t mio_pin_03;
    volatile uint32_t mio_pin_04;
    volatile uint32_t mio_pin_05;
    volatile uint32_t mio_pin_06;
    volatile uint32_t mio_pin_07;
    volatile uint32_t mio_pin_08;
    volatile uint32_t mio_pin_09;
    volatile uint32_t mio_pin_10;
    volatile uint32_t mio_pin_11;
    volatile uint32_t mio_pin_12;
    volatile uint32_t mio_pin_13;
    volatile uint32_t mio_pin_14;
    volatile uint32_t mio_pin_15;
    volatile uint32_t mio_pin_16;
    volatile uint32_t mio_pin_17;
    volatile uint32_t mio_pin_18;
    volatile uint32_t mio_pin_19;
    volatile uint32_t mio_pin_20;
    volatile uint32_t mio_pin_21;
    volatile uint32_t mio_pin_22;
    volatile uint32_t mio_pin_23;
    volatile uint32_t mio_pin_24;
    volatile uint32_t mio_pin_25;
    volatile uint32_t mio_pin_26;
    volatile uint32_t mio_pin_27;
    volatile uint32_t mio_pin_28;
    volatile uint32_t mio_pin_29;
    volatile uint32_t mio_pin_30;
    volatile uint32_t mio_pin_31;
    volatile uint32_t mio_pin_32;
    volatile uint32_t mio_pin_33;
    volatile uint32_t mio_pin_34;
    volatile uint32_t mio_pin_35;
    volatile uint32_t mio_pin_36;
    volatile uint32_t mio_pin_37;
    volatile uint32_t mio_pin_38;
    volatile uint32_t mio_pin_39;
    volatile uint32_t mio_pin_40;
    volatile uint32_t mio_pin_41;
    volatile uint32_t mio_pin_42;
    volatile uint32_t mio_pin_43;
    volatile uint32_t mio_pin_44;
    volatile uint32_t mio_pin_45;
    volatile uint32_t mio_pin_46;
    volatile uint32_t mio_pin_47;
    volatile uint32_t mio_pin_48;
    volatile uint32_t mio_pin_49;
    volatile uint32_t mio_pin_50;
    volatile uint32_t mio_pin_51;
    volatile uint32_t mio_pin_52;
    volatile uint32_t mio_pin_53;
    uint32_t reserved14[11];
    volatile uint32_t mio_loopback;
    uint32_t reserved15;
    volatile uint32_t mio_mst_tri0;
    volatile uint32_t mio_mst_tri1;
    uint32_t reserved16[7];
    volatile uint32_t sd0_wp_cd_sel;
    volatile uint32_t sd1_wp_cd_sel;
    uint32_t reserved17[50];
    volatile uint32_t lvl_shftr_en;
    uint32_t reserved18[3];
    volatile uint32_t ocm_cfg;
    uint32_t reserved19[66];
    volatile uint32_t l2c_ram;
    uint32_t reserved20[56];
    volatile uint32_t gpiob_ctrl;
    volatile uint32_t gpiob_cfg_cmos18;
    volatile uint32_t gpiob_cfg_cmos25;
    volatile uint32_t gpiob_cfg_cmos33;
    uint32_t reserved21;
    volatile uint32_t gpiob_cfg_hstl;
    volatile uint32_t gpiob_drvr_bias_ctrl;
    uint32_t reserved22[9];
    volatile uint32_t ddriob_addr0;
    volatile uint32_t ddriob_addr1;
    volatile uint32_t ddriob_data0;
    volatile uint32_t ddriob_data1;
    volatile uint32_t ddriob_diff0;
    volatile uint32_t ddriob_diff1;
    volatile uint32_t ddriob_clock;
    volatile uint32_t ddriob_drive_slew_addr;
    volatile uint32_t ddriob_drive_slew_data;
    volatile uint32_t ddriob_drive_slew_diff;
    volatile uint32_t ddriob_drive_slew_clock;
    volatile uint32_t ddriob_ddr_ctrl;
    volatile uint32_t ddriob_dci_ctrl;
    volatile uint32_t ddriob_dci_status;
};

struct ddrc
{
    volatile uint32_t ddrc_ctrl;
    volatile uint32_t two_rank_cfg;
    volatile uint32_t hpr;
    volatile uint32_t lpr;
    volatile uint32_t wr_reg;
    volatile uint32_t dram_param_reg0;
    volatile uint32_t dram_param_reg1;
    volatile uint32_t dram_param_reg2;
    volatile uint32_t dram_param_reg3;
    volatile uint32_t dram_param_reg4;
    volatile uint32_t dram_init_param;
    volatile uint32_t dram_emr;
    volatile uint32_t dram_emr_mr;
    volatile uint32_t dram_burst8_rdwr;
    volatile uint32_t dram_disable_dq;
    volatile uint32_t dram_addr_map_bank;
    volatile uint32_t dram_addr_map_col;
    volatile uint32_t dram_addr_map_row;
    volatile uint32_t dram_odt;
    volatile uint32_t phy_dbg;
    volatile uint32_t phy_cmd_timeout_rddata_cpt;
    volatile const uint32_t mode_sts;
    volatile uint32_t dll_calib;
    volatile uint32_t odt_delay_hold;
    volatile uint32_t ctrl_reg1;
    volatile uint32_t ctrl_reg2;
    volatile uint32_t ctrl_reg3;
    volatile uint32_t ctrl_reg4;
    uint32_t reserved1[2];
    volatile uint32_t ctrl_reg5;
    volatile uint32_t ctrl_reg6;
    uint32_t reserved2[8];
    volatile uint32_t che_refresh_timer01;
    volatile uint32_t che_t_zq;
    volatile uint32_t che_t_zq_short_interval;
    volatile uint32_t deep_pwrdwn;
    volatile uint32_t reg_2c;
    volatile uint32_t reg_2d;
    volatile uint32_t dfi_timing;
    uint32_t reserved3[2];
    volatile uint32_t che_ecc_control;
    volatile uint32_t che_corr_ecc_log;
    volatile const uint32_t che_corr_ecc_addr;
    volatile const uint32_t che_corr_ecc_data_31_0;
    volatile const uint32_t che_corr_ecc_data_63_32;
    volatile const uint32_t che_corr_ecc_data_71_64;
    volatile uint32_t che_uncorr_ecc_log;
    volatile const uint32_t che_uncorr_ecc_addr;
    volatile const uint32_t che_uncorr_ecc_data_31_0;
    volatile const uint32_t che_uncorr_ecc_data_63_32;
    volatile const uint32_t che_uncorr_ecc_data_71_64;
    volatile uint32_t che_ecc_stats;
    volatile uint32_t ecc_scrub;
    volatile const uint32_t che_ecc_corr_bit_mask_31_0;
    volatile const uint32_t che_ecc_corr_bit_mask_63_32;
    uint32_t reserved4[5];
    volatile uint32_t phy_rcvr_enable;
    volatile uint32_t phy_config0;
    volatile uint32_t phy_config1;
    volatile uint32_t phy_config2;
    volatile uint32_t phy_config3;
    uint32_t reserved5;
    volatile uint32_t phy_init_ratio0;
    volatile uint32_t phy_init_ratio1;
    volatile uint32_t phy_init_ratio2;
    volatile uint32_t phy_init_ratio3;
    uint32_t reserved6;
    volatile uint32_t phy_rd_dqs_cfg0;
    volatile uint32_t phy_rd_dqs_cfg1;
    volatile uint32_t phy_rd_dqs_cfg2;
    volatile uint32_t phy_rd_dqs_cfg3;
    uint32_t reserved7;
    volatile uint32_t phy_wr_dqs_cfg0;
    volatile uint32_t phy_wr_dqs_cfg1;
    volatile uint32_t phy_wr_dqs_cfg2;
    volatile uint32_t phy_wr_dqs_cfg3;
    uint32_t reserved8;
    volatile uint32_t phy_we_cfg0;
    volatile uint32_t phy_we_cfg1;
    volatile uint32_t phy_we_cfg2;
    volatile uint32_t phy_we_cfg3;
    uint32_t reserved9;
    volatile uint32_t wr_data_slv0;
    volatile uint32_t wr_data_slv1;
    volatile uint32_t wr_data_slv2;
    volatile uint32_t wr_data_slv3;
    uint32_t reserved10;
    volatile uint32_t reg_64;
    volatile uint32_t reg_65;
    uint32_t reserved11[3];
    volatile const uint32_t reg69_6a0;
    volatile uint32_t reg69_6a1;
    uint32_t reserved12;
    volatile const uint32_t reg6c_6d2;
    volatile const uint32_t reg6c_6d3;
    volatile const uint32_t reg6e_710;
    volatile uint32_t reg6e_711;
    volatile uint32_t reg6e_712;
    volatile uint32_t reg6e_713;
    uint32_t reserved13;
    volatile const uint32_t phy_dll_sts0;
    volatile uint32_t phy_dll_sts1;
    volatile uint32_t phy_dll_sts2;
    volatile uint32_t phy_dll_sts3;
    uint32_t reserved14;
    volatile uint32_t dll_lock_sts;
    volatile const uint32_t phy_ctrl_sts;
    volatile const uint32_t phy_ctrl_sts_reg2;
    uint32_t reserved15[5];
    volatile const uint32_t axi_id;
    volatile uint32_t page_mask;
    volatile uint32_t axi_priority_wr_port0;
    uint32_t reserved16[3];
    volatile uint32_t axi_priority_rd_port0;
    uint32_t reserved17[30];
    volatile uint32_t excl_access_cfg0;
    volatile uint32_t excl_access_cfg1;
    volatile uint32_t excl_access_cfg2;
    volatile uint32_t excl_access_cfg3;
    volatile const uint32_t mode_reg_read;
    volatile uint32_t lpddr_ctrl0;
    volatile uint32_t lpddr_ctrl1;
    volatile uint32_t lpddr_ctrl2;
    volatile uint32_t lpddr_ctrl3;
};

struct devcfg
{
    volatile uint32_t ctrl;
    volatile uint32_t lock;
    volatile uint32_t cfg;
    volatile uint32_t int_sts;
    volatile uint32_t int_mask;
    volatile uint32_t status;
    volatile uint32_t dma_src_addr;
    volatile uint32_t dma_dest_addr;
    volatile uint32_t dma_src_len;
    volatile uint32_t dma_dest_len;
    volatile uint32_t rom_shadow;
    volatile uint32_t multiboot_addr;
    volatile uint32_t sw_id;
    volatile uint32_t unlock;
    uint32_t reserved0[18];
    volatile uint32_t mctrl;
    uint32_t reserved1[31];
    volatile uint32_t xadcif_cfg;
    volatile uint32_t xadcif_int_sts;
    volatile uint32_t xadcif_int_mask;
    volatile const uint32_t xadcif_msts;
    volatile uint32_t xadcif_cmdfifo;
    volatile const uint32_t xadcif_rdfifo;
    volatile uint32_t xadcif_mctl;
};

struct dmac
{
    volatile uint32_t ds;
    volatile uint32_t dpc;
    uint32_t reserved0[6];
    volatile uint32_t inten;
    volatile uint32_t es;
    volatile uint32_t intmis;
    volatile uint32_t intclr;
    volatile uint32_t fsrd;
    volatile uint32_t fsrc;
    volatile uint32_t ftm;
    uint32_t reserved1;
    volatile uint32_t ftc0;
    volatile uint32_t ftr1;
    volatile uint32_t ftr2;
    volatile uint32_t ftr3;
    volatile uint32_t ftr4;
    volatile uint32_t ftr5;
    volatile uint32_t ftr6;
    volatile uint32_t ftr7;
    uint32_t reserved2[40];
    volatile uint32_t cs0;
    volatile uint32_t cpc0;
    volatile uint32_t csr1;
    volatile uint32_t cpc1;
    volatile uint32_t csr2;
    volatile uint32_t cpc2;
    volatile uint32_t csr3;
    volatile uint32_t cpc3;
    volatile uint32_t csr4;
    volatile uint32_t cpc4;
    volatile uint32_t csr5;
    volatile uint32_t cpc5;
    volatile uint32_t csr6;
    volatile uint32_t cpc6;
    volatile uint32_t csr7;
    volatile uint32_t cpc7;
    uint32_t reserved3[176];
    volatile uint32_t sar0;
    volatile uint32_t dar0;
    volatile uint32_t ccr0;
    volatile uint32_t lc0_0;
    volatile uint32_t lc1_0;
    uint32_t reserved4[3];
    volatile uint32_t sar1;
    volatile uint32_t dar1;
    volatile uint32_t ccr1;
    volatile uint32_t lc0_1;
    volatile uint32_t lc1_1;
    uint32_t reserved5[3];
    volatile uint32_t sar2;
    volatile uint32_t dar2;
    volatile uint32_t ccr2;
    volatile uint32_t lc0_2;
    volatile uint32_t lc1_2;
    uint32_t reserved6[3];
    volatile uint32_t sar3;
    volatile uint32_t dar3;
    volatile uint32_t ccr3;
    volatile uint32_t lc0_3;
    volatile uint32_t lc1_3;
    uint32_t reserved7[3];
    volatile uint32_t sar4;
    volatile uint32_t dar4;
    volatile uint32_t ccr4;
    volatile uint32_t lc0_4;
    volatile uint32_t lc1_4;
    uint32_t reserved8[3];
    volatile uint32_t sar5;
    volatile uint32_t dar5;
    volatile uint32_t ccr5;
    volatile uint32_t lc0_5;
    volatile uint32_t lc1_5;
    uint32_t reserved9[3];
    volatile uint32_t sar6;
    volatile uint32_t dar6;
    volatile uint32_t ccr6;
    volatile uint32_t lc0_6;
    volatile uint32_t lc1_6;
    uint32_t reserved10[3];
    volatile uint32_t sar7;
    volatile uint32_t dar7;
    volatile uint32_t ccr7;
    volatile uint32_t lc0_7;
    volatile uint32_t lc1_7;
    uint32_t reserved11[515];
    volatile uint32_t dbgstatus;
    volatile uint32_t dbgcmd;
    volatile uint32_t dbginst0;
    volatile uint32_t dbginst1;
    uint32_t reserved12[60];
    volatile uint32_t cr0;
    volatile uint32_t cr1;
    volatile uint32_t cr2;
    volatile uint32_t cr3;
    volatile uint32_t cr4;
    volatile uint32_t crdn;
    uint32_t reserved13[26];
    volatile uint32_t wd;
    uint32_t reserved14[87];
    volatile uint32_t periph_id_0;
    volatile uint32_t periph_id_1;
    volatile uint32_t periph_id_2;
    volatile uint32_t periph_id_3;
    volatile uint32_t pcell_id_0;
    volatile uint32_t pcell_id_1;
    volatile uint32_t pcell_id_2;
    volatile uint32_t pcell_id_3;
};

struct gem
{
    volatile uint32_t nwctrl;
    volatile uint32_t nwcfg;
    volatile const uint32_t nwsr;
    uint32_t reserved0;
    volatile uint32_t dmacr;
    volatile uint32_t txsr;
    volatile uint32_t rxqbase;
    volatile uint32_t txqbase;
    volatile uint32_t rxsr;
    volatile uint32_t isr;
    volatile uint32_t ier;
    volatile uint32_t idr;
    volatile uint32_t imr;
    volatile uint32_t phymntnc;
    volatile const uint32_t rxpause;
    volatile uint32_t txpause;
    uint32_t reserved1[16];
    volatile uint32_t hashl;
    volatile uint32_t hashh;
    volatile uint32_t laddr1l;
    volatile uint32_t laddr1h;
    volatile uint32_t laddr2l;
    volatile uint32_t laddr2h;
    volatile uint32_t laddr3l;
    volatile uint32_t laddr3h;
    volatile uint32_t laddr4l;
    volatile uint32_t laddr4h;
    volatile uint32_t match1;
    volatile uint32_t match2;
    volatile uint32_t match3;
    volatile uint32_t match4;
    volatile uint32_t wake_on_lan;
    volatile uint32_t stretch;
    volatile uint32_t stacked_vlan;
    volatile uint32_t tx_pfc_pause;
    volatile uint32_t spec_addr1_mask_bot;
    volatile uint32_t spec_addr1_mask_top;
    uint32_t reserved2[11];
    volatile const uint32_t module_id;
    volatile const uint32_t octets_tx_bot;
    volatile const uint32_t octets_tx_top;
    volatile const uint32_t frames_tx;
    volatile const uint32_t broadcast_frames_tx;
    volatile uint32_t multi_frames_tx;
    volatile const uint32_t pause_frames_tx;
    volatile const uint32_t frames_64b_tx;
    volatile const uint32_t frames_65to127b_tx;
    volatile const uint32_t frames_128to255b_tx;
    volatile const uint32_t frames_256to511b_tx;
    volatile const uint32_t frames_512to1023b_tx;
    volatile const uint32_t frames_1024to1518b_tx;
    uint32_t reserved3;
    volatile const uint32_t tx_under_runs;
    volatile const uint32_t single_collisn_frames;
    volatile const uint32_t multi_collisn_frames;
    volatile const uint32_t excessive_collisns;
    volatile const uint32_t late_collisns;
    volatile const uint32_t deferred_tx_frames;
    volatile const uint32_t carrier_sense_errs;
    volatile const uint32_t octets_rx_bot;
    volatile const uint32_t octets_rx_top;
    volatile uint32_t frames_rx;
    volatile const uint32_t bdcast_fames_rx;
    volatile const uint32_t multi_frames_rx;
    volatile const uint32_t pause_rx;
    volatile const uint32_t frames_64b_rx;
    volatile const uint32_t frames_65to127b_rx;
    volatile const uint32_t frames_128to255b_rx;
    volatile const uint32_t frames_256to511b_rx;
    volatile const uint32_t frames_512to1023b_rx;
    volatile const uint32_t frames_1024to1518b_rx;
    uint32_t reserved4;
    volatile const uint32_t undersz_rx;
    volatile const uint32_t oversz_rx;
    volatile const uint32_t jab_rx;
    volatile const uint32_t fcs_errors;
    volatile const uint32_t length_field_errors;
    volatile const uint32_t rxsymbcnt;
    volatile const uint32_t rxaligncnt;
    volatile const uint32_t rx_resource_errors;
    volatile const uint32_t rxorcnt;
    volatile uint32_t ip_hdr_csum_errors;
    volatile const uint32_t tcp_csum_errors;
    volatile const uint32_t udp_csum_errors;
    uint32_t reserved5[5];
    volatile uint32_t timer_strobe_s;
    volatile uint32_t timer_strobe_ns;
    volatile uint32_t ptp_sec;
    volatile uint32_t ptp_nanosec;
    volatile uint32_t timer_adjust;
    volatile uint32_t timer_incr;
    volatile const uint32_t ptp_tx_s;
    volatile const uint32_t ptp_tx_ns;
    volatile const uint32_t ptp_rx_s;
    volatile const uint32_t ptp_rx_ns;
    volatile const uint32_t ptp_peer_tx_s;
    volatile const uint32_t ptp_peer_tx_ns;
    volatile const uint32_t ptp_peer_rx_s;
    volatile const uint32_t ptp_peer_rx_ns;
    uint32_t reserved6[33];
    volatile const uint32_t design_cfg2;
    volatile const uint32_t design_cfg3;
    volatile const uint32_t design_cfg4;
    volatile const uint32_t design_cfg5;
};

struct gpio
{
    volatile uint32_t mask_data_0_lsw;
    volatile uint32_t mask_data_0_msw;
    volatile uint32_t mask_data_1_lsw;
    volatile uint32_t mask_data_1_msw;
    volatile uint32_t mask_data_2_lsw;
    volatile uint32_t mask_data_2_msw;
    volatile uint32_t mask_data_3_lsw;
    volatile uint32_t mask_data_3_msw;
    uint32_t reserved0[8];
    volatile uint32_t data_0;
    volatile uint32_t data_1;
    volatile uint32_t data_2;
    volatile uint32_t data_3;
    uint32_t reserved1[4];
    volatile const uint32_t data_0_ro;
    volatile const uint32_t data_1_ro;
    volatile const uint32_t data_2_ro;
    volatile const uint32_t data_3_ro;
    uint32_t reserved2[101];
    volatile uint32_t dirm_0;
    volatile uint32_t oen_0;
    volatile const uint32_t int_mask_0;
    volatile uint32_t int_en_0;
    volatile uint32_t int_dis_0;
    volatile uint32_t int_stat_0;
    volatile uint32_t int_type_0;
    volatile uint32_t int_polarity_0;
    volatile uint32_t int_any_0;
    uint32_t reserved3[7];
    volatile uint32_t dirm_1;
    volatile uint32_t oen_1;
    volatile const uint32_t int_mask_1;
    volatile uint32_t int_en_1;
    volatile uint32_t int_dis_1;
    volatile uint32_t int_stat_1;
    volatile uint32_t int_type_1;
    volatile uint32_t int_polarity_1;
    volatile uint32_t int_any_1;
    uint32_t reserved4[7];
    volatile uint32_t dirm_2;
    volatile uint32_t oen_2;
    volatile const uint32_t int_mask_2;
    volatile uint32_t int_en_2;
    volatile uint32_t int_dis_2;
    volatile uint32_t int_stat_2;
    volatile uint32_t int_type_2;
    volatile uint32_t int_polarity_2;
    volatile uint32_t int_any_2;
    uint32_t reserved5[7];
    volatile uint32_t dirm_3;
    volatile uint32_t oen_3;
    volatile const uint32_t int_mask_3;
    volatile uint32_t int_en_3;
    volatile uint32_t int_dis_3;
    volatile uint32_t int_stat_3;
    volatile uint32_t int_type_3;
    volatile uint32_t int_polarity_3;
    volatile uint32_t int_any_3;
};

struct i2c
{
    volatile uint32_t cr;
    volatile const uint32_t sr;
    volatile uint32_t addr;
    volatile uint32_t data;
    volatile uint32_t isr;
    volatile uint32_t trans_size;
    volatile uint32_t slv_pause;
    volatile uint32_t time_out;
    volatile const uint32_t imr;
    volatile uint32_t ier;
    volatile uint32_t idr;
};

struct qspi
{
    volatile uint32_t cr;
    volatile uint32_t sr;
    volatile uint32_t ier;
    volatile uint32_t idr;
    volatile const uint32_t imr;
    volatile uint32_t er;
    volatile uint32_t dr;
    volatile uint32_t txd_00;
    volatile const uint32_t rxd;
    volatile uint32_t sicr;
    volatile uint32_t txwr;
    volatile uint32_t rxwr;
    volatile uint32_t gpio;
    uint32_t reserved0;
    volatile uint32_t lpbk_dly_adj;
    uint32_t reserved1[17];
    volatile uint32_t txd_01;
    volatile uint32_t txd_10;
    volatile uint32_t txd_11;
    uint32_t reserved2[5];
    volatile uint32_t lqspi_cr;
    volatile uint32_t lqspi_sr;
    uint32_t reserved3[21];
    volatile uint32_t mod_id;
};

struct sdio
{
    volatile uint32_t argmt2_lo;
    volatile uint32_t blk_size;
    volatile uint32_t argmt;
    volatile uint32_t xfer_mode;
    volatile const uint32_t resp0;
    volatile uint32_t resp1;
    volatile uint32_t resp2;
    volatile uint32_t resp3;
    volatile uint32_t buf_dat_port;
    volatile const uint32_t pres_state;
    volatile uint32_t host_ctrl1;
    volatile uint32_t clk_ctrl;
    volatile uint32_t normal_interrupt_stat;
    volatile uint32_t norm_intr_stat_en;
    volatile uint32_t norm_intr_sig_en;
    volatile const uint32_t auto_cmd12_error_status;
    volatile const uint32_t caps;
    uint32_t reserved1;
    volatile const uint32_t maximum_current_capabilities;
    uint32_t reserved2[2];
    volatile uint32_t adma_error_status;
    volatile uint32_t adma_system_address;
    uint32_t reserved3;
    volatile uint32_t pre_val_1;
    volatile uint32_t pre_val_2;
    uint32_t reserved4[34];
    volatile uint32_t spi_interrupt_support;
};

struct spi
{
    volatile uint32_t cr;
    volatile uint32_t sr;
    volatile uint32_t ier;
    volatile uint32_t idr;
    volatile const uint32_t imr;
    volatile uint32_t er;
    volatile uint32_t dr;
    volatile uint32_t txd;
    volatile const uint32_t rxd;
    volatile uint32_t sicr;
    volatile uint32_t txwr;
    volatile uint32_t rxwr;
    uint32_t reserved0[51];
    volatile const uint32_t mod_id_reg0;
};

struct swdt
{
    volatile uint32_t mode;
    volatile uint32_t control;
    volatile uint32_t restart;
    volatile const uint32_t status;
};

struct ttc
{
    volatile uint32_t clock_control_1;
    volatile uint32_t clock_control_2;
    volatile uint32_t clock_control_3;
    volatile uint32_t counter_control_1;
    volatile uint32_t counter_control_2;
    volatile uint32_t counter_control_3;
    volatile const uint32_t counter_value_1;
    volatile const uint32_t counter_value_2;
    volatile const uint32_t counter_value_3;
    volatile uint32_t interval_counter_1;
    volatile uint32_t interval_counter_2;
    volatile uint32_t interval_counter_3;
    volatile uint32_t match_1_counter_1;
    volatile uint32_t match_1_counter_2;
    volatile uint32_t match_1_counter_3;
    volatile uint32_t match_2_counter_1;
    volatile uint32_t match_2_counter_2;
    volatile uint32_t match_2_counter_3;
    volatile uint32_t match_3_counter_1;
    volatile uint32_t match_3_counter_2;
    volatile uint32_t match_3_counter_3;
    volatile uint32_t interrupt_register_1;
    volatile uint32_t interrupt_register_2;
    volatile uint32_t interrupt_register_3;
    volatile uint32_t interrupt_enable_1;
    volatile uint32_t interrupt_enable_2;
    volatile uint32_t interrupt_enable_3;
    volatile uint32_t event_control_timer_1;
    volatile uint32_t event_control_timer_2;
    volatile uint32_t event_control_timer_3;
    volatile const uint32_t event_register_1;
    volatile const uint32_t event_register_2;
    volatile const uint32_t event_register_3;
};

struct uart
{
    volatile uint32_t cr;
    volatile uint32_t mr;
    volatile uint32_t ier;
    volatile uint32_t idr;
    volatile const uint32_t imr;
    volatile uint32_t isr;
    volatile uint32_t baudgen;
    volatile uint32_t rxtout;
    volatile uint32_t rxwm;
    volatile uint32_t modemcr;
    volatile uint32_t modemsr;
    volatile const uint32_t sr;
    volatile uint32_t fifo;
    volatile uint32_t bauddiv;
    volatile uint32_t flowdel;
    uint32_t reserved0[2];
    volatile uint32_t txwm;
};

struct can
{
    volatile uint32_t srr;
    volatile uint32_t msr;
    volatile uint32_t brpr;
    volatile uint32_t btr;
    volatile const uint32_t ecr;
    volatile uint32_t esr;
    volatile uint32_t sr;
    volatile uint32_t isr;
    volatile uint32_t ier;
    volatile uint32_t icr;
    volatile uint32_t tcr;
    volatile uint32_t wir;
    volatile uint32_t txfifo_id;
    volatile uint32_t txfifo_dlc;
    volatile uint32_t txfifo_dw1;
    volatile uint32_t txfifo_dw2;
    volatile uint32_t txhpb_id;
    volatile uint32_t txhpb_dlc;
    volatile uint32_t txhpb_dw1;
    volatile uint32_t txhpb_dw2;
    volatile const uint32_t rxfifo_id;
    volatile uint32_t rxfifo_dlc;
    volatile uint32_t rxfifo_dw1;
    volatile uint32_t rxfifo_dw2;
    volatile uint32_t afr;
    volatile uint32_t afmr1;
    volatile uint32_t afir1;
    volatile uint32_t afmr2;
    volatile uint32_t afir2;
    volatile uint32_t afmr3;
    volatile uint32_t afir3;
    volatile uint32_t afmr4;
    volatile uint32_t afir4;
};

struct usb
{
    volatile const uint32_t id;
    volatile uint32_t hwgeneral;
    volatile const uint32_t hwhost;
    volatile const uint32_t hwdevice;
    volatile const uint32_t hwtxbuf;
    volatile const uint32_t hwrxbuf;
    uint32_t reserved0[26];
    volatile uint32_t gptimer0ld;
    volatile uint32_t gptimer0ctrl;
    volatile uint32_t gptimer1ld;
    volatile uint32_t gptimer1ctrl;
    volatile uint32_t sbuscfg;
    uint32_t reserved1[27];
    volatile const uint32_t caplength_hciversion;
    volatile const uint32_t hcsparams;
    volatile const uint32_t hccparams;
    uint32_t reserved2[5];
    volatile const uint32_t dciversion;
    volatile const uint32_t dccparams;
    uint32_t reserved3[6];
    volatile uint32_t usbcmd;
    volatile uint32_t usbsts;
    volatile uint32_t usbintr;
    volatile uint32_t frindex;
    uint32_t reserved4;
    volatile uint32_t periodiclistbase_deviceaddr;
    volatile uint32_t asynclistaddr_endpointlistaddr;
    volatile uint32_t ttctrl;
    volatile uint32_t burstsize;
    volatile uint32_t txfilltuning;
    volatile uint32_t txttfilltuning;
    volatile uint32_t ic_usb;
    volatile uint32_t ulpi_viewport;
    uint32_t reserved5;
    volatile uint32_t endptnak;
    volatile uint32_t endptnaken;
    volatile const uint32_t configflag;
    volatile uint32_t portsc1;
    uint32_t reserved6[7];
    volatile uint32_t otgsc;
    volatile uint32_t usbmode;
    volatile uint32_t endptsetupstat;
    volatile uint32_t endptprime;
    volatile uint32_t endptflush;
    volatile uint32_t endptstat;
    volatile uint32_t endptcomplete;
    volatile uint32_t endptctrl0;
    volatile uint32_t endptctrl1;
    volatile uint32_t endptctrl2;
    volatile uint32_t endptctrl3;
    volatile uint32_t endptctrl4;
    volatile uint32_t endptctrl5;
    volatile uint32_t endptctrl6;
    volatile uint32_t endptctrl7;
    volatile uint32_t endptctrl8;
    volatile uint32_t endptctrl9;
    volatile uint32_t endptctrl10;
    volatile uint32_t endptctrl11;
};

struct ocm
{
    volatile uint32_t ocm_parity_ctrl;
    volatile uint32_t ocm_parity_erraddress;
    volatile uint32_t ocm_irq_sts;
    volatile uint32_t ocm_control;
};

struct smc
{
    volatile const uint32_t memc_status;
    volatile const uint32_t memif_cfg;
    volatile uint32_t memc_cfg_set;
    volatile uint32_t memc_cfg_clr;
    volatile uint32_t direct_cmd;
    volatile uint32_t set_cycles;
    volatile uint32_t set_opmode;
    uint32_t reserved0;
    volatile uint32_t refresh_period_0;
    volatile uint32_t refresh_period_1;
    uint32_t reserved1[54];
    volatile const uint32_t sram_cycles0_0;
    volatile const uint32_t opmode0_0;
    uint32_t reserved2[6];
    volatile const uint32_t sram_cycles0_1;
    volatile const uint32_t opmode0_1;
    uint32_t reserved3[22];
    volatile const uint32_t nand_cycles1_0;
    volatile const uint32_t opmode1_0;
    uint32_t reserved4[30];
    volatile const uint32_t user_status;
    volatile uint32_t user_config;
    uint32_t reserved5[126];
    volatile const uint32_t ecc_status_1;
    volatile uint32_t ecc_memcfg_1;
    volatile uint32_t ecc_memcommand1_1;
    volatile uint32_t ecc_memcommand2_1;
    volatile const uint32_t ecc_addr0_1;
    volatile uint32_t ecc_addr1_1;
    volatile const uint32_t ecc_value0_1;
    volatile const uint32_t ecc_value1_1;
    volatile const uint32_t ecc_value2_1;
    volatile const uint32_t ecc_value3_1;
};

struct afi
{
    volatile uint32_t afi_rdchan_ctrl;
    volatile uint32_t afi_rdchan_issuingcap;
    volatile uint32_t afi_rdqos;
    volatile uint32_t afi_rddatafifo_level;
    volatile uint32_t afi_rddebug;
    volatile uint32_t afi_wrchan_ctrl;
    volatile uint32_t afi_wrchan_issuingcap;
    volatile uint32_t afi_wrqos;
    volatile uint32_t afi_wrdatafifo_level;
    volatile uint32_t afi_wrdebug;
};

struct xadc
{
    volatile uint32_t cfg;
    volatile uint32_t int_sts;
    volatile uint32_t int_mask;
    volatile const uint32_t msts;
    volatile uint32_t cmdfifo;
    volatile const uint32_t rdfifo;
    volatile uint32_t mctl;
};

struct mpcore
{
    volatile uint32_t scu_control;
    volatile const uint32_t scu_config;
    volatile uint32_t scu_cpu_power_status;
    volatile uint32_t scu_invalidate;
    uint32_t reserved0[12];
    volatile uint32_t scu_filtering_start;
    volatile uint32_t scu_filtering_end;
    uint32_t reserved1[2];
    volatile uint32_t scu_access_control;
    volatile uint32_t scu_nonsecure_access;
    uint32_t reserved2[106];
    volatile uint32_t gt_counter_lo;
    volatile uint32_t gt_counter_hi;
    volatile uint32_t gt_control;
    volatile const uint32_t gt_isr;
    volatile uint32_t gt_comparator_lo;
    volatile uint32_t gt_comparator_hi;
    volatile uint32_t gt_auto_increment;
    uint32_t reserved3[249];
    volatile uint32_t ptim_load;
    volatile const uint32_t ptim_counter;
    volatile uint32_t ptim_control;
    volatile uint32_t ptim_isr;
    uint32_t reserved4[4];
    volatile uint32_t wd_load;
    volatile const uint32_t wd_counter;
    volatile uint32_t wd_control;
    volatile uint32_t wd_isr;
};

struct gicd
{
    volatile uint32_t ctlr;        /* 0x000 */
    volatile const uint32_t typer; /* 0x004 */
    uint32_t reserved0[30];
    volatile uint32_t igroupr[3]; /* 0x080 */
    uint32_t reserved1[29];
    volatile uint32_t isenabler[3]; /* 0x100 */
    uint32_t reserved2[29];
    volatile uint32_t icenabler[3]; /* 0x180 */
    uint32_t reserved3[157];
    volatile uint32_t ipriorityr[24]; /* 0x400 */
    uint32_t reserved4[232];
    volatile uint32_t itargetsr[24]; /* 0x800 */
};

static_assert(offsetof(struct gicd, igroupr) == 0x080, "gicd layout");
static_assert(offsetof(struct gicd, isenabler) == 0x100, "gicd layout");
static_assert(offsetof(struct gicd, icenabler) == 0x180, "gicd layout");
static_assert(offsetof(struct gicd, ipriorityr) == 0x400, "gicd layout");
static_assert(offsetof(struct gicd, itargetsr) == 0x800, "gicd layout");

struct gicc
{
    volatile uint32_t ctlr;      /* 0x000 */
    volatile uint32_t pmr;       /* 0x004 */
    volatile uint32_t bpr;       /* 0x008 */
    volatile const uint32_t iar; /* 0x00C */
    volatile uint32_t eoir;      /* 0x010 */
};

struct l2c
{
    uint32_t reserved0[64];
    volatile uint32_t control;        /* 0x100 */
    volatile uint32_t aux_cnt;        /* 0x104 */
    volatile uint32_t tag_ram_cntrl;  /* 0x108 */
    volatile uint32_t data_ram_cntrl; /* 0x10C */
    uint32_t reserved1[392];
    volatile uint32_t cache_sync; /* 0x730 */
    uint32_t reserved2[15];
    volatile uint32_t inv_line_pa; /* 0x770 */
    uint32_t reserved3[2];
    volatile uint32_t inv_way; /* 0x77C */
    uint32_t reserved4[12];
    volatile uint32_t clean_line_pa; /* 0x7B0 */
};

static_assert(offsetof(struct l2c, control) == 0x100, "l2c layout");
static_assert(offsetof(struct l2c, cache_sync) == 0x730, "l2c layout");
static_assert(offsetof(struct l2c, inv_line_pa) == 0x770, "l2c layout");
static_assert(offsetof(struct l2c, inv_way) == 0x77C, "l2c layout");
static_assert(offsetof(struct l2c, clean_line_pa) == 0x7B0, "l2c layout");

#define slcr    ((struct slcr *)SLCR_BASE)
#define ddrc    ((struct ddrc *)DDRC_BASE)
#define devcfg  ((struct devcfg *)DEVCFG_BASE)
#define dmac    ((struct dmac *)DMAC_BASE)
#define dmac_ns ((struct dmac *)DMAC_NS_BASE)
#define gem0    ((struct gem *)GEM0_BASE)
#define gem1    ((struct gem *)GEM1_BASE)
#define gpio    ((struct gpio *)GPIO_BASE)
#define i2c0    ((struct i2c *)I2C0_BASE)
#define i2c1    ((struct i2c *)I2C1_BASE)
#define qspi    ((struct qspi *)QSPI_BASE)
#define sdio0   ((struct sdio *)SDIO0_BASE)
#define sdio1   ((struct sdio *)SDIO1_BASE)
#define spi0    ((struct spi *)SPI0_BASE)
#define spi1    ((struct spi *)SPI1_BASE)
#define swdt    ((struct swdt *)SWDT_BASE)
#define ttc0    ((struct ttc *)TTC0_BASE)
#define ttc1    ((struct ttc *)TTC1_BASE)
#define uart0   ((struct uart *)UART0_BASE)
#define uart1   ((struct uart *)UART1_BASE)
#define can0    ((struct can *)CAN0_BASE)
#define can1    ((struct can *)CAN1_BASE)
#define usb0    ((struct usb *)USB0_BASE)
#define usb1    ((struct usb *)USB1_BASE)
#define ocm     ((struct ocm *)OCM_BASE)
#define smc     ((struct smc *)SMC_BASE)
#define afi0    ((struct afi *)AFI0_BASE)
#define afi1    ((struct afi *)AFI1_BASE)
#define afi2    ((struct afi *)AFI2_BASE)
#define afi3    ((struct afi *)AFI3_BASE)
#define mpcore  ((struct mpcore *)MPCORE_BASE)
#define gicd    ((struct gicd *)GICD_BASE)
#define gicc    ((struct gicc *)GICC_BASE)
#define l2c     ((struct l2c *)L2C_BASE)

#define xadc ((struct xadc *)XADC_BASE)

#define SLCR_LOCK_KEY   (0x767BU)
#define SLCR_UNLOCK_KEY (0xDF0DU)

#define MIO_PIN_OPT_DIS_RCVR       (1U << 13)
#define MIO_PIN_OPT_ENA_PULLUP     (1U << 12)
#define MIO_PIN_OPT_IO_HSTL        (4U << 9)
#define MIO_PIN_OPT_IO_CMOS_3V3    (3U << 9)
#define MIO_PIN_OPT_IO_CMOS_2V5    (2U << 9)
#define MIO_PIN_OPT_IO_CMOS_1V8    (1U << 9)
#define MIO_PIN_OPT_ENA_SPEED_FAST (1U << 8)
#define MIO_PIN_OPT_LVL37_MUX      (7U << 5)
#define MIO_PIN_OPT_LVL36_MUX      (6U << 5)
#define MIO_PIN_OPT_LVL35_MUX      (5U << 5)
#define MIO_PIN_OPT_LVL34_MUX      (4U << 5)
#define MIO_PIN_OPT_LVL33_MUX      (3U << 5)
#define MIO_PIN_OPT_LVL32_MUX      (2U << 5)
#define MIO_PIN_OPT_LVL31_MUX      (1U << 5)
#define MIO_PIN_OPT_LVL30_MUX      (0U << 5)
#define MIO_PIN_OPT_LVL23_MUX      (3U << 3)
#define MIO_PIN_OPT_LVL22_MUX      (2U << 3)
#define MIO_PIN_OPT_LVL21_MUX      (1U << 3)
#define MIO_PIN_OPT_LVL20_MUX      (0U << 3)
#define MIO_PIN_OPT_LVL12_MUX      (1U << 2)
#define MIO_PIN_OPT_LVL11_MUX      (1U << 2)
#define MIO_PIN_OPT_LVL10_MUX      (0U << 2)
#define MIO_PIN_OPT_LVL01_MUX      (1U << 1)
#define MIO_PIN_OPT_LVL00_MUX      (0U << 1)

#define UART_CR_RXRST      (0x1U)
#define UART_CR_TXRST      (0x2U)
#define UART_CR_RX_EN      (0x4U)
#define UART_CR_RX_DIS     (0x8U)
#define UART_CR_TX_EN      (0x10U)
#define UART_CR_TX_DIS     (0x20U)
#define UART_CR_EN_DIS_Pos (2U)
#define UART_CR_EN_DIS_Msk (0x3CU)
#define UART_CR_TORST      (0x40U)
#define UART_CR_STARTBRK   (0x80U)
#define UART_CR_STOPBRK    (0x100U)

#define UART_MR_CHARLEN_8_BIT    (0x0U)
#define UART_MR_CHMODE_NORM      (0x0U)
#define UART_MR_PARITY_EVEN      (0x0U)
#define UART_MR_STOPMODE_1_BIT   (0x0U)
#define UART_MR_CLKSEL           (0x1U)
#define UART_MR_CHARLEN_7_BIT    (0x4U)
#define UART_MR_CHARLEN_6_BIT    (0x6U)
#define UART_MR_CHARLEN_Pos      (1U)
#define UART_MR_CHARLEN_Msk      (0x6U)
#define UART_MR_PARITY_ODD       (0x8U)
#define UART_MR_PARITY_SPACE     (0x10U)
#define UART_MR_PARITY_MARK      (0x18U)
#define UART_MR_PARITY_NONE      (0x20U)
#define UART_MR_PARITY_Pos       (3U)
#define UART_MR_PARITY_Msk       (0x38U)
#define UART_MR_STOPMODE_1_5_BIT (0x40U)
#define UART_MR_STOPMODE_2_BIT   (0x80U)
#define UART_MR_STOPMODE_Msk     (0xC0U)
#define UART_MR_CHMODE_ECHO      (0x100U)
#define UART_MR_CHMODE_L_LOOP    (0x200U)
#define UART_MR_CHMODE_Pos       (8U)
#define UART_MR_CHMODE_Msk       (0x300U)
#define UART_MR_CHMODE_R_LOOP    (0x300U)
#define UART_MR_CCLK             (0x400U)

#define UART_IXR_RXOVR    (0x1U)
#define UART_IXR_RXEMPTY  (0x2U)
#define UART_IXR_RXFULL   (0x4U)
#define UART_IXR_TXEMPTY  (0x8U)
#define UART_IXR_TXFULL   (0x10U)
#define UART_IXR_OVER     (0x20U)
#define UART_IXR_FRAMING  (0x40U)
#define UART_IXR_PARITY   (0x80U)
#define UART_IXR_TOUT     (0x100U)
#define UART_IXR_DMS      (0x200U)
#define UART_IXR_TTRIG    (0x400U)
#define UART_IXR_TNFUL    (0x800U)
#define UART_IXR_TOVR     (0x1000U)
#define UART_IXR_RBRK     (0x2000U)
#define UART_IXR_MASK_Pos (0U)
#define UART_IXR_MASK_Msk (0x3FFFU)

#define UART_BAUDGEN_DISABLE   (0x0U)
#define UART_BAUDGEN_RESET_VAL (0x28BU)
#define UART_BAUDGEN_MASK_Pos  (0U)
#define UART_BAUDGEN_MASK_Msk  (0xFFFFU)

#define UART_BAUDDIV_RESET_VAL (0xFU)
#define UART_BAUDDIV_MASK_Pos  (0U)
#define UART_BAUDDIV_MASK_Msk  (0xFFU)

#define UART_RXTOUT_DISABLE  (0x0U)
#define UART_RXTOUT_MASK_Pos (0U)
#define UART_RXTOUT_MASK_Msk (0xFFU)

#define UART_RXWM_DISABLE   (0x0U)
#define UART_RXWM_RESET_VAL (0x20U)
#define UART_RXWM_MASK_Pos  (0U)
#define UART_RXWM_MASK_Msk  (0x3FU)

#define UART_TXWM_RESET_VAL (0x20U)
#define UART_TXWM_MASK_Pos  (0U)
#define UART_TXWM_MASK_Msk  (0x3FU)

#define UART_MODEMCR_DTR (0x1U)
#define UART_MODEMCR_RTS (0x2U)
#define UART_MODEMCR_FCM (0x20U)

#define UART_MODEMSR_DCTS (0x1U)
#define UART_MODEMSR_DDSR (0x2U)
#define UART_MODEMSR_TERI (0x4U)
#define UART_MODEMSR_DDCD (0x8U)
#define UART_MODEMSR_CTS  (0x10U)
#define UART_MODEMSR_DSR  (0x20U)
#define UART_MODEMSR_RI   (0x40U)
#define UART_MODEMSR_DCD  (0x80U)
#define UART_MODEMSR_FCMS (0x100U)

#define UART_SR_RXOVR   (0x1U)
#define UART_SR_RXEMPTY (0x2U)
#define UART_SR_RXFULL  (0x4U)
#define UART_SR_TXEMPTY (0x8U)
#define UART_SR_TXFULL  (0x10U)
#define UART_SR_RACTIVE (0x400U)
#define UART_SR_TACTIVE (0x800U)
#define UART_SR_FLOWDEL (0x1000U)
#define UART_SR_TTRIG   (0x2000U)
#define UART_SR_TNFUL   (0x4000U)

#define UART_RXBS_BYTE0_PARE (0x1U)
#define UART_RXBS_BYTE0_FRME (0x2U)
#define UART_RXBS_BYTE0_BRKE (0x4U)
#define UART_RXBS_MASK_Pos   (0U)
#define UART_RXBS_MASK_Msk   (0x7U)
#define UART_RXBS_BYTE1_PARE (0x8U)
#define UART_RXBS_BYTE1_FRME (0x10U)
#define UART_RXBS_BYTE1_BRKE (0x20U)
#define UART_RXBS_BYTE2_PARE (0x40U)
#define UART_RXBS_BYTE2_FRME (0x80U)
#define UART_RXBS_BYTE2_BRKE (0x100U)
#define UART_RXBS_BYTE3_PARE (0x200U)
#define UART_RXBS_BYTE3_FRME (0x400U)
#define UART_RXBS_BYTE3_BRKE (0x800U)

#define GPIO_INTTYPE_BANK1_RESET (0x3FFFFFU)
#define GPIO_INTTYPE_BANK0_RESET (0xFFFFFFFFU)
#define GPIO_INTTYPE_BANK2_RESET (0xFFFFFFFFU)
#define GPIO_INTTYPE_BANK3_RESET (0xFFFFFFFFU)
#define GPIO_INTTYPE_BANK4_RESET (0xFFFFFFFFU)
#define GPIO_INTTYPE_BANK5_RESET (0xFFFFFFFFU)

#define GPIO_PS_GPIO_BASEADDR (0xFF0B0000U)

#define GPIO_ZERO (0x0U)

#define GPIO_ONE (0x1U)

#define GPIO_TWO (0x2U)

#define GPIO_THREE (0x3U)

#define GPIO_FOUR (0x4U)

#define GPIO_SIX (0x6U)

#define QSPI_CR_MSTREN_Pos    (0U)
#define QSPI_CR_MSTREN_Msk    (0x1U)
#define QSPI_CR_CPOL_Pos      (1U)
#define QSPI_CR_CPOL_Msk      (0x2U)
#define QSPI_CR_CPHA_Pos      (2U)
#define QSPI_CR_CPHA_Msk      (0x4U)
#define QSPI_CR_PRESC_MAXIMUM (0x7U)
#define QSPI_CR_PRESC_Pos     (3U)
#define QSPI_CR_PRESC_Msk     (0x38U)
#define QSPI_CR_REF_CLK_Pos   (8U)
#define QSPI_CR_REF_CLK_Msk   (0x100U)
#define QSPI_CR_SSCTRL_Pos    (10U)
#define QSPI_CR_SSCTRL_Msk    (0x400U)
#define QSPI_CR_SSFORCE_Pos   (14U)
#define QSPI_CR_SSFORCE_Msk   (0x4000U)
#define QSPI_CR_MANSTRT_Pos   (16U)
#define QSPI_CR_MANSTRT_Msk   (0x10000U)
#define QSPI_CR_HOLD_B_Pos    (19U)
#define QSPI_CR_HOLD_B_Msk    (0x80000U)
#define QSPI_CR_ENDIAN_Pos    (26U)
#define QSPI_CR_ENDIAN_Msk    (0x4000000U)
#define QSPI_CR_IFMODE_Pos    (31U)
#define QSPI_CR_IFMODE_Msk    (0x80000000U)

#define QSPI_IXR_RXOVR_Pos    (0U)
#define QSPI_IXR_RXOVR_Msk    (0x1U)
#define QSPI_IXR_TXOW_Pos     (2U)
#define QSPI_IXR_TXOW_Msk     (0x4U)
#define QSPI_IXR_TXFULL_Pos   (3U)
#define QSPI_IXR_TXFULL_Msk   (0x8U)
#define QSPI_IXR_RXNEMPTY_Pos (4U)
#define QSPI_IXR_RXNEMPTY_Msk (0x10U)
#define QSPI_IXR_RXFULL_Pos   (5U)
#define QSPI_IXR_RXFULL_Msk   (0x20U)
#define QSPI_IXR_TXUF_Pos     (6U)
#define QSPI_IXR_TXUF_Msk     (0x40U)
#define QSPI_IXR_DISABLE_ALL  (0x7DU)

#define QSPI_ISR_RESET_STATE (0x4U)

#define QSPI_ER_ENABLE_Pos (0U)
#define QSPI_ER_ENABLE_Msk (0x1U)

#define QSPI_DR_INIT_Pos  (0U)
#define QSPI_DR_INIT_Msk  (0xFFU)
#define QSPI_DR_AFTER_Pos (8U)
#define QSPI_DR_AFTER_Msk (0xFF00U)

#define QSPI_SICR_MASK_Pos (0U)
#define QSPI_SICR_MASK_Msk (0xFFU)

#define QSPI_TXWR_MASK_Pos (0U)
#define QSPI_TXWR_MASK_Msk (0x3FU)

#define QSPI_RXWR_MASK_Pos (0U)
#define QSPI_RXWR_MASK_Msk (0x3FU)

#define QSPI_FIFO_DEPTH (0x3FU)

#define QSPI_LQSPI_CR_INST_Pos    (0U)
#define QSPI_LQSPI_CR_INST_Msk    (0xFFU)
#define QSPI_LQSPI_CR_MODE_ON_Pos (24U)
#define QSPI_LQSPI_CR_MODE_ON_Msk (0x1000000U)
#define QSPI_LQSPI_CR_MODE_EN_Pos (25U)
#define QSPI_LQSPI_CR_MODE_EN_Msk (0x2000000U)
#define QSPI_LQSPI_CR_U_PAGE_Pos  (28U)
#define QSPI_LQSPI_CR_U_PAGE_Msk  (0x10000000U)
#define QSPI_LQSPI_CR_SEP_BUS_Pos (29U)
#define QSPI_LQSPI_CR_SEP_BUS_Msk (0x20000000U)
#define QSPI_LQSPI_CR_TWO_MEM_Pos (30U)
#define QSPI_LQSPI_CR_TWO_MEM_Msk (0x40000000U)
#define QSPI_LQSPI_CR_LINEAR_Pos  (31U)
#define QSPI_LQSPI_CR_LINEAR_Msk  (0x80000000U)
#define QSPI_LQSPI_CR_RST_STATE   (0x8000016BU)

#define QSPI_LPBK_DLY_ADJ_USE_LPBK_Pos (5U)
#define QSPI_LPBK_DLY_ADJ_USE_LPBK_Msk (0x20U)

#define QSPI_DUMMY_TX_DATA (0xFFFFFFFFU)

#define I2C_CR_RD_WR_Pos    (0U)
#define I2C_CR_RD_WR_Msk    (0x1U)
#define I2C_CR_MS_Pos       (1U)
#define I2C_CR_MS_Msk       (0x2U)
#define I2C_CR_NEA_Pos      (2U)
#define I2C_CR_NEA_Msk      (0x4U)
#define I2C_CR_ACKEN_Pos    (3U)
#define I2C_CR_ACKEN_Msk    (0x8U)
#define I2C_CR_HOLD_Pos     (4U)
#define I2C_CR_HOLD_Msk     (0x10U)
#define I2C_CR_SLVMON_Pos   (5U)
#define I2C_CR_SLVMON_Msk   (0x20U)
#define I2C_CR_CLR_FIFO_Pos (6U)
#define I2C_CR_CLR_FIFO_Msk (0x40U)
#define I2C_CR_DIV_B_Pos    (8U)
#define I2C_CR_DIV_B_Msk    (0x3F00U)
#define I2C_CR_DIV_A_Pos    (14U)
#define I2C_CR_DIV_A_Msk    (0xC000U)

#define I2C_DIV_A_MAX (0x4U)

#define I2C_SR_RXRW_Pos  (3U)
#define I2C_SR_RXRW_Msk  (0x8U)
#define I2C_SR_RXDV_Pos  (5U)
#define I2C_SR_RXDV_Msk  (0x20U)
#define I2C_SR_TXDV_Pos  (6U)
#define I2C_SR_TXDV_Msk  (0x40U)
#define I2C_SR_RXOVF_Pos (7U)
#define I2C_SR_RXOVF_Msk (0x80U)
#define I2C_SR_BA_Pos    (8U)
#define I2C_SR_BA_Msk    (0x100U)

#define I2C_ADDR_MASK_Pos (0U)
#define I2C_ADDR_MASK_Msk (0x3FFU)

#define I2C_DATA_INTR_DEPTH (0xEU)
#define I2C_DATA_MASK_Pos   (0U)
#define I2C_DATA_MASK_Msk   (0xFFU)

#define I2C_IXR_COMP_Pos     (0U)
#define I2C_IXR_COMP_Msk     (0x1U)
#define I2C_IXR_DATA_Pos     (1U)
#define I2C_IXR_DATA_Msk     (0x2U)
#define I2C_IXR_NACK_Pos     (2U)
#define I2C_IXR_NACK_Msk     (0x4U)
#define I2C_IXR_TO_Pos       (3U)
#define I2C_IXR_TO_Msk       (0x8U)
#define I2C_IXR_SLV_RDY_Pos  (4U)
#define I2C_IXR_SLV_RDY_Msk  (0x10U)
#define I2C_IXR_RX_OVR_Pos   (5U)
#define I2C_IXR_RX_OVR_Msk   (0x20U)
#define I2C_IXR_TX_OVR_Pos   (6U)
#define I2C_IXR_TX_OVR_Msk   (0x40U)
#define I2C_IXR_RX_UNF_Pos   (7U)
#define I2C_IXR_RX_UNF_Msk   (0x80U)
#define I2C_IXR_ARB_LOST_Pos (9U)
#define I2C_IXR_ARB_LOST_Msk (0x200U)
#define I2C_IXR_ALL_INTR_Msk (0x2FFU)
#define I2C_IXR_DEFAULT_Msk  (0x2FFU)

#define I2C_TRANS_SIZE_Pos (0U)
#define I2C_TRANS_SIZE_Msk (0xFFU)

#define I2C_FIFO_DEPTH (0x10U)

#define I2C_SLV_PAUSE_Pos (0U)
#define I2C_SLV_PAUSE_Msk (0xFU)

#define I2C_TIME_OUT_Pos (0U)
#define I2C_TIME_OUT_Msk (0xFFU)

#define I2C_TO_RESET_VALUE (0xFFU)

#define I2C_POLL_DEFAULT_TIMEOUT_VAL (0x3E8U)

#define SPI_CR_MSTREN_Pos     (0U)
#define SPI_CR_MSTREN_Msk     (0x1U)
#define SPI_CR_CPOL_Pos       (1U)
#define SPI_CR_CPOL_Msk       (0x2U)
#define SPI_CR_CPHA_Pos       (2U)
#define SPI_CR_CPHA_Msk       (0x4U)
#define SPI_CR_PRESC_MAXIMUM  (0x7U)
#define SPI_CR_SSCTRL_MAXIMUM (0xFU)
#define SPI_CR_PRESC_Pos      (3U)
#define SPI_CR_PRESC_Msk      (0x38U)
#define SPI_CR_SSDECEN_Pos    (9U)
#define SPI_CR_SSDECEN_Msk    (0x200U)
#define SPI_CR_SSCTRL_Pos     (10U)
#define SPI_CR_SSCTRL_Msk     (0x3C00U)
#define SPI_CR_SSFORCE_Pos    (14U)
#define SPI_CR_SSFORCE_Msk    (0x4000U)
#define SPI_CR_MANSTRT_Pos    (16U)
#define SPI_CR_MANSTRT_Msk    (0x10000U)
#define SPI_CR_RESET_STATE    (0x20000U)

#define SPI_IXR_RXOVR_Pos    (0U)
#define SPI_IXR_RXOVR_Msk    (0x1U)
#define SPI_IXR_MODF_Pos     (1U)
#define SPI_IXR_MODF_Msk     (0x2U)
#define SPI_IXR_TXOW_Pos     (2U)
#define SPI_IXR_TXOW_Msk     (0x4U)
#define SPI_IXR_TXFULL_Pos   (3U)
#define SPI_IXR_TXFULL_Msk   (0x8U)
#define SPI_IXR_RXNEMPTY_Pos (4U)
#define SPI_IXR_RXNEMPTY_Msk (0x10U)
#define SPI_IXR_RXFULL_Pos   (5U)
#define SPI_IXR_RXFULL_Msk   (0x20U)
#define SPI_IXR_TXUF_Pos     (6U)
#define SPI_IXR_TXUF_Msk     (0x40U)
#define SPI_IXR_MASK_Msk     (0x7FU)

#define SPI_ER_ENABLE_Pos (0U)
#define SPI_ER_ENABLE_Msk (0x1U)

#define SPI_DR_INIT_Pos  (0U)
#define SPI_DR_INIT_Msk  (0xFFU)
#define SPI_DR_AFTER_Pos (8U)
#define SPI_DR_AFTER_Msk (0xFF00U)
#define SPI_DR_BTWN_Pos  (16U)
#define SPI_DR_BTWN_Msk  (0xFF0000U)

#define SPI_SICR_MASK_Pos (0U)
#define SPI_SICR_MASK_Msk (0xFFU)

#define SPI_TXWR_MASK_Pos (0U)
#define SPI_TXWR_MASK_Msk (0x7FU)

#define SPI_RXWR_MASK_Pos (0U)
#define SPI_RXWR_MASK_Msk (0x7FU)

#define SPI_FIFO_DEPTH (0x80U)

#define GEM_MAX_TYPE_ID   (0x4U)
#define GEM_MAX_HASH_BITS (0x40U)
#define GEM_MAX_RXBD      (0x80U)
#define GEM_MAX_TXBD      (0x80U)

#define GEM_SEND (0x1U)

#define GEM_RECV (0x2U)

#define GEM_RX_SCR_BYPASS_SIZE (0x1U)
#define GEM_RX_SYNC_RESET_SIZE (0x1U)
#define GEM_RX_BUF_SIZE_JUMBO  (0x2800U)

#define GEM_SCREEN_TYPE2_REG0 (0x540U)

#define GEM_ENABLE_HS_MAC_SIZE (0x1U)

#define GEM_NWCTRL_LOOPEN_Pos  (1U)
#define GEM_NWCTRL_LOOPEN_Msk  (0x2U)
#define GEM_NWCTRL_RXEN_Pos    (2U)
#define GEM_NWCTRL_RXEN_Msk    (0x4U)
#define GEM_NWCTRL_TXEN_Pos    (3U)
#define GEM_NWCTRL_TXEN_Msk    (0x8U)
#define GEM_NWCTRL_MDEN_Pos    (4U)
#define GEM_NWCTRL_MDEN_Msk    (0x10U)
#define GEM_NWCTRL_STARTTX_Pos (9U)
#define GEM_NWCTRL_STARTTX_Msk (0x200U)
#define GEM_NWCTRL_PAUSETX_Pos (11U)
#define GEM_NWCTRL_PAUSETX_Msk (0x800U)

#define GEM_NWCFG_100_Pos         (0U)
#define GEM_NWCFG_100_Msk         (0x1U)
#define GEM_NWCFG_FDEN_Pos        (1U)
#define GEM_NWCFG_FDEN_Msk        (0x2U)
#define GEM_NWCFG_JUMBO_Pos       (3U)
#define GEM_NWCFG_JUMBO_Msk       (0x8U)
#define GEM_NWCFG_COPYALLEN_Pos   (4U)
#define GEM_NWCFG_COPYALLEN_Msk   (0x10U)
#define GEM_NWCFG_MDC_SHIFT_Msk   (0x12U)
#define GEM_NWCFG_XTADDMACHEN_Pos (9U)
#define GEM_NWCFG_XTADDMACHEN_Msk (0x200U)
#define GEM_NWCFG_1000_Pos        (10U)
#define GEM_NWCFG_1000_Msk        (0x400U)
#define GEM_NWCFG_PCSSEL_Pos      (11U)
#define GEM_NWCFG_PCSSEL_Msk      (0x800U)
#define GEM_NWCFG_RETRYTESTEN_Pos (12U)
#define GEM_NWCFG_RETRYTESTEN_Msk (0x1000U)
#define GEM_NWCFG_PAUSEEN_Pos     (13U)
#define GEM_NWCFG_PAUSEEN_Msk     (0x2000U)
#define GEM_NWCFG_RXOFFS_Pos      (14U)
#define GEM_NWCFG_RXOFFS_Msk      (0xC000U)
#define GEM_NWCFG_LENERRDSCRD_Pos (16U)
#define GEM_NWCFG_LENERRDSCRD_Msk (0x10000U)
#define GEM_NWCFG_RESET_Pos       (19U)
#define GEM_NWCFG_RESET_Msk       (0x80000U)
#define GEM_NWCFG_MDCCLKDIV_Pos   (18U)
#define GEM_NWCFG_MDCCLKDIV_Msk   (0x1C0000U)
#define GEM_NWCFG_DWIDTH_64_Pos   (21U)
#define GEM_NWCFG_DWIDTH_64_Msk   (0x200000U)
#define GEM_NWCFG_DWIDTH_128_Pos  (22U)
#define GEM_NWCFG_DWIDTH_128_Msk  (0x400000U)
#define GEM_NWCFG_HDRXEN_Pos      (25U)
#define GEM_NWCFG_HDRXEN_Msk      (0x2000000U)
#define GEM_NWCFG_SGMIIEN_Pos     (27U)
#define GEM_NWCFG_SGMIIEN_Msk     (0x8000000U)
#define GEM_NWCFG_IPDSTRETCH_Pos  (28U)
#define GEM_NWCFG_IPDSTRETCH_Msk  (0x10000000U)

#define GEM_NWSR_MDIO_Pos     (1U)
#define GEM_NWSR_MDIO_Msk     (0x2U)
#define GEM_NWSR_MDIOIDLE_Pos (2U)
#define GEM_NWSR_MDIOIDLE_Msk (0x4U)

#define GEM_DMACR_SINGLE_AHB_BURST (0x1U)
#define GEM_DMACR_INCR4_AHB_BURST  (0x4U)
#define GEM_DMACR_INCR8_AHB_BURST  (0x8U)
#define GEM_DMACR_INCR16_AHB_BURST (0x10U)
#define GEM_DMACR_BLENGTH_Pos      (0U)
#define GEM_DMACR_BLENGTH_Msk      (0x1FU)
#define GEM_DMACR_ENDIAN_Pos       (7U)
#define GEM_DMACR_ENDIAN_Msk       (0x80U)
#define GEM_DMACR_RXSIZE_Pos       (8U)
#define GEM_DMACR_RXSIZE_Msk       (0x300U)
#define GEM_DMACR_TXSIZE_Pos       (10U)
#define GEM_DMACR_TXSIZE_Msk       (0x400U)
#define GEM_DMACR_RXEXTEND_Pos     (28U)
#define GEM_DMACR_RXEXTEND_Msk     (0x10000000U)
#define GEM_DMACR_TXEXTEND_Pos     (29U)
#define GEM_DMACR_TXEXTEND_Msk     (0x20000000U)
#define GEM_DMACR_ADDR_WIDTH_64    (0x40000000U)

#define GEM_TXSR_USEDREAD_Pos (0U)
#define GEM_TXSR_USEDREAD_Msk (0x1U)
#define GEM_TXSR_FRAMERX_Pos  (1U)
#define GEM_TXSR_FRAMERX_Msk  (0x2U)
#define GEM_TXSR_RXOVR_Pos    (2U)
#define GEM_TXSR_RXOVR_Msk    (0x4U)
#define GEM_TXSR_TXGO_Pos     (3U)
#define GEM_TXSR_TXGO_Msk     (0x8U)
#define GEM_TXSR_TXCOMPL_Pos  (5U)
#define GEM_TXSR_TXCOMPL_Msk  (0x20U)
#define GEM_TXSR_URUN_Pos     (6U)
#define GEM_TXSR_URUN_Msk     (0x40U)
#define GEM_TXSR_HRESPNOK_Pos (8U)
#define GEM_TXSR_HRESPNOK_Msk (0x100U)

#define GEM_RXSR_BUFFNA_Pos   (0U)
#define GEM_RXSR_BUFFNA_Msk   (0x1U)
#define GEM_RXSR_FRAMERX_Pos  (1U)
#define GEM_RXSR_FRAMERX_Msk  (0x2U)
#define GEM_RXSR_RXOVR_Pos    (2U)
#define GEM_RXSR_RXOVR_Msk    (0x4U)
#define GEM_RXSR_HRESPNOK_Pos (3U)
#define GEM_RXSR_HRESPNOK_Msk (0x8U)

#define GEM_SR_ALL_Pos (0U)
#define GEM_SR_ALL_Msk (0xFFFFFFFFU)

#define GEM_HS_MAC_SPEED_SIZE (0x3U)

#define GEM_PCS_CON_AUTO_NEG_Pos (12U)
#define GEM_PCS_CON_AUTO_NEG_Msk (0x1000U)

#define GEM_LINK_STATUS_SIZE (0x1U)

#define GEM_USX_SPEED_SIZE (0x3U)

#define GEM_SERDES_RATE_SIZE (0x2U)

#define GEM_TX_EN_SIZE         (0x1U)
#define GEM_TX_SCR_BYPASS_SIZE (0x1U)

#define GEM_SIGNAL_OK_SIZE (0x1U)

#define GEM_BLOCK_LOCK_SIZE (0x1U)

#define GEM_NO_PCS_SIZE (0x1U)

#define GEM_HIGH_SPEED_SIZE (0x1U)

#define GEM_INTQ1SR_RXCOMPL_Pos (1U)
#define GEM_INTQ1SR_RXCOMPL_Msk (0x2U)
#define GEM_INTQ1SR_TXERR_Pos   (6U)
#define GEM_INTQ1SR_TXERR_Msk   (0x40U)
#define GEM_INTQ1SR_TXCOMPL_Pos (7U)
#define GEM_INTQ1SR_TXCOMPL_Msk (0x80U)

#define GEM_IXR_MGMNT_Pos      (0U)
#define GEM_IXR_MGMNT_Msk      (0x1U)
#define GEM_IXR_FRAMERX_Pos    (1U)
#define GEM_IXR_FRAMERX_Msk    (0x2U)
#define GEM_IXR_RXUSED_Pos     (2U)
#define GEM_IXR_RXUSED_Msk     (0x4U)
#define GEM_IXR_TXUSED_Pos     (3U)
#define GEM_IXR_TXUSED_Msk     (0x8U)
#define GEM_IXR_URUN_Pos       (4U)
#define GEM_IXR_URUN_Msk       (0x10U)
#define GEM_IXR_RETRY_Pos      (5U)
#define GEM_IXR_RETRY_Msk      (0x20U)
#define GEM_IXR_TXCOMPL_Pos    (7U)
#define GEM_IXR_TXCOMPL_Msk    (0x80U)
#define GEM_IXR_USXGMII_Pos    (8U)
#define GEM_IXR_USXGMII_Msk    (0x100U)
#define GEM_IXR_RXOVR_Pos      (10U)
#define GEM_IXR_RXOVR_Msk      (0x400U)
#define GEM_IXR_HRESPNOK_Pos   (11U)
#define GEM_IXR_HRESPNOK_Msk   (0x800U)
#define GEM_IXR_PAUSENZERO_Pos (12U)
#define GEM_IXR_PAUSENZERO_Msk (0x1000U)
#define GEM_IXR_PAUSETX_Pos    (14U)
#define GEM_IXR_PAUSETX_Msk    (0x4000U)
#define GEM_IXR_ALL_Pos        (0U)
#define GEM_IXR_ALL_Msk        (0x7FFFU)
#define GEM_IXR_PTPDRRX_Pos    (18U)
#define GEM_IXR_PTPDRRX_Msk    (0x40000U)
#define GEM_IXR_PTPSRX_Pos     (19U)
#define GEM_IXR_PTPSRX_Msk     (0x80000U)
#define GEM_IXR_PTPDRTX_Pos    (20U)
#define GEM_IXR_PTPDRTX_Msk    (0x100000U)
#define GEM_IXR_PTPSTX_Pos     (21U)
#define GEM_IXR_PTPSTX_Msk     (0x200000U)
#define GEM_IXR_PTPPDRRX_Pos   (22U)
#define GEM_IXR_PTPPDRRX_Msk   (0x400000U)
#define GEM_IXR_PTPPSRX_Pos    (23U)
#define GEM_IXR_PTPPSRX_Msk    (0x800000U)
#define GEM_IXR_PTPPDRTX_Pos   (24U)
#define GEM_IXR_PTPPDRTX_Msk   (0x1000000U)
#define GEM_IXR_PTPPSTX_Pos    (25U)
#define GEM_IXR_PTPPSTX_Msk    (0x2000000U)

#define GEM_PHYMNTNC_PREG_SHFT_MSK (0x12U)
#define GEM_PHYMNTNC_PHAD_SHFT_MSK (0x17U)
#define GEM_PHYMNTNC_DATA_Pos      (0U)
#define GEM_PHYMNTNC_DATA_Msk      (0xFFFU)
#define GEM_PHYMNTNC_REG_Pos       (18U)
#define GEM_PHYMNTNC_REG_Msk       (0x7C0000U)
#define GEM_PHYMNTNC_ADDR_Pos      (23U)
#define GEM_PHYMNTNC_ADDR_Msk      (0xF800000U)
#define GEM_PHYMNTNC_OP_W_Pos      (28U)
#define GEM_PHYMNTNC_OP_W_Msk      (0x10000000U)
#define GEM_PHYMNTNC_OP_R_Pos      (29U)
#define GEM_PHYMNTNC_OP_R_Msk      (0x20000000U)
#define GEM_PHYMNTNC_OP_Msk        (0x40020000U)

#define GEM_RXWM_LOW_SHFT_MSK (0x10U)
#define GEM_RXWM_HIGH_Pos     (0U)
#define GEM_RXWM_HIGH_Msk     (0xFFFFU)
#define GEM_RXWM_LOW_Pos      (16U)
#define GEM_RXWM_LOW_Msk      (0xFFFF0000U)

#define GEM_CMPA_MASK_Pos   (13U)
#define GEM_CMPA_MASK_Msk   (0x3E000U)
#define GEM_CMPA_ENABLE_Pos (18U)
#define GEM_CMPA_ENABLE_Msk (0x40000U)

#define GEM_TXBUF_LEN_Pos   (0U)
#define GEM_TXBUF_LEN_Msk   (0x3FFFU)
#define GEM_TXBUF_LAST_Pos  (15U)
#define GEM_TXBUF_LAST_Msk  (0x8000U)
#define GEM_TXBUF_NOCRC_Pos (16U)
#define GEM_TXBUF_NOCRC_Msk (0x10000U)
#define GEM_TXBUF_TCP_Pos   (26U)
#define GEM_TXBUF_TCP_Msk   (0x4000000U)
#define GEM_TXBUF_EXH_Pos   (27U)
#define GEM_TXBUF_EXH_Msk   (0x8000000U)
#define GEM_TXBUF_URUN_Pos  (28U)
#define GEM_TXBUF_URUN_Msk  (0x10000000U)
#define GEM_TXBUF_RETRY_Pos (29U)
#define GEM_TXBUF_RETRY_Msk (0x20000000U)
#define GEM_TXBUF_WRAP_Pos  (30U)
#define GEM_TXBUF_WRAP_Msk  (0x40000000U)
#define GEM_TXBUF_USED_Pos  (31U)
#define GEM_TXBUF_USED_Msk  (0x80000000U)

#define GEM_RXBUF_NEW_Pos       (0U)
#define GEM_RXBUF_NEW_Msk       (0x1U)
#define GEM_RXBUF_WRAP_Pos      (1U)
#define GEM_RXBUF_WRAP_Msk      (0x2U)
#define GEM_RXBUF_LEN_Pos       (0U)
#define GEM_RXBUF_LEN_Msk       (0x1FFFU)
#define GEM_RXBUF_LEN_JUMBO_Pos (0U)
#define GEM_RXBUF_LEN_JUMBO_Msk (0x3FFFU)
#define GEM_RXBUF_SOF_Pos       (14U)
#define GEM_RXBUF_SOF_Msk       (0x4000U)
#define GEM_RXBUF_EOF_Pos       (15U)
#define GEM_RXBUF_EOF_Msk       (0x8000U)
#define GEM_RXBUF_CFI_Pos       (16U)
#define GEM_RXBUF_CFI_Msk       (0x10000U)
#define GEM_RXBUF_VPRI_Pos      (17U)
#define GEM_RXBUF_VPRI_Msk      (0xE0000U)
#define GEM_RXBUF_PRI_Pos       (20U)
#define GEM_RXBUF_PRI_Msk       (0x100000U)
#define GEM_RXBUF_VLAN_Pos      (21U)
#define GEM_RXBUF_VLAN_Msk      (0x200000U)
#define GEM_RXBUF_IDMATCH_Pos   (22U)
#define GEM_RXBUF_IDMATCH_Msk   (0xC00000U)
#define GEM_RXBUF_IDFOUND_Pos   (24U)
#define GEM_RXBUF_IDFOUND_Msk   (0x1000000U)
#define GEM_RXBUF_EXH_Pos       (27U)
#define GEM_RXBUF_EXH_Msk       (0x8000000U)
#define GEM_RXBUF_UNIHASH_Pos   (29U)
#define GEM_RXBUF_UNIHASH_Msk   (0x20000000U)
#define GEM_RXBUF_MULTIHASH_Pos (30U)
#define GEM_RXBUF_MULTIHASH_Msk (0x40000000U)
#define GEM_RXBUF_BCAST_Pos     (31U)
#define GEM_RXBUF_BCAST_Msk     (0x80000000U)
#define GEM_RXBUF_ADD_Pos       (2U)
#define GEM_RXBUF_ADD_Msk       (0xFFFFFFFCU)

#define SDIO_HC_DMA_SDMA_Msk     (0x0U)
#define SDIO_HC_SPEC_V1          (0x0U)
#define SDIO_HC_LED_Pos          (0U)
#define SDIO_HC_LED_Msk          (0x1U)
#define SDIO_HC_SPEC_V2          (0x1U)
#define SDIO_HC_BUS_WIDTH_4      (0x2U)
#define SDIO_HC_SPEC_V3          (0x2U)
#define SDIO_HC_WIDTH_Pos        (1U)
#define SDIO_HC_WIDTH_Msk        (0x2U)
#define SDIO_HC_SPEED_Pos        (2U)
#define SDIO_HC_SPEED_Msk        (0x4U)
#define SDIO_HC_DMA_ADMA1_Pos    (3U)
#define SDIO_HC_DMA_ADMA1_Msk    (0x8U)
#define SDIO_HC_DMA_ADMA2_32_Pos (4U)
#define SDIO_HC_DMA_ADMA2_32_Msk (0x10U)
#define SDIO_HC_DMA_ADMA2_64_Pos (3U)
#define SDIO_HC_DMA_ADMA2_64_Msk (0x18U)
#define SDIO_HC_DMA_Pos          (3U)
#define SDIO_HC_DMA_Msk          (0x18U)
#define SDIO_HC_EXT_BUS_WIDTH    (0x20U)
#define SDIO_HC_CARD_DET_TL_Pos  (6U)
#define SDIO_HC_CARD_DET_TL_Msk  (0x40U)
#define SDIO_HC_CARD_DET_SD_Pos  (7U)
#define SDIO_HC_CARD_DET_SD_Msk  (0x80U)

#define SDIO_PC_BUS_PWR_Pos      (0U)
#define SDIO_PC_BUS_PWR_Msk      (0x1U)
#define SDIO_PC_BUS_VSEL_1V8_Msk (0xAU)
#define SDIO_PC_BUS_VSEL_3V0_Pos (2U)
#define SDIO_PC_BUS_VSEL_3V0_Msk (0xCU)
#define SDIO_PC_BUS_VSEL_3V3_Pos (1U)
#define SDIO_PC_BUS_VSEL_3V3_Msk (0xEU)
#define SDIO_PC_BUS_VSEL_Pos     (1U)
#define SDIO_PC_BUS_VSEL_Msk     (0xEU)
#define SDIO_PC_EMMC_HW_RST_Pos  (4U)
#define SDIO_PC_EMMC_HW_RST_Msk  (0x10U)

#define SDIO_BGC_STP_REQ_Pos     (0U)
#define SDIO_BGC_STP_REQ_Msk     (0x1U)
#define SDIO_BGC_CNT_REQ_Pos     (1U)
#define SDIO_BGC_CNT_REQ_Msk     (0x2U)
#define SDIO_BGC_RWC_Pos         (2U)
#define SDIO_BGC_RWC_Msk         (0x4U)
#define SDIO_BGC_INTR_Pos        (3U)
#define SDIO_BGC_INTR_Msk        (0x8U)
#define SDIO_BGC_SPI_MODE_Pos    (4U)
#define SDIO_BGC_SPI_MODE_Msk    (0x10U)
#define SDIO_BGC_BOOT_EN_Pos     (5U)
#define SDIO_BGC_BOOT_EN_Msk     (0x20U)
#define SDIO_BGC_ALT_BOOT_EN_Pos (6U)
#define SDIO_BGC_ALT_BOOT_EN_Msk (0x40U)
#define SDIO_BGC_BOOT_ACK_Pos    (7U)
#define SDIO_BGC_BOOT_ACK_Msk    (0x80U)

#define SDIO_WC_WUP_ON_INTR_Pos  (0U)
#define SDIO_WC_WUP_ON_INTR_Msk  (0x1U)
#define SDIO_WC_WUP_ON_INSRT_Pos (1U)
#define SDIO_WC_WUP_ON_INSRT_Msk (0x2U)
#define SDIO_WC_WUP_ON_REM_Pos   (2U)
#define SDIO_WC_WUP_ON_REM_Msk   (0x4U)

#define SDIO_CC_SDCLK_FREQ_BASE_Msk    (0x0U)
#define SDIO_CC_INT_CLK_EN_Pos         (0U)
#define SDIO_CC_INT_CLK_EN_Msk         (0x1U)
#define SDIO_CC_INT_CLK_STABLE_Pos     (1U)
#define SDIO_CC_INT_CLK_STABLE_Msk     (0x2U)
#define SDIO_CC_SDCLK_FREQ_SEL_EXT_Pos (0U)
#define SDIO_CC_SDCLK_FREQ_SEL_EXT_Msk (0x3U)
#define SDIO_CC_SD_CLK_EN_Pos          (2U)
#define SDIO_CC_SD_CLK_EN_Msk          (0x4U)
#define SDIO_CC_MAX_NUM_OF_DIV         (0x9U)
#define SDIO_CC_SD_CLK_GEN_SEL_Pos     (5U)
#define SDIO_CC_SD_CLK_GEN_SEL_Msk     (0x20U)
#define SDIO_CC_SDCLK_FREQ_SEL_Pos     (0U)
#define SDIO_CC_SDCLK_FREQ_SEL_Msk     (0xFFU)
#define SDIO_CC_MAX_DIV_CNT            (0x100U)
#define SDIO_CC_SDCLK_FREQ_D2_Pos      (8U)
#define SDIO_CC_SDCLK_FREQ_D2_Msk      (0x100U)
#define SDIO_CC_SDCLK_FREQ_D4_Pos      (9U)
#define SDIO_CC_SDCLK_FREQ_D4_Msk      (0x200U)
#define SDIO_CC_SDCLK_FREQ_D8_Pos      (10U)
#define SDIO_CC_SDCLK_FREQ_D8_Msk      (0x400U)
#define SDIO_CC_EXT_MAX_DIV_CNT        (0x7FEU)
#define SDIO_CC_SDCLK_FREQ_D16_Pos     (11U)
#define SDIO_CC_SDCLK_FREQ_D16_Msk     (0x800U)
#define SDIO_CC_SDCLK_FREQ_D32_Pos     (12U)
#define SDIO_CC_SDCLK_FREQ_D32_Msk     (0x1000U)
#define SDIO_CC_SDCLK_FREQ_D64_Pos     (13U)
#define SDIO_CC_SDCLK_FREQ_D64_Msk     (0x2000U)
#define SDIO_CC_SDCLK_FREQ_D128_Pos    (14U)
#define SDIO_CC_SDCLK_FREQ_D128_Msk    (0x4000U)
#define SDIO_CC_SDCLK_FREQ_D256_Pos    (15U)
#define SDIO_CC_SDCLK_FREQ_D256_Msk    (0x8000U)

#define SDIO_TC_CNTR_VAL_Pos (0U)
#define SDIO_TC_CNTR_VAL_Msk (0xFU)

#define SDIO_SWRST_ALL_Pos      (0U)
#define SDIO_SWRST_ALL_Msk      (0x1U)
#define SDIO_SWRST_CMD_LINE_Pos (1U)
#define SDIO_SWRST_CMD_LINE_Msk (0x2U)
#define SDIO_SWRST_DAT_LINE_Pos (2U)
#define SDIO_SWRST_DAT_LINE_Msk (0x4U)

#define SDIO_INTR_CC_Pos             (0U)
#define SDIO_INTR_CC_Msk             (0x1U)
#define SDIO_INTR_ERR_CCRC_Pos       (1U)
#define SDIO_INTR_ERR_CCRC_Msk       (0x2U)
#define SDIO_INTR_TC_Pos             (1U)
#define SDIO_INTR_TC_Msk             (0x2U)
#define SDIO_INTR_BGE_Pos            (2U)
#define SDIO_INTR_BGE_Msk            (0x4U)
#define SDIO_INTR_DMA_Pos            (3U)
#define SDIO_INTR_DMA_Msk            (0x8U)
#define SDIO_INTR_ERR_CI_Pos         (3U)
#define SDIO_INTR_ERR_CI_Msk         (0x8U)
#define SDIO_INTR_BWR_Pos            (4U)
#define SDIO_INTR_BWR_Msk            (0x10U)
#define SDIO_INTR_ERR_DT_Pos         (4U)
#define SDIO_INTR_ERR_DT_Msk         (0x10U)
#define SDIO_INTR_BRR_Pos            (5U)
#define SDIO_INTR_BRR_Msk            (0x20U)
#define SDIO_INTR_ERR_DCRC_Pos       (5U)
#define SDIO_INTR_ERR_DCRC_Msk       (0x20U)
#define SDIO_INTR_CARD_INSRT_Pos     (6U)
#define SDIO_INTR_CARD_INSRT_Msk     (0x40U)
#define SDIO_INTR_ERR_DEB_Pos        (6U)
#define SDIO_INTR_ERR_DEB_Msk        (0x40U)
#define SDIO_INTR_CARD_REM_Pos       (7U)
#define SDIO_INTR_CARD_REM_Msk       (0x80U)
#define SDIO_INTR_ERR_CUR_LMT_Pos    (7U)
#define SDIO_INTR_ERR_CUR_LMT_Msk    (0x80U)
#define SDIO_INTR_CARD_Pos           (8U)
#define SDIO_INTR_CARD_Msk           (0x100U)
#define SDIO_INTR_ERR_AUTO_CMD12_Pos (8U)
#define SDIO_INTR_ERR_AUTO_CMD12_Msk (0x100U)
#define SDIO_INTR_ERR_ADMA_Pos       (9U)
#define SDIO_INTR_ERR_ADMA_Msk       (0x200U)
#define SDIO_INTR_INT_A_Pos          (9U)
#define SDIO_INTR_INT_A_Msk          (0x200U)
#define SDIO_INTR_INT_B_Pos          (10U)
#define SDIO_INTR_INT_B_Msk          (0x400U)
#define SDIO_INTR_INT_C_Pos          (11U)
#define SDIO_INTR_INT_C_Msk          (0x800U)
#define SDIO_INTR_ERR_TR_Pos         (12U)
#define SDIO_INTR_ERR_TR_Msk         (0x1000U)
#define SDIO_INTR_RE_TUNING_Pos      (12U)
#define SDIO_INTR_RE_TUNING_Msk      (0x1000U)
#define SDIO_INTR_ERR_Pos            (15U)
#define SDIO_INTR_ERR_Msk            (0x8000U)

#define SDIO_NORM_INTR_ALL_Pos (0U)
#define SDIO_NORM_INTR_ALL_Msk (0xFFFFU)

#define SDIO_ERROR_INTR_ALL_Msk (0xF3FFU)

#define SDIO_BLK_SIZE_1024 (0x400U)
#define SDIO_BLK_SIZE_2048 (0x800U)
#define SDIO_BLK_SIZE_Pos  (0U)
#define SDIO_BLK_SIZE_Msk  (0xFFFU)

#define SDIO_SDMA_BUFF_SIZE_Pos (12U)
#define SDIO_SDMA_BUFF_SIZE_Msk (0x7000U)

#define SDIO_TM_DMA_EN_Pos        (0U)
#define SDIO_TM_DMA_EN_Msk        (0x1U)
#define SDIO_TM_BLK_CNT_EN_Pos    (1U)
#define SDIO_TM_BLK_CNT_EN_Msk    (0x2U)
#define SDIO_TM_AUTO_CMD12_EN_Pos (2U)
#define SDIO_TM_AUTO_CMD12_EN_Msk (0x4U)

#define SDIO_CMD_RESP_NONE_Msk    (0x0U)
#define SDIO_CMD_SYS_DEF          (0x0U)
#define SDIO_CMD_TYPE_NORM_Msk    (0x0U)
#define SDIO_CMD_RESP_L136_Pos    (0U)
#define SDIO_CMD_RESP_L136_Msk    (0x1U)
#define SDIO_CMD_SYS_eC           (0x1U)
#define SDIO_CMD_RESP_L48_Pos     (1U)
#define SDIO_CMD_RESP_L48_Msk     (0x2U)
#define SDIO_CMD_SYS_OTP          (0x3U)
#define SDIO_CMD_SYS_ASSD         (0x4U)
#define SDIO_CMD_SYS_VEND         (0x5U)
#define SDIO_CMD_TYPE_SUSPEND_Pos (6U)
#define SDIO_CMD_TYPE_SUSPEND_Msk (0x40U)
#define SDIO_CMD_TYPE_RESUME_Pos  (7U)
#define SDIO_CMD_TYPE_RESUME_Msk  (0x80U)
#define SDIO_CMD_TYPE_ABORT_Pos   (6U)
#define SDIO_CMD_TYPE_ABORT_Msk   (0xC0U)
#define SDIO_CMD_TYPE_Pos         (6U)
#define SDIO_CMD_TYPE_Msk         (0xC0U)

#define SDIO_DAT_PRESENT_SEL_Pos (5U)
#define SDIO_DAT_PRESENT_SEL_Msk (0x20U)

#define SDIO_AUTO_CMD_CRC_Pos (2U)
#define SDIO_AUTO_CMD_CRC_Msk (0x4U)
#define SDIO_AUTO_CMD_IND_Pos (4U)
#define SDIO_AUTO_CMD_IND_Msk (0x10U)

#define SDIO_HC2_DRV_STR_B_Msk       (0x0U)
#define SDIO_HC2_UHS_MODE_SDR12_Msk  (0x0U)
#define SDIO_HC2_UHS_MODE_SDR25_Pos  (0U)
#define SDIO_HC2_UHS_MODE_SDR25_Msk  (0x1U)
#define SDIO_HC2_UHS_MODE_SDR50_Pos  (1U)
#define SDIO_HC2_UHS_MODE_SDR50_Msk  (0x2U)
#define SDIO_HC2_UHS_MODE_SDR104_Pos (0U)
#define SDIO_HC2_UHS_MODE_SDR104_Msk (0x3U)
#define SDIO_HC2_UHS_MODE_DDR50_Pos  (2U)
#define SDIO_HC2_UHS_MODE_DDR50_Msk  (0x4U)
#define SDIO_HC2_HS400_Msk           (0x5U)
#define SDIO_HC2_UHS_MODE_Pos        (0U)
#define SDIO_HC2_UHS_MODE_Msk        (0x7U)
#define SDIO_HC2_1V8_EN_Pos          (3U)
#define SDIO_HC2_1V8_EN_Msk          (0x8U)
#define SDIO_HC2_DRV_STR_A_Pos       (4U)
#define SDIO_HC2_DRV_STR_A_Msk       (0x10U)
#define SDIO_HC2_DRV_STR_C_Pos       (5U)
#define SDIO_HC2_DRV_STR_C_Msk       (0x20U)
#define SDIO_HC2_DRV_STR_D_Pos       (4U)
#define SDIO_HC2_DRV_STR_D_Msk       (0x30U)
#define SDIO_HC2_EXEC_TNG_Pos        (6U)
#define SDIO_HC2_EXEC_TNG_Msk        (0x40U)
#define SDIO_HC2_PRE_VAL_EN_Pos      (15U)
#define SDIO_HC2_PRE_VAL_EN_Msk      (0x8000U)

#define SDIO_CAP_MAX_BLK_LEN_512B_Msk (0x0U)
#define SDIO_CAP_MAX_BL_LN_1024_Pos   (16U)
#define SDIO_CAP_MAX_BL_LN_1024_Msk   (0x10000U)
#define SDIO_CAP_MAX_BL_LN_2048_Pos   (17U)
#define SDIO_CAP_MAX_BL_LN_2048_Msk   (0x20000U)
#define SDIO_CAP_MAX_BLK_LEN_Pos      (16U)
#define SDIO_CAP_MAX_BLK_LEN_Msk      (0x30000U)
#define SDIO_CAP_MAX_BL_LN_4096_Pos   (16U)
#define SDIO_CAP_MAX_BL_LN_4096_Msk   (0x30000U)
#define SDIO_CAP_EXT_MEDIA_BUS_Pos    (18U)
#define SDIO_CAP_EXT_MEDIA_BUS_Msk    (0x40000U)
#define SDIO_CAP_ADMA2_Pos            (19U)
#define SDIO_CAP_ADMA2_Msk            (0x80000U)
#define SDIO_CAP_HIGH_SPEED_Pos       (21U)
#define SDIO_CAP_HIGH_SPEED_Msk       (0x200000U)
#define SDIO_CAP_SDMA_Pos             (22U)
#define SDIO_CAP_SDMA_Msk             (0x400000U)
#define SDIO_CAP_VOLT_3V3_Pos         (24U)
#define SDIO_CAP_VOLT_3V3_Msk         (0x1000000U)
#define SDIO_CAP_VOLT_3V0_Pos         (25U)
#define SDIO_CAP_VOLT_3V0_Msk         (0x2000000U)
#define SDIO_CAP_VOLT_1V8_Pos         (26U)
#define SDIO_CAP_VOLT_1V8_Msk         (0x4000000U)
#define SDIO_CAP_SPI_MODE_Pos         (29U)
#define SDIO_CAP_SPI_MODE_Msk         (0x20000000U)
#define SDIO_CAP_SPI_BLOCK_MODE_Pos   (30U)
#define SDIO_CAP_SPI_BLOCK_MODE_Msk   (0x40000000U)

#define SDIO_CAPS_REM_CARD      (0x0U)
#define SDIO_CAPS_EMB_SLOT      (0x40000000U)
#define SDIO_CAPS_SHR_BUS       (0x80000000U)
#define SDIO_CAPS_SLOT_TYPE_Pos (30U)
#define SDIO_CAPS_SLOT_TYPE_Msk (0xC0000000U)

#define SDIO_ECAPS_RE_TNG_MODE1_Msk (0x0U)
#define SDIO_ECAPS_SDR50_Pos        (0U)
#define SDIO_ECAPS_SDR50_Msk        (0x1U)
#define SDIO_ECAPS_SDR104_Pos       (1U)
#define SDIO_ECAPS_SDR104_Msk       (0x2U)
#define SDIO_ECAPS_DDR50_Pos        (2U)
#define SDIO_ECAPS_DDR50_Msk        (0x4U)
#define SDIO_ECAPS_DRV_TYPE_A_Pos   (4U)
#define SDIO_ECAPS_DRV_TYPE_A_Msk   (0x10U)
#define SDIO_ECAPS_DRV_TYPE_C_Pos   (5U)
#define SDIO_ECAPS_DRV_TYPE_C_Msk   (0x20U)
#define SDIO_ECAPS_DRV_TYPE_D_Pos   (6U)
#define SDIO_ECAPS_DRV_TYPE_D_Msk   (0x40U)
#define SDIO_ECAPS_RE_TNG_MODE2_Pos (14U)
#define SDIO_ECAPS_RE_TNG_MODE2_Msk (0x4000U)
#define SDIO_ECAPS_RE_TNG_MODE3_Pos (15U)
#define SDIO_ECAPS_RE_TNG_MODE3_Msk (0x8000U)
#define SDIO_ECAPS_SPI_MODE_Pos     (24U)
#define SDIO_ECAPS_SPI_MODE_Msk     (0x1000000U)
#define SDIO_ECAPS_SPI_BLK_MODE_Pos (25U)
#define SDIO_ECAPS_SPI_BLK_MODE_Msk (0x2000000U)

#define SDIO_PSR_INHIBIT_CMD_Pos   (0U)
#define SDIO_PSR_INHIBIT_CMD_Msk   (0x1U)
#define SDIO_PSR_INHIBIT_DAT_Pos   (1U)
#define SDIO_PSR_INHIBIT_DAT_Msk   (0x2U)
#define SDIO_PSR_DAT_ACTIVE_Pos    (2U)
#define SDIO_PSR_DAT_ACTIVE_Msk    (0x4U)
#define SDIO_PSR_RE_TUNING_REQ_Pos (3U)
#define SDIO_PSR_RE_TUNING_REQ_Msk (0x8U)
#define SDIO_PSR_WR_ACTIVE_Pos     (8U)
#define SDIO_PSR_WR_ACTIVE_Msk     (0x100U)
#define SDIO_PSR_RD_ACTIVE_Pos     (9U)
#define SDIO_PSR_RD_ACTIVE_Msk     (0x200U)
#define SDIO_PSR_BUFF_WR_EN_Pos    (10U)
#define SDIO_PSR_BUFF_WR_EN_Msk    (0x400U)
#define SDIO_PSR_BUFF_RD_EN_Pos    (11U)
#define SDIO_PSR_BUFF_RD_EN_Msk    (0x800U)
#define SDIO_PSR_CARD_INSRT_Pos    (16U)
#define SDIO_PSR_CARD_INSRT_Msk    (0x10000U)
#define SDIO_PSR_CARD_STABLE_Pos   (17U)
#define SDIO_PSR_CARD_STABLE_Msk   (0x20000U)
#define SDIO_PSR_CARD_DPL_Pos      (18U)
#define SDIO_PSR_CARD_DPL_Msk      (0x40000U)
#define SDIO_PSR_DAT30_SG_LVL_Pos  (20U)
#define SDIO_PSR_DAT30_SG_LVL_Msk  (0xF00000U)
#define SDIO_PSR_CMD_SG_LVL_Pos    (24U)
#define SDIO_PSR_CMD_SG_LVL_Msk    (0x1000000U)
#define SDIO_PSR_DAT74_SG_LVL_Pos  (25U)
#define SDIO_PSR_DAT74_SG_LVL_Msk  (0x1E000000U)

#define SDIO_FE_INTR_ERR_CCRC_Pos     (1U)
#define SDIO_FE_INTR_ERR_CCRC_Msk     (0x2U)
#define SDIO_FE_AUTO_CMD_CRC_Pos      (2U)
#define SDIO_FE_AUTO_CMD_CRC_Msk      (0x4U)
#define SDIO_FE_INTR_ERR_CI_Pos       (3U)
#define SDIO_FE_INTR_ERR_CI_Msk       (0x8U)
#define SDIO_FE_AUTO_CMD_IND_Pos      (4U)
#define SDIO_FE_AUTO_CMD_IND_Msk      (0x10U)
#define SDIO_FE_INTR_ERR_DT_Pos       (4U)
#define SDIO_FE_INTR_ERR_DT_Msk       (0x10U)
#define SDIO_FE_INTR_ERR_DCRC_Pos     (5U)
#define SDIO_FE_INTR_ERR_DCRC_Msk     (0x20U)
#define SDIO_FE_INTR_ERR_DEB_Pos      (6U)
#define SDIO_FE_INTR_ERR_DEB_Msk      (0x40U)
#define SDIO_FE_INTR_ERR_CUR_LMT_Pos  (7U)
#define SDIO_FE_INTR_ERR_CUR_LMT_Msk  (0x80U)
#define SDIO_FE_INTR_ERR_AUTO_CMD_Pos (8U)
#define SDIO_FE_INTR_ERR_AUTO_CMD_Msk (0x100U)
#define SDIO_FE_INTR_ERR_ADMA_Pos     (9U)
#define SDIO_FE_INTR_ERR_ADMA_Msk     (0x200U)
#define SDIO_FE_INTR_ERR_TR_Pos       (12U)
#define SDIO_FE_INTR_ERR_TR_Msk       (0x1000U)

#define SDIO_ADMA_ERR_STATE_Pos (0U)
#define SDIO_ADMA_ERR_STATE_Msk (0x3U)

#define SDIO_PHYREG1_ITAP_EN_Pos     (0U)
#define SDIO_PHYREG1_ITAP_EN_Msk     (0x1U)
#define SDIO_PHYREG1_ITAP_DLY_Msk    (0x2EU)
#define SDIO_PHYREG1_ITAP_CHGWIN_Pos (6U)
#define SDIO_PHYREG1_ITAP_CHGWIN_Msk (0x40U)
#define SDIO_PHYREG1_OTAP_EN_Pos     (8U)
#define SDIO_PHYREG1_OTAP_EN_Msk     (0x100U)
#define SDIO_PHYREG1_OTAP_DLY_Pos    (12U)
#define SDIO_PHYREG1_OTAP_DLY_Msk    (0xF000U)
#define SDIO_PHYREG1_STROBE_SEL_Pos  (16U)
#define SDIO_PHYREG1_STROBE_SEL_Msk  (0xFF0000U)

#define SDIO_PHYREG2_DLL_EN_Pos       (0U)
#define SDIO_PHYREG2_DLL_EN_Msk       (0x1U)
#define SDIO_PHYREG2_DLL_RDY_Pos      (1U)
#define SDIO_PHYREG2_DLL_RDY_Msk      (0x2U)
#define SDIO_PHYREG2_TRIM_ICP_DEF_VAL (0x8U)
#define SDIO_PHYREG2_FREQ_SEL_Pos     (4U)
#define SDIO_PHYREG2_FREQ_SEL_Msk     (0x70U)
#define SDIO_PHYREG2_TRIM_ICP_Pos     (8U)
#define SDIO_PHYREG2_TRIM_ICP_Msk     (0xF00U)
#define SDIO_PHYREG2_DLYTX_SEL_Pos    (16U)
#define SDIO_PHYREG2_DLYTX_SEL_Msk    (0x10000U)
#define SDIO_PHYREG2_DLYRX_SEL_Pos    (17U)
#define SDIO_PHYREG2_DLYRX_SEL_Msk    (0x20000U)

#define SDIO_FREQ_SEL_170MHZ_200MHz (0x0U)
#define SDIO_FREQ_SEL_140MHZ_169MHz (0x1U)
#define SDIO_FREQ_SEL_110MHZ_139MHz (0x2U)
#define SDIO_FREQ_SEL_80MHZ_109MHz  (0x3U)
#define SDIO_FREQ_SEL_50MHZ_79MHz   (0x4U)

#define SDIO_APP_CMD_PREFIX (0x8000U)

#define SDIO_CIC_CHK_PATTERN (0xAAU)

#define SDIO_OCR_LOW_VOL  (0x80U)
#define SDIO_OCR_HIGH_VOL (0xFF8000U)

#define SDIO_SCR_SEC_SUPP_NONE (0x0U)
#define SDIO_SCR_SPEC3_2V0     (0x0U)
#define SDIO_SCR_SPEC_1V0      (0x0U)
#define SDIO_SCR_BLKCNT        (0x1U)
#define SDIO_SCR_CMD_SUPP_Pos  (0U)
#define SDIO_SCR_CMD_SUPP_Msk  (0x3U)
#define SDIO_SCR_BLKSIZE       (0x8U)
#define SDIO_SCR_REG_LEN       (0x8U)

#define SDIO_SWITCH_FUNC_CHECK     (0x0U)
#define SDIO_SWITCH_CMD_BLKCNT     (0x1U)
#define SDIO_SWITCH_CMD_BLKSIZE    (0x40U)
#define SDIO_SWITCH_SD_RESP_LEN    (0x40U)
#define SDIO_SWITCH_CMD_HS_GET     (0xFFFFF0U)
#define SDIO_SWITCH_CMD_SDR12_SET  (0x80FFFFF0U)
#define SDIO_SWITCH_CMD_HS_SET     (0x80FFFFF1U)
#define SDIO_SWITCH_CMD_SDR25_SET  (0x80FFFFF1U)
#define SDIO_SWITCH_CMD_SDR50_SET  (0x80FFFFF2U)
#define SDIO_SWITCH_CMD_SDR104_SET (0x80FFFFF3U)
#define SDIO_SWITCH_CMD_DDR50_SET  (0x80FFFFF4U)

#define SDIO_MODE_FUNC_GRP1 (0x1U)
#define SDIO_MODE_FUNC_GRP2 (0x2U)
#define SDIO_MODE_FUNC_GRP3 (0x3U)
#define SDIO_MODE_FUNC_GRP4 (0x4U)
#define SDIO_MODE_FUNC_GRP5 (0x5U)
#define SDIO_MODE_FUNC_GRP6 (0x6U)

#define SDIO_FUNC_GRP_DEF_VAL     (0xFU)
#define SDIO_FUNC_ALL_GRP_DEF_VAL (0xFFFFFFU)

#define SDIO_ACC_MODE_DEF_SDR12 (0x0U)
#define SDIO_ACC_MODE_HS_SDR25  (0x1U)
#define SDIO_ACC_MODE_SDR50     (0x2U)
#define SDIO_ACC_MODE_SDR104    (0x3U)
#define SDIO_ACC_MODE_DDR50     (0x4U)

#define SDIO_DRV_TYPE_B (0x0U)
#define SDIO_DRV_TYPE_A (0x1U)
#define SDIO_DRV_TYPE_C (0x2U)
#define SDIO_DRV_TYPE_D (0x3U)

#define SDIO_CUR_LIM_200 (0x0U)
#define SDIO_CUR_LIM_400 (0x1U)
#define SDIO_CUR_LIM_600 (0x2U)
#define SDIO_CUR_LIM_800 (0x3U)

#define SDIO_EXT_CSD_CMD_SET     (0x0U)
#define SDIO_EXT_CSD_CMD_BLKCNT  (0x1U)
#define SDIO_EXT_CSD_SET_BITS    (0x1U)
#define SDIO_EXT_CSD_CLR_BITS    (0x2U)
#define SDIO_EXT_CSD_WRITE_BYTE  (0x3U)
#define SDIO_EXT_CSD_CMD_BLKSIZE (0x200U)
#define SDIO_EXT_CSD_SIZE        (0x200U)

#define SDIO_MMC_DELAY_FOR_SWITCH (0x3E8U)
#define SDIO_MMC_DDR_MAX_CLK      (0x3197500U)
#define SDIO_MMC_HSD_MAX_CLK      (0x3197500U)
#define SDIO_MMC_HS200_MAX_CLK    (0xBEBC200U)

#define SDIO_74CLK_DELAY (0xB90U)

#define SDIO_100CLK_DELAY (0xFA0U)

#define SDIO_INIT_DELAY (0x2710U)

#define SDIO_CARD_STATE_IDLE (0x0U)
#define SDIO_CARD_SD         (0x1U)
#define SDIO_CARD_STATE_RDY  (0x1U)
#define SDIO_CARD_MMC        (0x2U)
#define SDIO_CARD_STATE_IDEN (0x2U)
#define SDIO_CARD_SDIO       (0x3U)
#define SDIO_CARD_STATE_STBY (0x3U)
#define SDIO_CARD_SDCOMBO    (0x4U)
#define SDIO_CARD_STATE_TRAN (0x4U)
#define SDIO_CARD_STATE_DATA (0x5U)
#define SDIO_CARD_STATE_RCV  (0x6U)
#define SDIO_CARD_STATE_PROG (0x7U)
#define SDIO_CARD_STATE_DIS  (0x8U)
#define SDIO_CARD_STATE_BTST (0x9U)
#define SDIO_CARD_STATE_SLP  (0xAU)
#define SDIO_CARD_DEF_ADDR   (0x1234U)

#define SDIO_CHIP_EMMC (0x5U)

#define SDIO_DESC_MAX_LENGTH (0x10000U)

#define SDIO_CLK_400_KHZ (0x61A80U)
#define SDIO_CLK_50_MHZ  (0x2FAF080U)
#define SDIO_CLK_52_MHZ  (0x3197500U)

#define SDIO_SD_STATUS_BLKCNT  (0x1U)
#define SDIO_SD_VER_1_0        (0x1U)
#define SDIO_SD_VER_2_0        (0x2U)
#define SDIO_SD_STATUS_BLKSIZE (0x40U)
#define SDIO_SD_SDR12_MAX_CLK  (0x17D7840U)
#define SDIO_SD_DDR50_MAX_CLK  (0x2FAF080U)
#define SDIO_SD_SDR25_MAX_CLK  (0x2FAF080U)
#define SDIO_SD_SDR50_MAX_CLK  (0x5F5E100U)
#define SDIO_SD_INPUT_MAX_CLK  (0xA6E49C0U)
#define SDIO_SD_SDR104_MAX_CLK (0xC65D400U)

#define SDIO_1_BIT_WIDTH (0x1U)

#define SDIO_4_BIT_WIDTH (0x2U)

#define SDIO_8_BIT_WIDTH (0x3U)

#define SDIO_UHS_SPEED_MODE_SDR12  (0x0U)
#define SDIO_UHS_SPEED_MODE_SDR25  (0x1U)
#define SDIO_UHS_SPEED_MODE_SDR50  (0x2U)
#define SDIO_UHS_SPEED_MODE_SDR104 (0x3U)
#define SDIO_UHS_SPEED_MODE_DDR50  (0x4U)
#define SDIO_UHS_SDR12_MAX_CLK     (0x17D7840U)
#define SDIO_UHS_DDR50_MAX_CLK     (0x2FAF080U)
#define SDIO_UHS_SDR25_MAX_CLK     (0x2FAF080U)
#define SDIO_UHS_SDR50_MAX_CLK     (0x5F5E100U)
#define SDIO_UHS_SDR104_MAX_CLK    (0xC65D400U)

#define SDIO_HIGH_SPEED_MODE    (0x5U)
#define SDIO_HIGH_SPEED_MAX_CLK (0x2FAF080U)

#define SDIO_DEFAULT_SPEED_MODE (0x6U)

#define SDIO_HS200_MODE (0x7U)

#define SDIO_HS400_MODE (0x8U)

#define SDIO_DDR52_MODE (0x4U)

#define SDIO_TUNING_CMD_BLKCNT  (0x1U)
#define SDIO_TUNING_CMD_BLKSIZE (0x40U)

#define SDIO_SLOT_REM (0x0U)
#define SDIO_SLOT_EMB (0x1U)

#define SDIO_WIDTH_4 (0x4U)
#define SDIO_WIDTH_8 (0x8U)

#define SDIO_SLOTTYPE_SDADIR (0x4U)

#define CAN_SRR_SRST_Pos (0U)
#define CAN_SRR_SRST_Msk (0x1U)
#define CAN_SRR_CEN_Pos  (1U)
#define CAN_SRR_CEN_Msk  (0x2U)

#define CAN_MSR_SLEEP_Pos (0U)
#define CAN_MSR_SLEEP_Msk (0x1U)
#define CAN_MSR_LBACK_Pos (1U)
#define CAN_MSR_LBACK_Msk (0x2U)
#define CAN_MSR_SNOOP_Pos (2U)
#define CAN_MSR_SNOOP_Msk (0x4U)

#define CAN_BRPR_BRP_Pos (0U)
#define CAN_BRPR_BRP_Msk (0xFFU)

#define CAN_BTR_TS1_Pos (0U)
#define CAN_BTR_TS1_Msk (0xFU)
#define CAN_BTR_TS2_Pos (4U)
#define CAN_BTR_TS2_Msk (0x70U)
#define CAN_BTR_SJW_Pos (7U)
#define CAN_BTR_SJW_Msk (0x180U)

#define CAN_ECR_TEC_Pos (0U)
#define CAN_ECR_TEC_Msk (0xFFU)
#define CAN_ECR_REC_Pos (8U)
#define CAN_ECR_REC_Msk (0xFF00U)

#define CAN_ESR_CRCER_Pos (0U)
#define CAN_ESR_CRCER_Msk (0x1U)
#define CAN_ESR_FMER_Pos  (1U)
#define CAN_ESR_FMER_Msk  (0x2U)
#define CAN_ESR_STER_Pos  (2U)
#define CAN_ESR_STER_Msk  (0x4U)
#define CAN_ESR_BERR_Pos  (3U)
#define CAN_ESR_BERR_Msk  (0x8U)
#define CAN_ESR_ACKER_Pos (4U)
#define CAN_ESR_ACKER_Msk (0x10U)

#define CAN_SR_CONFIG_Pos (0U)
#define CAN_SR_CONFIG_Msk (0x1U)
#define CAN_SR_LBACK_Pos  (1U)
#define CAN_SR_LBACK_Msk  (0x2U)
#define CAN_SR_SLEEP_Pos  (2U)
#define CAN_SR_SLEEP_Msk  (0x4U)
#define CAN_SR_NORMAL_Pos (3U)
#define CAN_SR_NORMAL_Msk (0x8U)
#define CAN_SR_BIDLE_Pos  (4U)
#define CAN_SR_BIDLE_Msk  (0x10U)
#define CAN_SR_BBSY_Pos   (5U)
#define CAN_SR_BBSY_Msk   (0x20U)
#define CAN_SR_ERRWRN_Pos (6U)
#define CAN_SR_ERRWRN_Msk (0x40U)
#define CAN_SR_ESTAT_Pos  (7U)
#define CAN_SR_ESTAT_Msk  (0x180U)
#define CAN_SR_TXBFLL_Pos (9U)
#define CAN_SR_TXBFLL_Msk (0x200U)
#define CAN_SR_TXFLL_Pos  (10U)
#define CAN_SR_TXFLL_Msk  (0x400U)
#define CAN_SR_ACFBSY_Pos (11U)
#define CAN_SR_ACFBSY_Msk (0x800U)
#define CAN_SR_SNOOP_Pos  (12U)
#define CAN_SR_SNOOP_Msk  (0x1000U)

#define CAN_IXR_ARBLST_Pos   (0U)
#define CAN_IXR_ARBLST_Msk   (0x1U)
#define CAN_IXR_TXOK_Pos     (1U)
#define CAN_IXR_TXOK_Msk     (0x2U)
#define CAN_IXR_TXFLL_Pos    (2U)
#define CAN_IXR_TXFLL_Msk    (0x4U)
#define CAN_IXR_TXBFLL_Pos   (3U)
#define CAN_IXR_TXBFLL_Msk   (0x8U)
#define CAN_IXR_RXOK_Pos     (4U)
#define CAN_IXR_RXOK_Msk     (0x10U)
#define CAN_IXR_RXUFLW_Pos   (5U)
#define CAN_IXR_RXUFLW_Msk   (0x20U)
#define CAN_IXR_RXOFLW_Pos   (6U)
#define CAN_IXR_RXOFLW_Msk   (0x40U)
#define CAN_IXR_RXNEMP_Pos   (7U)
#define CAN_IXR_RXNEMP_Msk   (0x80U)
#define CAN_IXR_ERROR_Pos    (8U)
#define CAN_IXR_ERROR_Msk    (0x100U)
#define CAN_IXR_BSOFF_Pos    (9U)
#define CAN_IXR_BSOFF_Msk    (0x200U)
#define CAN_IXR_SLP_Pos      (10U)
#define CAN_IXR_SLP_Msk      (0x400U)
#define CAN_IXR_WKUP_Pos     (11U)
#define CAN_IXR_WKUP_Msk     (0x800U)
#define CAN_IXR_RXFWMFLL_Pos (12U)
#define CAN_IXR_RXFWMFLL_Msk (0x1000U)
#define CAN_IXR_TXFWMEMP_Pos (13U)
#define CAN_IXR_TXFWMEMP_Msk (0x2000U)
#define CAN_IXR_TXFEMP_Pos   (14U)
#define CAN_IXR_TXFEMP_Msk   (0x4000U)

#define CAN_TCR_CTS_Pos (0U)
#define CAN_TCR_CTS_Msk (0x1U)

#define CAN_WIR_FW_Pos (0U)
#define CAN_WIR_FW_Msk (0x3FU)
#define CAN_WIR_EW_Pos (8U)
#define CAN_WIR_EW_Msk (0x3F00U)

#define CAN_IDR_RTR_Pos (0U)
#define CAN_IDR_RTR_Msk (0x1U)
#define CAN_IDR_ID2_Pos (1U)
#define CAN_IDR_ID2_Msk (0x7FFFEU)
#define CAN_IDR_IDE_Pos (19U)
#define CAN_IDR_IDE_Msk (0x80000U)
#define CAN_IDR_SRR_Pos (20U)
#define CAN_IDR_SRR_Msk (0x100000U)
#define CAN_IDR_ID1_Pos (21U)
#define CAN_IDR_ID1_Msk (0xFFE00000U)

#define CAN_DLCR_TIMESTAMP_Pos (0U)
#define CAN_DLCR_TIMESTAMP_Msk (0xFFFFU)
#define CAN_DLCR_DLC_Pos       (28U)
#define CAN_DLCR_DLC_Msk       (0xF0000000U)

#define CAN_DW1R_DB3_Pos (0U)
#define CAN_DW1R_DB3_Msk (0xFFU)
#define CAN_DW1R_DB2_Pos (8U)
#define CAN_DW1R_DB2_Msk (0xFF00U)
#define CAN_DW1R_DB1_Pos (16U)
#define CAN_DW1R_DB1_Msk (0xFF0000U)
#define CAN_DW1R_DB0_Pos (24U)
#define CAN_DW1R_DB0_Msk (0xFF000000U)

#define CAN_DW2R_DB7_Pos (0U)
#define CAN_DW2R_DB7_Msk (0xFFU)
#define CAN_DW2R_DB6_Pos (8U)
#define CAN_DW2R_DB6_Msk (0xFF00U)
#define CAN_DW2R_DB5_Pos (16U)
#define CAN_DW2R_DB5_Msk (0xFF0000U)
#define CAN_DW2R_DB4_Pos (24U)
#define CAN_DW2R_DB4_Msk (0xFF000000U)

#define CAN_AFR_UAF1_Pos (0U)
#define CAN_AFR_UAF1_Msk (0x1U)
#define CAN_AFR_UAF2_Pos (1U)
#define CAN_AFR_UAF2_Msk (0x2U)
#define CAN_AFR_UAF3_Pos (2U)
#define CAN_AFR_UAF3_Msk (0x4U)
#define CAN_AFR_UAF4_Pos (3U)
#define CAN_AFR_UAF4_Msk (0x8U)

#define TTC_CLK_CNTRL_PS_EN_Pos    (0U)
#define TTC_CLK_CNTRL_PS_EN_Msk    (0x1U)
#define TTC_CLK_CNTRL_PS_DISABLE   (0x10U)
#define TTC_CLK_CNTRL_PS_VAL_Pos   (1U)
#define TTC_CLK_CNTRL_PS_VAL_Msk   (0x1EU)
#define TTC_CLK_CNTRL_SRC_Pos      (5U)
#define TTC_CLK_CNTRL_SRC_Msk      (0x20U)
#define TTC_CLK_CNTRL_EXT_EDGE_Pos (6U)
#define TTC_CLK_CNTRL_EXT_EDGE_Msk (0x40U)

#define TTC_CNT_CNTRL_DIS_Pos      (0U)
#define TTC_CNT_CNTRL_DIS_Msk      (0x1U)
#define TTC_CNT_CNTRL_INT_Pos      (1U)
#define TTC_CNT_CNTRL_INT_Msk      (0x2U)
#define TTC_CNT_CNTRL_DECR_Pos     (2U)
#define TTC_CNT_CNTRL_DECR_Msk     (0x4U)
#define TTC_CNT_CNTRL_MATCH_Pos    (3U)
#define TTC_CNT_CNTRL_MATCH_Msk    (0x8U)
#define TTC_CNT_CNTRL_RST_Pos      (4U)
#define TTC_CNT_CNTRL_RST_Msk      (0x10U)
#define TTC_CNT_CNTRL_EN_WAVE_Pos  (5U)
#define TTC_CNT_CNTRL_EN_WAVE_Msk  (0x20U)
#define TTC_CNT_CNTRL_RESET_VALUE  (0x21U)
#define TTC_CNT_CNTRL_POL_WAVE_Pos (6U)
#define TTC_CNT_CNTRL_POL_WAVE_Msk (0x40U)

#define TTC_COUNT_VALUE_Pos (0U)
#define TTC_COUNT_VALUE_Msk (0xFFFFFFFFU)

#define TTC_INTERVAL_VAL_Pos (0U)
#define TTC_INTERVAL_VAL_Msk (0xFFFFFFFFU)

#define TTC_MATCH_MASK_Pos (0U)
#define TTC_MATCH_MASK_Msk (0xFFFFFFFFU)

#define TTC_NUM_MATCH_REG (0x3U)

#define TTC_IXR_INTERVAL_Pos (0U)
#define TTC_IXR_INTERVAL_Msk (0x1U)
#define TTC_IXR_MATCH_0_Pos  (1U)
#define TTC_IXR_MATCH_0_Msk  (0x2U)
#define TTC_IXR_MATCH_1_Pos  (2U)
#define TTC_IXR_MATCH_1_Msk  (0x4U)
#define TTC_IXR_MATCH_2_Pos  (3U)
#define TTC_IXR_MATCH_2_Msk  (0x8U)
#define TTC_IXR_CNT_OVR_Pos  (4U)
#define TTC_IXR_CNT_OVR_Msk  (0x10U)
#define TTC_IXR_ALL_Pos      (0U)
#define TTC_IXR_ALL_Msk      (0x1FU)

#define XADC_CFG_TCKRATE_Pos (8U)
#define XADC_CFG_TCKRATE_Msk (0x300U)
#define XADC_CFG_REDGE_Pos   (12U)
#define XADC_CFG_REDGE_Msk   (0x1000U)
#define XADC_CFG_WEDGE_Pos   (13U)
#define XADC_CFG_WEDGE_Msk   (0x2000U)
#define XADC_CFG_DFIFOTH_Pos (16U)
#define XADC_CFG_DFIFOTH_Msk (0xF0000U)
#define XADC_CFG_CFIFOTH_Pos (20U)
#define XADC_CFG_CFIFOTH_Msk (0xF00000U)
#define XADC_CFG_ENABLE_Pos  (31U)
#define XADC_CFG_ENABLE_Msk  (0x80000000U)

#define XADC_INTX_ALM0_Pos      (0U)
#define XADC_INTX_ALM0_Msk      (0x1U)
#define XADC_INTX_ALM1_Pos      (1U)
#define XADC_INTX_ALM1_Msk      (0x2U)
#define XADC_INTX_ALM2_Pos      (2U)
#define XADC_INTX_ALM2_Msk      (0x4U)
#define XADC_INTX_ALM3_Pos      (3U)
#define XADC_INTX_ALM3_Msk      (0x8U)
#define XADC_INTX_ALM4_Pos      (4U)
#define XADC_INTX_ALM4_Msk      (0x10U)
#define XADC_INTX_ALM5_Pos      (5U)
#define XADC_INTX_ALM5_Msk      (0x20U)
#define XADC_INTX_ALM6_Pos      (6U)
#define XADC_INTX_ALM6_Msk      (0x40U)
#define XADC_INTX_ALM_ALL_Pos   (0U)
#define XADC_INTX_ALM_ALL_Msk   (0x7FU)
#define XADC_INTX_OT_Pos        (7U)
#define XADC_INTX_OT_Msk        (0x80U)
#define XADC_INTX_DFIFO_GTH_Pos (8U)
#define XADC_INTX_DFIFO_GTH_Msk (0x100U)
#define XADC_INTX_CFIFO_LTH_Pos (9U)
#define XADC_INTX_CFIFO_LTH_Msk (0x200U)
#define XADC_INTX_ALL_Pos       (0U)
#define XADC_INTX_ALL_Msk       (0x3FFU)

#define XADC_MSTS_ALM_Pos       (0U)
#define XADC_MSTS_ALM_Msk       (0x7FU)
#define XADC_MSTS_OT_Pos        (7U)
#define XADC_MSTS_OT_Msk        (0x80U)
#define XADC_MSTS_DFIFOE_Pos    (8U)
#define XADC_MSTS_DFIFOE_Msk    (0x100U)
#define XADC_MSTS_DFIFOF_Pos    (9U)
#define XADC_MSTS_DFIFOF_Msk    (0x200U)
#define XADC_MSTS_CFIFOE_Pos    (10U)
#define XADC_MSTS_CFIFOE_Msk    (0x400U)
#define XADC_MSTS_CFIFOF_Pos    (11U)
#define XADC_MSTS_CFIFOF_Msk    (0x800U)
#define XADC_MSTS_DFIFO_LVL_Pos (12U)
#define XADC_MSTS_DFIFO_LVL_Msk (0xF000U)
#define XADC_MSTS_CFIFO_LVL_Pos (16U)
#define XADC_MSTS_CFIFO_LVL_Msk (0xF0000U)

#define XADC_MCTL_FLUSH_Pos (0U)
#define XADC_MCTL_FLUSH_Msk (0x1U)
#define XADC_MCTL_RESET_Pos (4U)
#define XADC_MCTL_RESET_Msk (0x10U)

#define XADC_CFR0_AVG1_Msk      (0x0U)
#define XADC_CFR0_CHANNEL_Pos   (0U)
#define XADC_CFR0_CHANNEL_Msk   (0x1FU)
#define XADC_CFR0_ACQ_Pos       (8U)
#define XADC_CFR0_ACQ_Msk       (0x100U)
#define XADC_CFR0_DU_Pos        (10U)
#define XADC_CFR0_DU_Msk        (0x400U)
#define XADC_CFR0_MUX_Pos       (11U)
#define XADC_CFR0_MUX_Msk       (0x800U)
#define XADC_CFR0_AVG16_Pos     (12U)
#define XADC_CFR0_AVG16_Msk     (0x1000U)
#define XADC_CFR0_AVG64_Pos     (13U)
#define XADC_CFR0_AVG64_Msk     (0x2000U)
#define XADC_CFR0_AVG256_Pos    (12U)
#define XADC_CFR0_AVG256_Msk    (0x3000U)
#define XADC_CFR0_AVG_VALID_Pos (12U)
#define XADC_CFR0_AVG_VALID_Msk (0x3000U)
#define XADC_CFR0_CAL_AVG_Pos   (15U)
#define XADC_CFR0_CAL_AVG_Msk   (0x8000U)

#define XADC_CFR1_CAL_DISABLE_Msk        (0x0U)
#define XADC_CFR1_SEQ_SAFEMODE_Msk       (0x0U)
#define XADC_CFR1_OT_Pos                 (0U)
#define XADC_CFR1_OT_Msk                 (0x1U)
#define XADC_CFR1_ALM_TEMP_Pos           (1U)
#define XADC_CFR1_ALM_TEMP_Msk           (0x2U)
#define XADC_CFR1_ALM_VCCINT_Pos         (2U)
#define XADC_CFR1_ALM_VCCINT_Msk         (0x4U)
#define XADC_CFR1_ALM_VCCAUX_Pos         (3U)
#define XADC_CFR1_ALM_VCCAUX_Msk         (0x8U)
#define XADC_CFR1_CAL_VALID_Pos          (4U)
#define XADC_CFR1_CAL_VALID_Msk          (0xF0U)
#define XADC_CFR1_ALM_VBRAM_Pos          (8U)
#define XADC_CFR1_ALM_VBRAM_Msk          (0x100U)
#define XADC_CFR1_ALM_VCCPINT_Pos        (9U)
#define XADC_CFR1_ALM_VCCPINT_Msk        (0x200U)
#define XADC_CFR1_ALM_VCCPAUX_Pos        (10U)
#define XADC_CFR1_ALM_VCCPAUX_Msk        (0x400U)
#define XADC_CFR1_ALM_VCCPDRO_Pos        (11U)
#define XADC_CFR1_ALM_VCCPDRO_Msk        (0x800U)
#define XADC_CFR1_ALM_ALL_Msk            (0xF0FU)
#define XADC_CFR1_SEQ_ONEPASS_Pos        (12U)
#define XADC_CFR1_SEQ_ONEPASS_Msk        (0x1000U)
#define XADC_CFR1_SEQ_CONTINPASS_Pos     (13U)
#define XADC_CFR1_SEQ_CONTINPASS_Msk     (0x2000U)
#define XADC_CFR1_SEQ_SINGCHAN_Pos       (12U)
#define XADC_CFR1_SEQ_SINGCHAN_Msk       (0x3000U)
#define XADC_CFR1_SEQ_SIMUL_SAMPLING_Pos (14U)
#define XADC_CFR1_SEQ_SIMUL_SAMPLING_Msk (0x4000U)
#define XADC_CFR1_SEQ_INDEPENDENT_Pos    (15U)
#define XADC_CFR1_SEQ_INDEPENDENT_Msk    (0x8000U)
#define XADC_CFR1_SEQ_VALID_Pos          (12U)
#define XADC_CFR1_SEQ_VALID_Msk          (0xF000U)

#define XADC_CFR2_CD_MIN       (0x8U)
#define XADC_CFR2_PD_ADC1_Pos  (5U)
#define XADC_CFR2_PD_ADC1_Msk  (0x20U)
#define XADC_CFR2_PD_Pos       (4U)
#define XADC_CFR2_PD_Msk       (0x30U)
#define XADC_CFR2_PD_XADC_Pos  (4U)
#define XADC_CFR2_PD_XADC_Msk  (0x30U)
#define XADC_CFR2_CD_MAX       (0xFFU)
#define XADC_CFR2_CD_VALID_Pos (8U)
#define XADC_CFR2_CD_VALID_Msk (0xFF00U)

#define XADC_SEQ_CH_CALIB   (0x1U)
#define XADC_SEQ_CH_VCCPINT (0x20U)
#define XADC_SEQ_CH_VCCPAUX (0x40U)
#define XADC_SEQ_CH_VCCPDRO (0x80U)
#define XADC_SEQ_CH_TEMP    (0x100U)
#define XADC_SEQ_CH_VCCINT  (0x200U)
#define XADC_SEQ_CH_VCCAUX  (0x400U)
#define XADC_SEQ_CH_VPVN    (0x800U)
#define XADC_SEQ_CH_VREFP   (0x1000U)
#define XADC_SEQ_CH_VREFN   (0x2000U)
#define XADC_SEQ_CH_VBRAM   (0x4000U)
#define XADC_SEQ_CH_AUX00   (0x10000U)
#define XADC_SEQ_CH_AUX01   (0x20000U)
#define XADC_SEQ_CH_AUX02   (0x40000U)
#define XADC_SEQ_CH_AUX03   (0x80000U)
#define XADC_SEQ_CH_AUX04   (0x100000U)
#define XADC_SEQ_CH_AUX05   (0x200000U)
#define XADC_SEQ_CH_AUX06   (0x400000U)
#define XADC_SEQ_CH_AUX07   (0x800000U)
#define XADC_SEQ_CH_AUX08   (0x1000000U)
#define XADC_SEQ_CH_AUX09   (0x2000000U)
#define XADC_SEQ_CH_AUX10   (0x4000000U)
#define XADC_SEQ_CH_AUX11   (0x8000000U)
#define XADC_SEQ_CH_AUX12   (0x10000000U)
#define XADC_SEQ_CH_AUX13   (0x20000000U)
#define XADC_SEQ_CH_AUX14   (0x40000000U)
#define XADC_SEQ_CH_AUX15   (0x80000000U)

#define XADC_SEQ00_CH_VALID_Msk (0x7FE1U)

#define XADC_SEQ01_CH_VALID_Pos (0U)
#define XADC_SEQ01_CH_VALID_Msk (0xFFFFU)

#define XADC_SEQ02_CH_VALID_Pos (5U)
#define XADC_SEQ02_CH_VALID_Msk (0x7FE0U)

#define XADC_SEQ03_CH_VALID_Pos (0U)
#define XADC_SEQ03_CH_VALID_Msk (0xFFFFU)

#define XADC_SEQ04_CH_VALID_Pos (11U)
#define XADC_SEQ04_CH_VALID_Msk (0x800U)

#define XADC_SEQ05_CH_VALID_Pos (0U)
#define XADC_SEQ05_CH_VALID_Msk (0xFFFFU)

#define XADC_SEQ06_CH_VALID_Pos (11U)
#define XADC_SEQ06_CH_VALID_Msk (0x800U)

#define XADC_SEQ07_CH_VALID_Pos (0U)
#define XADC_SEQ07_CH_VALID_Msk (0xFFFFU)

#define XADC_ATR_OT_UPPER_ENB_VAL (0x3U)
#define XADC_ATR_OT_UPPER_ENB_Pos (0U)
#define XADC_ATR_OT_UPPER_ENB_Msk (0xFU)
#define XADC_ATR_OT_UPPER_VAL_MAX (0xFFFU)
#define XADC_ATR_OT_UPPER_VAL_Pos (4U)
#define XADC_ATR_OT_UPPER_VAL_Msk (0xFFF0U)

#define XADC_JTAG_DATA_Pos      (0U)
#define XADC_JTAG_DATA_Msk      (0xFFFFU)
#define XADC_JTAG_ADDR_Pos      (16U)
#define XADC_JTAG_ADDR_Msk      (0x3FF0000U)
#define XADC_JTAG_CMD_READ_Pos  (26U)
#define XADC_JTAG_CMD_READ_Msk  (0x4000000U)
#define XADC_JTAG_CMD_WRITE_Pos (27U)
#define XADC_JTAG_CMD_WRITE_Msk (0x8000000U)
#define XADC_JTAG_CMD_Pos       (26U)
#define XADC_JTAG_CMD_Msk       (0x3C000000U)

#define DMAC_DS_DMA_STATUS_STOPPED (0x0U)
#define DMAC_DS_DMA_STATUS         (0xFU)

#define DMAC_DBGSTATUS_BUSY (0x1U)

#define DMAC_CR1_I_CACHE_LEN_Pos (0U)
#define DMAC_CR1_I_CACHE_LEN_Msk (0x7U)

#define DMAC_CHANNELS_PER_DEV (0x8U)

#define DMAC_INTCLR_ALL_Pos (0U)
#define DMAC_INTCLR_ALL_Msk (0xFFU)

#define DEVCFG_CTRL_DAP_EN_Pos          (0U)
#define DEVCFG_CTRL_DAP_EN_Msk          (0x7U)
#define DEVCFG_CTRL_SEU_EN_Pos          (8U)
#define DEVCFG_CTRL_SEU_EN_Msk          (0x100U)
#define DEVCFG_CTRL_PCFG_AES_EN_Pos     (9U)
#define DEVCFG_CTRL_PCFG_AES_EN_Msk     (0xE00U)
#define DEVCFG_CTRL_PCFG_AES_FUSE_Pos   (12U)
#define DEVCFG_CTRL_PCFG_AES_FUSE_Msk   (0x1000U)
#define DEVCFG_CTRL_USER_MODE_Pos       (15U)
#define DEVCFG_CTRL_USER_MODE_Msk       (0x8000U)
#define DEVCFG_CTRL_JTAG_CHAIN_DIS_Pos  (23U)
#define DEVCFG_CTRL_JTAG_CHAIN_DIS_Msk  (0x800000U)
#define DEVCFG_CTRL_MULTIBOOT_EN_Pos    (24U)
#define DEVCFG_CTRL_MULTIBOOT_EN_Msk    (0x1000000U)
#define DEVCFG_CTRL_PCAP_RATE_EN_Pos    (25U)
#define DEVCFG_CTRL_PCAP_RATE_EN_Msk    (0x2000000U)
#define DEVCFG_CTRL_PCAP_MODE_Pos       (26U)
#define DEVCFG_CTRL_PCAP_MODE_Msk       (0x4000000U)
#define DEVCFG_CTRL_PCAP_PR_Pos         (27U)
#define DEVCFG_CTRL_PCAP_PR_Msk         (0x8000000U)
#define DEVCFG_CTRL_PCFG_POR_CNT_4K_Pos (29U)
#define DEVCFG_CTRL_PCFG_POR_CNT_4K_Msk (0x20000000U)
#define DEVCFG_CTRL_PCFG_PROG_B_Pos     (30U)
#define DEVCFG_CTRL_PCFG_PROG_B_Msk     (0x40000000U)
#define DEVCFG_CTRL_FORCE_RST_Pos       (31U)
#define DEVCFG_CTRL_FORCE_RST_Msk       (0x80000000U)

#define DEVCFG_LOCK_SEU_Pos       (2U)
#define DEVCFG_LOCK_SEU_Msk       (0x4U)
#define DEVCFG_LOCK_AES_EN_Pos    (3U)
#define DEVCFG_LOCK_AES_EN_Msk    (0x8U)
#define DEVCFG_LOCK_AES_EFUSE_Pos (4U)
#define DEVCFG_LOCK_AES_EFUSE_Msk (0x10U)

#define DEVCFG_IXR_PCFG_DONE_Pos     (2U)
#define DEVCFG_IXR_PCFG_DONE_Msk     (0x4U)
#define DEVCFG_IXR_PCFG_CFG_RST_Pos  (3U)
#define DEVCFG_IXR_PCFG_CFG_RST_Msk  (0x8U)
#define DEVCFG_IXR_PCFG_POR_B_Pos    (4U)
#define DEVCFG_IXR_PCFG_POR_B_Msk    (0x10U)
#define DEVCFG_IXR_PCFG_SEU_ERR_Pos  (5U)
#define DEVCFG_IXR_PCFG_SEU_ERR_Msk  (0x20U)
#define DEVCFG_IXR_PCFG_HMAC_ERR_Pos (6U)
#define DEVCFG_IXR_PCFG_HMAC_ERR_Msk (0x40U)
#define DEVCFG_IXR_DMA_DONE_Pos      (13U)
#define DEVCFG_IXR_DMA_DONE_Msk      (0x2000U)
#define DEVCFG_IXR_P2D_LEN_ERR_Pos   (11U)
#define DEVCFG_IXR_P2D_LEN_ERR_Msk   (0x800U)
#define DEVCFG_IXR_DMA_Q_OV_Pos      (14U)
#define DEVCFG_IXR_DMA_Q_OV_Msk      (0x4000U)
#define DEVCFG_IXR_DMA_CMD_ERR_Pos   (15U)
#define DEVCFG_IXR_DMA_CMD_ERR_Msk   (0x8000U)
#define DEVCFG_IXR_RD_FIFO_LVL_Pos   (16U)
#define DEVCFG_IXR_RD_FIFO_LVL_Msk   (0x10000U)
#define DEVCFG_IXR_WR_FIFO_LVL_Pos   (17U)
#define DEVCFG_IXR_WR_FIFO_LVL_Msk   (0x20000U)
#define DEVCFG_IXR_RX_FIFO_OV_Pos    (18U)
#define DEVCFG_IXR_RX_FIFO_OV_Msk    (0x40000U)
#define DEVCFG_IXR_AXI_RERR_Pos      (20U)
#define DEVCFG_IXR_AXI_RERR_Msk      (0x100000U)
#define DEVCFG_IXR_AXI_RTO_Pos       (21U)
#define DEVCFG_IXR_AXI_RTO_Msk       (0x200000U)
#define DEVCFG_IXR_AXI_WERR_Pos      (22U)
#define DEVCFG_IXR_AXI_WERR_Msk      (0x400000U)
#define DEVCFG_IXR_AXI_WTO_Pos       (23U)
#define DEVCFG_IXR_AXI_WTO_Msk       (0x800000U)
#define DEVCFG_IXR_ALL_Msk           (0xF7F8EFU)
#define DEVCFG_IXR_PSS_GPWRDWN_B_Pos (29U)
#define DEVCFG_IXR_PSS_GPWRDWN_B_Msk (0x20000000U)

#define DEVCFG_STATUS_EFUSE_BBRAM_KEY_DISABLE_Pos (3U)
#define DEVCFG_STATUS_EFUSE_BBRAM_KEY_DISABLE_Msk (0x8U)
#define DEVCFG_STATUS_PCFG_INIT_Pos               (4U)
#define DEVCFG_STATUS_PCFG_INIT_Msk               (0x10U)
#define DEVCFG_STATUS_PSS_GPWRDWN_B               (0x200U)
#define DEVCFG_STATUS_DMA_DONE_CNT_Pos            (28U)
#define DEVCFG_STATUS_DMA_DONE_CNT_Msk            (0x30000000U)
#define DEVCFG_STATUS_TX_FIFO_LVL_Pos             (12U)
#define DEVCFG_STATUS_TX_FIFO_LVL_Msk             (0x7F000U)
#define DEVCFG_STATUS_RX_FIFO_LVL_Pos             (20U)
#define DEVCFG_STATUS_RX_FIFO_LVL_Msk             (0x1F00000U)
#define DEVCFG_STATUS_DMA_CMD_Q_E_Pos             (30U)
#define DEVCFG_STATUS_DMA_CMD_Q_E_Msk             (0x40000000U)
#define DEVCFG_STATUS_DMA_CMD_Q_F_Pos             (31U)
#define DEVCFG_STATUS_DMA_CMD_Q_F_Msk             (0x80000000U)

#define DEVCFG_DMA_LEN_Pos         (0U)
#define DEVCFG_DMA_LEN_Msk         (0x7FFFFFFU)
#define DEVCFG_DMA_INVALID_ADDRESS (0xFFFFFFFFU)

#define DEVCFG_MCTRL_PCFG_POR_B_Pos      (8U)
#define DEVCFG_MCTRL_PCFG_POR_B_Msk      (0x100U)
#define DEVCFG_MCTRL_PCAP_LPBK_Pos       (4U)
#define DEVCFG_MCTRL_PCAP_LPBK_Msk       (0x10U)
#define DEVCFG_MCTRL_PCAP_PS_VERSION_Pos (28U)
#define DEVCFG_MCTRL_PCAP_PS_VERSION_Msk (0xF0000000U)

#define DEVCFG_CFG_FIFO_QUARTER  (0x0U)
#define DEVCFG_CFG_FIFO_HALF     (0x1U)
#define DEVCFG_CFG_FIFO_3QUARTER (0x2U)
#define DEVCFG_CFG_FIFO_EMPTY    (0x4U)

#define DEVCFG_UNLOCK_DATA (0x757BDF0DU)

#define DEVCFG_CONFIG_RESET_VALUE (0x508U)

#define NAND_REFRESH_PERIOD_0_Pos (0U)
#define NAND_REFRESH_PERIOD_0_Msk (0xFU)
#define NAND_REFRESH_PERIOD_1_Pos (0U)
#define NAND_REFRESH_PERIOD_1_Msk (0xFU)

#define NAND_OPMODE_MW_Pos            (0U)
#define NAND_OPMODE_MW_Msk            (0x3U)
#define NAND_OPMODE_RD_SYNC_Pos       (2U)
#define NAND_OPMODE_RD_SYNC_Msk       (0x4U)
#define NAND_OPMODE_RD_BL_Pos         (3U)
#define NAND_OPMODE_RD_BL_Msk         (0x38U)
#define NAND_OPMODE_WR_SYNC_Pos       (6U)
#define NAND_OPMODE_WR_SYNC_Msk       (0x40U)
#define NAND_OPMODE_WR_BL_Pos         (7U)
#define NAND_OPMODE_WR_BL_Msk         (0x380U)
#define NAND_OPMODE_BAA_Pos           (10U)
#define NAND_OPMODE_BAA_Msk           (0x400U)
#define NAND_OPMODE_ADV_Pos           (11U)
#define NAND_OPMODE_ADV_Msk           (0x800U)
#define NAND_OPMODE_BLS_Pos           (12U)
#define NAND_OPMODE_BLS_Msk           (0x1000U)
#define NAND_OPMODE_BURST_ALIGN_Pos   (13U)
#define NAND_OPMODE_BURST_ALIGN_Msk   (0xE000U)
#define NAND_OPMODE_ADDRESS_Pos       (16U)
#define NAND_OPMODE_ADDRESS_Msk       (0xFF0000U)
#define NAND_OPMODE_ADDRESS_MATCH_Pos (24U)
#define NAND_OPMODE_ADDRESS_MATCH_Msk (0xFF000000U)

#define NAND_ECC_MEMCFG_PAGE_SIZE_Pos                    (0U)
#define NAND_ECC_MEMCFG_PAGE_SIZE_Msk                    (0x3U)
#define NAND_ECC_MEMCFG_ECC_MODE_Pos                     (2U)
#define NAND_ECC_MEMCFG_ECC_MODE_Msk                     (0xCU)
#define NAND_ECC_MEMCFG_ECC_READ_END_Pos                 (4U)
#define NAND_ECC_MEMCFG_ECC_READ_END_Msk                 (0x10U)
#define NAND_ECC_MEMCFG_ECC_JUMP_Pos                     (5U)
#define NAND_ECC_MEMCFG_ECC_JUMP_Msk                     (0x60U)
#define NAND_ECC_MEMCFG_IGNORE_ADD8_Pos                  (7U)
#define NAND_ECC_MEMCFG_IGNORE_ADD8_Msk                  (0x80U)
#define NAND_ECC_MEMCOMMAND1_WR_CMD_Pos                  (0U)
#define NAND_ECC_MEMCOMMAND1_WR_CMD_Msk                  (0xFFU)
#define NAND_ECC_MEMCOMMAND2_WR_COL_CHANGE_Pos           (0U)
#define NAND_ECC_MEMCOMMAND2_WR_COL_CHANGE_Msk           (0xFFU)
#define NAND_ECC_MEMCFG_ECC_INT_PASS_Pos                 (8U)
#define NAND_ECC_MEMCFG_ECC_INT_PASS_Msk                 (0x100U)
#define NAND_ECC_MEMCFG_ECC_INT_ABORT_Pos                (9U)
#define NAND_ECC_MEMCFG_ECC_INT_ABORT_Msk                (0x200U)
#define NAND_ECC_MEMCFG_ECC_EXTRA_BLOCK_Pos              (10U)
#define NAND_ECC_MEMCFG_ECC_EXTRA_BLOCK_Msk              (0x400U)
#define NAND_ECC_MEMCFG_ECC_EXTRA_BLOCK_SIZE_Pos         (11U)
#define NAND_ECC_MEMCFG_ECC_EXTRA_BLOCK_SIZE_Msk         (0x1800U)
#define NAND_ECC_MEMCOMMAND1_RD_CMD_Pos                  (8U)
#define NAND_ECC_MEMCOMMAND1_RD_CMD_Msk                  (0xFF00U)
#define NAND_ECC_MEMCOMMAND2_RD_COL_CHANGE_Pos           (8U)
#define NAND_ECC_MEMCOMMAND2_RD_COL_CHANGE_Msk           (0xFF00U)
#define NAND_ECC_MEMCOMMAND1_RD_CMD_END_Pos              (16U)
#define NAND_ECC_MEMCOMMAND1_RD_CMD_END_Msk              (0xFF0000U)
#define NAND_ECC_MEMCOMMAND2_RD_COL_CHANGE_END_Pos       (16U)
#define NAND_ECC_MEMCOMMAND2_RD_COL_CHANGE_END_Msk       (0xFF0000U)
#define NAND_ECC_MEMCOMMAND2_RD_COL_CHANGE_END_VALID_Pos (16U)
#define NAND_ECC_MEMCOMMAND2_RD_COL_CHANGE_END_VALID_Msk (0xFF0000U)
#define NAND_ECC_VALUE_Pos                               (0U)
#define NAND_ECC_VALUE_Msk                               (0xFFFFFFU)
#define NAND_ECC_MEMCOMMAND1_RD_CMD_END_VALID_Pos        (24U)
#define NAND_ECC_MEMCOMMAND1_RD_CMD_END_VALID_Msk        (0x1000000U)
#define NAND_ECC_VALUE_CORRECT_Pos                       (27U)
#define NAND_ECC_VALUE_CORRECT_Msk                       (0x8000000U)
#define NAND_ECC_VALUE_FAIL_Pos                          (28U)
#define NAND_ECC_VALUE_FAIL_Msk                          (0x10000000U)
#define NAND_ECC_VALUE_READ_Pos                          (29U)
#define NAND_ECC_VALUE_READ_Msk                          (0x20000000U)
#define NAND_ECC_VALUE_VALID_Pos                         (30U)
#define NAND_ECC_VALUE_VALID_Msk                         (0x40000000U)
#define NAND_ECC_VALUE_INT_Pos                           (31U)
#define NAND_ECC_VALUE_INT_Msk                           (0x80000000U)

#define NAND_PERIPH_ID_PART_NUM_Pos    (0U)
#define NAND_PERIPH_ID_PART_NUM_Msk    (0xFFFU)
#define NAND_PERIPH_ID_DESIGNER_ID_Pos (12U)
#define NAND_PERIPH_ID_DESIGNER_ID_Msk (0xFF000U)
#define NAND_PERIPH_ID_REVISION_Pos    (20U)
#define NAND_PERIPH_ID_REVISION_Msk    (0xF00000U)
#define NAND_PERIPH_ID_INTG_CFG_Pos    (24U)
#define NAND_PERIPH_ID_INTG_CFG_Msk    (0x1000000U)

#define NAND_PCELL_ID_Pos (0U)
#define NAND_PCELL_ID_Msk (0xFFU)

#endif
