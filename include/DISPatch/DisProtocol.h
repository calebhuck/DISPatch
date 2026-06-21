#pragma once

#include <DISPatch/Constants.h>
#include <DISPatch/DisTypes.h>

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QtGlobal>

namespace dispatch {

auto stateName(SimulationState state) -> QString;
auto stopFreezeReasonLabel(quint8 reason) -> QString;
auto pduTypeName(quint8 pduType) -> QString;
auto entityIdsMatch(const EntityId &left, const EntityId &right) -> bool;
auto entityIdString(const EntityId &entityId) -> QString;
auto makeStartResumePdu(const DisConfig &config, quint32 requestId) -> QByteArray;
auto makeStopFreezePdu(const DisConfig &config, quint32 requestId) -> QByteArray;
auto makeActionRequestPdu(const DisConfig &config, quint32 requestId, SimulationState state) -> QByteArray;
auto makeAcknowledgePdu(const DisConfig &config, quint32 requestId, PduType requestType) -> QByteArray;
auto makeActionResponsePdu(const DisConfig &config, quint32 requestId) -> QByteArray;
auto readU16(const QByteArray &bytes, int offset) -> quint16;
auto readU32(const QByteArray &bytes, int offset) -> quint32;
auto readEntityId(const QByteArray &bytes, int offset) -> EntityId;
auto entityIdString(const QByteArray &bytes, int offset) -> QString;
auto responseSummary(const QByteArray &bytes) -> QString;
auto requestIdFromResponse(const QByteArray &datagram, quint8 pduType) -> quint32;

} // namespace dispatch
