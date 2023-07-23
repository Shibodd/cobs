/*
ZCP: Zero-termination and Checksum Protocol

Frame format:
DATA  [variable]
CRC16 [2]
ZERO  [1]

*/

#include <cobs.h>
#include <ring_buf.h>
#include <stdint.h>

typedef bool (*MmrZcpBeginTransmission)(uint8_t* data, int size);
typedef bool (*MmrZcpTransmissionCompleted)();
typedef void (*MmrZcpResetCrc)();
typedef void (*MmrZcpAccumulateCrc)(uint8_t* data, int size);
typedef uint16_t (*MmrZcpGetCrc)();

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

  // Start transmitting data. If this returns false, transmission will be re-attempted at the next iteration.
  MmrZcpBeginTransmission txBegin;

  // Whether the transmission started by the last call to txBegin is completed (and thus the data can be forgotten).
  MmrZcpTransmissionCompleted txCmplt;

  MmrZcpResetCrc crcReset;
  MmrZcpGetCrc crcGet;
  MmrZcpAccumulateCrc crcAccumulate;
} MmrZcpInstance;

void MMR_ZCP_Init(
    MmrZcpInstance* instance,
    uint8_t* txbuf,
    int txbuf_size,
    MmrZcpBeginTransmission txBegin,
    MmrZcpTransmissionCompleted txCmplt,
    MmrZcpResetCrc crcReset,
    MmrZcpGetCrc crcGet,
    MmrZcpAccumulateCrc crcAccumulate) {
  instance->txBuf = (RingBuffer) {
    .data = txbuf,
    .len = txbuf_size,
    .count = 0,
    .first = 0,
  };
  instance->txTransactionInProgress = false;
  instance->txTransactionSize = 0;
  instance->txTransmitting = false;
  instance->txTransmissionSize = 0;

  instance->txBegin = txBegin;
  instance->txCmplt = txCmplt;
  instance->crcReset = crcReset;
  instance->crcGet = crcGet;
  instance->crcAccumulate = crcAccumulate;
}

void MMR_ZCP_Run(MmrZcpInstance* instance) {
  RingBuffer* txBuf = &instance->txBuf;

  if (instance->txTransmitting) {
    if (instance->txCmplt()) {
      instance->txTransmitting = false;
      txBuf->first = ring_buf_idx(txBuf, instance->txTransmissionSize);
      txBuf->count -= instance->txTransmissionSize;
    } else {
      return;
    }
  }

  if (txBuf->count <= 0) {
    return;
  }

  int size;
  if (txBuf->first + txBuf->count > txBuf->len) {
    size = txBuf->len - txBuf->first;
  } else {
    size = txBuf->count;
  }

  if (!instance->txBegin(ring_buf_ref(txBuf, 0), size)) {
    return;
  }

  instance->txTransmitting = true;
  instance->txTransmissionSize = size;
}

bool MMR_ZCP_TxTransactionBegin(MmrZcpInstance* instance) {
  if (instance->txTransactionInProgress) {
    return false;
  }

  MmrCobsResult result = MMR_COBS_BeginEncode(&instance->txBuf, &instance->txTransactionDstIdx, &instance->txTransactionCobsIdx);
  if (result != MMR_COBS_OK) {
    return false;
  }

  instance->txTransactionSize = 0;
  instance->txTransactionInProgress = true;
  instance->crcReset();
  return true;
}

bool MMR_ZCP_TxTransactionAppend(MmrZcpInstance* instance, uint8_t* data, int len) {
  if (!instance->txTransactionInProgress) {
    return false;
  }

  MmrCobsResult result = MMR_COBS_ContinueEncode(
    data, len, &instance->txBuf, 
    &instance->txTransactionDstIdx, &instance->txTransactionCobsIdx
  );
  if (result != MMR_COBS_OK) {
    return false;
  }

  instance->crcAccumulate(data, len);
  instance->txTransactionSize += len;
  return true;
}

bool MMR_ZCP_TxTransactionCommit(MmrZcpInstance* instance) {
  if (!instance->txTransactionInProgress) {
    return false;
  }

  uint16_t crc16 = instance->crcGet();
  uint8_t crc16_little_endian[sizeof(uint16_t)] = {
	  crc16 & 0xFF,
	  (crc16 >> 8) & 0xFF
  };

  // Prepare to rollback
  int old_dst_idx = instance->txTransactionDstIdx;
  int old_cobs_idx = instance->txTransactionCobsIdx;
  
  MmrCobsResult result;
  result = MMR_COBS_ContinueEncode(
    crc16_little_endian,
    sizeof(crc16_little_endian),
    &instance->txBuf,
    &instance->txTransactionDstIdx,
    &instance->txTransactionCobsIdx
  );
  if (result != MMR_COBS_OK) {
    return false;
  }

  result = MMR_COBS_EndEncode(&instance->txBuf, &instance->txTransactionDstIdx, &instance->txTransactionCobsIdx);
  if (result != MMR_COBS_OK) {
    instance->txTransactionDstIdx = old_dst_idx;
    instance->txTransactionCobsIdx = old_cobs_idx;
    return false;
  }
  instance->txTransactionInProgress = false;
  return true;
}

void MMR_ZCP_TxTransactionAbort(MmrZcpInstance* instance) {
  instance->txTransactionInProgress = false;
}


/*

#include "main.h"
#include <cobs.h>
#include <ring_buf.h>
#include <stdint.h>

extern UART_HandleTypeDef huart2;
extern CRC_HandleTypeDef hcrc;

#define HUART &huart2




uint8_t __txb[1024];
RingBuffer txBuf = {
  .count = 0,
  .data = __txb,
  .first = 0,
  .len = sizeof(__txb)
};

typedef enum TxTransferState {
  TxTransfer_Idle,
  TxTransfer_Transfering
} TxTransferState;


TxTransferState txTransferState;
int txTransferSize;

void run() {
  if (txTransferState == TxTransfer_Transfering) {
    if (__HAL_UART_GET_FLAG(HUART, UART_FLAG_TC)) {
      txTransferState = TxTransfer_Idle;
      txBuf.first = ring_buf_idx(&txBuf, txTransferSize);
      txBuf.count -= txTransferSize;
    } else {
      return;
    }
  }

  if (txBuf.count <= 0) {
    return;
  }

  int size;

  if (txBuf.first + txBuf.count > txBuf.len) {
    size = txBuf.len - txBuf.first;
  } else {
    size = txBuf.count;
  }

  HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(HUART, ring_buf_ref(&txBuf, 0), size);
  if (status != HAL_OK) {
    return;
  }

  txTransferState = TxTransfer_Transfering;
  txTransferSize = size;
}


bool txTransactionInProgress = false;
int txTransactionLen;
int txTransaction_dst_idx;
int txTransaction_cobs_idx;


bool tx_transaction_begin() {
  if (txTransactionInProgress) {
    return false;
  }

  MmrCobsResult result = MMR_COBS_BeginEncode(&txBuf, &txTransaction_dst_idx, &txTransaction_cobs_idx);
  if (result != MMR_COBS_OK) {
    return false;
  }

  txTransactionLen = 0;
  txTransactionInProgress = true;
  __HAL_CRC_DR_RESET(&hcrc);
  return true;
}

bool tx_transaction_append(const uint8_t* data, int len) {
  if (!txTransactionInProgress) {
    return false;
  }

  MmrCobsResult result = MMR_COBS_ContinueEncode(
    data, len, &txBuf, 
    &txTransaction_dst_idx, &txTransaction_cobs_idx
  );
  if (result != MMR_COBS_OK) {
    return false;
  }

  HAL_CRC_Accumulate(&hcrc, (uint32_t*)data, len);
  txTransactionLen += len;
  return true;
}

bool tx_transaction_commit() {
  if (!txTransactionInProgress) {
    return false;
  }

  uint16_t crc16 = (uint16_t)(hcrc.Instance->DR & 0xFFFF);
  uint8_t crc16_little_endian[sizeof(uint16_t)] = {
	  crc16 & 0xFF,
	  (crc16 >> 8) & 0xFF
  };

  // Prepare to rollback
  int old_dst_idx = txTransaction_dst_idx;
  int old_cobs_idx = txTransaction_cobs_idx;
  
  MmrCobsResult result;
  result = MMR_COBS_ContinueEncode(
    crc16_little_endian,
    sizeof(crc16_little_endian),
    &txBuf,
    &txTransaction_dst_idx,
    &txTransaction_cobs_idx
  );
  if (result != MMR_COBS_OK) {
    return false;
  }

  result = MMR_COBS_EndEncode(&txBuf, &txTransaction_dst_idx, &txTransaction_cobs_idx);
  if (result != MMR_COBS_OK) {
    txTransaction_dst_idx = old_dst_idx;
    txTransaction_cobs_idx = old_cobs_idx;
    return false;
  }
  txTransactionInProgress = false;
  return true;
}
void tx_transaction_abort() {
  txTransactionInProgress = false;
}


*/