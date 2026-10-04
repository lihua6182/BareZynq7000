/**
 * @file lzma_stream.c
 * @brief Streaming wrapper over the LZMA SDK decoder (Dictionary
 *        Interface): raw LZMA1 in, whole-word chunks out. The encoder
 *        contract is documented in lzma_stream.h; the sdk/ files stay
 *        verbatim upstream. Working memory (probs + window, ~38 KiB)
 *        comes from the heap and is freed on every exit, so runtime
 *        fpga_load() can decode again whenever the heap has room.
 */

#include "lzma_stream.h"

#include <stdlib.h>
#include <string.h>

#include "LzmaDec.h"

/* LZMA props: one byte (pb*5 + lp)*9 + lc, then dicSize little-endian.
   lc=0 lp=0 pb=0 and dicSize=8192 give 00 00 20 00 00 - the exact
   image of tools/convert_bitstream.py LZMA_FILTERS. */
static const uint8_t lzma_props[LZMA_PROPS_SIZE] = {
    0x00U, 0x00U, 0x20U, 0x00U, 0x00U,
};

/* Heap-backed ISzAlloc: probs come from LzmaDec_AllocateProbs() through
   this vtable (its size check stays in force); the window is malloc'd
   directly below. */
static void *lzma_alloc(ISzAllocPtr ctx, size_t size) {
    (void)ctx;
    return malloc(size);
}

static void lzma_free(ISzAllocPtr ctx, void *ptr) {
    (void)ctx;
    free(ptr);
}

static const ISzAlloc lzma_alloc_vt = {lzma_alloc, lzma_free};

enum lzma_stream_status lzma_decode_stream(const uint8_t *src, uint32_t src_len,
                                           uint32_t raw_len,
                                           lzma_stream_sink_fn sink, void *user) {
    CLzmaDec dec;
    uint8_t *dic;
    uint32_t consumed = 0U; /* dic bytes already handed to the sink */
    uint32_t decoded = 0U;
    enum lzma_stream_status st_out = LZMA_STREAM_OK;
    int done = 0;

    LzmaDec_Construct(&dec);
    if (LzmaDec_AllocateProbs(&dec, lzma_props, LZMA_PROPS_SIZE,
                              &lzma_alloc_vt) != SZ_OK) {
        return LZMA_STREAM_MEM; /* probs malloc failed */
    }
    dic = malloc(LZMA_STREAM_WIN_BYTES);
    if (dic == NULL) {
        LzmaDec_FreeProbs(&dec, &lzma_alloc_vt);
        return LZMA_STREAM_MEM;
    }
    /* Manual dictionary (AllocateProbs variant): dic/dicBufSize/dicPos
       are ours to set; LzmaDec_Init() resets the range coder only. */
    dec.dic = dic;
    dec.dicBufSize = LZMA_STREAM_WIN_BYTES;
    dec.dicPos = 0U;
    LzmaDec_Init(&dec);

    while (done == 0) {
        SizeT in_used = (SizeT)src_len;
        ELzmaStatus st;
        uint32_t avail;
        uint32_t words;
        SRes r = LzmaDec_DecodeToDic(&dec, LZMA_STREAM_WIN_BYTES, src,
                                     &in_used, LZMA_FINISH_ANY, &st);

        if (r != SZ_OK) {
            st_out = LZMA_STREAM_DATA;
            break;
        }
        src += in_used;
        src_len -= (uint32_t)in_used;

        /* Whole words to the sink; a sub-word tail waits for more
           decoded data (raw_len is word-aligned, so the end of the
           stream lands on a word boundary too). */
        avail = (uint32_t)dec.dicPos - consumed;
        words = avail & ~3U;
        if (words != 0U) {
            sink(user, &dic[consumed], words);
        }
        consumed += words;
        decoded += words;
        if (decoded > raw_len) {
            st_out = LZMA_STREAM_SIZE;
            break;
        }

        if (st == LZMA_STATUS_FINISHED_WITH_MARK) {
            done = 1; /* end marker: the stream is complete */
        } else if (st == LZMA_STATUS_NEEDS_MORE_INPUT) {
            if (src_len == 0U) {
                st_out = LZMA_STREAM_TRUNCATED;
                break;
            }
        } else if (st == LZMA_STATUS_NOT_FINISHED) {
            if (dec.dicPos == LZMA_STREAM_WIN_BYTES) {
                /* Window full: keep the last DICT bytes (the longest
                   match the encoder can emit) at the front, decode on. */
                memmove(dic,
                        &dic[LZMA_STREAM_WIN_BYTES - LZMA_STREAM_DICT_BYTES],
                        LZMA_STREAM_DICT_BYTES);
                consumed -= LZMA_STREAM_WIN_BYTES - LZMA_STREAM_DICT_BYTES;
                dec.dicPos = LZMA_STREAM_DICT_BYTES;
            } else if (src_len == 0U) {
                st_out = LZMA_STREAM_TRUNCATED;
                break;
            } else {
                /* Stopped with window space and input left: cannot
                   happen - refuse instead of spinning. */
                st_out = LZMA_STREAM_DATA;
                break;
            }
        } else {
            /* MAYBE_FINISHED_WITHOUT_MARK: our encoder always writes
               the end marker, so this is a foreign or broken stream. */
            st_out = LZMA_STREAM_DATA;
            break;
        }
    }

    if ((st_out == LZMA_STREAM_OK) &&
        ((decoded != raw_len) || (src_len != 0U) ||
         ((uint32_t)dec.dicPos != consumed))) {
        st_out = LZMA_STREAM_SIZE;
    }

    LzmaDec_FreeProbs(&dec, &lzma_alloc_vt);
    free(dic);
    return st_out;
}
