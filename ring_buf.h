#ifndef RING_BUF_H
#define RING_BUF_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct RingBuffer {
  uint8_t* data;
  int len;

  int first;
  int count;
} RingBuffer;

static inline int ring_buf_absolute_idx(int base, int idx, int len) {
  return (base + idx) % len;
}

static inline int ring_buf_idx(const RingBuffer* buf, int idx) {
  return ring_buf_absolute_idx(buf->first, idx, buf->len);
}

static inline uint8_t* ring_buf_ref(const RingBuffer* buf, int idx) {
  return &buf->data[ring_buf_idx(buf, idx)];
}

static inline int ring_buf_space_available(const RingBuffer* buf) {
  return buf->len - buf->count;
}

static inline bool ring_buf_append(RingBuffer* buf, const uint8_t* data, int size) {
  if (ring_buf_space_available(buf) < size) {
    return false;
  }

  int start_idx = ring_buf_idx(buf, buf->count);

  if (start_idx + size < buf->len) {
    memcpy(&buf->data[start_idx], data, size);
  } else {
    int batch1_len = buf->len - start_idx;
    memcpy(&buf->data[start_idx], data, batch1_len);
    memcpy(buf->data, &data[batch1_len], size - batch1_len);
  }

  buf->count += size;

  return true;
}

#endif // !RING_BUF_H
