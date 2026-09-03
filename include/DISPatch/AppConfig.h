#pragma once

#include <DISPatch/Constants.h>
#include <DISPatch/DisTypes.h>
#include <DISPatch/Theme.h>

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QtGlobal>
#include <QtNetwork/QHostAddress>

namespace dispatch {

enum class LogLevel : quint8 {
    Debug,
    Warn,
    Error
};

struct AppConfig {
    QString destinationAddress = QStringLiteral("255.255.255.255");
    quint16 destinationPort = 3000;
    QString listenAddress = QStringLiteral("0.0.0.0");
    quint16 listenPort = 3000;
    QString interfaceName;
    QString multicastGroupAddress;
    QString multicastInterfaceName;
    bool shareAddress = true;
    bool reuseAddress = true;
    bool joinMulticast = false;
    bool multicastLoopback = false;
    bool heartbeatEnabled = false;
    int heartbeatTimeoutSeconds = DefaultHeartbeatTimeoutSeconds;
    quint8 exerciseId = 1;
    EntityId managerId;
    EntityId targetId = EntityId{1, 1, 0};
    quint32 initializeActionId = 39;
    int startRealWorldTimeOffsetSeconds = 0;
    int startSimulationTimeOffsetSeconds = 0;
    quint8 pauseFrozenBehavior = 0;
    quint8 stopFrozenBehavior = 0;
    quint8 resetFrozenBehavior = 0;
    Theme theme = Theme::Dark;
    bool logs = false;
    QString logFile = QStringLiteral("DISPatch.log");
    LogLevel logLevel = LogLevel::Debug;
    bool messageLogs = false;
    QString messageLogFile = QStringLiteral("DISPatch_messages.log");
    QString logDir;
    bool testFederateEnabled = false;
    QList<EntityId> testFederateIds{EntityId{1, 1, 0}};
    QString configPath;
};

auto parseConfigAddress(const QString &text, QHostAddress *address) -> bool;
auto isAnyAddress(const QHostAddress &address) -> bool;
auto isBroadcastAddress(const QHostAddress &address) -> bool;
auto loadAppConfig(const QString &path, QStringList *warnings) -> AppConfig;
auto loadAppConfig(const QStringList &arguments, QStringList *warnings) -> AppConfig;
auto loadAppConfig(QStringList *warnings) -> AppConfig;

} // namespace dispatch
