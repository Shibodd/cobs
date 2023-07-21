#ifndef MMR_COBS_H
#define MMR_COBS_H

#include <stdint.h>
#include "ring_buf.h"

typedef enum MmrCobsResult {
  MMR_COBS_OK,
  MMR_COBS_BAD_PARAM,
  MMR_COBS_NOT_IMPLEMENTED,
  MMR_COBS_INVALID_SOURCE_LENGTH,
  MMR_COBS_NO_DST_SPACE_AVAILABLE,
  MMR_COBS_DECODE_BAD_ENCODING,
  MMR_COBS_DECODE_REACHED_EOF
} MmrCobsResult;

bool MMR_COBS_EnoughSpaceToEncode(const RingBuffer* dst, int src_len);
MmrCobsResult MMR_COBS_BeginEncode(RingBuffer* dst, int* dst_idx, int* cobs_idx);
MmrCobsResult MMR_COBS_ContinueEncode(const uint8_t* src, int src_len, RingBuffer* dst, int* dst_idx, int* cobs_idx);
MmrCobsResult MMR_COBS_EndEncode(RingBuffer* dst, int* dst_idx, int* cobs_idx);
#endif // !MMR_COBS_H
