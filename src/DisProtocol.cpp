#include <DISPatch/DisProtocol.h>

#include <DISPatch/Constants.h>

#include <QtCore/QDataStream>
#include <QtCore/QDateTime>
#include <QtCore/QIODevice>
#include <QtCore/QTime>

namespace dispatch {

namespace {

auto disTimestamp() -> quint32
{
    const QTime now = QDateTime::currentDateTimeUtc().time();
    const quint64 milliseconds = static_cast<quint64>(now.minute() * SecondsPerMinute * MillisecondsPerSecond
                                                      + now.second() * MillisecondsPerSecond
                                                      + now.msec());
    const quint64 timePastHour = (milliseconds * DisTimeUnitsPerHour) / MillisecondsPerHour;
    return static_cast<quint32>((timePastHour << 1U) | DisTimestampAbsoluteBit);
}

void writeEntityId(QDataStream &out, const EntityId &entityId)
{
    out << entityId.site << entityId.application << entityId.entity;
}

auto makeHeader(const DisConfig &config, PduType pduType, quint16 length) -> QByteArray
{
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);
    out << DisVersion;
    out << config.exerciseId;
    out << static_cast<quint8>(pduType);
    out << SimManagementFamily;
    out << disTimestamp();
    out << length;
    out << static_cast<quint16>(0);
    return bytes;
}

void appendClockTime(QDataStream &out, int offsetSeconds)
{
    const QDateTime time = QDateTime::currentDateTimeUtc().addSecs(offsetSeconds);
    const qint64 secondsSinceEpoch = time.toSecsSinceEpoch();
    const auto hour = static_cast<quint32>(secondsSinceEpoch / SecondsPerHour);
    const quint64 millisecondsPastHour =
        static_cast<quint64>((secondsSinceEpoch % SecondsPerHour) * MillisecondsPerSecond
                             + time.time().msec());
    const auto timePastHour =
        static_cast<quint32>((millisecondsPastHour * DisTimeUnitsPerHour) / MillisecondsPerHour);
    out << hour;
    out << timePastHour;
}

} // namespace

auto commandName(SimulationCommand command) -> QString
{
    switch (command) {
    case SimulationCommand::Initialize:
        return QStringLiteral("Initialize");
    case SimulationCommand::Start:
        return QStringLiteral("Start");
    case SimulationCommand::Pause:
        return QStringLiteral("Pause");
    case SimulationCommand::Stop:
        return QStringLiteral("Stop");
    case SimulationCommand::Reset:
        return QStringLiteral("Reset");
    }

    return QStringLiteral("Unknown");
}

auto stopFreezeReasonLabel(quint8 reason) -> QString
{
    for (const auto &option : StopFreezeReasonOptions) {
        if (option.value == reason) {
            return QString::fromLatin1(option.label);
        }
    }

    return QStringLiteral("Reason %1").arg(reason);
}

auto stopFreezeReasonForCommand(SimulationCommand command) -> quint8
{
    switch (command) {
    case SimulationCommand::Pause:
        return RecessReason;
    case SimulationCommand::Stop:
        return TerminationReason;
    case SimulationCommand::Reset:
        return StopForResetReason;
    case SimulationCommand::Initialize:
    case SimulationCommand::Start:
        return RecessReason;
    }

    return RecessReason;
}

auto frozenBehaviorForCommand(const DisConfig &config, SimulationCommand command) -> quint8
{
    switch (command) {
    case SimulationCommand::Pause:
        return config.pauseFrozenBehavior;
    case SimulationCommand::Stop:
        return config.stopFrozenBehavior;
    case SimulationCommand::Reset:
        return config.resetFrozenBehavior;
    case SimulationCommand::Initialize:
    case SimulationCommand::Start:
        return 0;
    }

    return 0;
}

auto pduTypeName(quint8 pduType) -> QString
{
    switch (pduType) {
    case StartResumePdu:
        return QStringLiteral("Start/Resume");
    case StopFreezePdu:
        return QStringLiteral("Stop/Freeze");
    case AcknowledgePdu:
        return QStringLiteral("Acknowledge");
    case ActionRequestPdu:
        return QStringLiteral("Action Request");
    case ActionResponsePdu:
        return QStringLiteral("Action Response");
    default:
        return QStringLiteral("PDU %1").arg(pduType);
    }
}

auto entityIdsMatch(const EntityId &left, const EntityId &right) -> bool
{
    return left.site == right.site && left.application == right.application
        && left.entity == right.entity;
}

auto entityIdString(const EntityId &entityId) -> QString
{
    return QStringLiteral("%1:%2:%3")
        .arg(entityId.site)
        .arg(entityId.application)
        .arg(entityId.entity);
}

auto makeStartResumePdu(const DisConfig &config, quint32 requestId) -> QByteArray
{
    constexpr quint16 length = StartResumePduLength;
    QByteArray bytes = makeHeader(config, StartResumePdu, length);
    QDataStream out(&bytes, QIODevice::Append);
    out.setByteOrder(QDataStream::BigEndian);

    writeEntityId(out, config.managerId);
    writeEntityId(out, config.targetId);
    appendClockTime(out, config.startRealWorldTimeOffsetSeconds);
    appendClockTime(out, config.startSimulationTimeOffsetSeconds);
    out << requestId;

    return bytes;
}

auto makeStopFreezePdu(const DisConfig &config, quint32 requestId, SimulationCommand command) -> QByteArray
{
    constexpr quint16 length = StopFreezePduLength;
    QByteArray bytes = makeHeader(config, StopFreezePdu, length);
    QDataStream out(&bytes, QIODevice::Append);
    out.setByteOrder(QDataStream::BigEndian);

    writeEntityId(out, config.managerId);
    writeEntityId(out, config.targetId);
    appendClockTime(out, 0);
    out << stopFreezeReasonForCommand(command);
    out << frozenBehaviorForCommand(config, command);
    out << static_cast<quint16>(0);
    out << requestId;

    return bytes;
}

auto makeActionRequestPdu(const DisConfig &config, quint32 requestId) -> QByteArray
{
    constexpr quint16 length = ActionRequestPduLength;

    QByteArray bytes = makeHeader(config, ActionRequestPdu, length);
    QDataStream out(&bytes, QIODevice::Append);
    out.setByteOrder(QDataStream::BigEndian);

    writeEntityId(out, config.managerId);
    writeEntityId(out, config.targetId);
    out << requestId;
    out << config.initializeActionId;
    out << static_cast<quint32>(0);
    out << static_cast<quint32>(0);

    return bytes;
}

auto makeAcknowledgePdu(const DisConfig &config, quint32 requestId, PduType requestType) -> QByteArray
{
    constexpr quint16 length = AcknowledgePduLength;
    constexpr quint16 ableToComply = 1;
    const quint16 acknowledgeFlag = requestType == StartResumePdu ? 3 : 4;

    QByteArray bytes = makeHeader(config, AcknowledgePdu, length);
    QDataStream out(&bytes, QIODevice::Append);
    out.setByteOrder(QDataStream::BigEndian);

    writeEntityId(out, config.managerId);
    writeEntityId(out, config.targetId);
    out << acknowledgeFlag;
    out << ableToComply;
    out << requestId;

    return bytes;
}

auto makeActionResponsePdu(const DisConfig &config, quint32 requestId) -> QByteArray
{
    constexpr quint16 length = ActionResponsePduLength;
    constexpr quint32 requestComplete = 1;

    QByteArray bytes = makeHeader(config, ActionResponsePdu, length);
    QDataStream out(&bytes, QIODevice::Append);
    out.setByteOrder(QDataStream::BigEndian);

    writeEntityId(out, config.managerId);
    writeEntityId(out, config.targetId);
    out << requestId;
    out << requestComplete;
    out << static_cast<quint32>(0);
    out << static_cast<quint32>(0);

    return bytes;
}

auto readU16(const QByteArray &bytes, int offset) -> quint16
{
    if (offset + EntityIdApplicationOffset > bytes.size()) {
        return 0;
    }

    return static_cast<quint16>((static_cast<quint8>(bytes[offset]) << BitsPerByte)
                               | static_cast<quint8>(bytes[offset + 1]));
}

auto readU32(const QByteArray &bytes, int offset) -> quint32
{
    if (offset + RequestIdByteLength > bytes.size()) {
        return 0;
    }

    return (static_cast<quint32>(static_cast<quint8>(bytes[offset])) << ThreeByteShift)
        | (static_cast<quint32>(static_cast<quint8>(bytes[offset + 1])) << TwoByteShift)
        | (static_cast<quint32>(static_cast<quint8>(bytes[offset + 2])) << BitsPerByte)
        | static_cast<quint32>(static_cast<quint8>(bytes[offset + 3]));
}

auto readEntityId(const QByteArray &bytes, int offset) -> EntityId
{
    return EntityId{readU16(bytes, offset),
                    readU16(bytes, offset + EntityIdApplicationOffset),
                    readU16(bytes, offset + EntityIdEntityOffset)};
}

auto entityIdString(const QByteArray &bytes, int offset) -> QString
{
    return QStringLiteral("%1:%2:%3")
        .arg(readU16(bytes, offset))
        .arg(readU16(bytes, offset + EntityIdApplicationOffset))
        .arg(readU16(bytes, offset + EntityIdEntityOffset));
}

auto responseSummary(const QByteArray &bytes) -> QString
{
    if (bytes.size() < DisHeaderLength) {
        return QStringLiteral("Received %1 byte datagram that is too short for a DIS header")
            .arg(bytes.size());
    }

    const auto version = static_cast<quint8>(bytes[PduVersionOffset]);
    const auto exerciseId = static_cast<quint8>(bytes[PduExerciseIdOffset]);
    const auto pduType = static_cast<quint8>(bytes[PduTypeOffset]);
    const auto family = static_cast<quint8>(bytes[PduFamilyOffset]);
    const quint16 pduLength = readU16(bytes, PduLengthOffset);
    QString summary = QStringLiteral("DIS%1 exercise %2 %3, family %4, length %5")
                          .arg(version)
                          .arg(exerciseId)
                          .arg(pduTypeName(pduType))
                          .arg(family)
                          .arg(pduLength);

    if (family != SimManagementFamily) {
        return summary + QStringLiteral(" (non-Simulation Management)");
    }

    if (pduType == AcknowledgePdu && bytes.size() >= AcknowledgePduLength) {
        summary += QStringLiteral("; origin %1, target %2, ack flag %3, response flag %4, request %5")
                       .arg(entityIdString(bytes, OriginEntityOffset))
                       .arg(entityIdString(bytes, TargetEntityOffset))
                       .arg(readU16(bytes, AcknowledgeFlagOffset))
                       .arg(readU16(bytes, AcknowledgeResponseFlagOffset))
                       .arg(readU32(bytes, AcknowledgeRequestIdOffset));
    } else if (pduType == ActionResponsePdu && bytes.size() >= ActionResponsePduLength) {
        summary += QStringLiteral("; origin %1, target %2, request %3, response %4")
                       .arg(entityIdString(bytes, OriginEntityOffset))
                       .arg(entityIdString(bytes, TargetEntityOffset))
                       .arg(readU32(bytes, RequestIdOffset))
                       .arg(readU32(bytes, ActionResponseStatusOffset));
    } else if (bytes.size() >= TargetEntityOffset + EntityIdByteLength) {
        summary += QStringLiteral("; origin %1, target %2")
                       .arg(entityIdString(bytes, OriginEntityOffset))
                       .arg(entityIdString(bytes, TargetEntityOffset));
    }

    return summary;
}

auto requestIdFromResponse(const QByteArray &datagram, quint8 pduType) -> quint32
{
    if (pduType == AcknowledgePdu && datagram.size() >= AcknowledgePduLength) {
        return readU32(datagram, AcknowledgeRequestIdOffset);
    }
    if (pduType == ActionResponsePdu && datagram.size() >= ActionResponseStatusOffset) {
        return readU32(datagram, RequestIdOffset);
    }
    if (pduType == ActionRequestPdu && datagram.size() >= ActionResponseStatusOffset) {
        return readU32(datagram, RequestIdOffset);
    }
    if ((pduType == StartResumePdu || pduType == StopFreezePdu) && datagram.size() >= StopFreezePduLength) {
        return readU32(datagram, datagram.size() - RequestIdByteLength);
    }
    return 0;
}

} // namespace dispatch
