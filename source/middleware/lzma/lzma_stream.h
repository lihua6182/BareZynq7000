#ifndef LZMA_STREAM_H
#define LZMA_STREAM_H

#include <stdint.h>

#define LZMA_STREAM_DICT_BYTES (8192U)

#define LZMA_STREAM_WIN_BYTES (32768U)

typedef void (*lzma_stream_sink_fn)(void *user, const uint8_t *data,
                                    uint32_t len);

enum lzma_stream_status {
    LZMA_STREAM_OK = 0,
    LZMA_STREAM_DATA,
    LZMA_STREAM_TRUNCATED,
    LZMA_STREAM_SIZE,
    LZMA_STREAM_MEM,
};

enum lzma_stream_status lzma_decode_stream(const uint8_t *src, uint32_t src_len,
                                           uint32_t raw_len,
                                           lzma_stream_sink_fn sink, void *user);

#endif
