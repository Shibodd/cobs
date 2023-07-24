#include <stdint.h>
#include "back_logger.h"

static MmrZcpInstance loggerZcpInstance;
static uint8_t loggerTxBuffer[512];
static uint8_t seq_id = 0;

extern uint32_t uwTick;
static inline uint32_t MMR_GetTick() {
  return uwTick;
}



#define WRITE_BEGIN(id) writeBegin(id)
#define WRITE_ASSERT(x) if (!(x)) { MMR_ZCP_TxTransactionAbort(HZCP); return false; }
#define WRITE_APPEND(x) MMR_ZCP_TxTransactionAppend(HZCP, little_endian(x), sizeof(x))
#define WRITE_END() writeEnd()
#define little_endian(x) ((uint8_t*)(&(x)))
#define HZCP (&loggerZcpInstance)

static bool writeBegin(uint16_t msg_id) {
  uint32_t tick = MMR_GetTick();

  return MMR_ZCP_TxTransactionBegin(HZCP)
      && MMR_ZCP_TxTransactionAppend(HZCP, little_endian(seq_id), sizeof(seq_id))
      && MMR_ZCP_TxTransactionAppend(HZCP, little_endian(tick), sizeof(tick))
      && MMR_ZCP_TxTransactionAppend(HZCP, little_endian(msg_id), sizeof(msg_id));
}
static bool writeEnd() {
  if (MMR_ZCP_TxTransactionCommit(HZCP)) {
    ++seq_id;
    return true;
  }
  return false;
}

#include "log_gen_defs.h"

void MMR_BACK_LOGGER_Init(MmrZcpBsp zcpBsp) {
  MMR_ZCP_Init(&loggerZcpInstance, loggerTxBuffer, sizeof(loggerTxBuffer), zcpBsp);
}

void MMR_BACK_LOGGER_Run() {
  MMR_ZCP_Run(&loggerZcpInstance);
}