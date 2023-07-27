#ifndef LOG_GEN_DEFS_H
#define LOG_GEN_DEFS_H

/* Generated from file "back_logger_def.json", last modified 2023-07-25 12:48:05.763090 */
bool MMR_BACK_LOG_Heartbeat() {
  uint16_t msg_id = 0;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));

  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_AutonomousLaunchStateChanged(uint8_t state) {
  uint16_t msg_id = 1;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));
  WRITE_ASSERT(WRITE_APPEND(state));
  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_ClutchStateChanged(uint8_t state) {
  uint16_t msg_id = 2;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));
  WRITE_ASSERT(WRITE_APPEND(state));
  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_EbsCheckStateChanged(uint8_t state) {
  uint16_t msg_id = 3;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));
  WRITE_ASSERT(WRITE_APPEND(state));
  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_InfiR2D() {
  uint16_t msg_id = 4;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));

  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_LapChanged(uint8_t lap) {
  uint16_t msg_id = 5;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));
  WRITE_ASSERT(WRITE_APPEND(lap));
  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_LaunchControlChanged(uint8_t state) {
  uint16_t msg_id = 6;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));
  WRITE_ASSERT(WRITE_APPEND(state));
  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_ManualStateChanged(uint8_t state) {
  uint16_t msg_id = 7;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));
  WRITE_ASSERT(WRITE_APPEND(state));
  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_MissionFinished() {
  uint16_t msg_id = 8;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));

  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_MissionReady() {
  uint16_t msg_id = 9;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));

  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_ResButtonsChanged(uint8_t buttons) {
  uint16_t msg_id = 10;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));
  WRITE_ASSERT(WRITE_APPEND(buttons));
  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_SetLaunchControlAttempted() {
  uint16_t msg_id = 11;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));

  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_ShiftDownAttempted() {
  uint16_t msg_id = 12;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));

  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_ShiftNeutralAttempted() {
  uint16_t msg_id = 13;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));

  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_ShiftUpAttempted() {
  uint16_t msg_id = 14;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));

  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_UnsetLaunchControlAttempted() {
  uint16_t msg_id = 15;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));

  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_VehicleStandstill() {
  uint16_t msg_id = 16;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));

  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_WritingInfiAth(float infiAth) {
  uint16_t msg_id = 17;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));
  WRITE_ASSERT(WRITE_APPEND(infiAth));
  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_AsmsChanged(bool asms) {
  uint16_t msg_id = 18;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));
  WRITE_ASSERT(WRITE_APPEND(asms));
  WRITE_ASSERT(WRITE_END());
  return true;
}

bool MMR_BACK_LOG_MissionChanged(uint8_t mission) {
  uint16_t msg_id = 19;
  WRITE_ASSERT(WRITE_BEGIN(msg_id));
  WRITE_ASSERT(WRITE_APPEND(mission));
  WRITE_ASSERT(WRITE_END());
  return true;
}


#endif // !LOG_GEN_DEFS_H