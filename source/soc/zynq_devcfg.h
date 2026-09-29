#ifndef ZYNQ_DEVCFG_H
#define ZYNQ_DEVCFG_H

#include <stdint.h>

/*
 * PS-side PL configuration over the Device Configuration Interface (DevC)
 * and its PCAP bridge, following UG585 (v1.14):
 *   ch.6 "Configure the PL via PCAP Bridge Example"  (step numbers quoted in
 *        devcfg_load refer to that list)
 *   ch.2 "PS-PL Voltage Level Shifter Enables"       (power-up sequence)
 *   Appendix A                                       (devcfg register bits)
 *
 * Two image forms are accepted, selected by the first word:
 *
 *   Packed (the build's normal output, tools/convert_bitstream.py): a 16-byte
 *   header (magic "ZBSP", raw length, packed length, CRC-32) followed by the
 *   RLE payload in the SEGGER BSP_FPGA.c encoding. The DevC DMA cannot read
 *   compressed data, so the loader decodes through a 4 KB OCM staging buffer
 *   and issues one DMA command per chunk (~500 for a full image). The DMA is
 *   still the datapath and PCAP still throttles it; the per-chunk CPU cost
 *   (decode + cache clean + a register write burst) is noise, and the image
 *   itself shrinks ~32:1, which is the whole point.
 *
 *   Direct (fallback / debug escape hatch): the raw PCAP-form bitstream,
 *   handed to the DevC DMA in one command, FSBL style. UG585 ch.6 step 8 is
 *   explicit about the register sequence:
 *   c. Source Length:      Total number of 32-bit words in the new PL bitstream
 *   d. Destination Length: Total number of 32-bit words in the new PL bitstream
 *   The CPU only programs the four DMA registers once (writing DMA_DEST_LEN
 *   last is what starts the transfer) and then waits for completion.
 *
 * Why the single command is legal despite the 4 KB rule: the same chapter
 * requires DMA transactions to be 64-byte aligned "to prevent accidently
 * crossing a 4K byte boundary". Alignment is the mechanism, not an extra
 * constraint - a 64-byte aligned source makes the DMA emit 64-byte bursts,
 * and 4 KB is a multiple of 64, so no burst ever straddles a page. The only
 * requirement on the caller is a 64-byte aligned base (the linker script
 * places the embedded image accordingly).
 *
 * Completion is polled on devcfg.INT_STS, which UG585 lists as one of the two
 * valid methods ("monitored using either interrupts or a polling method");
 * it keeps this boot-time path independent of the GIC and of the timer tick.
 */

enum devcfg_status {
    DEVCFG_OK = 0,
    DEVCFG_ERR_SRC_ALIGN,    /* bitstream base is not 64-byte aligned  */
    DEVCFG_ERR_LENGTH,       /* byte count is not a multiple of 4      */
    DEVCFG_ERR_NO_SYNC,      /* no PCAP sync word: wrong byte order?   */
    DEVCFG_ERR_HDR,          /* packed header magic/lengths invalid    */
    DEVCFG_ERR_STREAM,       /* RLE payload malformed/length mismatch  */
    DEVCFG_ERR_CRC,          /* decompressed image failed CRC-32       */
    DEVCFG_ERR_PL_NO_POWER,  /* MCTRL[PCFG_POR_B] = 0: PL not powered  */
    DEVCFG_ERR_PL_NOT_RESET, /* PCFG_INIT did not clear after PROG_B   */
    DEVCFG_ERR_PL_NOT_READY, /* PCFG_INIT never asserted: PL not ready */
    DEVCFG_ERR_CMD_Q_FULL,   /* DMA command queue stayed full          */
    DEVCFG_ERR_DMA,          /* DMA command / AXI error / FIFO overflow*/
    DEVCFG_ERR_DMA_TIMEOUT,  /* DMA_DONE never arrived                 */
    DEVCFG_ERR_NO_DONE,      /* DMA done, but PL DONE never asserted   */
    DEVCFG_NO_IMAGE,         /* boot(): this build embeds no bitstream */
};

/*
 * One-time DevC setup: PS->PL level shifters (ch.2 step 2), the APB unlock
 * word, cleared interrupt status and PCAP selected on the bridge. Safe to
 * call before the bitstream source is known.
 */
void devcfg_init(void);

/*
 * Configure the PL from an image resident in the QSPI linear window (the
 * DevC DMA reads it straight from flash: UG585 explicitly lists "linearly
 * addressable flash devices (Quad-SPI or NOR)" as a PCAP DMA source).
 * devcfg_init() must have been called once before. The image must be
 * 64-byte aligned; 'len' counts the image size, i.e. it must be a multiple
 * of 4.
 *
 * Byte order: the DevC/PCAP path consumes 32-bit words in little-endian
 * memory order, so the image (packed payload or direct) must be the "byte
 * swapped .bin" form (kernel zynq-fpga.c): the 0xAA995566 sync word appears
 * as the bytes 66 55 99 AA. Vivado's native 'write_bitstream -bin' output is
 * byte-reversed and is rejected here with DEVCFG_ERR_NO_SYNC; the build
 * embeds the image pre-swapped and pre-packed (see tools/convert_bitstream.py).
 *
 * Returns DEVCFG_OK once the PL asserts DONE and the PL->PS level
 * shifters are enabled, or the first error encountered.
 */
enum devcfg_status devcfg_load(const void* img, uint32_t len);

#endif /* ZYNQ_DEVCFG_H */
