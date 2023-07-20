#include <cobs.h>

/**
 * @brief Frames data with the COBS encoding into the ring buffer dst.
 * @note Support for lengths > 255 has not been implemented.
*/
MmrCobsResult MMR_COBS_Encode(uint8_t* src, int src_len, RingBuffer* dst) {
  if (src_len > 0xFF) {
    return MMR_COBS_NOT_IMPLEMENTED;
  }

  if (src_len <= 0) {
    return MMR_COBS_INVALID_SOURCE_LENGTH;
  }

  int dst_idx = dst->count;
  if (dst_idx + src_len + 3 > dst->len) {
    return MMR_COBS_NO_DST_SPACE_AVAILABLE;
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
  return MMR_COBS_OK;
}

/**
 * @brief Decodes data framed and encoded with COBS contained in the ring buffer src.
*/
MmrCobsResult MMR_COBS_Decode(RingBuffer* src, uint8_t* dst, int dst_len, int* decoded_bytes) {
  if (src->count < 3) {
    return MMR_COBS_INVALID_SOURCE_LENGTH;
  }

  int src_idx = 0;

  // Assert first byte == 0
  if (*ring_buf_ref(src, src_idx) != 0) {
    return MMR_COBS_DECODE_BAD_ENCODING;
  }
  ++src_idx;
  
  int cobs_idx = src_idx + *ring_buf_ref(src, src_idx);
  ++src_idx;

  int dst_idx = 0;
  uint8_t byte;

  while ((byte = *ring_buf_ref(src, src_idx)) != 0) {
    if (dst_idx >= dst_len) {
      return MMR_COBS_NO_DST_SPACE_AVAILABLE;
    }
    if (src_idx >= src->count) {
      return MMR_COBS_INVALID_SOURCE_LENGTH;
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
    return MMR_COBS_DECODE_BAD_ENCODING;
  }

  ++src_idx;
  src->first = ring_buf_absolute_idx(0, src->first + src_idx, src->len);
  src->count -= src_idx;
  *decoded_bytes = dst_idx;
  return MMR_COBS_OK;
}