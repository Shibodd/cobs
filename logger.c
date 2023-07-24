#include <stdint.h>
#include <zcp.h>

static MmrZcpInstance loggerZcpInstance;
static uint8_t loggerTxBuffer[512];

#define little_endian(x) ((uint8_t*)(&(x)))
#define HZCP (&loggerZcpInstance)

#define WRITE_ASSERT(x) if (!(x)) { MMR_ZCP_TxTransactionAbort(HZCP); return false; }
#define WRITE_BEGIN() MMR_ZCP_TxTransactionBegin(HZCP)
#define WRITE_APPEND(x) MMR_ZCP_TxTransactionAppend(HZCP, little_endian(x), sizeof(x))
#define WRITE_END() MMR_ZCP_TxTransactionCommit(HZCP)

#include "logger_gen.h"

void MMR_BACK_LOGGER_Init(MmrZcpBsp zcpBsp) {
  MMR_ZCP_Init(&loggerZcpInstance, &loggerTxBuffer);
}

void MMR_BACK_LOGGER_Run() {
  MMR_ZCP_Run(&loggerZcpInstance);
}