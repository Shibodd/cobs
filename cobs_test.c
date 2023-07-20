#include <stdio.h>
#include "cobs.h"
#include "ring_buf.h"

static inline int id(int x) {
  return x;
}


static void print(uint8_t* data, int len) {
  for (int i = 0; i < len; ++i) {
    printf("%02X", data[i]);
  }
}

static void printRb(RingBuffer* rb) {
  int first = rb->first;
  int len = rb->len;
  int count = rb->count;

  if (first + count < len) {
    print(&rb->data[first], count);
  } else {
    print(&rb->data[first], len - first);
    print(rb->data, count - len - first);
  }
}

#include <memory.h>
static void cobs_test() {
  uint8_t raw[] = { 0x00, 0xFF };
  
  uint8_t __encoded[(sizeof(raw) + 2) * 2];
  RingBuffer encoded = {
    .data = __encoded,
    .first = 0,
    .count = 0,
    .len = sizeof(__encoded)
  };

  if (COBS_OK != cobs_encode(raw, sizeof(raw), &encoded)) {
    printf("ERROR\n");
    return;
  }
  
  printRb(&encoded);
  printf("\n");
/*
  int decoded_bytes;
  uint8_t back[sizeof(raw)];

  if (COBS_OK != cobs_decode(&encoded, back, &decoded_bytes)) {
    printf("ERROR\n");
    return;
  }
  print(back, decoded_bytes);
  printf("\n");
  */
}

int main() {
  cobs_test();
}