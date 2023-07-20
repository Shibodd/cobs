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
    print(rb->data, count - (len - first));
  }
}

#include <memory.h>
#include <math.h>

double rand(double min, double max) {
  static double x;
  const double F = 105913091150139;

  x += 1;
  return min + (max - min) * 0.5 * (1 + sin(F * x));
}

static void gen_raw(uint8_t* buf, int len) {
  for (int i = 0; i < len; ++i) {
    buf[i] = (uint8_t)rand(0, 3);
  }
}

static void cobs_test() {
  uint8_t raw[256];
  int raw_count;
  uint8_t encoded[1024];
  RingBuffer encoded_rb = {
    .count = 0,
    .data = encoded,
    .first = 0,
    .len = sizeof(encoded)
  };

  while (1) {
    int action = rand(0, 100);
    
    if (action < 75) {
      raw_count = (int)rand(1, 10);
      gen_raw(raw, raw_count);
      
      MmrCobsResult res = MMR_COBS_Encode(raw, raw_count, &encoded_rb);
      if (res != MMR_COBS_OK && res != MMR_COBS_NO_DST_SPACE_AVAILABLE) {
        printf("ENCODE ERROR %d\n", res);
        return;
      }
      if (res == MMR_COBS_OK) {
        printf("ENCODED ");
        print(raw, raw_count);
        printf("\n");
      }
    } else {
      MmrCobsResult res = MMR_COBS_Decode(&encoded_rb, raw, sizeof(raw), &raw_count);
      if (res != MMR_COBS_OK && res != MMR_COBS_INVALID_SOURCE_LENGTH) {
        printf("DECODE ERROR %d\n", res);
        return;
      }
      if (res == MMR_COBS_OK) {
        printf("DECODED ");
        print(raw, raw_count);
        printf("\n");
      }
    }
  }
}

int main() {
  cobs_test();
}