#include <stdint.h>
#include <zcp.h>

MmrZcpInstance zcpInstance;

#define little_endian(x) ((uint8_t*)(&(x)))
#define HZCP (&zcpInstance)

#define WRITE_ASSERT(x) if (!(x)) { MMR_ZCP_TxTransactionAbort(HZCP); return false; }
#define WRITE_BEGIN() MMR_ZCP_TxTransactionBegin(HZCP)
#define WRITE_APPEND(x) MMR_ZCP_TxTransactionAppend(HZCP, little_endian(x), sizeof(x))
#define WRITE_END() MMR_ZCP_TxTransactionCommit(HZCP)

#include "logger_gen.h"