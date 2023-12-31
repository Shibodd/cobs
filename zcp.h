#ifndef MMR_ZCP_H
#define MMR_ZCP_H

#include <ring_buf.h>
#include <stdint.h>

typedef struct MmrZcpBsp {
  // Start transmitting data. If this returns false, transmission will be re-attempted at the next iteration.
  bool (*txBegin)(uint8_t* data, int size);

  // Whether the transmission started by the last call to txBegin is completed (and thus the data can be forgotten).
  bool (*txCmplt)();

  // Reset the CRC accumulator.
  void (*crcReset)();

  // Update the CRC with new data.
  void (*crcAccumulate)(uint8_t* data, int size);

  // Get the CRC of the data appended until now.
  uint16_t (*crcGet)();
} MmrZcpBsp;

/*
ZCP: Zero-termination and Checksum Protocol

Frame format:
DATA  [variable]
CRC16 [2]
ZERO  [1]

*/
typedef struct MmrZcpInstance {
  // The buffer holding the data waiting for transmission.
  RingBuffer txBuf;

  // Whether a write-to-txBuf transaction is in progress.
  bool txTransactionInProgress;

  // The size of the write-to-txBuf transaction.
  int txTransactionSize;

  // Temporary indices for COBS encoding during a transaction.
  int txTransactionDstIdx;

  // Temporary indices for COBS encoding during a transaction.
  int txTransactionCobsIdx;

  // Whether a transmission started with txBegin is in progress.
  bool txTransmitting;

  // The size of the data which is being transmitted.
  int txTransmissionSize;

  MmrZcpBsp bsp;
} MmrZcpInstance;

void MMR_ZCP_Init(
    MmrZcpInstance* instance,
    uint8_t* txbuf,
    int txbuf_size,
    MmrZcpBsp bsp);
    
void MMR_ZCP_Run(MmrZcpInstance* instance);
bool MMR_ZCP_TxTransactionBegin(MmrZcpInstance* instance);
bool MMR_ZCP_TxTransactionAppend(MmrZcpInstance* instance, uint8_t* data, int len);
bool MMR_ZCP_TxTransactionCommit(MmrZcpInstance* instance);
void MMR_ZCP_TxTransactionAbort(MmrZcpInstance* instance);

#endif // !MMR_ZCP_H