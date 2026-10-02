#ifndef LZMA_STREAM_H
#define LZMA_STREAM_H

/*
 * Middleware lzma: raw-LZMA1 streaming decoder over the public-domain
 * LZMA SDK (sdk/LzmaDec.c, Igor Pavlov, v18.05), driven through its
 * Dictionary Interface.
 *
 * Encoder contract - tools/convert_bitstream.py --codec lzma
 * (LZMA_FILTERS in that script): lc=0 lp=0 pb=0, dictionary 8192
 * bytes, stream ends with an end marker. The props bytes live in
 * lzma_stream.c; the dictionary size must also equal
 * LZMA_STREAM_DICT_BYTES below. Changing one side without the other
 * fails every load loudly (LZMA_STREAM_DATA), never silently.
 *
 * Working memory (static, single loader context):
 *   probs   5504 B   2752 CLzmaProb = 1984 base + 0x300 literals
 *                    (lc=0), handed to LzmaDec through a one-shot
 *                    arena allocator so the SDK checks the size
 *   dic     32 KB    decode window; when it fills, the last
 *                   LZMA_STREAM_DICT_BYTES slide to the front - the
 *                    longest match the encoder can emit, so every
 *                    distance still resolves inside the buffer
 *
 * Sink contract: called in decode order with whole-word chunks
 * (len % 4 == 0), at most LZMA_STREAM_WIN_BYTES per call; the total
 * across calls equals raw_len exactly when LZMA_STREAM_OK returns.
 */

#include <stdint.h>

/* Encoder dictionary = longest possible match distance; the slide tail. */
#define LZMA_STREAM_DICT_BYTES (8192U)
/* Decode window: larger than the dictionary so a slide frees space. */
#define LZMA_STREAM_WIN_BYTES (32768U)

/* Sink: receive one chunk of decoded data (whole words, in order). */
typedef void (*lzma_stream_sink_fn)(void *user, const uint8_t *data,
                                    uint32_t len);

/* Outcome of lzma_decode_stream(). */
enum lzma_stream_status {
    LZMA_STREAM_OK = 0,
    LZMA_STREAM_DATA,      /* malformed stream / decoder error         */
    LZMA_STREAM_TRUNCATED, /* input exhausted before raw_len decoded   */
    LZMA_STREAM_SIZE,      /* decoded or consumed length violates hdr  */
    LZMA_STREAM_MEM,       /* probs arena too small: build-time bug    */
};

/*
 * Decode the raw LZMA1 stream src/src_len, calling sink() with
 * whole-word chunks until exactly raw_len bytes are out; the end
 * marker must close the stream and consume every input byte. Returns
 * LZMA_STREAM_OK or the reason. On failure the sink may already have
 * seen data - the caller still owns totals/CRC validation.
 */
enum lzma_stream_status lzma_decode_stream(const uint8_t *src, uint32_t src_len,
                                           uint32_t raw_len,
                                           lzma_stream_sink_fn sink, void *user);

#endif /* LZMA_STREAM_H */
