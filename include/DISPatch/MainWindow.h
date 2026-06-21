#pragma once

#include <DISPatch/AppConfig.h>
#include <DISPatch/DisTypes.h>

#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QtGlobal>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QUdpSocket>
#include <QtWidgets/QMainWindow>

class QCheckBox;
class QComboBox;
class QGridLayout;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;
class QTableWidget;
class QWidget;

namespace dispatch {

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    enum class DestinationMode : quint8 {
        Normal,
        Broadcast,
        Localhost
    };

    static auto makePortSpinBox(QWidget *parent, int value) -> QSpinBox *;
    static auto makeSmallSpinBox(QWidget *parent, int minimum, int maximum, int value) -> QSpinBox *;
    [[nodiscard]] auto udpBindMode() const -> QUdpSocket::BindMode;
    void populateNetworkInterfaces();
    auto autoSelectedNetworkInterface() const -> QNetworkInterface;
    auto selectedNetworkInterface() const -> QNetworkInterface;
    static auto interfaceLabel(const QNetworkInterface &networkInterface) -> QString;
    static auto primaryIpv4Address(const QNetworkInterface &networkInterface) -> QHostAddress;
    static auto interfaceForAddress(const QHostAddress &address) -> QNetworkInterface;
    auto effectiveListenAddress(const QHostAddress &listenAddress, const QHostAddress &destinationAddress) const -> QHostAddress;
    auto dummyFederateBindAddress(const QHostAddress &destinationAddress) const -> QHostAddress;
    void updateSocketOptions(QUdpSocket *socket, const QHostAddress &destinationAddress) const;
    auto configuredMulticastGroup(QString *error = nullptr) const -> QHostAddress;
    auto configuredMulticastInterface(QString *error = nullptr) const -> QNetworkInterface;
    static auto sameNetworkInterface(const QNetworkInterface &left, const QNetworkInterface &right) -> bool;
    void clearListenMulticastGroup();
    void clearDummyFederateMulticastGroup();
    auto updateListenMulticastGroup() -> bool;
    void updateDummyFederateMulticastGroup(const QHostAddress &group);
    void addStateButton(QGridLayout *layout, const QString &label, SimulationState state, int row, int column);
    auto currentConfig(bool *configOk = nullptr) const -> DisConfig;
    [[nodiscard]] auto currentTestFederateId() const -> EntityId;
    [[nodiscard]] auto currentTargetId() const -> EntityId;
    static auto makeEntityId(const QSpinBox *site, const QSpinBox *application, const QSpinBox *entity) -> EntityId;
    void setTargetIdControls(const EntityId &entityId);
    void setTargetIdControlsEnabled(bool enabled);
    void setTargetBroadcast(bool enabled);
    void setDestinationMode(DestinationMode mode);
    void bindListenSocket();
    void bindDummyFederateSocket();
    void sendStateCommand(SimulationState state);
    void readDatagrams();
    void readDummyFederateDatagrams();
    void respondFromDummyFederate(const QByteArray &datagram, const QHostAddress &sender, quint16 senderPort);
    void appendMessageRow(const QByteArray &datagram,
                          const QHostAddress &peer,
                          quint16 peerPort,
                          const QString &direction);
    void recordResponse(const QByteArray &datagram, const QHostAddress &sender, quint16 senderPort);
    void appendLog(const QString &message, LogLevel level = LogLevel::Debug);
    static auto shouldLog(LogLevel messageLevel, LogLevel configuredLevel) -> bool;
    static auto logLevelLabel(LogLevel level) -> QString;
    void setupLogFiles();
    auto configuredLogPath(const QString &path) const -> QString;
    void writeLogFileLine(QFile *file, const QString &line);
    void clearMessageLog();
    void clearEventLog();
    void applyTheme(Theme theme);

    QComboBox *themeCombo_ = nullptr;
    QComboBox *networkInterfaceCombo_ = nullptr;
    QLineEdit *destinationAddressEdit_ = nullptr;
    QSpinBox *destinationPortSpin_ = nullptr;
    QLineEdit *listenAddressEdit_ = nullptr;
    QSpinBox *listenPortSpin_ = nullptr;
    QCheckBox *destinationBroadcastCheck_ = nullptr;
    QCheckBox *destinationLocalhostCheck_ = nullptr;
    QSpinBox *exerciseSpin_ = nullptr;
    QSpinBox *managerSiteSpin_ = nullptr;
    QSpinBox *managerApplicationSpin_ = nullptr;
    QSpinBox *managerEntitySpin_ = nullptr;
    QSpinBox *targetSiteSpin_ = nullptr;
    QSpinBox *targetApplicationSpin_ = nullptr;
    QSpinBox *targetEntitySpin_ = nullptr;
    QCheckBox *targetBroadcastCheck_ = nullptr;
    EntityId savedTargetIdBeforeBroadcast_;
    QLabel *dummyFederateStatusLabel_ = nullptr;
    QSpinBox *dummyFederateSiteSpin_ = nullptr;
    QSpinBox *dummyFederateApplicationSpin_ = nullptr;
    QSpinBox *dummyFederateEntitySpin_ = nullptr;
    QTableWidget *responseTable_ = nullptr;
    QPlainTextEdit *log_ = nullptr;
    QFile logFile_;
    QFile messageLogFile_;
    QUdpSocket *socket_ = nullptr;
    QUdpSocket *dummyFederateSocket_ = nullptr;
    QHostAddress boundAddress_;
    QHostAddress dummyFederateBoundAddress_;
    QHostAddress joinedListenMulticastGroup_;
    QHostAddress joinedDummyFederateMulticastGroup_;
    QNetworkInterface joinedListenMulticastInterface_;
    QNetworkInterface joinedDummyFederateMulticastInterface_;
    quint16 boundPort_ = 0;
    quint16 dummyFederateBoundPort_ = 0;
    quint32 nextRequestId_ = 1;
    bool dummyFederateEnabled_ = false;
    DestinationMode destinationMode_ = DestinationMode::Normal;
    QString savedDestinationAddressBeforeMode_;
    QString savedInterfaceNameBeforeMode_;
    AppConfig appConfig_;
    QStringList configWarnings_;
    QMap<quint32, QString> requestStates_;
};

} // namespace dispatch
