#ifndef LOG_GEN_PROTO_H
#define LOG_GEN_PROTO_H

/* Generated from file "back_logger_def.json", last modified 2023-07-25 12:48:05.763090 */
bool MMR_BACK_LOG_Heartbeat();
bool MMR_BACK_LOG_AutonomousLaunchStateChanged(uint8_t state);
bool MMR_BACK_LOG_ClutchStateChanged(uint8_t state);
bool MMR_BACK_LOG_EbsCheckStateChanged(uint8_t state);
bool MMR_BACK_LOG_InfiR2D();
bool MMR_BACK_LOG_LapChanged(uint8_t lap);
bool MMR_BACK_LOG_LaunchControlChanged(uint8_t state);
bool MMR_BACK_LOG_ManualStateChanged(uint8_t state);
bool MMR_BACK_LOG_MissionFinished();
bool MMR_BACK_LOG_MissionReady();
bool MMR_BACK_LOG_ResButtonsChanged(uint8_t buttons);
bool MMR_BACK_LOG_SetLaunchControlAttempted();
bool MMR_BACK_LOG_ShiftDownAttempted();
bool MMR_BACK_LOG_ShiftNeutralAttempted();
bool MMR_BACK_LOG_ShiftUpAttempted();
bool MMR_BACK_LOG_UnsetLaunchControlAttempted();
bool MMR_BACK_LOG_VehicleStandstill();
bool MMR_BACK_LOG_WritingInfiAth(float infiAth);
bool MMR_BACK_LOG_AsmsChanged(bool asms);
bool MMR_BACK_LOG_MissionChanged(uint8_t mission);

#endif // !LOG_GEN_PROTO_H