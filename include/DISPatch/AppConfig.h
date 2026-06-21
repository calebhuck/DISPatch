#pragma once

#include <DISPatch/DisTypes.h>
#include <DISPatch/Theme.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QtGlobal>
#include <QtNetwork/QHostAddress>

namespace dispatch {

struct AppConfig {
    QString destinationAddress = QStringLiteral("239.1.2.3");
    quint16 destinationPort = 3000;
    QString listenAddress = QStringLiteral("0.0.0.0");
    quint16 listenPort = 3000;
    QString multicastGroupAddress;
    QString multicastInterfaceName;
    bool shareAddress = true;
    bool reuseAddress = true;
    bool joinMulticast = true;
    bool multicastLoopback = true;
    quint8 exerciseId = 1;
    EntityId managerId;
    EntityId targetId = EntityId{1, 1, 0};
    quint32 startupActionId = 1;
    quint32 shutdownActionId = 2;
    quint8 standbyReason = 1;
    quint8 standbyFrozenBehavior = 0;
    Theme theme = Theme::Dark;
    bool testFederateEnabled = false;
    EntityId testFederateId = EntityId{1, 1, 0};
    QString configPath;
};

auto parseConfigAddress(const QString &text, QHostAddress *address) -> bool;
auto isAnyAddress(const QHostAddress &address) -> bool;
auto loadAppConfig(QStringList *warnings) -> AppConfig;

} // namespace dispatch
