#pragma once

#include <QtCore/QtGlobal>
#include <QtNetwork/QHostAddress>

namespace dispatch {

enum class SimulationState : quint8 {
    Startup,
    Standby,
    Operate,
    Shutdown
};

struct EntityId {
    quint16 site = 1;
    quint16 application = 1;
    quint16 entity = 1;
};

struct DisConfig {
    QHostAddress destinationAddress;
    quint16 destinationPort = 3000;
    QHostAddress listenAddress = QHostAddress::AnyIPv4;
    quint16 listenPort = 3000;
    quint8 exerciseId = 1;
    EntityId managerId;
    EntityId targetId;
    quint32 startupActionId = 1;
    quint32 shutdownActionId = 2;
    quint8 standbyReason = 1;
    quint8 standbyFrozenBehavior = 0;
};

} // namespace dispatch
