#pragma once

#include <DISPatch/AppConfig.h>
#include <DISPatch/DisTypes.h>

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
class QHBoxLayout;
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
    static auto makePortSpinBox(QWidget *parent, int value) -> QSpinBox *;
    static auto makeSmallSpinBox(QWidget *parent, int minimum, int maximum, int value) -> QSpinBox *;
    [[nodiscard]] auto udpBindMode() const -> QUdpSocket::BindMode;
    auto configuredMulticastGroup(QString *error = nullptr) const -> QHostAddress;
    auto configuredMulticastInterface(QString *error = nullptr) const -> QNetworkInterface;
    static auto sameNetworkInterface(const QNetworkInterface &left, const QNetworkInterface &right) -> bool;
    void clearListenMulticastGroup();
    void clearDummyFederateMulticastGroup();
    auto updateListenMulticastGroup() -> bool;
    void updateDummyFederateMulticastGroup(const QHostAddress &group);
    void addStateButton(QHBoxLayout *layout, const QString &label, SimulationState state);
    auto currentConfig(bool *configOk = nullptr) const -> DisConfig;
    [[nodiscard]] auto currentTestFederateId() const -> EntityId;
    [[nodiscard]] auto currentTargetId() const -> EntityId;
    static auto makeEntityId(const QSpinBox *site, const QSpinBox *application, const QSpinBox *entity) -> EntityId;
    void setTargetIdControls(const EntityId &entityId);
    void setTargetIdControlsEnabled(bool enabled);
    void setTargetBroadcast(bool enabled);
    void bindListenSocket();
    void bindDummyFederateSocket();
    void sendStateCommand(SimulationState state);
    void readDatagrams();
    void readDummyFederateDatagrams();
    void respondFromDummyFederate(const QByteArray &datagram, const QHostAddress &sender, quint16 senderPort);
    void recordResponse(const QByteArray &datagram, const QHostAddress &sender, quint16 senderPort);
    void appendLog(const QString &message);
    void applyTheme(Theme theme);

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
    QCheckBox *targetBroadcastCheck_ = nullptr;
    EntityId savedTargetIdBeforeBroadcast_;
    QLabel *dummyFederateStatusLabel_ = nullptr;
    QSpinBox *dummyFederateSiteSpin_ = nullptr;
    QSpinBox *dummyFederateApplicationSpin_ = nullptr;
    QSpinBox *dummyFederateEntitySpin_ = nullptr;
    QTableWidget *responseTable_ = nullptr;
    QPlainTextEdit *log_ = nullptr;
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
    AppConfig appConfig_;
    QStringList configWarnings_;
    QMap<quint32, QString> requestStates_;
};

} // namespace dispatch
