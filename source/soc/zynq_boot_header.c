#include <stdint.h>
#include <assert.h>

#include "zynq_soc.h"

struct reg_pair
{
    uint32_t addr;
    uint32_t val;
};

struct boot_header
{
    uint32_t arm_vector_table[8];
    uint32_t width_detection_word;
    uint32_t header_signature;
    uint32_t key_source;
    uint32_t header_version;
    uint32_t source_offset;
    uint32_t fsbl_image_length;
    uint32_t fsbl_load_address_ram;
    uint32_t fsbl_exe_address_ram;
    uint32_t total_length;
    uint32_t qspi_configuration_word;
    uint32_t boot_header_checksum;
    uint32_t user_define_fields[19];
    uint32_t image_header_table_offset;
    uint32_t partition_header_table_offset;
    struct reg_pair reg_init[256];
    uint32_t fsbl_user_defined[8];
};

static_assert(sizeof(struct boot_header) == 0x8C0, "boot header must occupy the full 0x8C0 BootROM header window");

#if defined(__GNUC__)
const struct boot_header boot_header __attribute__((section(".boot_header")));
#else
#error "boot_header could not be placed in .boot_header section"
#endif

const struct boot_header boot_header = {
    .arm_vector_table = {0xEAFFFFFE, 0xEAFFFFFE, 0xEAFFFFFE, 0xEAFFFFFE, 0xEAFFFFFE, 0xEAFFFFFE, 0xEAFFFFFE,
                         0xEAFFFFFE},
    .width_detection_word = 0xAA995566,
    .header_signature = 0x584C4E58,
    .key_source = 0xFFFFFFFF,
    .header_version = 0x01010000,
    .source_offset = 0x000008C0,
    .fsbl_image_length = 0x00000000,
    .fsbl_load_address_ram = 0x00000000,
    .fsbl_exe_address_ram = LQSPI_BASE + 0x8C0U,
    .total_length = 0x00000000,
    .qspi_configuration_word = 0x00000001,
    .boot_header_checksum = 0x00194AC1,
    .reg_init[0].addr = 0xFFFFFFFF,
    .reg_init[0].val = 0x00000000,
};
