#ifndef RING_BUF_H
#define RING_BUF_H

#include <stdint.h>

typedef struct RingBuffer {
  uint8_t* data;
  int len;

  int first;
  int count;
} RingBuffer;

static inline int ring_buf_absolute_idx(int base, int idx, int len) {
  return (base + idx) % len;
}

static inline int ring_buf_idx(RingBuffer* buf, int idx) {
  return ring_buf_absolute_idx(buf->first, idx, buf->len);
}

static inline uint8_t* ring_buf_ref(RingBuffer* buf, int idx) {
  return &buf->data[ring_buf_idx(buf, idx)];
}

#endif // !RING_BUF_H