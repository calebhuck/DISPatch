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
#include <QtCore/QVariantAnimation>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPixmap>
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
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>

#include <cmath>

namespace dispatch {

namespace {

auto deadHeartbeatPixmap() -> QPixmap
{
    QPixmap pixmap(HeartbeatIconExtent, HeartbeatIconExtent);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(20.0, 20.0);
    painter.scale(0.80, 0.80);
    painter.translate(-20.0, -20.0);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(138, 143, 152));

    const QColor iconColor(138, 143, 152);
    auto drawBone = [&painter, &iconColor](const QPointF &start, const QPointF &end) -> void {
        const double dx = end.x() - start.x();
        const double dy = end.y() - start.y();
        const double length = std::sqrt(dx * dx + dy * dy);
        const double perpendicularX = -dy / length;
        const double perpendicularY = dx / length;
        constexpr double capOffset = 3.4;
        constexpr double capDiameter = 7.0;

        auto capCenter = [perpendicularX, perpendicularY](const QPointF &point, double offset) -> QPointF {
            return QPointF(point.x() + perpendicularX * offset,
                           point.y() + perpendicularY * offset);
        };
        auto drawCap = [&painter](const QPointF &center) -> void {
            constexpr double radius = capDiameter / 2.0;
            painter.drawEllipse(QRectF(center.x() - radius,
                                       center.y() - radius,
                                       capDiameter,
                                       capDiameter));
        };

        painter.setBrush(iconColor);
        painter.setPen(QPen(iconColor, 4.6, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(start, end);
        painter.setPen(Qt::NoPen);
        drawCap(capCenter(start, -capOffset));
        drawCap(capCenter(start, capOffset));
        drawCap(capCenter(end, -capOffset));
        drawCap(capCenter(end, capOffset));
    };

    drawBone(QPointF(7.3, 9.1), QPointF(32.7, 31.9));
    drawBone(QPointF(32.7, 9.1), QPointF(7.3, 31.9));

    QPainterPath skull;
    skull.moveTo(20.0, 8.0);
    skull.cubicTo(12.0, 8.0, 8.4, 13.2, 8.4, 20.4);
    skull.cubicTo(8.4, 21.8, 8.8, 23.2, 9.3, 24.3);
    skull.cubicTo(8.3, 25.1, 8.1, 27.0, 9.0, 28.4);
    skull.cubicTo(9.8, 29.9, 11.2, 30.5, 12.9, 30.3);
    skull.cubicTo(14.1, 30.2, 14.7, 30.8, 14.8, 32.1);
    skull.cubicTo(15.0, 34.3, 15.8, 35.4, 17.4, 35.4);
    skull.lineTo(22.6, 35.4);
    skull.cubicTo(24.2, 35.4, 25.0, 34.3, 25.2, 32.1);
    skull.cubicTo(25.3, 30.8, 25.9, 30.2, 27.1, 30.3);
    skull.cubicTo(28.8, 30.5, 30.2, 29.9, 31.0, 28.4);
    skull.cubicTo(31.9, 27.0, 31.7, 25.1, 30.7, 24.3);
    skull.cubicTo(31.2, 23.2, 31.6, 21.8, 31.6, 20.4);
    skull.cubicTo(31.6, 13.2, 28.0, 8.0, 20.0, 8.0);
    skull.closeSubpath();
    painter.drawPath(skull);

    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.setBrush(Qt::transparent);

    QPainterPath leftEye;
    leftEye.moveTo(15.1, 17.9);
    leftEye.cubicTo(17.5, 17.9, 18.8, 19.5, 18.4, 21.7);
    leftEye.cubicTo(18.0, 23.8, 16.3, 24.8, 14.2, 24.3);
    leftEye.cubicTo(12.2, 23.9, 11.3, 22.4, 11.8, 20.4);
    leftEye.cubicTo(12.2, 18.7, 13.3, 17.9, 15.1, 17.9);
    leftEye.closeSubpath();
    painter.drawPath(leftEye);

    QPainterPath rightEye;
    rightEye.moveTo(24.9, 17.9);
    rightEye.cubicTo(22.5, 17.9, 21.2, 19.5, 21.6, 21.7);
    rightEye.cubicTo(22.0, 23.8, 23.7, 24.8, 25.8, 24.3);
    rightEye.cubicTo(27.8, 23.9, 28.7, 22.4, 28.2, 20.4);
    rightEye.cubicTo(27.8, 18.7, 26.7, 17.9, 24.9, 17.9);
    rightEye.closeSubpath();
    painter.drawPath(rightEye);

    QPainterPath nose;
    nose.moveTo(20.0, 25.2);
    nose.lineTo(17.9, 29.3);
    nose.cubicTo(18.7, 29.9, 19.5, 29.4, 20.0, 28.8);
    nose.cubicTo(20.5, 29.4, 21.3, 29.9, 22.1, 29.3);
    nose.closeSubpath();
    painter.drawPath(nose);

    painter.setPen(QPen(Qt::transparent, 1.6, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(QPointF(16.8, 31.5), QPointF(16.8, 35.4));
    painter.drawLine(QPointF(20.0, 31.3), QPointF(20.0, 35.5));
    painter.drawLine(QPointF(23.2, 31.5), QPointF(23.2, 35.4));

    return pixmap;
}

auto peerString(const QHostAddress &address, quint16 port) -> QString
{
    return QStringLiteral("%1:%2").arg(address.toString()).arg(port);
}

auto minimumPduLength(quint8 pduType) -> int
{
    switch (pduType) {
    case EntityStatePdu:
        return EntityStatePduLength;
    case StartResumePdu:
        return StartResumePduLength;
    case StopFreezePdu:
        return StopFreezePduLength;
    case AcknowledgePdu:
        return AcknowledgePduLength;
    case ActionRequestPdu:
        return ActionRequestPduLength;
    case ActionResponsePdu:
        return ActionResponsePduLength;
    case CommentPdu:
        return CommentPduLength;
    default:
        return 0;
    }
}

auto isResponsePduType(quint8 pduType) -> bool
{
    return pduType == AcknowledgePdu || pduType == ActionResponsePdu;
}

auto isRequestPduType(quint8 pduType) -> bool
{
    return pduType == StartResumePdu || pduType == StopFreezePdu || pduType == ActionRequestPdu;
}

auto isBroadcastEntityId(const EntityId &entityId) -> bool
{
    return entityId.site == BroadcastEntityIdValue
        && entityId.application == BroadcastEntityIdValue
        && entityId.entity == BroadcastEntityIdValue;
}

auto stateForAcceptedRequest(quint8 pduType, const QByteArray &datagram) -> QString
{
    if (pduType == ActionRequestPdu) {
        return QStringLiteral("Initialized");
    }
    if (pduType == StartResumePdu) {
        return QStringLiteral("Running");
    }
    if (pduType != StopFreezePdu) {
        return QStringLiteral("Active");
    }

    switch (static_cast<quint8>(datagram[StopFreezeReasonOffset])) {
    case RecessReason:
        return QStringLiteral("Paused");
    case TerminationReason:
        return QStringLiteral("Stopped");
    case StopForResetReason:
        return QStringLiteral("Reset");
    default:
        return QStringLiteral("Frozen");
    }
}

void addCommonDatagramWarnings(const QByteArray &datagram, const DisConfig &config, QStringList *warnings)
{
    if (datagram.size() < DisHeaderLength) {
        warnings->append(QStringLiteral("too short for a DIS header"));
        return;
    }

    const auto version = static_cast<quint8>(datagram[PduVersionOffset]);
    if (version != DisVersion) {
        warnings->append(QStringLiteral("DIS version %1 does not match expected DIS%2")
                             .arg(version)
                             .arg(DisVersion));
    }

    const auto exerciseId = static_cast<quint8>(datagram[PduExerciseIdOffset]);
    if (exerciseId != config.exerciseId) {
        warnings->append(QStringLiteral("exercise ID %1 does not match configured exercise %2")
                             .arg(exerciseId)
                             .arg(config.exerciseId));
    }

    const auto family = static_cast<quint8>(datagram[PduFamilyOffset]);
    if (family != SimManagementFamily) {
        warnings->append(QStringLiteral("PDU family %1 is not Simulation Management family %2")
                             .arg(family)
                             .arg(SimManagementFamily));
    }

    const quint16 declaredLength = readU16(datagram, PduLengthOffset);
    if (declaredLength != datagram.size()) {
        warnings->append(QStringLiteral("header length %1 does not match datagram size %2")
                             .arg(declaredLength)
                             .arg(datagram.size()));
    }

    const auto pduType = static_cast<quint8>(datagram[PduTypeOffset]);
    const int minimumLength = minimumPduLength(pduType);
    if (minimumLength == 0) {
        warnings->append(QStringLiteral("PDU type %1 is not handled by DISPatch").arg(pduType));
    } else if (datagram.size() < minimumLength) {
        warnings->append(QStringLiteral("%1 PDU is %2 bytes; expected at least %3")
                             .arg(pduTypeName(pduType))
                             .arg(datagram.size())
                             .arg(minimumLength));
    }
}

auto incomingResponseWarnings(const QByteArray &datagram,
                              const DisConfig &config,
                              const EntityId &configuredTarget,
                              bool requestIdKnown) -> QStringList
{
    QStringList warnings;
    addCommonDatagramWarnings(datagram, config, &warnings);
    if (datagram.size() < DisHeaderLength) {
        return warnings;
    }

    const auto pduType = static_cast<quint8>(datagram[PduTypeOffset]);
    if (!isResponsePduType(pduType)) {
        warnings.append(QStringLiteral("%1 is not an expected federate response PDU")
                            .arg(pduTypeName(pduType)));
        return warnings;
    }

    if (datagram.size() >= TargetEntityOffset + EntityIdByteLength) {
        const EntityId target = readEntityId(datagram, TargetEntityOffset);
        if (!entityIdsMatch(target, config.managerId)) {
            warnings.append(QStringLiteral("response target %1 does not match manager ID %2")
                                .arg(entityIdString(target), entityIdString(config.managerId)));
        }

        const EntityId origin = readEntityId(datagram, OriginEntityOffset);
        if (!entityIdAddresses(configuredTarget, origin)) {
            warnings.append(QStringLiteral("response origin %1 does not match configured target ID %2")
                                .arg(entityIdString(origin), entityIdString(configuredTarget)));
        }
    }

    const quint32 requestId = requestIdFromResponse(datagram, pduType);
    if (requestId == 0) {
        warnings.append(QStringLiteral("response request ID could not be decoded"));
    } else if (!requestIdKnown) {
        warnings.append(QStringLiteral("response request ID %1 does not match a request sent in this session")
                            .arg(requestId));
    }

    return warnings;
}

auto dummyRequestWarnings(const QByteArray &datagram,
                          const DisConfig &config,
                          const EntityId &federateId) -> QStringList
{
    QStringList warnings;
    addCommonDatagramWarnings(datagram, config, &warnings);
    if (datagram.size() < DisHeaderLength) {
        return warnings;
    }

    const auto pduType = static_cast<quint8>(datagram[PduTypeOffset]);
    if (!isRequestPduType(pduType)) {
        warnings.append(QStringLiteral("%1 is not a Simulation Management request PDU")
                            .arg(pduTypeName(pduType)));
        return warnings;
    }

    if (datagram.size() >= TargetEntityOffset + EntityIdByteLength) {
        const EntityId receivingEntity = readEntityId(datagram, TargetEntityOffset);
        if (!entityIdAddresses(receivingEntity, federateId)) {
            warnings.append(QStringLiteral("request target %1 does not match dummy federate ID %2")
                                .arg(entityIdString(receivingEntity), entityIdString(federateId)));
        }
    }

    return warnings;
}

auto responseLooksIntendedForManager(const QByteArray &datagram,
                                     const DisConfig &config,
                                     const EntityId &configuredTarget) -> bool
{
    if (datagram.size() < TargetEntityOffset + EntityIdByteLength
        || static_cast<quint8>(datagram[PduVersionOffset]) != DisVersion
        || static_cast<quint8>(datagram[PduExerciseIdOffset]) != config.exerciseId
        || static_cast<quint8>(datagram[PduFamilyOffset]) != SimManagementFamily) {
        return false;
    }

    const quint8 pduType = static_cast<quint8>(datagram[PduTypeOffset]);
    if (!isResponsePduType(pduType)) {
        return false;
    }

    const EntityId target = readEntityId(datagram, TargetEntityOffset);
    if (!entityIdsMatch(target, config.managerId)) {
        return false;
    }

    const EntityId origin = readEntityId(datagram, OriginEntityOffset);
    return entityIdAddresses(configuredTarget, origin);
}

auto committedSpinBoxValue(const QSpinBox *spinBox) -> int
{
    // Commit in-progress edits so live validation sees typed values before focus changes.
    const_cast<QSpinBox *>(spinBox)->interpretText();
    return spinBox->value();
}

} // namespace

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
    themeCombo_->addItem(QStringLiteral("One Dark"), static_cast<int>(Theme::OneDark));
    themeCombo_->addItem(QStringLiteral("VS Code Default"), static_cast<int>(Theme::VsCodeDefault));
    themeCombo_->addItem(QStringLiteral("Tokyo Night"), static_cast<int>(Theme::TokyoNight));
    themeCombo_->addItem(QStringLiteral("Dracula"), static_cast<int>(Theme::Dracula));
    themeCombo_->setFixedWidth(ThemeComboWidth);
    settingsLayout->addWidget(themeCombo_);
    settingsLayout->addStretch(1);

    auto *networkGroup = new QGroupBox(QStringLiteral("Network"), central);
    auto *networkLayout = new QFormLayout(networkGroup);
    networkLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    networkLayout->setLabelAlignment(Qt::AlignLeft);
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
        updateBroadcastDestinationAddress();
        bindListenSocket();
        bindDummyFederateSocket();
    });
    QHostAddress configuredDestination;
    if (parseConfigAddress(appConfig_.destinationAddress, &configuredDestination)) {
        if (isBroadcastDestination(configuredDestination)) {
            setDestinationMode(DestinationMode::Broadcast);
        } else if (configuredDestination == QHostAddress(QHostAddress::LocalHost)) {
            setDestinationMode(DestinationMode::Localhost);
        }
    }

    auto *stateGroup = new QGroupBox(QStringLiteral("Simulation Commands"), central);
    auto *stateLayout = new QGridLayout(stateGroup);
    startRealWorldTimeOffsetSpin_ =
        makeSmallSpinBox(stateGroup, 0, MaxTimeOffsetSeconds, appConfig_.startRealWorldTimeOffsetSeconds);
    startRealWorldTimeOffsetSpin_->setSuffix(QStringLiteral(" s"));
    startSimulationTimeOffsetSpin_ =
        makeSmallSpinBox(stateGroup, 0, MaxTimeOffsetSeconds, appConfig_.startSimulationTimeOffsetSeconds);
    startSimulationTimeOffsetSpin_->setSuffix(QStringLiteral(" s"));
    stateLayout->addWidget(new QLabel(QStringLiteral("Start real-world offset"), stateGroup), 0, 0, 1, 3);
    stateLayout->addWidget(startRealWorldTimeOffsetSpin_, 0, 3, 1, 3);
    stateLayout->addWidget(new QLabel(QStringLiteral("Start simulation offset"), stateGroup), 1, 0, 1, 3);
    stateLayout->addWidget(startSimulationTimeOffsetSpin_, 1, 3, 1, 3);
    addStateButton(stateLayout, QStringLiteral("Initialize"), SimulationCommand::Initialize, 2, 0, 3);
    addStateButton(stateLayout, QStringLiteral("Start"), SimulationCommand::Start, 2, 3, 3);
    addStateButton(stateLayout, QStringLiteral("Pause"), SimulationCommand::Pause, 3, 0, 2);
    addStateButton(stateLayout, QStringLiteral("Stop"), SimulationCommand::Stop, 3, 2, 2);
    addStateButton(stateLayout, QStringLiteral("Reset"), SimulationCommand::Reset, 3, 4, 2);
    for (int column = 0; column < 6; ++column) {
        stateLayout->setColumnStretch(column, 1);
    }

    auto *disGroup = new QGroupBox(QStringLiteral("DIS Identity"), central);
    auto *disLayout = new QGridLayout(disGroup);
    exerciseSpin_ = makeSmallSpinBox(disGroup, 0, MaxExerciseId, appConfig_.exerciseId);
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
    heartbeatLayout_ = new QVBoxLayout();
    heartbeatLayout_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    disLayout->addLayout(heartbeatLayout_, 0, 6, 3, 1);
    disLayout->setColumnStretch(IdentityStretchColumn, 1);
    connect(targetBroadcastCheck_, &QCheckBox::toggled, this, &MainWindow::setTargetBroadcast);
    if (isBroadcastEntityId(appConfig_.targetId)) {
        const QSignalBlocker blocker(targetBroadcastCheck_);
        targetBroadcastCheck_->setChecked(true);
        savedTargetIdBeforeBroadcast_ = appConfig_.testFederateIds.isEmpty()
            ? EntityId{1, 1, 0}
            : appConfig_.testFederateIds.first();
        setTargetIdControlsEnabled(false);
    }

    auto *testGroup = new QGroupBox(QStringLiteral("Test Federates"), central);
    auto *testLayout = new QGridLayout(testGroup);
    dummyFederateStatusLabel_ = new QLabel(QStringLiteral("Configured: enabled, waiting to bind"), testGroup);
    dummyFederateStatusLabel_->setWordWrap(true);
    testGroup->setMinimumWidth(TestFederateMinimumWidth);
    testLayout->addWidget(dummyFederateStatusLabel_, 0, 0, 1, 6);
    testLayout->addWidget(new QLabel(QStringLiteral("Entity ID")), 1, 0);
    testLayout->addWidget(new QLabel(QStringLiteral("Site")), 1, 1);
    testLayout->addWidget(new QLabel(QStringLiteral("App")), 1, 2);
    testLayout->addWidget(new QLabel(QStringLiteral("Entity")), 1, 3);
    testLayout->addWidget(new QLabel(QStringLiteral("Status")), 1, 4);

    const QList<EntityId> configuredTestFederates = appConfig_.testFederateIds.isEmpty()
        ? QList<EntityId>{EntityId{1, 1, 0}}
        : appConfig_.testFederateIds;
    for (int index = 0; index < configuredTestFederates.size(); ++index) {
        const EntityId federateId = configuredTestFederates.at(index);
        TestFederateControls controls;
        controls.siteSpin = makeSmallSpinBox(testGroup, 0, BroadcastEntityIdValue, federateId.site);
        controls.applicationSpin =
            makeSmallSpinBox(testGroup, 0, BroadcastEntityIdValue, federateId.application);
        controls.entitySpin = makeSmallSpinBox(testGroup, 0, BroadcastEntityIdValue, federateId.entity);
        controls.stateLabel = new QLabel(QStringLiteral("Idle"), testGroup);
        controls.stateLabel->setAlignment(Qt::AlignCenter);
        controls.deadCheck = new QCheckBox(QStringLiteral("Stop heartbeat"), testGroup);
        controls.deadCheck->setEnabled(appConfig_.heartbeatEnabled);
        controls.deadCheck->setMinimumWidth(controls.deadCheck->sizeHint().width());
        controls.stateLabel->setMinimumWidth(controls.deadCheck->sizeHint().width());
        auto *statusWidget = new QWidget(testGroup);
        auto *statusLayout = new QVBoxLayout(statusWidget);
        statusLayout->setContentsMargins(0, 0, 0, 0);
        statusLayout->setSpacing(2);
        statusLayout->addWidget(controls.stateLabel);
        statusLayout->addWidget(controls.deadCheck, 0, Qt::AlignLeft);

        const int row = index + 2;
        testLayout->addWidget(new QLabel(QStringLiteral("Federate %1").arg(index + 1)), row, 0);
        testLayout->addWidget(controls.siteSpin, row, 1);
        testLayout->addWidget(controls.applicationSpin, row, 2);
        testLayout->addWidget(controls.entitySpin, row, 3);
        testLayout->addWidget(statusWidget, row, 4, Qt::AlignLeft);
        connect(controls.deadCheck, &QCheckBox::toggled, this, [this, controls](bool dead) -> void {
            setDummyFederateDead(makeEntityId(controls.siteSpin, controls.applicationSpin, controls.entitySpin),
                                 dead);
        });
        testFederateControls_.append(controls);
    }
    testLayout->setColumnStretch(TestFederateStretchColumn, 1);
    testGroup->setVisible(appConfig_.testFederateEnabled);

    auto *identityLayout = new QHBoxLayout();
    identityLayout->setSpacing(StandardSpacing);
    identityLayout->addWidget(disGroup, 1);
    if (appConfig_.testFederateEnabled) {
        identityLayout->addWidget(testGroup, 0, Qt::AlignRight);
    }

    auto *logPane = new QWidget(central);
    auto *logPaneLayout = new QVBoxLayout(logPane);
    logPaneLayout->setContentsMargins(0, 0, 0, 0);
    logPaneLayout->setSpacing(0);

    auto *logTabBar = new QTabBar(logPane);
    logTabBar->setExpanding(false);
    logTabBar->addTab(QStringLiteral("Messages"));
    logTabBar->addTab(QStringLiteral("Event Log"));

    auto *logTabBarLayout = new QHBoxLayout();
    logTabBarLayout->setContentsMargins(0, 0, 0, 0);
    logTabBarLayout->setSpacing(0);
    logTabBarLayout->addWidget(logTabBar);
    logTabBarLayout->addStretch(1);

    auto *logStack = new QStackedWidget(logPane);

    responseTable_ = new QTableWidget(0, ResponseTableColumnCount, logStack);
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

    log_ = new QPlainTextEdit(logStack);
    log_->setReadOnly(true);
    log_->setMaximumBlockCount(MaxLogBlocks);
    setupLogFiles();

    logStack->addWidget(responseTable_);
    logStack->addWidget(log_);
    connect(logTabBar, &QTabBar::currentChanged, logStack, &QStackedWidget::setCurrentIndex);

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
    logPaneLayout->addLayout(logTabBarLayout);
    logPaneLayout->addWidget(logStack, 1);
    rootLayout->addWidget(logPane, 1);
    setCentralWidget(central);
    resize(DefaultWindowWidth, DefaultWindowHeight);

    socket_ = new QUdpSocket(this);
    updateSocketOptions(socket_, QHostAddress(destinationAddressEdit_->text()));
    connect(socket_, &QUdpSocket::readyRead, this, &MainWindow::readDatagrams);
    dummyFederateSocket_ = new QUdpSocket(this);
    connect(dummyFederateSocket_, &QUdpSocket::readyRead, this, &MainWindow::readDummyFederateDatagrams);
    heartbeatCheckTimer_ = new QTimer(this);
    heartbeatCheckTimer_->setInterval(HeartbeatCheckIntervalMilliseconds);
    connect(heartbeatCheckTimer_, &QTimer::timeout, this, &MainWindow::checkHeartbeatTimeouts);
    if (appConfig_.heartbeatEnabled) {
        heartbeatCheckTimer_->start();
    }
    dummyHeartbeatTimer_ = new QTimer(this);
    const int dummyHeartbeatInterval =
        qBound(HeartbeatCheckIntervalMilliseconds,
               appConfig_.heartbeatTimeoutSeconds * MillisecondsPerSecond / 2,
               RebindIntervalMilliseconds);
    dummyHeartbeatTimer_->setInterval(dummyHeartbeatInterval);
    connect(dummyHeartbeatTimer_, &QTimer::timeout, this, &MainWindow::sendDummyFederateHeartbeat);
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
        if (appConfig_.heartbeatEnabled) {
            sendDummyFederateHeartbeat();
            dummyHeartbeatTimer_->start();
        }
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

auto MainWindow::primaryIpv4BroadcastAddress(const QNetworkInterface &networkInterface) -> QHostAddress
{
    for (const QNetworkAddressEntry &entry : networkInterface.addressEntries()) {
        if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol
            && !entry.broadcast().isNull()) {
            return entry.broadcast();
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

auto MainWindow::isBroadcastDestination(const QHostAddress &address) const -> bool
{
    return destinationMode_ == DestinationMode::Broadcast || isBroadcastAddress(address)
        || address == primaryIpv4BroadcastAddress(selectedNetworkInterface());
}

auto MainWindow::effectiveListenAddress(const QHostAddress &listenAddress, const QHostAddress &destinationAddress) const -> QHostAddress
{
    if (!isAnyAddress(listenAddress) || destinationAddress.isMulticast()
        || isBroadcastDestination(destinationAddress)) {
        return listenAddress;
    }

    const QHostAddress interfaceAddress = primaryIpv4Address(selectedNetworkInterface());
    if (!interfaceAddress.isNull()) {
        return interfaceAddress;
    }

    return listenAddress;
}

auto MainWindow::effectiveSendAddress(const QHostAddress &destinationAddress) const -> QHostAddress
{
    if (!isBroadcastDestination(destinationAddress)) {
        return destinationAddress;
    }

    const QHostAddress directedBroadcast = primaryIpv4BroadcastAddress(selectedNetworkInterface());
    return directedBroadcast.isNull() ? destinationAddress : directedBroadcast;
}

auto MainWindow::dummyFederateBindAddress(const QHostAddress &destinationAddress) const -> QHostAddress
{
    if (destinationAddress.isMulticast() || isBroadcastDestination(destinationAddress)) {
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
    if (networkInterface.isValid()
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
        appendLogOnce(&lastListenSocketIssue_, groupError, LogLevel::Warn);
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
        appendLogOnce(&lastListenSocketIssue_, interfaceError, LogLevel::Warn);
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
        appendLogOnce(&lastListenSocketIssue_, message, LogLevel::Error);
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
    lastListenSocketIssue_.clear();
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
        appendLogOnce(&lastDummyFederateMulticastIssue_, interfaceError, LogLevel::Warn);
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
        appendLogOnce(&lastDummyFederateMulticastIssue_,
                      QStringLiteral("Dummy federate multicast join failed for %1: %2")
                          .arg(group.toString(), dummyFederateSocket_->errorString()),
                      LogLevel::Error);
        return;
    }

    joinedDummyFederateMulticastGroup_ = group;
    joinedDummyFederateMulticastInterface_ = interface;
    lastDummyFederateMulticastIssue_.clear();
}

void MainWindow::addStateButton(QGridLayout *layout,
                                const QString &label,
                                SimulationCommand command,
                                int row,
                                int column,
                                int columnSpan)
{
    auto *button = new QPushButton(label, this);
    button->setMinimumHeight(StateButtonMinimumHeight);
    connect(button, &QPushButton::clicked, this, [this, command]() -> void { sendStateCommand(command); });
    layout->addWidget(button, row, column, 1, columnSpan);
}

auto MainWindow::currentConfig(bool *configOk) const -> DisConfig
{
    DisConfig config;
    const bool destinationOk = parseConfigAddress(destinationAddressEdit_->text(), &config.destinationAddress);
    const bool listenOk = parseConfigAddress(listenAddressEdit_->text(), &config.listenAddress);
    config.destinationPort = static_cast<quint16>(committedSpinBoxValue(destinationPortSpin_));
    config.listenPort = static_cast<quint16>(committedSpinBoxValue(listenPortSpin_));
    config.exerciseId = static_cast<quint8>(committedSpinBoxValue(exerciseSpin_));
    config.managerId = makeEntityId(managerSiteSpin_, managerApplicationSpin_, managerEntitySpin_);
    config.targetId = makeEntityId(targetSiteSpin_, targetApplicationSpin_, targetEntitySpin_);
    config.initializeActionId = appConfig_.initializeActionId;
    config.startRealWorldTimeOffsetSeconds = committedSpinBoxValue(startRealWorldTimeOffsetSpin_);
    config.startSimulationTimeOffsetSeconds = committedSpinBoxValue(startSimulationTimeOffsetSpin_);
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
    if (testFederateControls_.isEmpty()) {
        return EntityId{1, 1, 0};
    }
    const TestFederateControls &controls = testFederateControls_.first();
    return makeEntityId(controls.siteSpin, controls.applicationSpin, controls.entitySpin);
}

auto MainWindow::currentTestFederateIds() const -> QList<EntityId>
{
    QList<EntityId> ids;
    for (const TestFederateControls &controls : testFederateControls_) {
        const EntityId id = makeEntityId(controls.siteSpin, controls.applicationSpin, controls.entitySpin);
        bool duplicate = false;
        for (const EntityId &existing : ids) {
            if (entityIdsMatch(existing, id)) {
                duplicate = true;
                break;
            }
        }
        if (!duplicate) {
            ids.append(id);
        }
    }
    if (ids.isEmpty()) {
        ids.append(EntityId{1, 1, 0});
    }
    return ids;
}

auto MainWindow::testFederateIdsForRequest(const QByteArray &datagram) const -> QList<EntityId>
{
    if (datagram.size() < TargetEntityOffset + EntityIdByteLength
        || static_cast<quint8>(datagram[PduFamilyOffset]) != SimManagementFamily) {
        return {};
    }

    const quint8 pduType = static_cast<quint8>(datagram[PduTypeOffset]);
    if (!isRequestPduType(pduType)) {
        return {};
    }

    const EntityId receivingEntity = readEntityId(datagram, TargetEntityOffset);
    QList<EntityId> matches;
    for (const EntityId &id : currentTestFederateIds()) {
        if (entityIdAddresses(receivingEntity, id)) {
            matches.append(id);
        }
    }
    return matches;
}

auto MainWindow::currentTargetId() const -> EntityId
{
    return makeEntityId(targetSiteSpin_, targetApplicationSpin_, targetEntitySpin_);
}

void MainWindow::updateHeartbeat(const EntityId &entityId)
{
    const QString id = entityIdString(entityId);
    auto status = heartbeatStatuses_.find(id);
    if (status == heartbeatStatuses_.end()) {
        HeartbeatStatus newStatus;
        auto *row = new QWidget();
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(6);
        newStatus.iconLabel = new QLabel(QStringLiteral("\u2665"), row);
        newStatus.iconLabel->setAlignment(Qt::AlignCenter);
        newStatus.iconLabel->setFixedSize(HeartbeatIconExtent, HeartbeatIconExtent);
        newStatus.idLabel = new QLabel(id, row);
        newStatus.idLabel->setStyleSheet(
            QStringLiteral("QLabel { font-size: 12pt; font-weight: 600; }"));
        rowLayout->addWidget(newStatus.iconLabel);
        rowLayout->addWidget(newStatus.idLabel);
        newStatus.pulseAnimation = new QVariantAnimation(row);
        newStatus.pulseAnimation->setDuration(HeartbeatPulseDurationMilliseconds);
        newStatus.pulseAnimation->setKeyValueAt(0.0, 20.0);
        newStatus.pulseAnimation->setKeyValueAt(0.18, 28.0);
        newStatus.pulseAnimation->setKeyValueAt(0.40, 20.0);
        newStatus.pulseAnimation->setKeyValueAt(0.58, 24.0);
        newStatus.pulseAnimation->setKeyValueAt(0.78, 20.0);
        newStatus.pulseAnimation->setKeyValueAt(1.0, 20.0);
        connect(newStatus.pulseAnimation,
                &QVariantAnimation::valueChanged,
                newStatus.iconLabel,
                [iconLabel = newStatus.iconLabel](const QVariant &value) -> void {
                    iconLabel->setStyleSheet(
                        QStringLiteral("QLabel { color: #e25555; font-size: %1pt; font-weight: 700; }")
                            .arg(value.toDouble(), 0, 'f', 1));
                });
        heartbeatLayout_->addWidget(row);
        status = heartbeatStatuses_.insert(id, newStatus);
    }

    status->lastUpdateMilliseconds = QDateTime::currentMSecsSinceEpoch();
    status->alive = true;
    status->iconLabel->clear();
    status->iconLabel->setText(QStringLiteral("\u2665"));
    status->idLabel->setStyleSheet(
        QStringLiteral("QLabel { font-size: 12pt; font-weight: 600; }"));
    status->pulseAnimation->stop();
    status->pulseAnimation->start();
}

void MainWindow::checkHeartbeatTimeouts()
{
    if (!appConfig_.heartbeatEnabled) {
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 timeout = static_cast<qint64>(appConfig_.heartbeatTimeoutSeconds) * MillisecondsPerSecond;
    for (auto status = heartbeatStatuses_.begin(); status != heartbeatStatuses_.end(); ++status) {
        if (!status->alive || now - status->lastUpdateMilliseconds < timeout) {
            continue;
        }

        status->alive = false;
        status->pulseAnimation->stop();
        status->iconLabel->clear();
        status->iconLabel->setPixmap(deadHeartbeatPixmap());
        status->iconLabel->setStyleSheet(
            QStringLiteral("QLabel { color: #8a8f98; }"));
        status->idLabel->setStyleSheet(
            QStringLiteral("QLabel { color: #8a8f98; font-size: 12pt; font-weight: 600; }"));
    }
}

auto MainWindow::dummyFederateStatusText(const DisConfig &config) const -> QString
{
    return QStringLiteral("Running on %1:%2")
        .arg(config.destinationAddress.toString())
        .arg(config.destinationPort);
}

void MainWindow::rememberRequest(quint32 requestId, const QString &command)
{
    requestStates_[requestId] = command;
    while (requestStates_.size() > MaxTrackedRequests) {
        requestStates_.erase(requestStates_.begin());
    }
}

void MainWindow::setDummyFederateDead(const EntityId &federateId, bool dead)
{
    if (!dummyFederateEnabled_ || !appConfig_.heartbeatEnabled) {
        return;
    }

    appendLog(dead ? QStringLiteral("Dummy federate %1 heartbeat stopped").arg(entityIdString(federateId))
                   : QStringLiteral("Dummy federate %1 heartbeat resumed").arg(entityIdString(federateId)));
    bool configOk = false;
    const DisConfig config = currentConfig(&configOk);
    if (configOk && dummyFederateStatusLabel_ != nullptr) {
        dummyFederateStatusLabel_->setText(dummyFederateStatusText(config));
    }
    if (!dead) {
        sendDummyFederateHeartbeat();
    }
}

void MainWindow::updateTestFederateState(const EntityId &federateId, const QString &state)
{
    for (const TestFederateControls &controls : testFederateControls_) {
        const EntityId currentId = makeEntityId(controls.siteSpin, controls.applicationSpin, controls.entitySpin);
        if (entityIdsMatch(currentId, federateId) && controls.stateLabel != nullptr) {
            controls.stateLabel->setText(state);
            return;
        }
    }
}

void MainWindow::sendDummyFederateHeartbeat()
{
    if (!dummyFederateEnabled_ || !appConfig_.heartbeatEnabled) {
        return;
    }

    for (const TestFederateControls &controls : testFederateControls_) {
        if (controls.deadCheck != nullptr && controls.deadCheck->isChecked()) {
            continue;
        }

        const EntityId federateId = makeEntityId(controls.siteSpin, controls.applicationSpin, controls.entitySpin);
        updateHeartbeat(federateId);
    }
}

auto MainWindow::makeEntityId(const QSpinBox *site, const QSpinBox *application, const QSpinBox *entity) -> EntityId
{
    return EntityId{static_cast<quint16>(committedSpinBoxValue(site)),
                    static_cast<quint16>(committedSpinBoxValue(application)),
                    static_cast<quint16>(committedSpinBoxValue(entity))};
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
        destinationAddressEdit_->setEnabled(false);
        const QNetworkInterface networkInterface = autoSelectedNetworkInterface();
        const int interfaceIndex = networkInterfaceCombo_->findData(networkInterface.name());
        if (interfaceIndex >= 0) {
            networkInterfaceCombo_->setCurrentIndex(interfaceIndex);
        }
        updateBroadcastDestinationAddress();
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

void MainWindow::updateBroadcastDestinationAddress()
{
    if (destinationMode_ != DestinationMode::Broadcast) {
        return;
    }

    const QHostAddress broadcastAddress = primaryIpv4BroadcastAddress(selectedNetworkInterface());
    destinationAddressEdit_->setText(broadcastAddress.toString());
    if (broadcastAddress.isNull()) {
        statusBar()->showMessage(QStringLiteral("Selected interface has no IPv4 broadcast address"));
    }
}

void MainWindow::bindListenSocket()
{
    QHostAddress listenAddress;
    if (!parseConfigAddress(listenAddressEdit_->text(), &listenAddress)) {
        const QString message = QStringLiteral("Invalid listen address");
        statusBar()->showMessage(message);
        appendLogOnce(&lastListenSocketIssue_, message, LogLevel::Warn);
        return;
    }
    QHostAddress destinationAddress;
    if (!parseConfigAddress(destinationAddressEdit_->text(), &destinationAddress)) {
        const QString message = QStringLiteral("Invalid destination address");
        statusBar()->showMessage(message);
        appendLogOnce(&lastListenSocketIssue_, message, LogLevel::Warn);
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
        lastListenSocketIssue_.clear();
        if (statusBar()->currentMessage().startsWith(QStringLiteral("Invalid "))) {
            statusBar()->showMessage(listeningMessage);
        }
        return;
    }

    socket_->close();
    joinedListenMulticastGroup_ = QHostAddress();
    const bool bound = socket_->bind(bindAddress, listenPort, udpBindMode());
    if (!bound) {
        const QString message = QStringLiteral("Listen bind failed: %1").arg(socket_->errorString());
        statusBar()->showMessage(message);
        appendLogOnce(&lastListenSocketIssue_, message, LogLevel::Error);
        return;
    }

    updateSocketOptions(socket_, destinationAddress);
    boundAddress_ = bindAddress;
    boundPort_ = listenPort;
    if (updateListenMulticastGroup()) {
        statusBar()->showMessage(listeningMessage);
        lastListenSocketIssue_.clear();
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
        const QString message = QStringLiteral("Invalid network address");
        statusBar()->showMessage(message);
        appendLogOnce(&lastDummyFederateIssue_, message, LogLevel::Warn);
        if (dummyFederateStatusLabel_ != nullptr) {
            dummyFederateStatusLabel_->setText(QStringLiteral("Configured: enabled, waiting for valid network settings"));
        }
        return;
    }

    if (dummyFederateShouldShareListenSocket(config)) {
        if (!dummyFederateSharesListenSocket_) {
            clearDummyFederateMulticastGroup();
            dummyFederateSocket_->close();
            dummyFederateBoundAddress_ = QHostAddress();
            dummyFederateBoundPort_ = 0;
            dummyFederateSharesListenSocket_ = true;
            appendLog(QStringLiteral("Dummy federate sharing the manager listen socket"));
        }
        if (dummyFederateStatusLabel_ != nullptr) {
            dummyFederateStatusLabel_->setText(dummyFederateStatusText(config));
        }
        lastDummyFederateIssue_.clear();
        return;
    }

    if (dummyFederateSharesListenSocket_) {
        dummyFederateSocket_->close();
        dummyFederateBoundAddress_ = QHostAddress();
        dummyFederateBoundPort_ = 0;
        dummyFederateSharesListenSocket_ = false;
    }

    if (dummyFederateSocket_->state() == QAbstractSocket::BoundState
        && dummyFederateBoundAddress_ == dummyFederateBindAddress(config.destinationAddress)
        && dummyFederateBoundPort_ == config.destinationPort) {
        updateDummyFederateMulticastGroup(config.destinationAddress);
        updateSocketOptions(dummyFederateSocket_, config.destinationAddress);
        if (dummyFederateStatusLabel_ != nullptr) {
            dummyFederateStatusLabel_->setText(dummyFederateStatusText(config));
        }
        lastDummyFederateIssue_.clear();
        return;
    }

    dummyFederateSocket_->close();
    joinedDummyFederateMulticastGroup_ = QHostAddress();
    const QHostAddress bindAddress = dummyFederateBindAddress(config.destinationAddress);
    const bool bound = dummyFederateSocket_->bind(bindAddress, config.destinationPort, udpBindMode());
    if (!bound) {
        appendLogOnce(&lastDummyFederateIssue_,
                      QStringLiteral("Dummy federate bind failed on %1:%2: %3")
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
        dummyFederateStatusLabel_->setText(dummyFederateStatusText(config));
    }
    lastDummyFederateIssue_.clear();
    appendLog(QStringLiteral("Dummy federate listening on %1:%2")
                  .arg(config.destinationAddress.toString())
                  .arg(config.destinationPort));
}

auto MainWindow::dummyFederateShouldShareListenSocket(const DisConfig &config) const -> bool
{
    if (config.destinationAddress.isMulticast() || isBroadcastDestination(config.destinationAddress)) {
        return false;
    }

    return dummyFederateBindAddress(config.destinationAddress)
            == effectiveListenAddress(config.listenAddress, config.destinationAddress)
        && config.destinationPort == config.listenPort;
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
    if (socket_->state() != QAbstractSocket::BoundState) {
        appendLog(QStringLiteral("Cannot send %1 request because the listen socket is not bound")
                      .arg(commandName(command)),
                  LogLevel::Error);
        return;
    }
    updateSocketOptions(socket_, config.destinationAddress);
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

    const QHostAddress destinationAddress = effectiveSendAddress(config.destinationAddress);
    const auto written = socket_->writeDatagram(pdu, destinationAddress, config.destinationPort);
    if (written != pdu.size()) {
        appendLog(QStringLiteral("Failed to send %1 request %2: %3")
                      .arg(commandName(command))
                      .arg(requestId)
                      .arg(socket_->errorString()),
                  LogLevel::Error);
        return;
    }

    rememberRequest(requestId, commandName(command));
    QString detail;
    if (command == SimulationCommand::Start) {
        detail = QStringLiteral(
                    ", real-world +%1s (absolute), simulation +%2s (relative)")
                    .arg(config.startRealWorldTimeOffsetSeconds)
                    .arg(config.startSimulationTimeOffsetSeconds);
    } else if (command == SimulationCommand::Pause
            || command == SimulationCommand::Stop
            || command == SimulationCommand::Reset) {
        detail = QStringLiteral(", reason %1")
                    .arg(stopFreezeReasonLabel(
                        stopFreezeReasonForCommand(command)));
    } else if (command == SimulationCommand::Initialize) {
        detail = QStringLiteral(", action %1")
                    .arg(config.initializeActionId);
    }
    appendLog(QStringLiteral("Sent %1 request %2 to %3:%4 (%5 bytes%6)")
                  .arg(commandName(command))
                  .arg(requestId)
                  .arg(destinationAddress.toString())
                  .arg(config.destinationPort)
                  .arg(pdu.size())
                  .arg(detail));
    appendMessageRow(pdu, destinationAddress, config.destinationPort, QStringLiteral("Tx"));
}

void MainWindow::readDatagrams()
{
    while (socket_->hasPendingDatagrams()) {
        QByteArray datagram;
        const qint64 pendingSize = socket_->pendingDatagramSize();
        if (pendingSize < 0) {
            appendLog(QStringLiteral("Could not determine pending datagram size: %1").arg(socket_->errorString()),
                      LogLevel::Warn);
            return;
        }

        datagram.resize(static_cast<int>(pendingSize));
        QHostAddress sender;
        quint16 senderPort = 0;
        const qint64 bytesRead = socket_->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        if (bytesRead < 0) {
            appendLog(QStringLiteral("Failed to read datagram from listen socket: %1").arg(socket_->errorString()),
                      LogLevel::Warn);
            continue;
        }
        if (bytesRead != datagram.size()) {
            appendLog(QStringLiteral("Read %1 of %2 bytes from %3")
                          .arg(bytesRead)
                          .arg(datagram.size())
                          .arg(peerString(sender, senderPort)),
                      LogLevel::Warn);
            datagram.resize(static_cast<int>(bytesRead));
        }
        const QList<EntityId> federateIds = testFederateIdsForRequest(datagram);
        if (dummyFederateEnabled_ && dummyFederateSharesListenSocket_
            && !federateIds.isEmpty()) {
            appendMessageRow(datagram, sender, senderPort, QStringLiteral("Test Rx"));
            for (const EntityId &federateId : federateIds) {
                respondFromDummyFederate(datagram, sender, senderPort, federateId);
            }
            continue;
        }
        recordResponse(datagram, sender, senderPort);
    }
}

void MainWindow::readDummyFederateDatagrams()
{
    while (dummyFederateSocket_->hasPendingDatagrams()) {
        QByteArray datagram;
        const qint64 pendingSize = dummyFederateSocket_->pendingDatagramSize();
        if (pendingSize < 0) {
            appendLog(QStringLiteral("Dummy federate could not determine pending datagram size: %1")
                          .arg(dummyFederateSocket_->errorString()),
                      LogLevel::Warn);
            return;
        }

        datagram.resize(static_cast<int>(pendingSize));
        QHostAddress sender;
        quint16 senderPort = 0;
        const qint64 bytesRead =
            dummyFederateSocket_->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        if (bytesRead < 0) {
            appendLog(QStringLiteral("Dummy federate failed to read datagram: %1")
                          .arg(dummyFederateSocket_->errorString()),
                      LogLevel::Warn);
            continue;
        }
        if (bytesRead != datagram.size()) {
            appendLog(QStringLiteral("Dummy federate read %1 of %2 bytes from %3")
                          .arg(bytesRead)
                          .arg(datagram.size())
                          .arg(peerString(sender, senderPort)),
                      LogLevel::Warn);
            datagram.resize(static_cast<int>(bytesRead));
        }
        if (datagram.size() > PduTypeOffset) {
            const quint8 pduType = static_cast<quint8>(datagram[PduTypeOffset]);
            if (pduType == EntityStatePdu || pduType == CommentPdu) {
                continue;
            }
        }
        const QList<EntityId> federateIds = testFederateIdsForRequest(datagram);
        if (federateIds.isEmpty()) {
            continue;
        }
        appendMessageRow(datagram, sender, senderPort, QStringLiteral("Test Rx"));
        for (const EntityId &federateId : federateIds) {
            respondFromDummyFederate(datagram, sender, senderPort, federateId);
        }
    }
}

void MainWindow::respondFromDummyFederate(const QByteArray &datagram,
                                          const QHostAddress &sender,
                                          quint16 senderPort,
                                          const EntityId &federateId)
{
    if (datagram.size() > PduTypeOffset) {
        const quint8 pduType = static_cast<quint8>(datagram[PduTypeOffset]);
        if (pduType == EntityStatePdu || pduType == CommentPdu) {
            return;
        }
    }

    bool configOk = false;
    const auto config = currentConfig(&configOk);
    if (configOk) {
        const QStringList warnings = dummyRequestWarnings(datagram, config, federateId);
        if (!warnings.isEmpty()) {
            appendLog(QStringLiteral("Dummy federate received suspicious datagram from %1 (%2 bytes): %3")
                          .arg(peerString(sender, senderPort))
                          .arg(datagram.size())
                          .arg(warnings.join(QStringLiteral("; "))),
                      LogLevel::Warn);
        }
    }

    if (datagram.size() < MinRequestPduLength) {
        appendLog(QStringLiteral("Dummy federate ignored datagram from %1 because it is only %2 bytes")
                      .arg(peerString(sender, senderPort))
                      .arg(datagram.size()),
                  LogLevel::Warn);
        return;
    }

    if (static_cast<quint8>(datagram[PduFamilyOffset]) != SimManagementFamily) {
        appendLog(QStringLiteral("Dummy federate ignored %1 byte datagram from %2 because PDU family %3 is not %4")
                      .arg(datagram.size())
                      .arg(peerString(sender, senderPort))
                      .arg(static_cast<quint8>(datagram[PduFamilyOffset]))
                      .arg(SimManagementFamily),
                  LogLevel::Warn);
        return;
    }

    const auto pduType = static_cast<PduType>(static_cast<quint8>(datagram[PduTypeOffset]));
    if (pduType != StartResumePdu && pduType != StopFreezePdu && pduType != ActionRequestPdu) {
        appendLog(QStringLiteral("Dummy federate ignored %1 from %2 because it is not a request PDU")
                      .arg(pduTypeName(static_cast<quint8>(pduType)), peerString(sender, senderPort)),
                  LogLevel::Warn);
        return;
    }

    const int minimumLength = minimumPduLength(static_cast<quint8>(pduType));
    if (minimumLength > 0 && datagram.size() < minimumLength) {
        appendLog(QStringLiteral("Dummy federate ignored short %1 from %2: %3 bytes, expected at least %4")
                      .arg(pduTypeName(static_cast<quint8>(pduType)))
                      .arg(peerString(sender, senderPort))
                      .arg(datagram.size())
                      .arg(minimumLength),
                  LogLevel::Warn);
        return;
    }

    const quint32 requestId = requestIdFromResponse(datagram, static_cast<quint8>(pduType));
    const EntityId receivingEntity = readEntityId(datagram, TargetEntityOffset);
    if (!entityIdAddresses(receivingEntity, federateId)) {
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
    responseConfig.managerId = federateId;
    responseConfig.targetId = readEntityId(datagram, OriginEntityOffset);

    const QByteArray response = pduType == ActionRequestPdu
        ? makeActionResponsePdu(responseConfig, requestId)
        : makeAcknowledgePdu(responseConfig, requestId, pduType);
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

    appendLog(QStringLiteral("Dummy federate %1 accepted %2 request %3 from %4:%5")
                  .arg(entityIdString(federateId))
                  .arg(pduTypeName(static_cast<quint8>(pduType)))
                  .arg(requestId)
                  .arg(sender.toString())
                  .arg(senderPort));
    appendLog(QStringLiteral("Dummy federate %1 sent %2 response %3 to %4:%5")
                  .arg(entityIdString(federateId))
                  .arg(pduType == ActionRequestPdu ? QStringLiteral("Action") : QStringLiteral("Acknowledge"))
                  .arg(requestId)
                  .arg(responseAddress.toString())
                  .arg(responsePort));
    updateTestFederateState(federateId, stateForAcceptedRequest(static_cast<quint8>(pduType), datagram));
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
    while (responseTable_->rowCount() >= MaxMessageRows) {
        responseTable_->removeRow(0);
    }
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
    const quint8 pduType =
        datagram.size() > PduTypeOffset ? static_cast<quint8>(datagram[PduTypeOffset]) : 0;
    if (pduType == EntityStatePdu || pduType == CommentPdu) {
        if (!appConfig_.heartbeatEnabled) {
            return;
        }

        EntityId heartbeatEntity;
        if (heartbeatEntityId(datagram, currentConfig().managerId, &heartbeatEntity)) {
            updateHeartbeat(heartbeatEntity);
        }
        return;
    }

    if (!isResponsePduType(pduType)) {
        return;
    }

    const quint32 requestId = requestIdFromResponse(datagram, pduType);
    bool configOk = false;
    const auto config = currentConfig(&configOk);
    if (!requestStates_.contains(requestId)) {
        if (configOk && responseLooksIntendedForManager(datagram, config, currentTargetId())) {
            const QStringList warnings =
                incomingResponseWarnings(datagram, config, currentTargetId(), false);
            appendLog(QStringLiteral("Dropped expected response candidate from %1 (%2 bytes): %3")
                          .arg(peerString(sender, senderPort))
                          .arg(datagram.size())
                          .arg(warnings.join(QStringLiteral("; "))),
                      LogLevel::Warn);
        }
        return;
    }

    if (configOk) {
        const QStringList warnings =
            incomingResponseWarnings(datagram, config, currentTargetId(), true);
        if (!warnings.isEmpty()) {
            appendLog(QStringLiteral("Dropped expected response candidate from %1 (%2 bytes): %3")
                          .arg(peerString(sender, senderPort))
                          .arg(datagram.size())
                          .arg(warnings.join(QStringLiteral("; "))),
                      LogLevel::Warn);
            return;
        }
    }

    appendMessageRow(datagram, sender, senderPort, QStringLiteral("Rx"));
    appendLog(QStringLiteral("Received %1 for %2 from %3:%4")
                  .arg(pduTypeName(pduType))
                  .arg(requestStates_.value(requestId))
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

void MainWindow::appendLogOnce(QString *lastMessage, const QString &message, LogLevel level)
{
    if (lastMessage != nullptr && *lastMessage == message) {
        return;
    }

    if (lastMessage != nullptr) {
        *lastMessage = message;
    }
    appendLog(message, level);
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

    if (!appConfig_.logDir.isEmpty()) {
        return QDir(appConfig_.logDir).filePath(path);
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
