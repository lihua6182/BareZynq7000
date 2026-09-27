#include <stdint.h>

#include "zynq_soc_ip.h"

typedef struct _reg_init_table_t {
    uint32_t reg_addr;
    uint32_t reg_value;
} reg_init_table_t;

typedef struct _boot_header_t {
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
    reg_init_table_t reg_init_table[256];    // 0xA0 Register Initialization Pairs
    uint32_t fsbl_user_defined[8];           // 0x8A0 FSBL/User Defined (32 bytes), not read by the BootROM; keeps sizeof() == 0x8C0
} boot_header_t;

#if defined(__GNUC__)
const boot_header_t zynq_boot_header __attribute__((section(".boot_header")));
#else
#error "zynq_boot_header could not be placed in .boot_header section"
#endif

const boot_header_t zynq_boot_header = {
    .arm_vector_table = {0xEAFFFFFE, 0xEAFFFFFE,
                         0xEAFFFFFE, 0xEAFFFFFE,
                         0xEAFFFFFE, 0xEAFFFFFE,
                         0xEAFFFFFE, 0xEAFFFFFE}, /* Fill with self loop */
    .width_detection_word = 0xAA995566,           /* Key value needed by BootROM for QSPI boot */
    .header_signature = 0x584C4E58,               /* 'XLNX' (byte order 'X','N','L','X'). */
    .key_source = 0xFFFFFFFF,                     /* Not 0xA5C3C5A3 or 0x3A5C3C5A: non-encrypted FSBL/User code (no key) */
    .header_version = 0x01010000,                 /* Header Version */
    .source_offset = 0x000008C0,                  /* Offset into QSPI of the image copied on boot. Must be 64-byte aligned and >= 0x8C0 */
    .fsbl_image_length = 0x00000000,              /* Bytes to copy to OCM on boot. <= 192 KB; 0 selects execute-in-place instead */
    .fsbl_load_address_ram = 0x00000000,          /* Destination RAM address the image is copied to */
    .fsbl_exe_address_ram = LQSPI_BASE + 0x8C0U,  /* Start of Execution (UG585 0x3C): for XIP an absolute address inside the QSPI */
                                                  /* linear window, 64-byte aligned. Points at the .vectors section (0xFC0008C0), */
                                                  /* whose first instruction (ldr pc, =Reset_Handler) enters the startup code. */
    .total_length = 0x00000000,                   /* Non-secure boot: must equal fsbl_image_length; 0 together with the image */
                                                  /* length selects execute-in-place */
    .qspi_configuration_word = 0x00000001,        /* Hard coded to 0x00000001 */
    .boot_header_checksum = 0x00194AC1,           /* ~sum(words 0x20..0x44), one's complement (UG585: "inverting the result"; a plain */
                                                  /* negation is off by one and the BootROM rejects the header). Checked at build */
                                                  /* time by scripts/validate_boot_header.py. */
    .reg_init_table[0].reg_addr = 0xFFFFFFFF,     /* 0xFFFFFFFF terminates the list: unused pairs must be addr=0xFFFFFFFF, value=0x0 */
    .reg_init_table[0].reg_value = 0x00000000,
};
