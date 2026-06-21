#include <DISPatch/MainWindow.h>

#include <DISPatch/Constants.h>
#include <DISPatch/DisProtocol.h>
#include <DISPatch/Theme.h>

#include <QtCore/QDateTime>
#include <QtCore/QSignalBlocker>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>

namespace dispatch {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    appConfig_ = loadAppConfig(&configWarnings_);
    setWindowTitle(QStringLiteral("DIS6 Simulation Manager"));

    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setSpacing(StandardSpacing);
    rootLayout->setContentsMargins(WindowMargin, WindowMargin, WindowMargin, WindowBottomMargin);

    auto *settingsLayout = new QHBoxLayout();
    settingsLayout->addWidget(new QLabel(QStringLiteral("Theme"), central));
    themeCombo_ = new QComboBox(central);
    themeCombo_->addItem(QStringLiteral("Dark"), static_cast<int>(Theme::Dark));
    themeCombo_->addItem(QStringLiteral("Light"), static_cast<int>(Theme::Light));
    themeCombo_->addItem(QStringLiteral("Gruvbox"), static_cast<int>(Theme::Gruvbox));
    themeCombo_->setFixedWidth(ThemeComboWidth);
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
    exerciseSpin_ = makeSmallSpinBox(disGroup, 1, MaxExerciseId, appConfig_.exerciseId);
    managerSiteSpin_ = makeSmallSpinBox(disGroup, 0, BroadcastEntityIdValue, appConfig_.managerId.site);
    managerApplicationSpin_ = makeSmallSpinBox(disGroup, 0, BroadcastEntityIdValue, appConfig_.managerId.application);
    managerEntitySpin_ = makeSmallSpinBox(disGroup, 0, BroadcastEntityIdValue, appConfig_.managerId.entity);
    targetSiteSpin_ = makeSmallSpinBox(disGroup, 0, BroadcastEntityIdValue, appConfig_.targetId.site);
    targetApplicationSpin_ = makeSmallSpinBox(disGroup, 0, BroadcastEntityIdValue, appConfig_.targetId.application);
    targetEntitySpin_ = makeSmallSpinBox(disGroup, 0, BroadcastEntityIdValue, appConfig_.targetId.entity);
    targetBroadcastCheck_ = new QCheckBox(QStringLiteral("Broadcast"), disGroup);

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
    disLayout->addWidget(targetBroadcastCheck_, 2, 4);
    disLayout->setColumnStretch(IdentityStretchColumn, 1);
    connect(targetBroadcastCheck_, &QCheckBox::toggled, this, &MainWindow::setTargetBroadcast);

    auto *stateGroup = new QGroupBox(QStringLiteral("State Commands"), central);
    auto *stateLayout = new QHBoxLayout(stateGroup);
    addStateButton(stateLayout, QStringLiteral("Startup"), SimulationState::Startup);
    addStateButton(stateLayout, QStringLiteral("Standby"), SimulationState::Standby);
    addStateButton(stateLayout, QStringLiteral("Operate"), SimulationState::Operate);
    addStateButton(stateLayout, QStringLiteral("Shutdown"), SimulationState::Shutdown);

    auto *testGroup = new QGroupBox(QStringLiteral("Test Federate"), central);
    auto *testLayout = new QGridLayout(testGroup);
    dummyFederateStatusLabel_ = new QLabel(QStringLiteral("Configured: enabled, waiting to bind"), testGroup);
    dummyFederateStatusLabel_->setWordWrap(true);
    dummyFederateSiteSpin_ = makeSmallSpinBox(testGroup, 0, BroadcastEntityIdValue, appConfig_.testFederateId.site);
    dummyFederateApplicationSpin_ =
        makeSmallSpinBox(testGroup, 0, BroadcastEntityIdValue, appConfig_.testFederateId.application);
    dummyFederateEntitySpin_ = makeSmallSpinBox(testGroup, 0, BroadcastEntityIdValue, appConfig_.testFederateId.entity);
    testGroup->setMinimumWidth(TestFederateMinimumWidth);
    testLayout->addWidget(dummyFederateStatusLabel_, 0, 0, 1, 4);
    testLayout->addWidget(new QLabel(QStringLiteral("Entity ID")), 1, 0);
    testLayout->addWidget(dummyFederateSiteSpin_, 1, 1);
    testLayout->addWidget(dummyFederateApplicationSpin_, 1, 2);
    testLayout->addWidget(dummyFederateEntitySpin_, 1, 3);
    testLayout->setColumnStretch(TestFederateStretchColumn, 1);
    testGroup->setVisible(appConfig_.testFederateEnabled);

    auto *identityLayout = new QHBoxLayout();
    identityLayout->setSpacing(StandardSpacing);
    identityLayout->addWidget(disGroup, 2);
    if (appConfig_.testFederateEnabled) {
        identityLayout->addWidget(testGroup, 1);
    }

    responseTable_ = new QTableWidget(0, ResponseTableColumnCount, central);
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
    log_->setMaximumBlockCount(MaxLogBlocks);

    rootLayout->addLayout(settingsLayout);
    rootLayout->addWidget(networkGroup);
    rootLayout->addLayout(identityLayout);
    rootLayout->addWidget(stateGroup);
    rootLayout->addWidget(responseTable_, 1);
    rootLayout->addWidget(log_, 1);
    setCentralWidget(central);
    resize(DefaultWindowWidth, DefaultWindowHeight);

    socket_ = new QUdpSocket(this);
    socket_->setSocketOption(QAbstractSocket::MulticastLoopbackOption, appConfig_.multicastLoopback ? 1 : 0);
    connect(socket_, &QUdpSocket::readyRead, this, &MainWindow::readDatagrams);
    dummyFederateSocket_ = new QUdpSocket(this);
    connect(dummyFederateSocket_, &QUdpSocket::readyRead, this, &MainWindow::readDummyFederateDatagrams);
    connect(themeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() -> void {
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
    rebindTimer->setInterval(RebindIntervalMilliseconds);
    connect(rebindTimer, &QTimer::timeout, this, [this]() -> void {
        bindListenSocket();
        bindDummyFederateSocket();
    });
    rebindTimer->start();
}

auto MainWindow::makePortSpinBox(QWidget *parent, int value) -> QSpinBox *
{
    auto *spinBox = new QSpinBox(parent);
    spinBox->setRange(1, MaxUdpPort);
    spinBox->setValue(value);
    return spinBox;
}

auto MainWindow::makeSmallSpinBox(QWidget *parent, int minimum, int maximum, int value) -> QSpinBox *
{
    auto *spinBox = new QSpinBox(parent);
    spinBox->setRange(minimum, maximum);
    spinBox->setValue(value);
    return spinBox;
}

auto MainWindow::udpBindMode() const -> QUdpSocket::BindMode
{
    QUdpSocket::BindMode mode = QUdpSocket::DefaultForPlatform;
    if (appConfig_.shareAddress) {
        mode |= QUdpSocket::ShareAddress;
    }
    if (appConfig_.reuseAddress) {
        mode |= QUdpSocket::ReuseAddressHint;
    }
    return mode;
}

auto MainWindow::configuredMulticastGroup(QString *error) const -> QHostAddress
{
    QHostAddress group;
    if (!appConfig_.multicastGroupAddress.trimmed().isEmpty()) {
        if (!parseConfigAddress(appConfig_.multicastGroupAddress, &group)) {
            if (error != nullptr) {
                *error = QStringLiteral("Invalid multicast group address %1")
                             .arg(appConfig_.multicastGroupAddress);
            }
            return {};
        }
        if (!group.isMulticast()) {
            if (error != nullptr) {
                *error = QStringLiteral("Configured multicast group %1 is not multicast")
                             .arg(group.toString());
            }
            return {};
        }
        return group;
    }

    if (parseConfigAddress(destinationAddressEdit_->text(), &group) && group.isMulticast()) {
        return group;
    }

    return {};
}

auto MainWindow::configuredMulticastInterface(QString *error) const -> QNetworkInterface
{
    const QString interfaceName = appConfig_.multicastInterfaceName.trimmed();
    if (interfaceName.isEmpty()) {
        return {};
    }

    const QNetworkInterface interface = QNetworkInterface::interfaceFromName(interfaceName);
    if (interface.isValid()) {
        return interface;
    }

    if (error != nullptr) {
        *error = QStringLiteral("Unknown multicast interface %1").arg(interfaceName);
    }
    return {};
}

auto MainWindow::sameNetworkInterface(const QNetworkInterface &left, const QNetworkInterface &right) -> bool
{
    if (!left.isValid() && !right.isValid()) {
        return true;
    }
    return left.isValid() && right.isValid() && left.index() == right.index()
        && left.name() == right.name();
}

void MainWindow::clearListenMulticastGroup()
{
    if (!joinedListenMulticastGroup_.isNull()) {
        if (joinedListenMulticastInterface_.isValid()) {
            socket_->leaveMulticastGroup(joinedListenMulticastGroup_, joinedListenMulticastInterface_);
        } else {
            socket_->leaveMulticastGroup(joinedListenMulticastGroup_);
        }
        joinedListenMulticastGroup_ = QHostAddress();
        joinedListenMulticastInterface_ = QNetworkInterface();
    }
}

void MainWindow::clearDummyFederateMulticastGroup()
{
    if (!joinedDummyFederateMulticastGroup_.isNull()) {
        if (joinedDummyFederateMulticastInterface_.isValid()) {
            dummyFederateSocket_->leaveMulticastGroup(joinedDummyFederateMulticastGroup_,
                                                     joinedDummyFederateMulticastInterface_);
        } else {
            dummyFederateSocket_->leaveMulticastGroup(joinedDummyFederateMulticastGroup_);
        }
        joinedDummyFederateMulticastGroup_ = QHostAddress();
        joinedDummyFederateMulticastInterface_ = QNetworkInterface();
    }
}

auto MainWindow::updateListenMulticastGroup() -> bool
{
    if (!appConfig_.joinMulticast) {
        clearListenMulticastGroup();
        return true;
    }

    QString groupError;
    const QHostAddress group = configuredMulticastGroup(&groupError);
    if (!groupError.isEmpty()) {
        statusBar()->showMessage(groupError);
        return false;
    }
    if (group.isNull()) {
        clearListenMulticastGroup();
        return true;
    }

    QString interfaceError;
    const QNetworkInterface interface = configuredMulticastInterface(&interfaceError);
    if (!interfaceError.isEmpty()) {
        statusBar()->showMessage(interfaceError);
        return false;
    }

    if (joinedListenMulticastGroup_ == group
        && sameNetworkInterface(joinedListenMulticastInterface_, interface)) {
        return true;
    }

    clearListenMulticastGroup();
    const bool joined = interface.isValid() ? socket_->joinMulticastGroup(group, interface)
                                            : socket_->joinMulticastGroup(group);
    if (!joined) {
        statusBar()->showMessage(QStringLiteral("Listen multicast join failed for %1: %2")
                                     .arg(group.toString(), socket_->errorString()));
        return false;
    }

    joinedListenMulticastGroup_ = group;
    joinedListenMulticastInterface_ = interface;
    QString interfaceDetail;
    if (interface.isValid()) {
        interfaceDetail = QStringLiteral(" on %1").arg(interface.name());
    }
    appendLog(QStringLiteral("Listen socket joined multicast group %1%2")
                  .arg(group.toString(), interfaceDetail));
    return true;
}

void MainWindow::updateDummyFederateMulticastGroup(const QHostAddress &group)
{
    if (!appConfig_.joinMulticast || !group.isMulticast()) {
        clearDummyFederateMulticastGroup();
        return;
    }

    QString interfaceError;
    const QNetworkInterface interface = configuredMulticastInterface(&interfaceError);
    if (!interfaceError.isEmpty()) {
        appendLog(interfaceError);
        return;
    }

    if (joinedDummyFederateMulticastGroup_ == group
        && sameNetworkInterface(joinedDummyFederateMulticastInterface_, interface)) {
        return;
    }

    clearDummyFederateMulticastGroup();
    const bool joined = interface.isValid() ? dummyFederateSocket_->joinMulticastGroup(group, interface)
                                            : dummyFederateSocket_->joinMulticastGroup(group);
    if (!joined) {
        appendLog(QStringLiteral("Dummy federate multicast join failed for %1: %2")
                      .arg(group.toString(), dummyFederateSocket_->errorString()));
        return;
    }

    joinedDummyFederateMulticastGroup_ = group;
    joinedDummyFederateMulticastInterface_ = interface;
}

void MainWindow::addStateButton(QHBoxLayout *layout, const QString &label, SimulationState state)
{
    auto *button = new QPushButton(label, this);
    button->setMinimumHeight(StateButtonMinimumHeight);
    connect(button, &QPushButton::clicked, this, [this, state]() -> void { sendStateCommand(state); });
    layout->addWidget(button);
}

auto MainWindow::currentConfig(bool *configOk) const -> DisConfig
{
    DisConfig config;
    const bool destinationOk = parseConfigAddress(destinationAddressEdit_->text(), &config.destinationAddress);
    const bool listenOk = parseConfigAddress(listenAddressEdit_->text(), &config.listenAddress);
    config.destinationPort = static_cast<quint16>(destinationPortSpin_->value());
    config.listenPort = static_cast<quint16>(listenPortSpin_->value());
    config.exerciseId = static_cast<quint8>(exerciseSpin_->value());
    config.managerId = makeEntityId(managerSiteSpin_, managerApplicationSpin_, managerEntitySpin_);
    config.targetId = makeEntityId(targetSiteSpin_, targetApplicationSpin_, targetEntitySpin_);
    config.startupActionId = appConfig_.startupActionId;
    config.shutdownActionId = appConfig_.shutdownActionId;
    config.standbyReason = appConfig_.standbyReason;
    config.standbyFrozenBehavior = appConfig_.standbyFrozenBehavior;

    if (configOk != nullptr) {
        *configOk = destinationOk && listenOk;
    }
    return config;
}

auto MainWindow::currentTestFederateId() const -> EntityId
{
    return makeEntityId(dummyFederateSiteSpin_,
                        dummyFederateApplicationSpin_,
                        dummyFederateEntitySpin_);
}

auto MainWindow::currentTargetId() const -> EntityId
{
    return makeEntityId(targetSiteSpin_, targetApplicationSpin_, targetEntitySpin_);
}

auto MainWindow::makeEntityId(const QSpinBox *site, const QSpinBox *application, const QSpinBox *entity) -> EntityId
{
    return EntityId{static_cast<quint16>(site->value()),
                    static_cast<quint16>(application->value()),
                    static_cast<quint16>(entity->value())};
}

void MainWindow::setTargetIdControls(const EntityId &entityId)
{
    targetSiteSpin_->setValue(entityId.site);
    targetApplicationSpin_->setValue(entityId.application);
    targetEntitySpin_->setValue(entityId.entity);
}

void MainWindow::setTargetIdControlsEnabled(bool enabled)
{
    targetSiteSpin_->setEnabled(enabled);
    targetApplicationSpin_->setEnabled(enabled);
    targetEntitySpin_->setEnabled(enabled);
}

void MainWindow::setTargetBroadcast(bool enabled)
{
    if (enabled) {
        savedTargetIdBeforeBroadcast_ = currentTargetId();
        setTargetIdControls(EntityId{BroadcastEntityIdValue,
                                     BroadcastEntityIdValue,
                                     BroadcastEntityIdValue});
        setTargetIdControlsEnabled(false);
        return;
    }

    setTargetIdControlsEnabled(true);
    setTargetIdControls(savedTargetIdBeforeBroadcast_);
}

void MainWindow::bindListenSocket()
{
    QHostAddress listenAddress;
    if (!parseConfigAddress(listenAddressEdit_->text(), &listenAddress)) {
        statusBar()->showMessage(QStringLiteral("Invalid listen address"));
        return;
    }

    const auto listenPort = static_cast<quint16>(listenPortSpin_->value());
    const QString listeningMessage =
        QStringLiteral("Listening on %1:%2").arg(listenAddress.toString()).arg(listenPort);
    if (socket_->state() == QAbstractSocket::BoundState
        && boundAddress_ == listenAddress
        && boundPort_ == listenPort) {
        if (!updateListenMulticastGroup()) {
            return;
        }
        if (statusBar()->currentMessage().startsWith(QStringLiteral("Invalid "))) {
            statusBar()->showMessage(listeningMessage);
        }
        return;
    }

    socket_->close();
    joinedListenMulticastGroup_ = QHostAddress();
    const bool bound = socket_->bind(listenAddress, listenPort, udpBindMode());
    if (!bound) {
        statusBar()->showMessage(QStringLiteral("Listen bind failed: %1").arg(socket_->errorString()));
        return;
    }

    socket_->setSocketOption(QAbstractSocket::MulticastLoopbackOption, appConfig_.multicastLoopback ? 1 : 0);
    boundAddress_ = listenAddress;
    boundPort_ = listenPort;
    if (updateListenMulticastGroup()) {
        statusBar()->showMessage(listeningMessage);
    }
}

void MainWindow::bindDummyFederateSocket()
{
    if (!dummyFederateEnabled_) {
        return;
    }

    bool configOk = false;
    const auto config = currentConfig(&configOk);
    if (!configOk) {
        statusBar()->showMessage(QStringLiteral("Invalid network address"));
        if (dummyFederateStatusLabel_ != nullptr) {
            dummyFederateStatusLabel_->setText(QStringLiteral("Configured: enabled, waiting for valid network settings"));
        }
        return;
    }

    if (dummyFederateSocket_->state() == QAbstractSocket::BoundState
        && dummyFederateBoundAddress_ == config.destinationAddress
        && dummyFederateBoundPort_ == config.destinationPort) {
        updateDummyFederateMulticastGroup(config.destinationAddress);
        dummyFederateSocket_->setSocketOption(QAbstractSocket::MulticastLoopbackOption,
                                              appConfig_.multicastLoopback ? 1 : 0);
        if (dummyFederateStatusLabel_ != nullptr) {
            dummyFederateStatusLabel_->setText(QStringLiteral("Running on %1:%2 as %3")
                                                   .arg(config.destinationAddress.toString())
                                                   .arg(config.destinationPort)
                                                   .arg(entityIdString(currentTestFederateId())));
        }
        return;
    }

    dummyFederateSocket_->close();
    joinedDummyFederateMulticastGroup_ = QHostAddress();
    const QHostAddress bindAddress =
        config.destinationAddress.isMulticast() ? QHostAddress::AnyIPv4 : config.destinationAddress;
    const bool bound = dummyFederateSocket_->bind(bindAddress, config.destinationPort, udpBindMode());
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

    updateDummyFederateMulticastGroup(config.destinationAddress);
    dummyFederateSocket_->setSocketOption(QAbstractSocket::MulticastLoopbackOption,
                                          appConfig_.multicastLoopback ? 1 : 0);
    dummyFederateBoundAddress_ = config.destinationAddress;
    dummyFederateBoundPort_ = config.destinationPort;
    if (dummyFederateStatusLabel_ != nullptr) {
        dummyFederateStatusLabel_->setText(QStringLiteral("Running on %1:%2 as %3")
                                               .arg(config.destinationAddress.toString())
                                               .arg(config.destinationPort)
                                               .arg(entityIdString(currentTestFederateId())));
    }
    appendLog(QStringLiteral("Dummy federate listening on %1:%2")
                  .arg(config.destinationAddress.toString())
                  .arg(config.destinationPort));
}

void MainWindow::sendStateCommand(SimulationState state)
{
    bool configOk = false;
    const auto config = currentConfig(&configOk);
    if (!configOk) {
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

void MainWindow::readDatagrams()
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

void MainWindow::readDummyFederateDatagrams()
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

void MainWindow::respondFromDummyFederate(const QByteArray &datagram, const QHostAddress &sender, quint16 senderPort)
{
    if (datagram.size() < MinRequestPduLength || static_cast<quint8>(datagram[PduFamilyOffset]) != SimManagementFamily) {
        return;
    }

    const auto pduType = static_cast<PduType>(static_cast<quint8>(datagram[PduTypeOffset]));
    if (pduType != StartResumePdu && pduType != StopFreezePdu && pduType != ActionRequestPdu) {
        return;
    }

    const quint32 requestId = requestIdFromResponse(datagram, static_cast<quint8>(pduType));
    const EntityId receivingEntity = readEntityId(datagram, TargetEntityOffset);
    const EntityId federateId = currentTestFederateId();
    if (!entityIdsMatch(receivingEntity, federateId)) {
        appendLog(QStringLiteral("Dummy federate ignored %1 request %2 for entity %3; configured as %4")
                      .arg(pduTypeName(static_cast<quint8>(pduType)))
                      .arg(requestId)
                      .arg(entityIdString(receivingEntity))
                      .arg(entityIdString(federateId)));
        return;
    }

    DisConfig responseConfig;
    responseConfig.exerciseId = static_cast<quint8>(datagram[PduExerciseIdOffset]);
    responseConfig.managerId = receivingEntity;
    responseConfig.targetId = readEntityId(datagram, OriginEntityOffset);

    const QByteArray response = pduType == ActionRequestPdu
        ? makeActionResponsePdu(responseConfig, requestId)
        : makeAcknowledgePdu(responseConfig, requestId, pduType);
    bool configOk = false;
    const auto config = currentConfig(&configOk);
    const QHostAddress responseAddress =
        configOk && config.destinationAddress.isMulticast() ? config.destinationAddress : sender;
    const quint16 responsePort =
        configOk && config.destinationAddress.isMulticast() ? config.destinationPort : senderPort;
    const auto written = dummyFederateSocket_->writeDatagram(response, responseAddress, responsePort);
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
    appendLog(QStringLiteral("Dummy federate sent %1 response %2 to %3:%4")
                  .arg(pduType == ActionRequestPdu ? QStringLiteral("Action") : QStringLiteral("Acknowledge"))
                  .arg(requestId)
                  .arg(responseAddress.toString())
                  .arg(responsePort));
}

void MainWindow::recordResponse(const QByteArray &datagram, const QHostAddress &sender, quint16 senderPort)
{
    const quint8 pduType =
        datagram.size() > PduTypeOffset ? static_cast<quint8>(datagram[PduTypeOffset]) : 0;
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

void MainWindow::appendLog(const QString &message)
{
    log_->appendPlainText(QStringLiteral("[%1] %2")
                              .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
                              .arg(message));
}

void MainWindow::applyTheme(Theme theme)
{
    QApplication::setPalette(themePalette(theme));
    qApp->setStyleSheet(themeStyleSheet(theme));

    const QSignalBlocker blocker(themeCombo_);
    const int index = themeCombo_->findData(static_cast<int>(theme));
    if (index >= 0) {
        themeCombo_->setCurrentIndex(index);
    }
}

} // namespace dispatch
