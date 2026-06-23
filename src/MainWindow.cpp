#include <DISPatch/MainWindow.h>

#include <DISPatch/Constants.h>
#include <DISPatch/DisProtocol.h>
#include <DISPatch/Theme.h>

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSignalBlocker>
#include <QtCore/QTextStream>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QTextCharFormat>
#include <QtGui/QTextCursor>
#include <QtNetwork/QNetworkAddressEntry>
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
    networkInterfaceCombo_ = new QComboBox(networkGroup);
    populateNetworkInterfaces();
    auto *destinationModeLayout = new QHBoxLayout();
    destinationBroadcastCheck_ = new QCheckBox(QStringLiteral("Broadcast"), networkGroup);
    destinationLocalhostCheck_ = new QCheckBox(QStringLiteral("Localhost"), networkGroup);
    destinationModeLayout->addWidget(destinationBroadcastCheck_);
    destinationModeLayout->addWidget(destinationLocalhostCheck_);
    destinationModeLayout->addStretch(1);
    networkLayout->addRow(QStringLiteral("Destination address"), destinationAddressEdit_);
    networkLayout->addRow(QStringLiteral("Destination port"), destinationPortSpin_);
    networkLayout->addRow(QStringLiteral("Listen address"), listenAddressEdit_);
    networkLayout->addRow(QStringLiteral("Listen port"), listenPortSpin_);
    networkLayout->addRow(QStringLiteral("Interface"), networkInterfaceCombo_);
    networkLayout->addRow(QStringLiteral("Destination mode"), destinationModeLayout);
    connect(destinationBroadcastCheck_, &QCheckBox::toggled, this, [this](bool enabled) -> void {
        if (enabled) {
            setDestinationMode(DestinationMode::Broadcast);
        } else if (destinationMode_ == DestinationMode::Broadcast) {
            setDestinationMode(DestinationMode::Normal);
        }
    });
    connect(destinationLocalhostCheck_, &QCheckBox::toggled, this, [this](bool enabled) -> void {
        if (enabled) {
            setDestinationMode(DestinationMode::Localhost);
        } else if (destinationMode_ == DestinationMode::Localhost) {
            setDestinationMode(DestinationMode::Normal);
        }
    });
    connect(networkInterfaceCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() -> void {
        bindListenSocket();
        bindDummyFederateSocket();
    });
    QHostAddress configuredDestination;
    if (parseConfigAddress(appConfig_.destinationAddress, &configuredDestination)) {
        if (isBroadcastAddress(configuredDestination)) {
            setDestinationMode(DestinationMode::Broadcast);
        } else if (configuredDestination == QHostAddress(QHostAddress::LocalHost)) {
            setDestinationMode(DestinationMode::Localhost);
        }
    }

    auto *stateGroup = new QGroupBox(QStringLiteral("Simulation Commands"), central);
    auto *stateLayout = new QGridLayout(stateGroup);
    addStateButton(stateLayout, QStringLiteral("Initialize"), SimulationCommand::Initialize, 0, 0);
    addStateButton(stateLayout, QStringLiteral("Start"), SimulationCommand::Start, 0, 1);
    addStateButton(stateLayout, QStringLiteral("Pause"), SimulationCommand::Pause, 0, 2);
    addStateButton(stateLayout, QStringLiteral("Stop"), SimulationCommand::Stop, 1, 0);
    addStateButton(stateLayout, QStringLiteral("Reset"), SimulationCommand::Reset, 1, 1);

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
        {QStringLiteral("Time"), QStringLiteral("Dir"), QStringLiteral("Peer"), QStringLiteral("PDU"),
         QStringLiteral("Request"), QStringLiteral("Summary")});
    responseTable_->horizontalHeader()->setStretchLastSection(true);
    responseTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    responseTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    responseTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    responseTable_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    responseTable_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    responseTable_->verticalHeader()->setVisible(false);
    responseTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    responseTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    responseTable_->setAlternatingRowColors(true);

    log_ = new QPlainTextEdit(central);
    log_->setReadOnly(true);
    log_->setMaximumBlockCount(MaxLogBlocks);
    setupLogFiles();

    auto *logControlsLayout = new QHBoxLayout();
    auto *clearMessagesButton = new QPushButton(QStringLiteral("Clear Messages"), central);
    auto *clearLogButton = new QPushButton(QStringLiteral("Clear Log"), central);
    connect(clearMessagesButton, &QPushButton::clicked, this, &MainWindow::clearMessageLog);
    connect(clearLogButton, &QPushButton::clicked, this, &MainWindow::clearEventLog);
    logControlsLayout->addStretch(1);
    logControlsLayout->addWidget(clearMessagesButton);
    logControlsLayout->addWidget(clearLogButton);

    auto *networkCommandLayout = new QHBoxLayout();
    networkCommandLayout->setSpacing(StandardSpacing);
    networkCommandLayout->addWidget(networkGroup, 3);
    networkCommandLayout->addWidget(stateGroup, 1);

    rootLayout->addLayout(settingsLayout);
    rootLayout->addLayout(networkCommandLayout);
    rootLayout->addLayout(identityLayout);
    rootLayout->addLayout(logControlsLayout);
    rootLayout->addWidget(responseTable_, 1);
    rootLayout->addWidget(log_, 1);
    setCentralWidget(central);
    resize(DefaultWindowWidth, DefaultWindowHeight);

    socket_ = new QUdpSocket(this);
    updateSocketOptions(socket_, QHostAddress(destinationAddressEdit_->text()));
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
        appendLog(QStringLiteral("Config: %1").arg(warning), LogLevel::Warn);
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
    if (appConfig_.shareAddress || destinationMode_ == DestinationMode::Broadcast) {
        mode |= QUdpSocket::ShareAddress;
    }
    if (appConfig_.reuseAddress || destinationMode_ == DestinationMode::Broadcast) {
        mode |= QUdpSocket::ReuseAddressHint;
    }
    return mode;
}

void MainWindow::populateNetworkInterfaces()
{
    const QNetworkInterface selectedInterface =
        appConfig_.interfaceName.trimmed().isEmpty()
            ? autoSelectedNetworkInterface()
            : QNetworkInterface::interfaceFromName(appConfig_.interfaceName.trimmed());

    networkInterfaceCombo_->clear();
    for (const QNetworkInterface &networkInterface : QNetworkInterface::allInterfaces()) {
        if (!networkInterface.isValid() || primaryIpv4Address(networkInterface).isNull()) {
            continue;
        }

        networkInterfaceCombo_->addItem(interfaceLabel(networkInterface), networkInterface.name());
    }

    const int selectedIndex = networkInterfaceCombo_->findData(selectedInterface.name());
    if (selectedIndex >= 0) {
        networkInterfaceCombo_->setCurrentIndex(selectedIndex);
    }
}

auto MainWindow::autoSelectedNetworkInterface() const -> QNetworkInterface
{
    QHostAddress destinationAddress;
    if (parseConfigAddress(destinationAddressEdit_->text(), &destinationAddress)
        && destinationAddress == QHostAddress(QHostAddress::LocalHost)) {
        for (const QNetworkInterface &networkInterface : QNetworkInterface::allInterfaces()) {
            if (networkInterface.flags().testFlag(QNetworkInterface::IsLoopBack)
                && !primaryIpv4Address(networkInterface).isNull()) {
                return networkInterface;
            }
        }
    }

    QHostAddress listenAddress;
    if (parseConfigAddress(listenAddressEdit_->text(), &listenAddress) && !isAnyAddress(listenAddress)) {
        const QNetworkInterface networkInterface = interfaceForAddress(listenAddress);
        if (networkInterface.isValid()) {
            return networkInterface;
        }
    }

    for (const QNetworkInterface &networkInterface : QNetworkInterface::allInterfaces()) {
        const QNetworkInterface::InterfaceFlags flags = networkInterface.flags();
        if (flags.testFlag(QNetworkInterface::IsUp)
            && flags.testFlag(QNetworkInterface::IsRunning)
            && !flags.testFlag(QNetworkInterface::IsLoopBack)
            && !primaryIpv4Address(networkInterface).isNull()) {
            return networkInterface;
        }
    }

    for (const QNetworkInterface &networkInterface : QNetworkInterface::allInterfaces()) {
        if (networkInterface.flags().testFlag(QNetworkInterface::IsLoopBack)
            && !primaryIpv4Address(networkInterface).isNull()) {
            return networkInterface;
        }
    }

    return {};
}

auto MainWindow::selectedNetworkInterface() const -> QNetworkInterface
{
    if (networkInterfaceCombo_ == nullptr) {
        return QNetworkInterface::interfaceFromName(appConfig_.interfaceName.trimmed());
    }

    return QNetworkInterface::interfaceFromName(networkInterfaceCombo_->currentData().toString());
}

auto MainWindow::interfaceLabel(const QNetworkInterface &networkInterface) -> QString
{
    QString label = networkInterface.name();
    if (!networkInterface.humanReadableName().isEmpty()
        && networkInterface.humanReadableName() != networkInterface.name()) {
        label += QStringLiteral(" - %1").arg(networkInterface.humanReadableName());
    }

    const QHostAddress address = primaryIpv4Address(networkInterface);
    if (!address.isNull()) {
        label += QStringLiteral(" (%1)").arg(address.toString());
    }
    return label;
}

auto MainWindow::primaryIpv4Address(const QNetworkInterface &networkInterface) -> QHostAddress
{
    for (const QNetworkAddressEntry &entry : networkInterface.addressEntries()) {
        const QHostAddress address = entry.ip();
        if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isNull()) {
            return address;
        }
    }

    return {};
}

auto MainWindow::interfaceForAddress(const QHostAddress &address) -> QNetworkInterface
{
    for (const QNetworkInterface &networkInterface : QNetworkInterface::allInterfaces()) {
        for (const QNetworkAddressEntry &entry : networkInterface.addressEntries()) {
            if (entry.ip() == address) {
                return networkInterface;
            }
        }
    }

    return {};
}

auto MainWindow::effectiveListenAddress(const QHostAddress &listenAddress, const QHostAddress &destinationAddress) const -> QHostAddress
{
    if (!isAnyAddress(listenAddress) || destinationAddress.isMulticast()) {
        return listenAddress;
    }

    const QHostAddress interfaceAddress = primaryIpv4Address(selectedNetworkInterface());
    if (!interfaceAddress.isNull()) {
        return interfaceAddress;
    }

    return listenAddress;
}

auto MainWindow::dummyFederateBindAddress(const QHostAddress &destinationAddress) const -> QHostAddress
{
    if (destinationAddress.isMulticast() || isBroadcastAddress(destinationAddress)) {
        return QHostAddress::AnyIPv4;
    }

    const QHostAddress interfaceAddress = primaryIpv4Address(selectedNetworkInterface());
    if (!interfaceAddress.isNull() && !destinationAddress.isLoopback()) {
        return interfaceAddress;
    }

    return destinationAddress;
}

void MainWindow::updateSocketOptions(QUdpSocket *socket, const QHostAddress &destinationAddress) const
{
    socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, appConfig_.multicastLoopback ? 1 : 0);

    const QNetworkInterface networkInterface = selectedNetworkInterface();
    if (socket->state() == QAbstractSocket::BoundState
        && networkInterface.isValid()
        && destinationAddress.isMulticast()) {
        socket->setMulticastInterface(networkInterface);
    }
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
    const QNetworkInterface interface = selectedNetworkInterface();
    if (interface.isValid()) {
        return interface;
    }

    if (error != nullptr) {
        *error = QStringLiteral("Unknown network interface");
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
        appendLog(groupError, LogLevel::Warn);
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
        appendLog(interfaceError, LogLevel::Warn);
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
        const QString message = QStringLiteral("Listen multicast join failed for %1: %2")
                                    .arg(group.toString(), socket_->errorString());
        statusBar()->showMessage(message);
        appendLog(message, LogLevel::Error);
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
        appendLog(interfaceError, LogLevel::Warn);
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
                      .arg(group.toString(), dummyFederateSocket_->errorString()),
                  LogLevel::Error);
        return;
    }

    joinedDummyFederateMulticastGroup_ = group;
    joinedDummyFederateMulticastInterface_ = interface;
}

void MainWindow::addStateButton(QGridLayout *layout, const QString &label, SimulationCommand command, int row, int column)
{
    auto *button = new QPushButton(label, this);
    button->setMinimumHeight(StateButtonMinimumHeight);
    connect(button, &QPushButton::clicked, this, [this, command]() -> void { sendStateCommand(command); });
    layout->addWidget(button, row, column);
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
    config.initializeActionId = appConfig_.initializeActionId;
    config.startRealWorldTimeOffsetSeconds = appConfig_.startRealWorldTimeOffsetSeconds;
    config.startSimulationTimeOffsetSeconds = appConfig_.startSimulationTimeOffsetSeconds;
    config.pauseFrozenBehavior = appConfig_.pauseFrozenBehavior;
    config.stopFrozenBehavior = appConfig_.stopFrozenBehavior;
    config.resetFrozenBehavior = appConfig_.resetFrozenBehavior;

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

void MainWindow::setDestinationMode(DestinationMode mode)
{
    if (mode == destinationMode_) {
        return;
    }

    if (destinationMode_ == DestinationMode::Normal) {
        savedDestinationAddressBeforeMode_ = destinationAddressEdit_->text();
        savedInterfaceNameBeforeMode_ = networkInterfaceCombo_->currentData().toString();
    }

    destinationMode_ = mode;

    const QSignalBlocker broadcastBlocker(destinationBroadcastCheck_);
    const QSignalBlocker localhostBlocker(destinationLocalhostCheck_);
    const QSignalBlocker interfaceBlocker(networkInterfaceCombo_);
    destinationBroadcastCheck_->setChecked(mode == DestinationMode::Broadcast);
    destinationLocalhostCheck_->setChecked(mode == DestinationMode::Localhost);

    if (mode == DestinationMode::Normal) {
        destinationAddressEdit_->setEnabled(true);
        if (!savedDestinationAddressBeforeMode_.isEmpty()) {
            destinationAddressEdit_->setText(savedDestinationAddressBeforeMode_);
        }
        const int interfaceIndex = networkInterfaceCombo_->findData(savedInterfaceNameBeforeMode_);
        if (interfaceIndex >= 0) {
            networkInterfaceCombo_->setCurrentIndex(interfaceIndex);
        }
    } else if (mode == DestinationMode::Broadcast) {
        destinationAddressEdit_->setText(QString::fromLatin1(BroadcastDestinationAddress));
        destinationAddressEdit_->setEnabled(false);
        const QNetworkInterface networkInterface = autoSelectedNetworkInterface();
        const int interfaceIndex = networkInterfaceCombo_->findData(networkInterface.name());
        if (interfaceIndex >= 0) {
            networkInterfaceCombo_->setCurrentIndex(interfaceIndex);
        }
    } else {
        destinationAddressEdit_->setText(QString::fromLatin1(LocalhostDestinationAddress));
        destinationAddressEdit_->setEnabled(false);
        for (const QNetworkInterface &networkInterface : QNetworkInterface::allInterfaces()) {
            if (networkInterface.flags().testFlag(QNetworkInterface::IsLoopBack)
                && !primaryIpv4Address(networkInterface).isNull()) {
                const int interfaceIndex = networkInterfaceCombo_->findData(networkInterface.name());
                if (interfaceIndex >= 0) {
                    networkInterfaceCombo_->setCurrentIndex(interfaceIndex);
                }
                break;
            }
        }
    }

    if (socket_ != nullptr) {
        bindListenSocket();
    }
    if (dummyFederateSocket_ != nullptr) {
        bindDummyFederateSocket();
    }
}

void MainWindow::bindListenSocket()
{
    QHostAddress listenAddress;
    if (!parseConfigAddress(listenAddressEdit_->text(), &listenAddress)) {
        statusBar()->showMessage(QStringLiteral("Invalid listen address"));
        appendLog(QStringLiteral("Invalid listen address"), LogLevel::Warn);
        return;
    }
    QHostAddress destinationAddress;
    if (!parseConfigAddress(destinationAddressEdit_->text(), &destinationAddress)) {
        statusBar()->showMessage(QStringLiteral("Invalid destination address"));
        appendLog(QStringLiteral("Invalid destination address"), LogLevel::Warn);
        return;
    }

    const QHostAddress bindAddress = effectiveListenAddress(listenAddress, destinationAddress);
    const auto listenPort = static_cast<quint16>(listenPortSpin_->value());
    const QString listeningMessage =
        QStringLiteral("Listening on %1:%2").arg(bindAddress.toString()).arg(listenPort);
    if (socket_->state() == QAbstractSocket::BoundState
        && boundAddress_ == bindAddress
        && boundPort_ == listenPort) {
        updateSocketOptions(socket_, destinationAddress);
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
    const bool bound = socket_->bind(bindAddress, listenPort, udpBindMode());
    if (!bound) {
        statusBar()->showMessage(QStringLiteral("Listen bind failed: %1").arg(socket_->errorString()));
        appendLog(QStringLiteral("Listen bind failed: %1").arg(socket_->errorString()), LogLevel::Error);
        return;
    }

    updateSocketOptions(socket_, destinationAddress);
    boundAddress_ = bindAddress;
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
        appendLog(QStringLiteral("Invalid network address"), LogLevel::Warn);
        if (dummyFederateStatusLabel_ != nullptr) {
            dummyFederateStatusLabel_->setText(QStringLiteral("Configured: enabled, waiting for valid network settings"));
        }
        return;
    }

    if (dummyFederateSocket_->state() == QAbstractSocket::BoundState
        && dummyFederateBoundAddress_ == dummyFederateBindAddress(config.destinationAddress)
        && dummyFederateBoundPort_ == config.destinationPort) {
        updateDummyFederateMulticastGroup(config.destinationAddress);
        updateSocketOptions(dummyFederateSocket_, config.destinationAddress);
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
    const QHostAddress bindAddress = dummyFederateBindAddress(config.destinationAddress);
    const bool bound = dummyFederateSocket_->bind(bindAddress, config.destinationPort, udpBindMode());
    if (!bound) {
        appendLog(QStringLiteral("Dummy federate bind failed on %1:%2: %3")
                      .arg(bindAddress.toString())
                      .arg(config.destinationPort)
                      .arg(dummyFederateSocket_->errorString()),
                  LogLevel::Error);
        if (dummyFederateStatusLabel_ != nullptr) {
            dummyFederateStatusLabel_->setText(QStringLiteral("Bind failed on %1:%2")
                                                   .arg(bindAddress.toString())
                                                   .arg(config.destinationPort));
        }
        return;
    }

    updateDummyFederateMulticastGroup(config.destinationAddress);
    updateSocketOptions(dummyFederateSocket_, config.destinationAddress);
    dummyFederateBoundAddress_ = bindAddress;
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

void MainWindow::sendStateCommand(SimulationCommand command)
{
    bool configOk = false;
    const auto config = currentConfig(&configOk);
    if (!configOk) {
        QMessageBox::warning(this, QStringLiteral("Invalid Configuration"),
                             QStringLiteral("Enter valid destination and listen IP addresses."));
        appendLog(QStringLiteral("Cannot send state command because destination or listen address is invalid"),
                  LogLevel::Warn);
        return;
    }

    bindListenSocket();
    const quint32 requestId = nextRequestId_++;
    QByteArray pdu;
    switch (command) {
    case SimulationCommand::Initialize:
        pdu = makeActionRequestPdu(config, requestId);
        break;
    case SimulationCommand::Start:
        pdu = makeStartResumePdu(config, requestId);
        break;
    case SimulationCommand::Pause:
    case SimulationCommand::Stop:
    case SimulationCommand::Reset:
        pdu = makeStopFreezePdu(config, requestId, command);
        break;
    }

    const auto written = socket_->writeDatagram(pdu, config.destinationAddress, config.destinationPort);
    if (written != pdu.size()) {
        appendLog(QStringLiteral("Failed to send %1 request %2: %3")
                      .arg(commandName(command))
                      .arg(requestId)
                      .arg(socket_->errorString()),
                  LogLevel::Error);
        return;
    }

    requestStates_[requestId] = commandName(command);
    QString detail;
    if (command == SimulationCommand::Start) {
        detail = QStringLiteral(", real-world +%1s, simulation +%2s")
                     .arg(config.startRealWorldTimeOffsetSeconds)
                     .arg(config.startSimulationTimeOffsetSeconds);
    } else if (command == SimulationCommand::Pause
               || command == SimulationCommand::Stop
               || command == SimulationCommand::Reset) {
        detail = QStringLiteral(", reason %1").arg(stopFreezeReasonLabel(stopFreezeReasonForCommand(command)));
    } else if (command == SimulationCommand::Initialize) {
        detail = QStringLiteral(", action %1").arg(config.initializeActionId);
    }
    appendLog(QStringLiteral("Sent %1 request %2 to %3:%4 (%5 bytes%6)")
                  .arg(commandName(command))
                  .arg(requestId)
                  .arg(config.destinationAddress.toString())
                  .arg(config.destinationPort)
                  .arg(pdu.size())
                  .arg(detail));
    appendMessageRow(pdu, config.destinationAddress, config.destinationPort, QStringLiteral("Tx"));
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
        appendMessageRow(datagram, sender, senderPort, QStringLiteral("Test Rx"));
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
                      .arg(entityIdString(federateId)),
                  LogLevel::Warn);
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
                      .arg(dummyFederateSocket_->errorString()),
                  LogLevel::Error);
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
    appendMessageRow(response, responseAddress, responsePort, QStringLiteral("Test Tx"));
}

void MainWindow::appendMessageRow(const QByteArray &datagram,
                                  const QHostAddress &peer,
                                  quint16 peerPort,
                                  const QString &direction)
{
    const quint8 pduType =
        datagram.size() > PduTypeOffset ? static_cast<quint8>(datagram[PduTypeOffset]) : 0;
    const quint32 requestId = requestIdFromResponse(datagram, pduType);
    const int row = responseTable_->rowCount();
    responseTable_->insertRow(row);
    responseTable_->setItem(row, 0, new QTableWidgetItem(QTime::currentTime().toString("HH:mm:ss.zzz")));
    responseTable_->setItem(row, 1, new QTableWidgetItem(direction));
    responseTable_->setItem(row, 2, new QTableWidgetItem(QStringLiteral("%1:%2").arg(peer.toString()).arg(peerPort)));
    responseTable_->setItem(row, 3, new QTableWidgetItem(pduTypeName(pduType)));
    responseTable_->setItem(row, 4, new QTableWidgetItem(requestId == 0 ? QString() : QString::number(requestId)));
    const QString summary = responseSummary(datagram);
    responseTable_->setItem(row, 5, new QTableWidgetItem(summary));
    responseTable_->scrollToBottom();

    writeLogFileLine(&messageLogFile_,
                     QStringLiteral("%1\t%2\t%3:%4\t%5\t%6\t%7")
                         .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"),
                              direction,
                              peer.toString())
                         .arg(peerPort)
                         .arg(pduTypeName(pduType),
                              requestId == 0 ? QString() : QString::number(requestId),
                              summary));
}

void MainWindow::recordResponse(const QByteArray &datagram, const QHostAddress &sender, quint16 senderPort)
{
    appendMessageRow(datagram, sender, senderPort, QStringLiteral("Rx"));
    const quint8 pduType =
        datagram.size() > PduTypeOffset ? static_cast<quint8>(datagram[PduTypeOffset]) : 0;
    const quint32 requestId = requestIdFromResponse(datagram, pduType);
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

void MainWindow::appendLog(const QString &message, LogLevel level)
{
    if (!shouldLog(level, appConfig_.logLevel)) {
        return;
    }

    const QString line = QStringLiteral("[%1] %2")
                             .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"),
                                  QStringLiteral("%1: %2").arg(logLevelLabel(level), message));
    QTextCharFormat format;
    if (level == LogLevel::Warn) {
        format.setForeground(QColor(214, 137, 16));
        format.setFontWeight(QFont::DemiBold);
    } else if (level == LogLevel::Error) {
        format.setForeground(QColor(203, 36, 49));
        format.setFontWeight(QFont::Bold);
    }

    QTextCursor cursor = log_->textCursor();
    cursor.movePosition(QTextCursor::End);
    if (!log_->document()->isEmpty()) {
        cursor.insertBlock();
    }
    cursor.insertText(line, format);
    log_->setTextCursor(cursor);
    log_->ensureCursorVisible();
    writeLogFileLine(&logFile_, line);
}

auto MainWindow::shouldLog(LogLevel messageLevel, LogLevel configuredLevel) -> bool
{
    return static_cast<quint8>(messageLevel) >= static_cast<quint8>(configuredLevel);
}

auto MainWindow::logLevelLabel(LogLevel level) -> QString
{
    switch (level) {
    case LogLevel::Debug:
        return QStringLiteral("DEBUG");
    case LogLevel::Warn:
        return QStringLiteral("WARN");
    case LogLevel::Error:
        return QStringLiteral("ERROR");
    }

    return QStringLiteral("DEBUG");
}

void MainWindow::setupLogFiles()
{
    if (appConfig_.logs) {
        const QString path = configuredLogPath(appConfig_.logFile);
        logFile_.setFileName(path);
        QDir().mkpath(QFileInfo(path).absolutePath());
        if (!logFile_.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            appendLog(QStringLiteral("Could not open log file %1: %2").arg(path, logFile_.errorString()),
                      LogLevel::Error);
        }
    }

    if (appConfig_.messageLogs) {
        const QString path = configuredLogPath(appConfig_.messageLogFile);
        messageLogFile_.setFileName(path);
        QDir().mkpath(QFileInfo(path).absolutePath());
        if (!messageLogFile_.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            appendLog(QStringLiteral("Could not open message log file %1: %2").arg(path, messageLogFile_.errorString()),
                      LogLevel::Error);
        }
    }
}

auto MainWindow::configuredLogPath(const QString &path) const -> QString
{
    const QFileInfo fileInfo(path);
    if (fileInfo.isAbsolute()) {
        return path;
    }

    if (!appConfig_.configPath.isEmpty()) {
        return QDir(QFileInfo(appConfig_.configPath).absolutePath()).filePath(path);
    }

    return QDir::current().filePath(path);
}

void MainWindow::writeLogFileLine(QFile *file, const QString &line)
{
    if (file == nullptr || !file->isOpen()) {
        return;
    }

    QTextStream out(file);
    out << line << '\n';
    file->flush();
}

void MainWindow::clearMessageLog()
{
    responseTable_->setRowCount(0);
}

void MainWindow::clearEventLog()
{
    log_->clear();
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
