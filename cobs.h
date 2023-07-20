/**
 * Encode and decode COBS data, with the special assumption that the raw data has a length <= 0xFF.
 * This encoding replaces all zeroes from the raw data with values > 0.
 * In this special case where the source length is <= 0xFF, only one byte of overhead is added.
*/

#include <stdint.h>
#include "ring_buf.h"

typedef enum CobsResult {
  COBS_OK,
  COBS_INVALID_SOURCE_LENGTH,
  COBS_NO_DST_SPACE_AVAILABLE,
  COBS_DECODE_BAD_ENCODING,
  COBS_DECODE_REACHED_EOF
} CobsResult;


static inline CobsResult cobs_encode(uint8_t* src, int src_len, RingBuffer* dst) {
  if (src_len <= 0 || src_len > 0xFF) {
    return COBS_INVALID_SOURCE_LENGTH;
  }

  int dst_idx = dst->count;
  if (dst_idx + src_len + 3 > dst->len) {
    return COBS_NO_DST_SPACE_AVAILABLE;
  }
  
  *ring_buf_ref(dst, dst_idx++) = 0;
  int cobs_idx = dst_idx++;

  for (int src_i = 0; src_i < src_len; ++src_i, ++dst_idx) {
    uint8_t byte = src[src_i];

    if (byte == 0) {
      *ring_buf_ref(dst, cobs_idx) = dst_idx - cobs_idx;
      cobs_idx = dst_idx;
    } else {
      *ring_buf_ref(dst, dst_idx) = byte;
    }
  }

  *ring_buf_ref(dst, cobs_idx) = dst_idx - cobs_idx;
  *ring_buf_ref(dst, dst_idx) = 0;

  dst->count += src_len + 3;
  return COBS_OK;
}

static inline CobsResult cobs_decode(RingBuffer* src, uint8_t* dst, int dst_len, int* decoded_bytes) {
  if (src->count < 3) {
    return COBS_INVALID_SOURCE_LENGTH;
  }

  int src_idx = 0;

  // Assert first byte == 0
  if (*ring_buf_ref(src, src_idx) != 0) {
    return COBS_DECODE_BAD_ENCODING;
  }
  ++src_idx;
  
  int cobs_idx = src_idx + *ring_buf_ref(src, src_idx);
  ++src_idx;

  int dst_idx = 0;
  uint8_t byte;

  while ((byte = *ring_buf_ref(src, src_idx)) != 0) {
    if (dst_idx >= dst_len) {
      return COBS_NO_DST_SPACE_AVAILABLE;
    }
    if (src_idx >= src->count) {
      return COBS_INVALID_SOURCE_LENGTH;
    }

    if (src_idx == cobs_idx) {
      dst[dst_idx] = 0;
      cobs_idx = src_idx + byte;
    } else {
      dst[dst_idx] = byte;
    }

    ++src_idx;
    ++dst_idx;
  }

  if (cobs_idx != src_idx) {
    return COBS_DECODE_BAD_ENCODING;
  }

  ++src_idx;
  src->first = ring_buf_absolute_idx(0, src->first + src_idx, src->len);
  src->count -= src_idx;
  *decoded_bytes = dst_idx;
  return COBS_OK;
}