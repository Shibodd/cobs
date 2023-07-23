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
  *cobs_idx = 0;
  *dst_idx = 1;
  return MMR_COBS_OK;
}

MmrCobsResult MMR_COBS_ContinueEncode(const uint8_t* src, int src_len, RingBuffer* dst, int* p_dst_idx, int* p_cobs_idx) {
  if (src_len <= 0) {
    return MMR_COBS_INVALID_SOURCE_LENGTH;
  }

  int base_idx = dst->count;
  int dst_idx = *p_dst_idx;
  int cobs_idx = *p_cobs_idx;

  int src_idx = 0;

  while (src_idx < src_len) {
    if (dst_idx >= dst->len - base_idx) {
      return MMR_COBS_NO_DST_SPACE_AVAILABLE;
    }

    uint8_t byte = src[src_idx];

    int dist_from_cobs = dst_idx - cobs_idx;
    if (dist_from_cobs >= UINT8_MAX) {
      *ring_buf_ref(dst, base_idx + cobs_idx) = UINT8_MAX;
      cobs_idx = dst_idx;

      // Advance dst without consuming any input
      ++dst_idx;
    } else {
      if (byte == 0) {
        *ring_buf_ref(dst, base_idx + cobs_idx) = dist_from_cobs;
        cobs_idx = dst_idx;
      } else {
        *ring_buf_ref(dst, base_idx + dst_idx) = byte;
      }

      ++src_idx;
      ++dst_idx;
    }
  }

  // Commit
  *p_dst_idx = dst_idx;
  *p_cobs_idx = cobs_idx;
  return MMR_COBS_OK;
}

MmrCobsResult MMR_COBS_EndEncode(RingBuffer* dst, int* dst_idx, int* cobs_idx) {
  int base_idx = dst->count;
  if (*dst_idx >= dst->len - base_idx) {
    return MMR_COBS_NO_DST_SPACE_AVAILABLE;
  }

  *ring_buf_ref(dst, base_idx + *cobs_idx) = *dst_idx - *cobs_idx;
  *ring_buf_ref(dst, base_idx + *dst_idx) = 0;
  dst->count += *dst_idx + 1;
  return MMR_COBS_OK;
}
