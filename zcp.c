/*
ZCP: Zero-termination and Checksum Protocol

Frame format:
DATA  [variable]
CRC16 [2]
ZERO  [1]

*/

#include <zcp.h>
#include <cobs.h>

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