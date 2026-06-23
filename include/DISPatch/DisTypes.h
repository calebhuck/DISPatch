#pragma once

#include <QtCore/QtGlobal>
#include <QtNetwork/QHostAddress>

namespace dispatch {

enum class SimulationCommand : quint8 {
    Initialize,
    Start,
    Pause,
    Stop,
    Reset
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
    quint32 initializeActionId = 39;
    int startRealWorldTimeOffsetSeconds = 0;
    int startSimulationTimeOffsetSeconds = 0;
    bool startUseLiteralZero = false;
    quint8 pauseFrozenBehavior = 0;
    quint8 stopFrozenBehavior = 0;
    quint8 resetFrozenBehavior = 0;
};

} // namespace dispatch
