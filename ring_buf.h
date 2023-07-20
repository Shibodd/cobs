#ifndef RING_BUF_H
#define RING_BUF_H

#include <stdint.h>

typedef struct RingBuffer {
  uint8_t* data;
  int len;

  int first;
  int count;
} RingBuffer;

static inline int __ring_buf_idx(int base, int idx, int len) {
  return (base + idx) % len;
}

static inline uint8_t* ring_buf_idx(RingBuffer* buf, int idx) {
  return &buf->data[__ring_buf_idx(buf->first, idx, buf->len)];
}

#endif // !RING_BUF_H