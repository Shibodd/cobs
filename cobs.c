#include <cobs.h>

static inline unsigned int udiv_ceil(unsigned int x, unsigned int y) {
  return x / y + (x % y != 0);
}

bool MMR_COBS_EnoughSpaceToEncode(const RingBuffer* dst, int src_len) {
  int overhead = udiv_ceil(src_len, UINT8_MAX - 1) + 2;
  return ring_buf_space_available(dst) >= src_len + overhead;
}

MmrCobsResult MMR_COBS_BeginEncode(RingBuffer* dst, int* dst_idx, int* cobs_idx) {
  if (ring_buf_space_available(dst) <= 0) {
    return MMR_COBS_NO_DST_SPACE_AVAILABLE;
  }
  *dst_idx = dst->count + 1;
  *cobs_idx = dst->count;
  return MMR_COBS_OK;
}

MmrCobsResult MMR_COBS_ContinueEncode(const uint8_t* src, int src_len, RingBuffer* dst, int* dst_idx, int* cobs_idx) {
  if (src_len <= 0) {
    return MMR_COBS_INVALID_SOURCE_LENGTH;
  }

  int src_idx = 0;

  while (src_idx < src_len) {
    if (*dst_idx >= dst->len) {
      return MMR_COBS_NO_DST_SPACE_AVAILABLE;
    }

    uint8_t byte = src[src_idx];

    int dist_from_cobs = *dst_idx - *cobs_idx;
    if (dist_from_cobs >= UINT8_MAX) {
      *ring_buf_ref(dst, *cobs_idx) = UINT8_MAX;
      *cobs_idx = *dst_idx;

      // Advance dst without consuming any input
      ++(*dst_idx);
    } else {
      if (byte == 0) {
        *ring_buf_ref(dst, *cobs_idx) = dist_from_cobs;
        *cobs_idx = *dst_idx;
      } else {
        *ring_buf_ref(dst, *dst_idx) = byte;
      }

      ++src_idx;
      ++(*dst_idx);
    }
  }
  return MMR_COBS_OK;
}

MmrCobsResult MMR_COBS_EndEncode(RingBuffer* dst, int* dst_idx, int* cobs_idx) {
  if (*dst_idx >= dst->len) {
    return MMR_COBS_NO_DST_SPACE_AVAILABLE;
  }

  *ring_buf_ref(dst, *cobs_idx) = *dst_idx - *cobs_idx;
  *ring_buf_ref(dst, *dst_idx) = 0;
  dst->count = *dst_idx + 1;
  return MMR_COBS_OK;
}
