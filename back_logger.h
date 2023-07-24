#ifndef BACK_LOGGER_H
#define BACK_LOGGER_H

#include <zcp.h>

void MMR_BACK_LOGGER_Init(MmrZcpBsp zcpBsp);
void MMR_BACK_LOGGER_Run();

#include "log_gen_proto.h"

#endif // !BACK_LOGGER_H