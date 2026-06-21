#include <DISPatch/AppConfig.h>

#include <DISPatch/Constants.h>
#include <DISPatch/DisProtocol.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtNetwork/QNetworkInterface>

namespace dispatch {

namespace {

auto rootConfigKeys() -> const QStringList &
{
    static const QStringList keys{QStringLiteral("theme"),
                                  QStringLiteral("network"),
                                  QStringLiteral("dis"),
                                  QStringLiteral("commands"),
                                  QStringLiteral("log"),
                                  QStringLiteral("testFederate")};
    return keys;
}

auto networkConfigKeys() -> const QStringList &
{
    static const QStringList keys{QStringLiteral("destinationAddress"),
                                  QStringLiteral("destinationPort"),
                                  QStringLiteral("listenAddress"),
                                  QStringLiteral("listenPort"),
                                  QStringLiteral("interfaceName"),
                                  QStringLiteral("multicastGroupAddress"),
                                  QStringLiteral("multicastInterfaceName"),
                                  QStringLiteral("shareAddress"),
                                  QStringLiteral("reuseAddress"),
                                  QStringLiteral("joinMulticast"),
                                  QStringLiteral("multicastLoopback")};
    return keys;
}

auto disConfigKeys() -> const QStringList &
{
    static const QStringList keys{QStringLiteral("exerciseId"),
                                  QStringLiteral("managerId"),
                                  QStringLiteral("targetId")};
    return keys;
}

auto entityIdConfigKeys() -> const QStringList &
{
    static const QStringList keys{QStringLiteral("site"),
                                  QStringLiteral("application"),
                                  QStringLiteral("entity")};
    return keys;
}

auto commandsConfigKeys() -> const QStringList &
{
    static const QStringList keys{QStringLiteral("startup"),
                                  QStringLiteral("standby"),
                                  QStringLiteral("shutdown")};
    return keys;
}

auto actionConfigKeys() -> const QStringList &
{
    static const QStringList keys{QStringLiteral("actionId")};
    return keys;
}

auto standbyConfigKeys() -> const QStringList &
{
    static const QStringList keys{QStringLiteral("reason"),
                                  QStringLiteral("frozenBehavior")};
    return keys;
}

auto testFederateConfigKeys() -> const QStringList &
{
    static const QStringList keys{QStringLiteral("enabled"),
                                  QStringLiteral("entityId")};
    return keys;
}

auto logConfigKeys() -> const QStringList &
{
    static const QStringList keys{QStringLiteral("logs"),
                                  QStringLiteral("logFile"),
                                  QStringLiteral("logLevel"),
                                  QStringLiteral("messageLogs"),
                                  QStringLiteral("messageLogFile")};
    return keys;
}

auto normalizedConfigKey(QString text) -> QString
{
    text = text.trimmed().toLower();
    QString normalized;
    normalized.reserve(text.size());
    for (const QChar character : text) {
        if (character.isLetterOrNumber()) {
            normalized.append(character);
        }
    }
    return normalized;
}

auto parseTheme(const QJsonValue &value, Theme *theme) -> bool
{
    if (!value.isString()) {
        return false;
    }

    const QString key = normalizedConfigKey(value.toString());
    if (key == QStringLiteral("dark")) {
        *theme = Theme::Dark;
        return true;
    }
    if (key == QStringLiteral("light")) {
        *theme = Theme::Light;
        return true;
    }
    if (key == QStringLiteral("gruvbox")) {
        *theme = Theme::Gruvbox;
        return true;
    }
    return false;
}

auto parseLogLevel(const QJsonValue &value, LogLevel *level) -> bool
{
    if (!value.isString()) {
        return false;
    }

    const QString key = normalizedConfigKey(value.toString());
    if (key == QStringLiteral("debug")) {
        *level = LogLevel::Debug;
        return true;
    }
    if (key == QStringLiteral("warn") || key == QStringLiteral("warning")) {
        *level = LogLevel::Warn;
        return true;
    }
    if (key == QStringLiteral("error")) {
        *level = LogLevel::Error;
        return true;
    }
    return false;
}

auto readInt(const QJsonObject &object,
             const QString &key,
             int fallback,
             int minimum,
             int maximum,
             QStringList *warnings,
             const QString &path) -> int
{
    if (!object.contains(key)) {
        return fallback;
    }

    const QJsonValue value = object.value(key);
    int parsed = fallback;
    if (value.isDouble()) {
        const double number = value.toDouble();
        parsed = static_cast<int>(number);
        if (number != parsed) {
            warnings->append(QStringLiteral("%1.%2 must be an integer; using %3")
                                 .arg(path, key)
                                 .arg(fallback));
            return fallback;
        }
    } else if (value.isString()) {
        bool parsedOk = false;
        parsed = value.toString().toInt(&parsedOk);
        if (!parsedOk) {
            warnings->append(QStringLiteral("%1.%2 must be an integer; using %3")
                                 .arg(path, key)
                                 .arg(fallback));
            return fallback;
        }
    } else {
        warnings->append(QStringLiteral("%1.%2 must be an integer; using %3")
                             .arg(path, key)
                             .arg(fallback));
        return fallback;
    }

    if (parsed < minimum || parsed > maximum) {
        warnings->append(QStringLiteral("%1.%2 must be between %3 and %4; using %5")
                             .arg(path, key)
                             .arg(minimum)
                             .arg(maximum)
                             .arg(fallback));
        return fallback;
    }

    return parsed;
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto readString(const QJsonObject &object,
                const QString &key,
                const QString &fallback,
                QStringList *warnings,
                const QString &path) -> QString
{
    if (!object.contains(key)) {
        return fallback;
    }

    const QJsonValue value = object.value(key);
    if (!value.isString()) {
        warnings->append(QStringLiteral("%1.%2 must be a string; using %3")
                             .arg(path, key, fallback));
        return fallback;
    }

    return value.toString();
}
// NOLINTEND(bugprone-easily-swappable-parameters)

auto readBool(const QJsonObject &object,
              const QString &key,
              bool fallback,
              QStringList *warnings,
              const QString &path) -> bool
{
    if (!object.contains(key)) {
        return fallback;
    }

    const QJsonValue value = object.value(key);
    if (!value.isBool()) {
        warnings->append(QStringLiteral("%1.%2 must be true or false; using %3")
                             .arg(path, key, fallback ? QStringLiteral("true") : QStringLiteral("false")));
        return fallback;
    }

    return value.toBool();
}

auto readObject(const QJsonObject &object, const QString &key, QStringList *warnings, const QString &path) -> QJsonObject
{
    if (!object.contains(key)) {
        return {};
    }

    const QJsonValue value = object.value(key);
    if (!value.isObject()) {
        warnings->append(QStringLiteral("%1.%2 must be an object; ignoring it").arg(path, key));
        return {};
    }

    return value.toObject();
}

void warnUnknownKeys(const QJsonObject &object,
                     const QStringList &knownKeys,
                     QStringList *warnings,
                     const QString &path)
{
    for (const QString &key : object.keys()) {
        if (!knownKeys.contains(key)) {
            warnings->append(QStringLiteral("%1.%2 is not a recognized config key; ignoring it")
                                 .arg(path, key));
        }
    }
}

auto readEntityIdConfig(const QJsonObject &object,
                        const QString &key,
                        EntityId fallback,
                        QStringList *warnings,
                        const QString &path) -> EntityId
{
    const QJsonObject entity = readObject(object, key, warnings, path);
    if (entity.isEmpty()) {
        return fallback;
    }

    const QString entityPath = QStringLiteral("%1.%2").arg(path, key);
    warnUnknownKeys(entity, entityIdConfigKeys(), warnings, entityPath);
    return EntityId{static_cast<quint16>(readInt(entity, QStringLiteral("site"), fallback.site, 0, BroadcastEntityIdValue, warnings, entityPath)),
                    static_cast<quint16>(readInt(entity,
                                                 QStringLiteral("application"),
                                                 fallback.application,
                                                 0,
                                                 BroadcastEntityIdValue,
                                                 warnings,
                                                 entityPath)),
                    static_cast<quint16>(readInt(entity,
                                                 QStringLiteral("entity"),
                                                 fallback.entity,
                                                 0,
                                                 BroadcastEntityIdValue,
                                                 warnings,
                                                 entityPath))};
}

auto readReason(const QJsonObject &object,
                const QString &key,
                quint8 fallback,
                QStringList *warnings,
                const QString &path) -> quint8
{
    if (!object.contains(key)) {
        return fallback;
    }

    const QJsonValue value = object.value(key);
    if (value.isString()) {
        bool parsedOk = false;
        const int number = value.toString().toInt(&parsedOk);
        if (parsedOk && number >= 0 && number <= MaxUint8Value) {
            return static_cast<quint8>(number);
        }

        const QString reasonKey = normalizedConfigKey(value.toString());
        for (const auto &option : StopFreezeReasonOptions) {
            if (reasonKey == normalizedConfigKey(QString::fromLatin1(option.key))
                || reasonKey == normalizedConfigKey(QString::fromLatin1(option.label))) {
                return option.value;
            }
        }

        warnings->append(QStringLiteral("%1.%2 has unknown Stop/Freeze reason \"%3\"; using %4")
                             .arg(path, key, value.toString())
                             .arg(fallback));
        return fallback;
    }

    return static_cast<quint8>(readInt(object, key, fallback, 0, MaxUint8Value, warnings, path));
}

auto configSearchPaths() -> QStringList
{
    const QStringList arguments = QCoreApplication::arguments();
    for (int i = 1; i < arguments.size(); ++i) {
        if (arguments.at(i) == QStringLiteral("--config") && i + 1 < arguments.size()) {
            return {arguments.at(i + 1)};
        }
    }

    const QString fileName = QStringLiteral("DISPatch_config.json");
    return {QDir::current().filePath(fileName),
            QDir(QCoreApplication::applicationDirPath()).filePath(fileName)};
}

void validateNetworkConfig(const AppConfig &config, QStringList *warnings) // NOLINT(readability-function-cognitive-complexity)
{
    QHostAddress destinationAddress;
    const bool destinationValid = parseConfigAddress(config.destinationAddress, &destinationAddress);
    if (!destinationValid) {
        warnings->append(QStringLiteral("config.network.destinationAddress \"%1\" is not a valid IP address")
                             .arg(config.destinationAddress));
    } else if (isAnyAddress(destinationAddress)) {
        warnings->append(QStringLiteral(
            "config.network.destinationAddress is a wildcard address; sending commands to it is probably invalid"));
    }

    QHostAddress listenAddress;
    const bool listenValid = parseConfigAddress(config.listenAddress, &listenAddress);
    if (!listenValid) {
        warnings->append(QStringLiteral("config.network.listenAddress \"%1\" is not a valid bind address")
                             .arg(config.listenAddress));
    } else if (listenAddress.isMulticast()) {
        warnings->append(QStringLiteral(
            "config.network.listenAddress is multicast; use a local bind address like 0.0.0.0 and set multicastGroupAddress instead"));
    }

    QHostAddress explicitMulticastGroup;
    const bool hasExplicitMulticastGroup = !config.multicastGroupAddress.trimmed().isEmpty();
    bool explicitMulticastValid = false;
    if (hasExplicitMulticastGroup) {
        explicitMulticastValid = parseConfigAddress(config.multicastGroupAddress, &explicitMulticastGroup);
        if (!explicitMulticastValid) {
            warnings->append(QStringLiteral("config.network.multicastGroupAddress \"%1\" is not a valid IP address")
                                 .arg(config.multicastGroupAddress));
        } else if (!explicitMulticastGroup.isMulticast()) {
            warnings->append(QStringLiteral(
                "config.network.multicastGroupAddress \"%1\" is not multicast; expected 224.0.0.0-239.255.255.255 or an IPv6 multicast address")
                                 .arg(config.multicastGroupAddress));
        }
    }

    const bool destinationIsMulticast = destinationValid && destinationAddress.isMulticast();
    const bool hasEffectiveMulticastGroup =
        (hasExplicitMulticastGroup && explicitMulticastValid && explicitMulticastGroup.isMulticast())
        || (!hasExplicitMulticastGroup && destinationIsMulticast);

    if (config.joinMulticast && !hasEffectiveMulticastGroup) {
        warnings->append(QStringLiteral(
            "config.network.joinMulticast is true, but no valid multicast group is configured"));
    }
    if (!config.joinMulticast && destinationIsMulticast) {
        warnings->append(QStringLiteral(
            "config.network.destinationAddress is multicast, but joinMulticast is false; this instance may not receive multicast traffic"));
    }
    if (config.joinMulticast && listenValid && !isAnyAddress(listenAddress)) {
        warnings->append(QStringLiteral(
            "config.network.joinMulticast is true while listenAddress is specific; 0.0.0.0 is usually safer for multicast receives"));
    }

    if (!config.interfaceName.trimmed().isEmpty()) {
        const QNetworkInterface interface =
            QNetworkInterface::interfaceFromName(config.interfaceName.trimmed());
        if (!interface.isValid()) {
            warnings->append(QStringLiteral("config.network.interfaceName \"%1\" is not a known interface")
                                 .arg(config.interfaceName));
        }
        if (!config.multicastInterfaceName.trimmed().isEmpty()
            && config.multicastInterfaceName.trimmed() != config.interfaceName.trimmed()) {
            warnings->append(QStringLiteral(
                "config.network.interfaceName overrides legacy multicastInterfaceName"));
        }
    } else if (!config.multicastInterfaceName.trimmed().isEmpty()) {
        const QNetworkInterface interface =
            QNetworkInterface::interfaceFromName(config.multicastInterfaceName.trimmed());
        if (!interface.isValid()) {
            warnings->append(QStringLiteral("config.network.multicastInterfaceName \"%1\" is not a known interface")
                                 .arg(config.multicastInterfaceName));
        }
    }

    if (!config.interfaceName.trimmed().isEmpty()
        && (!config.joinMulticast || !hasEffectiveMulticastGroup)
        && destinationIsMulticast) {
        warnings->append(QStringLiteral(
            "config.network.interfaceName is set, but no multicast join will be attempted"));
    }

    if ((!config.shareAddress || !config.reuseAddress)
        && (destinationIsMulticast || config.destinationPort == config.listenPort)) {
        warnings->append(QStringLiteral(
            "config.network.shareAddress and reuseAddress should usually both be true when multiple DIS apps share one UDP port"));
    }

    if (!config.multicastLoopback && destinationIsMulticast
        && config.destinationPort == config.listenPort) {
        warnings->append(QStringLiteral(
            "config.network.multicastLoopback is false; local same-machine multicast testing may not see looped-back traffic"));
    }

    if (config.testFederateEnabled && config.destinationPort == config.listenPort
        && (!config.shareAddress || !config.reuseAddress)) {
        warnings->append(QStringLiteral(
            "testFederate.enabled uses a second local socket on the destination port; enable shareAddress and reuseAddress when ports overlap"));
    }
}

} // namespace

auto parseConfigAddress(const QString &text, QHostAddress *address) -> bool
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return false;
    }

    QHostAddress parsed;
    if (!parsed.setAddress(trimmed)) {
        return false;
    }

    *address = parsed;
    return true;
}

auto isAnyAddress(const QHostAddress &address) -> bool
{
    return address == QHostAddress(QHostAddress::Any)
        || address == QHostAddress(QHostAddress::AnyIPv4)
        || address == QHostAddress(QHostAddress::AnyIPv6);
}

auto isBroadcastAddress(const QHostAddress &address) -> bool
{
    return address == QHostAddress(QHostAddress::Broadcast)
        || address.toString() == QString::fromLatin1(BroadcastDestinationAddress);
}

auto loadAppConfig(QStringList *warnings) -> AppConfig
{
    AppConfig config;
    QString configPath;
    for (const QString &path : configSearchPaths()) {
        if (QFile::exists(path)) {
            configPath = path;
            break;
        }
    }

    if (configPath.isEmpty()) {
        warnings->append(QStringLiteral("No DISPatch_config.json found; using built-in defaults"));
        return config;
    }

    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        warnings->append(QStringLiteral("Could not open %1: %2; using built-in defaults")
                             .arg(configPath, file.errorString()));
        return config;
    }

    QJsonParseError parseError{};
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        warnings->append(QStringLiteral("Could not parse %1: %2; using built-in defaults")
                             .arg(configPath, parseError.errorString()));
        return config;
    }

    config.configPath = configPath;
    const QJsonObject root = document.object();
    warnUnknownKeys(root, rootConfigKeys(), warnings, QStringLiteral("config"));

    if (root.contains(QStringLiteral("theme"))) {
        Theme theme = config.theme;
        if (parseTheme(root.value(QStringLiteral("theme")), &theme)) {
            config.theme = theme;
        } else {
            warnings->append(QStringLiteral("theme must be \"dark\", \"light\", or \"gruvbox\"; using dark"));
        }
    }

    const QJsonObject network = readObject(root, QStringLiteral("network"), warnings, QStringLiteral("config"));
    warnUnknownKeys(network, networkConfigKeys(), warnings, QStringLiteral("config.network"));
    config.destinationAddress = readString(network,
                                           QStringLiteral("destinationAddress"),
                                           config.destinationAddress,
                                           warnings,
                                           QStringLiteral("config.network"));
    config.destinationPort = static_cast<quint16>(readInt(network,
                                                          QStringLiteral("destinationPort"),
                                                          config.destinationPort,
                                                          1,
                                                          MaxUdpPort,
                                                          warnings,
                                                          QStringLiteral("config.network")));
    config.listenAddress = readString(network,
                                      QStringLiteral("listenAddress"),
                                      config.listenAddress,
                                      warnings,
                                      QStringLiteral("config.network"));
    config.listenPort = static_cast<quint16>(readInt(network,
                                                     QStringLiteral("listenPort"),
                                                     config.listenPort,
                                                     1,
                                                     MaxUdpPort,
                                                     warnings,
                                                     QStringLiteral("config.network")));
    config.interfaceName = readString(network,
                                      QStringLiteral("interfaceName"),
                                      config.interfaceName,
                                      warnings,
                                      QStringLiteral("config.network"));
    config.multicastGroupAddress = readString(network,
                                              QStringLiteral("multicastGroupAddress"),
                                              config.multicastGroupAddress,
                                              warnings,
                                              QStringLiteral("config.network"));
    config.multicastInterfaceName = readString(network,
                                               QStringLiteral("multicastInterfaceName"),
                                               config.multicastInterfaceName,
                                               warnings,
                                               QStringLiteral("config.network"));
    if (config.interfaceName.trimmed().isEmpty()) {
        config.interfaceName = config.multicastInterfaceName;
    }
    config.shareAddress = readBool(network,
                                   QStringLiteral("shareAddress"),
                                   config.shareAddress,
                                   warnings,
                                   QStringLiteral("config.network"));
    config.reuseAddress = readBool(network,
                                   QStringLiteral("reuseAddress"),
                                   config.reuseAddress,
                                   warnings,
                                   QStringLiteral("config.network"));
    config.joinMulticast = readBool(network,
                                    QStringLiteral("joinMulticast"),
                                    config.joinMulticast,
                                    warnings,
                                    QStringLiteral("config.network"));
    config.multicastLoopback = readBool(network,
                                        QStringLiteral("multicastLoopback"),
                                        config.multicastLoopback,
                                        warnings,
                                        QStringLiteral("config.network"));

    const QJsonObject dis = readObject(root, QStringLiteral("dis"), warnings, QStringLiteral("config"));
    warnUnknownKeys(dis, disConfigKeys(), warnings, QStringLiteral("config.dis"));
    config.exerciseId = static_cast<quint8>(readInt(dis,
                                                    QStringLiteral("exerciseId"),
                                                    config.exerciseId,
                                                    1,
                                                    MaxExerciseId,
                                                    warnings,
                                                    QStringLiteral("config.dis")));
    config.managerId = readEntityIdConfig(dis,
                                          QStringLiteral("managerId"),
                                          config.managerId,
                                          warnings,
                                          QStringLiteral("config.dis"));
    config.targetId = readEntityIdConfig(dis,
                                         QStringLiteral("targetId"),
                                         config.targetId,
                                         warnings,
                                         QStringLiteral("config.dis"));

    const QJsonObject commands = readObject(root, QStringLiteral("commands"), warnings, QStringLiteral("config"));
    warnUnknownKeys(commands, commandsConfigKeys(), warnings, QStringLiteral("config.commands"));
    const QJsonObject startup = readObject(commands, QStringLiteral("startup"), warnings, QStringLiteral("config.commands"));
    warnUnknownKeys(startup, actionConfigKeys(), warnings, QStringLiteral("config.commands.startup"));
    config.startupActionId = static_cast<quint32>(readInt(startup,
                                                          QStringLiteral("actionId"),
                                                          static_cast<int>(config.startupActionId),
                                                          0,
                                                          MaxActionId,
                                                          warnings,
                                                          QStringLiteral("config.commands.startup")));
    const QJsonObject standby = readObject(commands, QStringLiteral("standby"), warnings, QStringLiteral("config.commands"));
    warnUnknownKeys(standby, standbyConfigKeys(), warnings, QStringLiteral("config.commands.standby"));
    config.standbyReason =
        readReason(standby, QStringLiteral("reason"), config.standbyReason, warnings, QStringLiteral("config.commands.standby"));
    config.standbyFrozenBehavior = static_cast<quint8>(readInt(standby,
                                                               QStringLiteral("frozenBehavior"),
                                                               config.standbyFrozenBehavior,
                                                               0,
                                                               MaxUint8Value,
                                                               warnings,
                                                               QStringLiteral("config.commands.standby")));
    const QJsonObject shutdown =
        readObject(commands, QStringLiteral("shutdown"), warnings, QStringLiteral("config.commands"));
    warnUnknownKeys(shutdown, actionConfigKeys(), warnings, QStringLiteral("config.commands.shutdown"));
    config.shutdownActionId = static_cast<quint32>(readInt(shutdown,
                                                           QStringLiteral("actionId"),
                                                           static_cast<int>(config.shutdownActionId),
                                                           0,
                                                           MaxActionId,
                                                           warnings,
                                                           QStringLiteral("config.commands.shutdown")));

    const QJsonObject log = readObject(root, QStringLiteral("log"), warnings, QStringLiteral("config"));
    warnUnknownKeys(log, logConfigKeys(), warnings, QStringLiteral("config.log"));
    config.logs = readBool(log,
                           QStringLiteral("logs"),
                           config.logs,
                           warnings,
                           QStringLiteral("config.log"));
    config.logFile = readString(log,
                                QStringLiteral("logFile"),
                                config.logFile,
                                warnings,
                                QStringLiteral("config.log"));
    if (log.contains(QStringLiteral("logLevel"))) {
        LogLevel logLevel = config.logLevel;
        if (parseLogLevel(log.value(QStringLiteral("logLevel")), &logLevel)) {
            config.logLevel = logLevel;
        } else {
            warnings->append(QStringLiteral("config.log.logLevel must be \"debug\", \"warn\", or \"error\"; using debug"));
        }
    }
    config.messageLogs = readBool(log,
                                  QStringLiteral("messageLogs"),
                                  config.messageLogs,
                                  warnings,
                                  QStringLiteral("config.log"));
    config.messageLogFile = readString(log,
                                       QStringLiteral("messageLogFile"),
                                       config.messageLogFile,
                                       warnings,
                                       QStringLiteral("config.log"));

    const QJsonObject testFederate = readObject(root,
                                                QStringLiteral("testFederate"),
                                                warnings,
                                                QStringLiteral("config"));
    warnUnknownKeys(testFederate, testFederateConfigKeys(), warnings, QStringLiteral("config.testFederate"));
    config.testFederateEnabled = readBool(testFederate,
                                          QStringLiteral("enabled"),
                                          config.testFederateEnabled,
                                          warnings,
                                          QStringLiteral("config.testFederate"));
    config.testFederateId = readEntityIdConfig(testFederate,
                                               QStringLiteral("entityId"),
                                               config.testFederateId,
                                               warnings,
                                               QStringLiteral("config.testFederate"));
    validateNetworkConfig(config, warnings);

    return config;
}

} // namespace dispatch
