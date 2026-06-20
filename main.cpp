#include <QtWidgets/QApplication>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>

#include <QtGui/QColor>
#include <QtGui/QPalette>

#include <QtNetwork/QHostAddress>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QUdpSocket>

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QMap>
#include <QtCore/QSignalBlocker>
#include <QtCore/QStringList>
#include <QtCore/QTime>
#include <QtCore/QTimer>

#include <cstdint>

namespace {

constexpr quint8 DisVersion = 6;
constexpr quint8 SimManagementFamily = 5;

enum PduType : quint8 {
    StartResumePdu = 13,
    StopFreezePdu = 14,
    AcknowledgePdu = 15,
    ActionRequestPdu = 16,
    ActionResponsePdu = 17
};

enum class SimulationState {
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
    quint16 listenPort = 3001;
    quint8 exerciseId = 1;
    EntityId managerId;
    EntityId targetId;
    quint32 startupActionId = 1;
    quint32 shutdownActionId = 2;
    quint8 standbyReason = 1;
    quint8 standbyFrozenBehavior = 0;
};

enum class Theme {
    Dark,
    Light
};

struct AppConfig {
    QString destinationAddress = QStringLiteral("239.1.2.3");
    quint16 destinationPort = 3000;
    QString listenAddress = QStringLiteral("0.0.0.0");
    quint16 listenPort = 3001;
    quint8 exerciseId = 1;
    EntityId managerId;
    EntityId targetId = EntityId{1, 1, 0};
    quint32 startupActionId = 1;
    quint32 shutdownActionId = 2;
    quint8 standbyReason = 1;
    quint8 standbyFrozenBehavior = 0;
    Theme theme = Theme::Dark;
    bool testFederateEnabled = false;
    QString configPath;
};

struct StopFreezeReasonOption {
    quint8 value;
    const char *key;
    const char *label;
};

constexpr StopFreezeReasonOption StopFreezeReasonOptions[] = {
    {0, "other", "Other"},
    {1, "recess", "Recess"},
    {2, "termination", "Termination"},
    {3, "system_failure", "System Failure"},
    {4, "security_violation", "Security Violation"},
    {5, "entity_reconstitution", "Entity Reconstitution"},
    {6, "stop_for_reset", "Stop For Reset"},
    {7, "stop_for_restart", "Stop For Restart"},
    {8, "abort_training_return_to_tactical_operations", "Abort Training Return To Tactical Operations"},
};

quint32 disTimestamp()
{
    const auto now = QTime::currentTime();
    const auto milliseconds =
        static_cast<quint32>((now.hour() * 3600 + now.minute() * 60 + now.second()) * 1000
                             + now.msec());
    return static_cast<quint32>((static_cast<quint64>(milliseconds) << 31) / 86400000ULL);
}

QString stateName(SimulationState state)
{
    switch (state) {
    case SimulationState::Startup:
        return QStringLiteral("Startup");
    case SimulationState::Standby:
        return QStringLiteral("Standby");
    case SimulationState::Operate:
        return QStringLiteral("Operate");
    case SimulationState::Shutdown:
        return QStringLiteral("Shutdown");
    }

    return QStringLiteral("Unknown");
}

QString stopFreezeReasonLabel(quint8 reason)
{
    for (const auto &option : StopFreezeReasonOptions) {
        if (option.value == reason) {
            return QString::fromLatin1(option.label);
        }
    }

    return QStringLiteral("Reason %1").arg(reason);
}

QString pduTypeName(quint8 pduType)
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

void writeEntityId(QDataStream &out, const EntityId &entityId)
{
    out << entityId.site << entityId.application << entityId.entity;
}

QByteArray makeHeader(const DisConfig &config, PduType pduType, quint16 length)
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

void appendRealWorldTime(QDataStream &out)
{
    const auto now = QDateTime::currentDateTimeUtc();
    out << static_cast<quint32>(now.date().toJulianDay());
    out << static_cast<quint32>((now.time().msecsSinceStartOfDay() * 65536LL) / 1000LL);
}

void appendSimulationTime(QDataStream &out)
{
    appendRealWorldTime(out);
}

QByteArray makeStartResumePdu(const DisConfig &config, quint32 requestId)
{
    constexpr quint16 length = 44;
    QByteArray bytes = makeHeader(config, StartResumePdu, length);
    QDataStream out(&bytes, QIODevice::Append);
    out.setByteOrder(QDataStream::BigEndian);

    writeEntityId(out, config.managerId);
    writeEntityId(out, config.targetId);
    appendRealWorldTime(out);
    appendSimulationTime(out);
    out << requestId;

    return bytes;
}

QByteArray makeStopFreezePdu(const DisConfig &config, quint32 requestId)
{
    constexpr quint16 length = 36;

    QByteArray bytes = makeHeader(config, StopFreezePdu, length);
    QDataStream out(&bytes, QIODevice::Append);
    out.setByteOrder(QDataStream::BigEndian);

    writeEntityId(out, config.managerId);
    writeEntityId(out, config.targetId);
    appendRealWorldTime(out);
    out << config.standbyReason;
    out << config.standbyFrozenBehavior;
    out << static_cast<quint16>(0);
    out << requestId;

    return bytes;
}

QByteArray makeActionRequestPdu(const DisConfig &config, quint32 requestId, SimulationState state)
{
    constexpr quint16 length = 40;
    const quint32 actionId =
        state == SimulationState::Startup ? config.startupActionId : config.shutdownActionId;

    QByteArray bytes = makeHeader(config, ActionRequestPdu, length);
    QDataStream out(&bytes, QIODevice::Append);
    out.setByteOrder(QDataStream::BigEndian);

    writeEntityId(out, config.managerId);
    writeEntityId(out, config.targetId);
    out << requestId;
    out << actionId;
    out << static_cast<quint32>(0);
    out << static_cast<quint32>(0);

    return bytes;
}

QByteArray makeAcknowledgePdu(const DisConfig &config, quint32 requestId, PduType requestType)
{
    constexpr quint16 length = 32;
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

QByteArray makeActionResponsePdu(const DisConfig &config, quint32 requestId)
{
    constexpr quint16 length = 40;
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

quint16 readU16(const QByteArray &bytes, int offset)
{
    if (offset + 2 > bytes.size()) {
        return 0;
    }

    return static_cast<quint16>((static_cast<quint8>(bytes[offset]) << 8)
                               | static_cast<quint8>(bytes[offset + 1]));
}

quint32 readU32(const QByteArray &bytes, int offset)
{
    if (offset + 4 > bytes.size()) {
        return 0;
    }

    return (static_cast<quint32>(static_cast<quint8>(bytes[offset])) << 24)
        | (static_cast<quint32>(static_cast<quint8>(bytes[offset + 1])) << 16)
        | (static_cast<quint32>(static_cast<quint8>(bytes[offset + 2])) << 8)
        | static_cast<quint32>(static_cast<quint8>(bytes[offset + 3]));
}

EntityId readEntityId(const QByteArray &bytes, int offset)
{
    return EntityId{readU16(bytes, offset),
                    readU16(bytes, offset + 2),
                    readU16(bytes, offset + 4)};
}

QString entityIdString(const QByteArray &bytes, int offset)
{
    return QStringLiteral("%1:%2:%3")
        .arg(readU16(bytes, offset))
        .arg(readU16(bytes, offset + 2))
        .arg(readU16(bytes, offset + 4));
}

QString responseSummary(const QByteArray &bytes)
{
    if (bytes.size() < 12) {
        return QStringLiteral("Received %1 byte datagram that is too short for a DIS header")
            .arg(bytes.size());
    }

    const quint8 version = static_cast<quint8>(bytes[0]);
    const quint8 exerciseId = static_cast<quint8>(bytes[1]);
    const quint8 pduType = static_cast<quint8>(bytes[2]);
    const quint8 family = static_cast<quint8>(bytes[3]);
    const quint16 pduLength = readU16(bytes, 8);
    QString summary = QStringLiteral("DIS%1 exercise %2 %3, family %4, length %5")
                          .arg(version)
                          .arg(exerciseId)
                          .arg(pduTypeName(pduType))
                          .arg(family)
                          .arg(pduLength);

    if (family != SimManagementFamily) {
        return summary + QStringLiteral(" (non-Simulation Management)");
    }

    if (pduType == AcknowledgePdu && bytes.size() >= 32) {
        summary += QStringLiteral("; origin %1, target %2, ack flag %3, response flag %4, request %5")
                       .arg(entityIdString(bytes, 12))
                       .arg(entityIdString(bytes, 18))
                       .arg(readU16(bytes, 24))
                       .arg(readU16(bytes, 26))
                       .arg(readU32(bytes, 28));
    } else if (pduType == ActionResponsePdu && bytes.size() >= 40) {
        summary += QStringLiteral("; origin %1, target %2, request %3, response %4")
                       .arg(entityIdString(bytes, 12))
                       .arg(entityIdString(bytes, 18))
                       .arg(readU32(bytes, 24))
                       .arg(readU32(bytes, 28));
    } else if (bytes.size() >= 24) {
        summary += QStringLiteral("; origin %1, target %2")
                       .arg(entityIdString(bytes, 12))
                       .arg(entityIdString(bytes, 18));
    }

    return summary;
}

QPalette themePalette(Theme theme)
{
    QPalette palette;
    if (theme == Theme::Dark) {
        palette.setColor(QPalette::Window, QColor(28, 31, 36));
        palette.setColor(QPalette::WindowText, QColor(232, 236, 241));
        palette.setColor(QPalette::Base, QColor(20, 23, 27));
        palette.setColor(QPalette::AlternateBase, QColor(33, 38, 45));
        palette.setColor(QPalette::ToolTipBase, QColor(232, 236, 241));
        palette.setColor(QPalette::ToolTipText, QColor(20, 23, 27));
        palette.setColor(QPalette::Text, QColor(232, 236, 241));
        palette.setColor(QPalette::Button, QColor(39, 44, 52));
        palette.setColor(QPalette::ButtonText, QColor(232, 236, 241));
        palette.setColor(QPalette::BrightText, QColor(255, 255, 255));
        palette.setColor(QPalette::Highlight, QColor(43, 130, 190));
        palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor(134, 142, 150));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(134, 142, 150));
        return palette;
    }

    palette.setColor(QPalette::Window, QColor(244, 246, 248));
    palette.setColor(QPalette::WindowText, QColor(31, 35, 40));
    palette.setColor(QPalette::Base, QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase, QColor(235, 239, 244));
    palette.setColor(QPalette::ToolTipBase, QColor(31, 35, 40));
    palette.setColor(QPalette::ToolTipText, QColor(255, 255, 255));
    palette.setColor(QPalette::Text, QColor(31, 35, 40));
    palette.setColor(QPalette::Button, QColor(250, 251, 252));
    palette.setColor(QPalette::ButtonText, QColor(31, 35, 40));
    palette.setColor(QPalette::BrightText, QColor(191, 38, 38));
    palette.setColor(QPalette::Highlight, QColor(0, 105, 170));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    return palette;
}

QString themeStyleSheet(Theme theme)
{
    if (theme == Theme::Dark) {
        return QStringLiteral(R"(
            QWidget { font-size: 10pt; }
            QMainWindow, QWidget { background: #1c1f24; color: #e8ecf1; }
            QGroupBox {
                border: 1px solid #3a414c;
                border-radius: 6px;
                margin-top: 16px;
                padding: 14px 12px 12px 12px;
                font-weight: 600;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 6px;
                color: #f4f7fb;
            }
            QLineEdit, QSpinBox, QComboBox, QPlainTextEdit, QTableWidget {
                background: #14171b;
                border: 1px solid #3a414c;
                border-radius: 4px;
                color: #e8ecf1;
                selection-background-color: #2b82be;
            }
            QLineEdit, QSpinBox, QComboBox { min-height: 26px; padding: 2px 6px; }
            QPushButton {
                background: #2b82be;
                border: 1px solid #3b91cc;
                border-radius: 4px;
                color: white;
                font-weight: 600;
                padding: 7px 14px;
            }
            QPushButton:hover { background: #3393d5; }
            QPushButton:pressed { background: #1f6fa6; }
            QHeaderView::section {
                background: #272c34;
                border: 0;
                border-right: 1px solid #3a414c;
                border-bottom: 1px solid #3a414c;
                color: #e8ecf1;
                font-weight: 600;
                padding: 6px;
            }
            QTableWidget::item { padding: 4px; }
            QStatusBar { border-top: 1px solid #3a414c; }
        )");
    }

    return QStringLiteral(R"(
        QWidget { font-size: 10pt; }
        QMainWindow, QWidget { background: #f4f6f8; color: #1f2328; }
        QGroupBox {
            background: #ffffff;
            border: 1px solid #d0d7de;
            border-radius: 6px;
            margin-top: 16px;
            padding: 14px 12px 12px 12px;
            font-weight: 600;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 6px;
            color: #1f2328;
        }
        QLineEdit, QSpinBox, QComboBox, QPlainTextEdit, QTableWidget {
            background: #ffffff;
            border: 1px solid #c9d1d9;
            border-radius: 4px;
            color: #1f2328;
            selection-background-color: #0069aa;
        }
        QLineEdit, QSpinBox, QComboBox { min-height: 26px; padding: 2px 6px; }
        QPushButton {
            background: #0069aa;
            border: 1px solid #005d97;
            border-radius: 4px;
            color: white;
            font-weight: 600;
            padding: 7px 14px;
        }
        QPushButton:hover { background: #0878bd; }
        QPushButton:pressed { background: #005489; }
        QHeaderView::section {
            background: #e9edf2;
            border: 0;
            border-right: 1px solid #d0d7de;
            border-bottom: 1px solid #d0d7de;
            color: #1f2328;
            font-weight: 600;
            padding: 6px;
        }
        QTableWidget::item { padding: 4px; }
        QStatusBar { border-top: 1px solid #d0d7de; }
    )");
}

QString normalizedConfigKey(QString text)
{
    text = text.trimmed().toLower();
    QString normalized;
    normalized.reserve(text.size());
    for (const QChar ch : text) {
        if (ch.isLetterOrNumber()) {
            normalized.append(ch);
        }
    }
    return normalized;
}

bool parseTheme(const QJsonValue &value, Theme *theme)
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
    return false;
}

int readInt(const QJsonObject &object,
            const QString &key,
            int fallback,
            int minimum,
            int maximum,
            QStringList *warnings,
            const QString &path)
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
        bool ok = false;
        parsed = value.toString().toInt(&ok);
        if (!ok) {
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

QString readString(const QJsonObject &object,
                   const QString &key,
                   const QString &fallback,
                   QStringList *warnings,
                   const QString &path)
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

bool readBool(const QJsonObject &object,
              const QString &key,
              bool fallback,
              QStringList *warnings,
              const QString &path)
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

QJsonObject readObject(const QJsonObject &object, const QString &key, QStringList *warnings, const QString &path)
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

EntityId readEntityIdConfig(const QJsonObject &object,
                            const QString &key,
                            EntityId fallback,
                            QStringList *warnings,
                            const QString &path)
{
    const QJsonObject entity = readObject(object, key, warnings, path);
    if (entity.isEmpty()) {
        return fallback;
    }

    const QString entityPath = QStringLiteral("%1.%2").arg(path, key);
    return EntityId{static_cast<quint16>(readInt(entity, QStringLiteral("site"), fallback.site, 0, 65535, warnings, entityPath)),
                    static_cast<quint16>(readInt(entity,
                                                 QStringLiteral("application"),
                                                 fallback.application,
                                                 0,
                                                 65535,
                                                 warnings,
                                                 entityPath)),
                    static_cast<quint16>(readInt(entity,
                                                 QStringLiteral("entity"),
                                                 fallback.entity,
                                                 0,
                                                 65535,
                                                 warnings,
                                                 entityPath))};
}

quint8 readReason(const QJsonObject &object,
                  const QString &key,
                  quint8 fallback,
                  QStringList *warnings,
                  const QString &path)
{
    if (!object.contains(key)) {
        return fallback;
    }

    const QJsonValue value = object.value(key);
    if (value.isString()) {
        bool ok = false;
        const int number = value.toString().toInt(&ok);
        if (ok && number >= 0 && number <= 255) {
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

    return static_cast<quint8>(readInt(object, key, fallback, 0, 255, warnings, path));
}

QStringList configSearchPaths()
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

AppConfig loadAppConfig(QStringList *warnings)
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

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        warnings->append(QStringLiteral("Could not parse %1: %2; using built-in defaults")
                             .arg(configPath, parseError.errorString()));
        return config;
    }

    config.configPath = configPath;
    const QJsonObject root = document.object();

    if (root.contains(QStringLiteral("theme"))) {
        Theme theme = config.theme;
        if (parseTheme(root.value(QStringLiteral("theme")), &theme)) {
            config.theme = theme;
        } else {
            warnings->append(QStringLiteral("theme must be \"dark\" or \"light\"; using dark"));
        }
    }

    const QJsonObject network = readObject(root, QStringLiteral("network"), warnings, QStringLiteral("config"));
    config.destinationAddress = readString(network,
                                           QStringLiteral("destinationAddress"),
                                           config.destinationAddress,
                                           warnings,
                                           QStringLiteral("config.network"));
    config.destinationPort = static_cast<quint16>(readInt(network,
                                                          QStringLiteral("destinationPort"),
                                                          config.destinationPort,
                                                          1,
                                                          65535,
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
                                                     65535,
                                                     warnings,
                                                     QStringLiteral("config.network")));

    const QJsonObject dis = readObject(root, QStringLiteral("dis"), warnings, QStringLiteral("config"));
    config.exerciseId = static_cast<quint8>(readInt(dis,
                                                    QStringLiteral("exerciseId"),
                                                    config.exerciseId,
                                                    1,
                                                    255,
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
    const QJsonObject startup = readObject(commands, QStringLiteral("startup"), warnings, QStringLiteral("config.commands"));
    config.startupActionId = static_cast<quint32>(readInt(startup,
                                                          QStringLiteral("actionId"),
                                                          static_cast<int>(config.startupActionId),
                                                          0,
                                                          2147483647,
                                                          warnings,
                                                          QStringLiteral("config.commands.startup")));
    const QJsonObject standby = readObject(commands, QStringLiteral("standby"), warnings, QStringLiteral("config.commands"));
    config.standbyReason =
        readReason(standby, QStringLiteral("reason"), config.standbyReason, warnings, QStringLiteral("config.commands.standby"));
    config.standbyFrozenBehavior = static_cast<quint8>(readInt(standby,
                                                               QStringLiteral("frozenBehavior"),
                                                               config.standbyFrozenBehavior,
                                                               0,
                                                               255,
                                                               warnings,
                                                               QStringLiteral("config.commands.standby")));
    const QJsonObject shutdown =
        readObject(commands, QStringLiteral("shutdown"), warnings, QStringLiteral("config.commands"));
    config.shutdownActionId = static_cast<quint32>(readInt(shutdown,
                                                           QStringLiteral("actionId"),
                                                           static_cast<int>(config.shutdownActionId),
                                                           0,
                                                           2147483647,
                                                           warnings,
                                                           QStringLiteral("config.commands.shutdown")));

    const QJsonObject testFederate = readObject(root,
                                                QStringLiteral("testFederate"),
                                                warnings,
                                                QStringLiteral("config"));
    config.testFederateEnabled = readBool(testFederate,
                                          QStringLiteral("enabled"),
                                          config.testFederateEnabled,
                                          warnings,
                                          QStringLiteral("config.testFederate"));

    return config;
}

} // namespace

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
    {
        appConfig_ = loadAppConfig(&configWarnings_);
        setWindowTitle(QStringLiteral("DIS6 Simulation Manager"));

        auto *central = new QWidget(this);
        auto *rootLayout = new QVBoxLayout(central);
        rootLayout->setSpacing(12);
        rootLayout->setContentsMargins(14, 14, 14, 10);

        auto *settingsLayout = new QHBoxLayout();
        settingsLayout->addWidget(new QLabel(QStringLiteral("Theme"), central));
        themeCombo_ = new QComboBox(central);
        themeCombo_->addItem(QStringLiteral("Dark"), static_cast<int>(Theme::Dark));
        themeCombo_->addItem(QStringLiteral("Light"), static_cast<int>(Theme::Light));
        themeCombo_->setFixedWidth(120);
        settingsLayout->addWidget(themeCombo_);
        settingsLayout->addStretch(1);

        auto *networkGroup = new QGroupBox(QStringLiteral("Network"), central);
        auto *networkLayout = new QFormLayout(networkGroup);
        destinationAddressEdit_ = new QLineEdit(appConfig_.destinationAddress, networkGroup);
        destinationPortSpin_ = makePortSpinBox(networkGroup, appConfig_.destinationPort);
        listenAddressEdit_ = new QLineEdit(appConfig_.listenAddress, networkGroup);
        listenPortSpin_ = makePortSpinBox(networkGroup, appConfig_.listenPort);
        networkLayout->addRow(QStringLiteral("Destination address"), destinationAddressEdit_);
        networkLayout->addRow(QStringLiteral("Destination port"), destinationPortSpin_);
        networkLayout->addRow(QStringLiteral("Listen address"), listenAddressEdit_);
        networkLayout->addRow(QStringLiteral("Listen port"), listenPortSpin_);

        auto *disGroup = new QGroupBox(QStringLiteral("DIS Identity"), central);
        auto *disLayout = new QGridLayout(disGroup);
        exerciseSpin_ = makeSmallSpinBox(disGroup, 1, 255, appConfig_.exerciseId);
        managerSiteSpin_ = makeSmallSpinBox(disGroup, 0, 65535, appConfig_.managerId.site);
        managerApplicationSpin_ = makeSmallSpinBox(disGroup, 0, 65535, appConfig_.managerId.application);
        managerEntitySpin_ = makeSmallSpinBox(disGroup, 0, 65535, appConfig_.managerId.entity);
        targetSiteSpin_ = makeSmallSpinBox(disGroup, 0, 65535, appConfig_.targetId.site);
        targetApplicationSpin_ = makeSmallSpinBox(disGroup, 0, 65535, appConfig_.targetId.application);
        targetEntitySpin_ = makeSmallSpinBox(disGroup, 0, 65535, appConfig_.targetId.entity);
        startupActionSpin_ = makeActionSpinBox(disGroup, appConfig_.startupActionId);
        shutdownActionSpin_ = makeActionSpinBox(disGroup, appConfig_.shutdownActionId);
        standbyReasonCombo_ = makeStandbyReasonCombo(disGroup, appConfig_.standbyReason);
        standbyFrozenBehaviorSpin_ = makeSmallSpinBox(disGroup, 0, 255, appConfig_.standbyFrozenBehavior);

        disLayout->addWidget(new QLabel(QStringLiteral("Exercise")), 0, 0);
        disLayout->addWidget(exerciseSpin_, 0, 1);
        disLayout->addWidget(new QLabel(QStringLiteral("Manager site/app/entity")), 1, 0);
        disLayout->addWidget(managerSiteSpin_, 1, 1);
        disLayout->addWidget(managerApplicationSpin_, 1, 2);
        disLayout->addWidget(managerEntitySpin_, 1, 3);
        disLayout->addWidget(new QLabel(QStringLiteral("Target site/app/entity")), 2, 0);
        disLayout->addWidget(targetSiteSpin_, 2, 1);
        disLayout->addWidget(targetApplicationSpin_, 2, 2);
        disLayout->addWidget(targetEntitySpin_, 2, 3);
        disLayout->addWidget(new QLabel(QStringLiteral("Startup/shutdown action ID")), 3, 0);
        disLayout->addWidget(startupActionSpin_, 3, 1);
        disLayout->addWidget(shutdownActionSpin_, 3, 2);
        disLayout->addWidget(new QLabel(QStringLiteral("Standby reason/frozen behavior")), 4, 0);
        disLayout->addWidget(standbyReasonCombo_, 4, 1, 1, 2);
        disLayout->addWidget(standbyFrozenBehaviorSpin_, 4, 3);
        disLayout->setColumnStretch(4, 1);

        auto *stateGroup = new QGroupBox(QStringLiteral("State Commands"), central);
        auto *stateLayout = new QHBoxLayout(stateGroup);
        addStateButton(stateLayout, QStringLiteral("Startup"), SimulationState::Startup);
        addStateButton(stateLayout, QStringLiteral("Standby"), SimulationState::Standby);
        addStateButton(stateLayout, QStringLiteral("Operate"), SimulationState::Operate);
        addStateButton(stateLayout, QStringLiteral("Shutdown"), SimulationState::Shutdown);

        auto *testGroup = new QGroupBox(QStringLiteral("Test Federate"), central);
        auto *testLayout = new QHBoxLayout(testGroup);
        dummyFederateStatusLabel_ = new QLabel(QStringLiteral("Configured: enabled, waiting to bind"), testGroup);
        testLayout->addWidget(dummyFederateStatusLabel_);
        testLayout->addStretch(1);
        testGroup->setVisible(appConfig_.testFederateEnabled);

        responseTable_ = new QTableWidget(0, 5, central);
        responseTable_->setHorizontalHeaderLabels(
            {QStringLiteral("Time"), QStringLiteral("Peer"), QStringLiteral("PDU"),
             QStringLiteral("Request"), QStringLiteral("Summary")});
        responseTable_->horizontalHeader()->setStretchLastSection(true);
        responseTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        responseTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        responseTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        responseTable_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        responseTable_->verticalHeader()->setVisible(false);
        responseTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        responseTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        responseTable_->setAlternatingRowColors(true);

        log_ = new QPlainTextEdit(central);
        log_->setReadOnly(true);
        log_->setMaximumBlockCount(1000);

        rootLayout->addLayout(settingsLayout);
        rootLayout->addWidget(networkGroup);
        rootLayout->addWidget(disGroup);
        rootLayout->addWidget(stateGroup);
        if (appConfig_.testFederateEnabled) {
            rootLayout->addWidget(testGroup);
        }
        rootLayout->addWidget(new QLabel(QStringLiteral("Received component responses"), central));
        rootLayout->addWidget(responseTable_, 1);
        rootLayout->addWidget(log_, 1);
        setCentralWidget(central);
        resize(980, 720);

        socket_ = new QUdpSocket(this);
        socket_->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);
        connect(socket_, &QUdpSocket::readyRead, this, &MainWindow::readDatagrams);
        dummyFederateSocket_ = new QUdpSocket(this);
        connect(dummyFederateSocket_, &QUdpSocket::readyRead, this, &MainWindow::readDummyFederateDatagrams);
        connect(themeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] {
            applyTheme(static_cast<Theme>(themeCombo_->currentData().toInt()));
        });
        applyTheme(appConfig_.theme);

        if (!appConfig_.configPath.isEmpty()) {
            appendLog(QStringLiteral("Loaded defaults from %1").arg(appConfig_.configPath));
        }
        for (const QString &warning : configWarnings_) {
            appendLog(QStringLiteral("Config: %1").arg(warning));
        }

        bindListenSocket();
        dummyFederateEnabled_ = appConfig_.testFederateEnabled;
        if (dummyFederateEnabled_) {
            appendLog(QStringLiteral("Dummy simulation federate enabled from config"));
            bindDummyFederateSocket();
        }
        auto *rebindTimer = new QTimer(this);
        rebindTimer->setInterval(1000);
        connect(rebindTimer, &QTimer::timeout, this, [this] {
            bindListenSocket();
            bindDummyFederateSocket();
        });
        rebindTimer->start();
    }

private:
    static QSpinBox *makePortSpinBox(QWidget *parent, int value)
    {
        auto *spinBox = new QSpinBox(parent);
        spinBox->setRange(1, 65535);
        spinBox->setValue(value);
        return spinBox;
    }

    static QSpinBox *makeSmallSpinBox(QWidget *parent, int minimum, int maximum, int value)
    {
        auto *spinBox = new QSpinBox(parent);
        spinBox->setRange(minimum, maximum);
        spinBox->setValue(value);
        return spinBox;
    }

    static QSpinBox *makeActionSpinBox(QWidget *parent, int value)
    {
        auto *spinBox = new QSpinBox(parent);
        spinBox->setRange(0, 2147483647);
        spinBox->setValue(value);
        return spinBox;
    }

    static QComboBox *makeStandbyReasonCombo(QWidget *parent, quint8 value)
    {
        auto *comboBox = new QComboBox(parent);
        for (const auto &option : StopFreezeReasonOptions) {
            comboBox->addItem(QStringLiteral("%1 (%2)")
                                  .arg(QString::fromLatin1(option.label))
                                  .arg(option.value),
                              option.value);
        }

        const int index = comboBox->findData(value);
        if (index >= 0) {
            comboBox->setCurrentIndex(index);
        }
        return comboBox;
    }

    static bool parseAddressText(const QString &text, QHostAddress *address)
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

    void addStateButton(QHBoxLayout *layout, const QString &label, SimulationState state)
    {
        auto *button = new QPushButton(label, this);
        button->setMinimumHeight(36);
        connect(button, &QPushButton::clicked, this, [this, state] { sendStateCommand(state); });
        layout->addWidget(button);
    }

    DisConfig currentConfig(bool *ok = nullptr) const
    {
        DisConfig config;
        const bool destinationOk = parseAddressText(destinationAddressEdit_->text(), &config.destinationAddress);
        const bool listenOk = parseAddressText(listenAddressEdit_->text(), &config.listenAddress);
        config.destinationPort = static_cast<quint16>(destinationPortSpin_->value());
        config.listenPort = static_cast<quint16>(listenPortSpin_->value());
        config.exerciseId = static_cast<quint8>(exerciseSpin_->value());
        config.managerId = makeEntityId(managerSiteSpin_, managerApplicationSpin_, managerEntitySpin_);
        config.targetId = makeEntityId(targetSiteSpin_, targetApplicationSpin_, targetEntitySpin_);
        config.startupActionId = static_cast<quint32>(startupActionSpin_->value());
        config.shutdownActionId = static_cast<quint32>(shutdownActionSpin_->value());
        config.standbyReason = static_cast<quint8>(standbyReasonCombo_->currentData().toUInt());
        config.standbyFrozenBehavior = static_cast<quint8>(standbyFrozenBehaviorSpin_->value());

        if (ok != nullptr) {
            *ok = destinationOk && listenOk;
        }
        return config;
    }

    static EntityId makeEntityId(const QSpinBox *site, const QSpinBox *application, const QSpinBox *entity)
    {
        return EntityId{static_cast<quint16>(site->value()),
                        static_cast<quint16>(application->value()),
                        static_cast<quint16>(entity->value())};
    }

    void bindListenSocket()
    {
        QHostAddress listenAddress;
        if (!parseAddressText(listenAddressEdit_->text(), &listenAddress)) {
            statusBar()->showMessage(QStringLiteral("Invalid listen address"));
            return;
        }

        const quint16 listenPort = static_cast<quint16>(listenPortSpin_->value());
        const QString listeningMessage =
            QStringLiteral("Listening on %1:%2").arg(listenAddress.toString()).arg(listenPort);
        if (socket_->state() == QAbstractSocket::BoundState
            && boundAddress_ == listenAddress
            && boundPort_ == listenPort) {
            if (statusBar()->currentMessage().startsWith(QStringLiteral("Invalid "))) {
                statusBar()->showMessage(listeningMessage);
            }
            return;
        }

        socket_->close();
        const bool bound = socket_->bind(listenAddress,
                                         listenPort,
                                         QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
        if (!bound) {
            statusBar()->showMessage(QStringLiteral("Listen bind failed: %1").arg(socket_->errorString()));
            return;
        }

        boundAddress_ = listenAddress;
        boundPort_ = listenPort;
        statusBar()->showMessage(listeningMessage);
    }

    void bindDummyFederateSocket()
    {
        if (!dummyFederateEnabled_) {
            return;
        }

        bool ok = false;
        const auto config = currentConfig(&ok);
        if (!ok) {
            statusBar()->showMessage(QStringLiteral("Invalid network address"));
            if (dummyFederateStatusLabel_ != nullptr) {
                dummyFederateStatusLabel_->setText(QStringLiteral("Configured: enabled, waiting for valid network settings"));
            }
            return;
        }

        if (dummyFederateSocket_->state() == QAbstractSocket::BoundState
            && dummyFederateBoundAddress_ == config.destinationAddress
            && dummyFederateBoundPort_ == config.destinationPort) {
            if (dummyFederateStatusLabel_ != nullptr) {
                dummyFederateStatusLabel_->setText(QStringLiteral("Running on %1:%2")
                                                       .arg(config.destinationAddress.toString())
                                                       .arg(config.destinationPort));
            }
            return;
        }

        dummyFederateSocket_->close();
        const QHostAddress bindAddress =
            config.destinationAddress.isMulticast() ? QHostAddress::AnyIPv4 : config.destinationAddress;
        const bool bound = dummyFederateSocket_->bind(
            bindAddress, config.destinationPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
        if (!bound) {
            appendLog(QStringLiteral("Dummy federate bind failed on %1:%2: %3")
                          .arg(bindAddress.toString())
                          .arg(config.destinationPort)
                          .arg(dummyFederateSocket_->errorString()));
            if (dummyFederateStatusLabel_ != nullptr) {
                dummyFederateStatusLabel_->setText(QStringLiteral("Bind failed on %1:%2")
                                                       .arg(bindAddress.toString())
                                                       .arg(config.destinationPort));
            }
            return;
        }

        if (config.destinationAddress.isMulticast()
            && !dummyFederateSocket_->joinMulticastGroup(config.destinationAddress)) {
            appendLog(QStringLiteral("Dummy federate multicast join failed for %1: %2")
                          .arg(config.destinationAddress.toString())
                          .arg(dummyFederateSocket_->errorString()));
        }

        dummyFederateSocket_->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);
        dummyFederateBoundAddress_ = config.destinationAddress;
        dummyFederateBoundPort_ = config.destinationPort;
        if (dummyFederateStatusLabel_ != nullptr) {
            dummyFederateStatusLabel_->setText(QStringLiteral("Running on %1:%2")
                                                   .arg(config.destinationAddress.toString())
                                                   .arg(config.destinationPort));
        }
        appendLog(QStringLiteral("Dummy federate listening on %1:%2")
                      .arg(config.destinationAddress.toString())
                      .arg(config.destinationPort));
    }

    void sendStateCommand(SimulationState state)
    {
        bool ok = false;
        const auto config = currentConfig(&ok);
        if (!ok) {
            QMessageBox::warning(this, QStringLiteral("Invalid Configuration"),
                                 QStringLiteral("Enter valid destination and listen IP addresses."));
            return;
        }

        bindListenSocket();
        const quint32 requestId = nextRequestId_++;
        QByteArray pdu;
        switch (state) {
        case SimulationState::Startup:
        case SimulationState::Shutdown:
            pdu = makeActionRequestPdu(config, requestId, state);
            break;
        case SimulationState::Standby:
            pdu = makeStopFreezePdu(config, requestId);
            break;
        case SimulationState::Operate:
            pdu = makeStartResumePdu(config, requestId);
            break;
        }

        const auto written = socket_->writeDatagram(pdu, config.destinationAddress, config.destinationPort);
        if (written != pdu.size()) {
            appendLog(QStringLiteral("Failed to send %1 request %2: %3")
                          .arg(stateName(state))
                          .arg(requestId)
                          .arg(socket_->errorString()));
            return;
        }

        requestStates_[requestId] = stateName(state);
        QString detail;
        if (state == SimulationState::Standby) {
            detail = QStringLiteral(", reason %1").arg(stopFreezeReasonLabel(config.standbyReason));
        }
        appendLog(QStringLiteral("Sent %1 request %2 to %3:%4 (%5 bytes%6)")
                      .arg(stateName(state))
                      .arg(requestId)
                      .arg(config.destinationAddress.toString())
                      .arg(config.destinationPort)
                      .arg(pdu.size())
                      .arg(detail));
    }

    void readDatagrams()
    {
        while (socket_->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(static_cast<int>(socket_->pendingDatagramSize()));
            QHostAddress sender;
            quint16 senderPort = 0;
            socket_->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            recordResponse(datagram, sender, senderPort);
        }
    }

    void readDummyFederateDatagrams()
    {
        while (dummyFederateSocket_->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(static_cast<int>(dummyFederateSocket_->pendingDatagramSize()));
            QHostAddress sender;
            quint16 senderPort = 0;
            dummyFederateSocket_->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            respondFromDummyFederate(datagram, sender, senderPort);
        }
    }

    void respondFromDummyFederate(const QByteArray &datagram, const QHostAddress &sender, quint16 senderPort)
    {
        if (datagram.size() < 28 || static_cast<quint8>(datagram[3]) != SimManagementFamily) {
            return;
        }

        const auto pduType = static_cast<PduType>(static_cast<quint8>(datagram[2]));
        if (pduType != StartResumePdu && pduType != StopFreezePdu && pduType != ActionRequestPdu) {
            return;
        }

        const quint32 requestId = requestIdFromResponse(datagram, static_cast<quint8>(pduType));
        DisConfig responseConfig;
        responseConfig.exerciseId = static_cast<quint8>(datagram[1]);
        responseConfig.managerId = readEntityId(datagram, 18);
        responseConfig.targetId = readEntityId(datagram, 12);

        const QByteArray response = pduType == ActionRequestPdu
            ? makeActionResponsePdu(responseConfig, requestId)
            : makeAcknowledgePdu(responseConfig, requestId, pduType);
        const auto written = dummyFederateSocket_->writeDatagram(response, sender, senderPort);
        if (written != response.size()) {
            appendLog(QStringLiteral("Dummy federate failed to acknowledge request %1: %2")
                          .arg(requestId)
                          .arg(dummyFederateSocket_->errorString()));
            return;
        }

        appendLog(QStringLiteral("Dummy federate accepted %1 request %2 from %3:%4")
                      .arg(pduTypeName(static_cast<quint8>(pduType)))
                      .arg(requestId)
                      .arg(sender.toString())
                      .arg(senderPort));
    }

    void recordResponse(const QByteArray &datagram, const QHostAddress &sender, quint16 senderPort)
    {
        const quint8 pduType = datagram.size() >= 3 ? static_cast<quint8>(datagram[2]) : 0;
        const quint32 requestId = requestIdFromResponse(datagram, pduType);
        const int row = responseTable_->rowCount();
        responseTable_->insertRow(row);
        responseTable_->setItem(row, 0, new QTableWidgetItem(QTime::currentTime().toString("HH:mm:ss.zzz")));
        responseTable_->setItem(row, 1, new QTableWidgetItem(QStringLiteral("%1:%2").arg(sender.toString()).arg(senderPort)));
        responseTable_->setItem(row, 2, new QTableWidgetItem(pduTypeName(pduType)));
        responseTable_->setItem(row, 3, new QTableWidgetItem(requestId == 0 ? QString() : QString::number(requestId)));
        responseTable_->setItem(row, 4, new QTableWidgetItem(responseSummary(datagram)));
        responseTable_->scrollToBottom();

        QString matchedState;
        if (requestStates_.contains(requestId)) {
            matchedState = QStringLiteral(" for %1").arg(requestStates_.value(requestId));
        }

        appendLog(QStringLiteral("Received %1%2 from %3:%4")
                      .arg(pduTypeName(pduType))
                      .arg(matchedState)
                      .arg(sender.toString())
                      .arg(senderPort));
    }

    static quint32 requestIdFromResponse(const QByteArray &datagram, quint8 pduType)
    {
        if (pduType == AcknowledgePdu && datagram.size() >= 32) {
            return readU32(datagram, 28);
        }
        if (pduType == ActionResponsePdu && datagram.size() >= 28) {
            return readU32(datagram, 24);
        }
        if (pduType == ActionRequestPdu && datagram.size() >= 28) {
            return readU32(datagram, 24);
        }
        if ((pduType == StartResumePdu || pduType == StopFreezePdu) && datagram.size() >= 36) {
            return readU32(datagram, datagram.size() - 4);
        }
        return 0;
    }

    void appendLog(const QString &message)
    {
        log_->appendPlainText(QStringLiteral("[%1] %2")
                                  .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
                                  .arg(message));
    }

    void applyTheme(Theme theme)
    {
        QApplication::setPalette(themePalette(theme));
        qApp->setStyleSheet(themeStyleSheet(theme));

        const QSignalBlocker blocker(themeCombo_);
        const int index = themeCombo_->findData(static_cast<int>(theme));
        if (index >= 0) {
            themeCombo_->setCurrentIndex(index);
        }
    }

    QComboBox *themeCombo_ = nullptr;
    QLineEdit *destinationAddressEdit_ = nullptr;
    QSpinBox *destinationPortSpin_ = nullptr;
    QLineEdit *listenAddressEdit_ = nullptr;
    QSpinBox *listenPortSpin_ = nullptr;
    QSpinBox *exerciseSpin_ = nullptr;
    QSpinBox *managerSiteSpin_ = nullptr;
    QSpinBox *managerApplicationSpin_ = nullptr;
    QSpinBox *managerEntitySpin_ = nullptr;
    QSpinBox *targetSiteSpin_ = nullptr;
    QSpinBox *targetApplicationSpin_ = nullptr;
    QSpinBox *targetEntitySpin_ = nullptr;
    QSpinBox *startupActionSpin_ = nullptr;
    QSpinBox *shutdownActionSpin_ = nullptr;
    QComboBox *standbyReasonCombo_ = nullptr;
    QSpinBox *standbyFrozenBehaviorSpin_ = nullptr;
    QLabel *dummyFederateStatusLabel_ = nullptr;
    QTableWidget *responseTable_ = nullptr;
    QPlainTextEdit *log_ = nullptr;
    QUdpSocket *socket_ = nullptr;
    QUdpSocket *dummyFederateSocket_ = nullptr;
    QHostAddress boundAddress_;
    QHostAddress dummyFederateBoundAddress_;
    quint16 boundPort_ = 0;
    quint16 dummyFederateBoundPort_ = 0;
    quint32 nextRequestId_ = 1;
    bool dummyFederateEnabled_ = false;
    AppConfig appConfig_;
    QStringList configWarnings_;
    QMap<quint32, QString> requestStates_;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
