/**
 * @file zynq_boot_header.c
 * @brief BootROM boot header, placed at the start of the QSPI image.
 *
 * UG585 ch.6. The header is what lets the BootROM execute this image in
 * place: fsbl_image_length = 0 means "do not copy, run from flash", and
 * fsbl_exe_address_ram is the XIP entry point.
 */

#include <stdint.h>

#include "zynq_soc_ip.h"

struct reg_pair {
    uint32_t addr;
    uint32_t val;
};

/* BootROM boot header (UG585 ch.6), placed at the start of the QSPI image. */
struct boot_header {
    uint32_t arm_vector_table[8];            // 0x00 Arm Vector Table (0xEAFFFFFE, infinite loop)
    uint32_t width_detection_word;           // 0x20 Width Detection word (0xAA995566)
    uint32_t header_signature;               // 0x24 Header Signature (0x584C4E58, bytes 'X','N','L','X')
    uint32_t key_source;                     // 0x28 Key Source
    uint32_t header_version;                 // 0x2C Header Version
    uint32_t source_offset;                  // 0x30 Source Offset
    uint32_t fsbl_image_length;              // 0x34 FSBL Image Length
    uint32_t fsbl_load_address_ram;          // 0x38 FSBL Load Address (RAM)
    uint32_t fsbl_exe_address_ram;           // 0x3C FSBL Execution address (RAM)
    uint32_t total_length;                   // 0x40 Total FSBL Length
    uint32_t qspi_configuration_word;        // 0x44 QSPI Configuration Word
    uint32_t boot_header_checksum;           // 0x48 Boot Header Checksum, 0x20 to 0x44
    uint32_t user_define_fields[19];         // 0x4C User Defined Fields, 76 bytes
    uint32_t image_header_table_offset;      // 0x98 Image Header Table Offset
    uint32_t partition_header_table_offset;  // 0x9C Partition Header Table Offset
    struct reg_pair reg_init[256];           // 0xA0 Register Initialization Pairs
    uint32_t fsbl_user_defined[8];           // 0x8A0 unused by BootROM; keeps sizeof() == 0x8C0
};

#if defined(__GNUC__)
const struct boot_header boot_header __attribute__((section(".boot_header")));
#else
#error "boot_header could not be placed in .boot_header section"
#endif

const struct boot_header boot_header = {
    .arm_vector_table = {0xEAFFFFFE, 0xEAFFFFFE,
                         0xEAFFFFFE, 0xEAFFFFFE,
                         0xEAFFFFFE, 0xEAFFFFFE,
                         0xEAFFFFFE, 0xEAFFFFFE},
    .width_detection_word = 0xAA995566,           /* BootROM QSPI-boot key value */
    .header_signature = 0x584C4E58,               /* 'XLNX' */
    .key_source = 0xFFFFFFFF,                     /* no key: unencrypted FSBL/User code */
    .header_version = 0x01010000,
    .source_offset = 0x000008C0,                  /* image offset; 64B aligned, >= 0x8C0 */
    .fsbl_image_length = 0x00000000,              /* 0: execute-in-place instead of copy */
    .fsbl_load_address_ram = 0x00000000,
    .fsbl_exe_address_ram = LQSPI_BASE + 0x8C0U,  /* XIP entry: .vectors at 0xFC0008C0 */
    .total_length = 0x00000000,                   /* non-secure: = fsbl_image_length, 0 = XIP */
    .qspi_configuration_word = 0x00000001,        /* fixed 0x00000001 */
    .boot_header_checksum = 0x00194AC1,           /* ~sum(words 0x20..0x44), one's complement;
                                                     build-checked by function_validate_boot_header */
    .reg_init[0].addr = 0xFFFFFFFF,               /* terminator: unused pairs are addr = 0xFFFFFFFF */
    .reg_init[0].val = 0x00000000,
};
