#include "ring_buf.h"
#include <stdio.h>

static void print(uint8_t* data, int len) {
  for (int i = 0; i < len; ++i) {
    printf("%c", data[i]);
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
    print(rb->data, count - (len - first));
  }
}

int main() {
  uint8_t data[8];
  RingBuffer buf = {
    .count = 0,
    .data = data,
    .first = 5,
    .len = sizeof(data)
  };

  ring_buf_append(&buf, (const uint8_t*)"ciao", 4);
  ring_buf_append(&buf, (const uint8_t*)"ses", 3);
  printRb(&buf);
  printf("\n");
}