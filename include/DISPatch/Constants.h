#pragma once

#include <QtCore/QtGlobal>

#include <array>

namespace dispatch {

constexpr quint8 DisVersion = 6;
constexpr quint8 SimManagementFamily = 5;
constexpr quint16 BroadcastEntityIdValue = 65535;
constexpr quint8 MaxExerciseId = 255;
constexpr quint8 MaxUint8Value = 255;
constexpr quint16 MaxUdpPort = 65535;
constexpr int MaxActionId = 2147483647;
constexpr int SecondsPerHour = 3600;
constexpr int SecondsPerMinute = 60;
constexpr int MillisecondsPerSecond = 1000;
constexpr quint64 MillisecondsPerDay = 86400000ULL;
constexpr int DisTimestampFractionBits = 31;
constexpr qint64 DisTimeUnitsPerSecond = 65536LL;
constexpr int BitsPerByte = 8;
constexpr int ThreeByteShift = 24;
constexpr int TwoByteShift = 16;
constexpr int DisHeaderLength = 12;
constexpr int EntityIdByteLength = 6;
constexpr int EntityIdApplicationOffset = 2;
constexpr int EntityIdEntityOffset = 4;
constexpr int OriginEntityOffset = 12;
constexpr int TargetEntityOffset = 18;
constexpr int RequestIdOffset = 24;
constexpr int RequestIdByteLength = 4;
constexpr int MinRequestPduLength = RequestIdOffset + RequestIdByteLength;
constexpr int ActionResponseStatusOffset = 28;
constexpr int AcknowledgeFlagOffset = 24;
constexpr int AcknowledgeResponseFlagOffset = 26;
constexpr int AcknowledgeRequestIdOffset = 28;
constexpr int AcknowledgePduLength = 32;
constexpr int ActionRequestPduLength = 40;
constexpr int StartResumePduLength = 44;
constexpr int StopFreezePduLength = 36;
constexpr int ActionResponsePduLength = 40;
constexpr int PduVersionOffset = 0;
constexpr int PduExerciseIdOffset = 1;
constexpr int PduTypeOffset = 2;
constexpr int PduFamilyOffset = 3;
constexpr int PduLengthOffset = 8;

constexpr int WindowMargin = 14;
constexpr int WindowBottomMargin = 10;
constexpr int StandardSpacing = 12;
constexpr int ThemeComboWidth = 140;
constexpr int IdentityStretchColumn = 5;
constexpr int TestFederateMinimumWidth = 260;
constexpr int TestFederateStretchColumn = 4;
constexpr int ResponseTableColumnCount = 5;
constexpr int MaxLogBlocks = 1000;
constexpr int DefaultWindowWidth = 980;
constexpr int DefaultWindowHeight = 720;
constexpr int RebindIntervalMilliseconds = 1000;
constexpr int StateButtonMinimumHeight = 36;

enum PduType : quint8 {
    StartResumePdu = 13,
    StopFreezePdu = 14,
    AcknowledgePdu = 15,
    ActionRequestPdu = 16,
    ActionResponsePdu = 17
};

struct StopFreezeReasonOption {
    quint8 value;
    const char *key;
    const char *label;
};

inline constexpr std::array<StopFreezeReasonOption, 9> StopFreezeReasonOptions{{
    {0, "other", "Other"},
    {1, "recess", "Recess"},
    {2, "termination", "Termination"},
    {3, "system_failure", "System Failure"},
    {4, "security_violation", "Security Violation"},
    {5, "entity_reconstitution", "Entity Reconstitution"},
    {6, "stop_for_reset", "Stop For Reset"},
    {7, "stop_for_restart", "Stop For Restart"},
    {8, "abort_training_return_to_tactical_operations", "Abort Training Return To Tactical Operations"},
}};

} // namespace dispatch
