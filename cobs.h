#ifndef MMR_COBS_H
#define MMR_COBS_H

/**
 * Encode and decode COBS data, with the special assumption that the raw data has a length <= 0xFF.
 * This encoding replaces all zeroes from the raw data with values > 0.
 * In this special case where the source length is <= 0xFF, only one byte of overhead is added.
*/

#include <stdint.h>
#include "ring_buf.h"

typedef enum MmrCobsResult {
  MMR_COBS_OK,
  MMR_COBS_NOT_IMPLEMENTED,
  MMR_COBS_INVALID_SOURCE_LENGTH,
  MMR_COBS_NO_DST_SPACE_AVAILABLE,
  MMR_COBS_DECODE_BAD_ENCODING,
  MMR_COBS_DECODE_REACHED_EOF
} MmrCobsResult;

/**
 * @brief Frames data with the COBS encoding into the ring buffer dst.
 * @note Support for lengths > 255 has not been implemented.
*/
MmrCobsResult MMR_COBS_Encode(uint8_t* src, int src_len, RingBuffer* dst);

/**
 * @brief Decodes data framed and encoded with COBS contained in the ring buffer src.
*/
MmrCobsResult MMR_COBS_Decode(RingBuffer* src, uint8_t* dst, int dst_len, int* decoded_bytes);

#endif // !MMR_COBS_H