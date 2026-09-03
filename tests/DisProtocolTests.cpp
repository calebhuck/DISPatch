#include <catch2/catch_test_macros.hpp>

#include <DISPatch/Constants.h>
#include <DISPatch/DisProtocol.h>

namespace dispatch {
namespace {

constexpr int HeaderTimestampOffset = 4;
constexpr int StartResumeRealWorldHourOffset = 24;
constexpr int StartResumeRealWorldTimePastHourOffset = 28;
constexpr int StartResumeSimulationHourOffset = 32;
constexpr int StartResumeSimulationTimePastHourOffset = 36;

auto testConfig() -> DisConfig
{
    DisConfig config;
    config.exerciseId = 7;
    config.managerId = EntityId{10, 20, 30};
    config.targetId = EntityId{40, 50, 60};
    config.initializeActionId = InitializeInternalParametersActionId;
    return config;
}

} // namespace

TEST_CASE("Start/Resume PDU uses DIS header fields and entity IDs")
{
    const DisConfig config = testConfig();
    const QByteArray pdu = makeStartResumePdu(config, 0x01020304U);

    REQUIRE(pdu.size() == StartResumePduLength);
    CHECK(static_cast<quint8>(pdu[PduVersionOffset]) == DisVersion);
    CHECK(static_cast<quint8>(pdu[PduExerciseIdOffset]) == 7);
    CHECK(static_cast<quint8>(pdu[PduTypeOffset]) == StartResumePdu);
    CHECK(static_cast<quint8>(pdu[PduFamilyOffset]) == SimManagementFamily);
    CHECK(readU16(pdu, PduLengthOffset) == StartResumePduLength);
    CHECK((readU32(pdu, HeaderTimestampOffset) & DisTimestampAbsoluteBit) == DisTimestampAbsoluteBit);
    CHECK(readEntityId(pdu, OriginEntityOffset).site == 10);
    CHECK(readEntityId(pdu, OriginEntityOffset).application == 20);
    CHECK(readEntityId(pdu, OriginEntityOffset).entity == 30);
    CHECK(readEntityId(pdu, TargetEntityOffset).site == 40);
    CHECK(readEntityId(pdu, TargetEntityOffset).application == 50);
    CHECK(readEntityId(pdu, TargetEntityOffset).entity == 60);
    CHECK(readU32(pdu, StartResumeRequestIdOffset) == 0x01020304U);
}

TEST_CASE("Start/Resume PDU uses absolute real-world and relative simulation time")
{
    const DisConfig config = testConfig();

    const QByteArray pdu = makeStartResumePdu(config, 1);

    REQUIRE(pdu.size() == StartResumePduLength);

    // Real-world time should be populated from UTC.
    CHECK(readU32(pdu, StartResumeRealWorldHourOffset) > 0);

    // LSB == 1 means absolute DIS timestamp.
    CHECK((readU32(pdu, StartResumeRealWorldTimePastHourOffset)
           & DisTimestampAbsoluteBit)
          == DisTimestampAbsoluteBit);

    // Default simulation time is zero.
    CHECK(readU32(pdu, StartResumeSimulationHourOffset) == 0);
    CHECK(readU32(pdu, StartResumeSimulationTimePastHourOffset) == 0);
}

TEST_CASE("Start/Resume PDU uses relative simulation time for non-zero offset")
{
    DisConfig config = testConfig();
    config.startSimulationTimeOffsetSeconds = 20;

    const QByteArray pdu = makeStartResumePdu(config, 1);

    REQUIRE(pdu.size() == StartResumePduLength);

    CHECK(readU32(pdu, StartResumeSimulationHourOffset) == 0);
    CHECK(readU32(pdu, StartResumeSimulationTimePastHourOffset) > 0);

    // LSB == 0 means relative DIS timestamp.
    CHECK((readU32(pdu, StartResumeSimulationTimePastHourOffset)
           & DisTimestampAbsoluteBit)
          == 0);
}

TEST_CASE("Stop/Freeze commands use their standard reasons")
{
    const DisConfig config = testConfig();

    const QByteArray pause = makeStopFreezePdu(config, 1, SimulationCommand::Pause);
    const QByteArray stop = makeStopFreezePdu(config, 2, SimulationCommand::Stop);
    const QByteArray reset = makeStopFreezePdu(config, 3, SimulationCommand::Reset);

    REQUIRE(pause.size() == StopFreezePduLength);
    REQUIRE(stop.size() == StopFreezePduLength);
    REQUIRE(reset.size() == StopFreezePduLength);
    CHECK(static_cast<quint8>(pause[32]) == RecessReason);
    CHECK(static_cast<quint8>(stop[32]) == TerminationReason);
    CHECK(static_cast<quint8>(reset[32]) == StopForResetReason);
    CHECK(requestIdFromResponse(reset, StopFreezePdu) == 3);
}

TEST_CASE("Malformed Start/Resume PDU does not report a bogus request ID")
{
    const DisConfig config = testConfig();
    QByteArray pdu = makeStartResumePdu(config, 0x01020304U);

    pdu.truncate(StopFreezePduLength);

    CHECK(requestIdFromResponse(pdu, StartResumePdu) == 0);
}

TEST_CASE("Simulation requests are matched to their receiving entity")
{
    const DisConfig config = testConfig();
    const QByteArray request = makeStartResumePdu(config, 1);
    const QByteArray response = makeAcknowledgePdu(config, 1, StartResumePdu);

    CHECK(isSimulationRequestForEntity(request, config.targetId));
    CHECK_FALSE(isSimulationRequestForEntity(request, EntityId{1, 2, 3}));
    CHECK_FALSE(isSimulationRequestForEntity(response, config.targetId));
}

TEST_CASE("Simulation request entity matching supports partial wildcards")
{
    DisConfig config = testConfig();
    config.targetId = EntityId{40, BroadcastEntityIdValue, 60};
    const QByteArray request = makeStartResumePdu(config, 1);

    CHECK(isSimulationRequestForEntity(request, EntityId{40, 50, 60}));
    CHECK(isSimulationRequestForEntity(request, EntityId{40, 99, 60}));
    CHECK_FALSE(isSimulationRequestForEntity(request, EntityId{41, 50, 60}));
    CHECK(entityIdAddresses(EntityId{40, BroadcastEntityIdValue, 60}, EntityId{40, 50, 60}));
}

TEST_CASE("Comment PDU identifies its sender as a heartbeat when addressed to the manager")
{
    const DisConfig config = testConfig();
    const QByteArray pdu = makeCommentPdu(config);
    EntityId heartbeatEntity;

    REQUIRE(pdu.size() == CommentPduLength);
    CHECK(static_cast<quint8>(pdu[PduTypeOffset]) == CommentPdu);
    CHECK(static_cast<quint8>(pdu[PduFamilyOffset]) == SimManagementFamily);
    CHECK(readU16(pdu, PduLengthOffset) == CommentPduLength);
    CHECK(heartbeatEntityId(pdu, config.targetId, &heartbeatEntity));
    CHECK(entityIdsMatch(heartbeatEntity, config.managerId));
    CHECK_FALSE(heartbeatEntityId(pdu, EntityId{1, 2, 3}, &heartbeatEntity));
}

TEST_CASE("Broadcast Comment PDU is addressed to the manager")
{
    DisConfig config = testConfig();
    config.targetId = EntityId{BroadcastEntityIdValue,
                               BroadcastEntityIdValue,
                               BroadcastEntityIdValue};
    const QByteArray pdu = makeCommentPdu(config);
    EntityId heartbeatEntity;

    CHECK(heartbeatEntityId(pdu, EntityId{1, 2, 3}, &heartbeatEntity));
    CHECK(entityIdsMatch(heartbeatEntity, config.managerId));
}

TEST_CASE("Entity State PDU identifies its entity as a heartbeat")
{
    const DisConfig config = testConfig();
    QByteArray pdu = makeCommentPdu(config);
    pdu.resize(EntityStatePduLength);
    pdu[PduTypeOffset] = static_cast<char>(EntityStatePdu);
    pdu[PduFamilyOffset] = static_cast<char>(EntityInformationFamily);
    pdu[PduLengthOffset] = static_cast<char>(EntityStatePduLength >> BitsPerByte);
    pdu[PduLengthOffset + 1] = static_cast<char>(EntityStatePduLength & MaxUint8Value);
    EntityId heartbeatEntity;

    CHECK(heartbeatEntityId(pdu, EntityId{1, 2, 3}, &heartbeatEntity));
    CHECK(entityIdsMatch(heartbeatEntity, config.managerId));
}

TEST_CASE("Malformed heartbeat PDUs are rejected")
{
    const DisConfig config = testConfig();
    QByteArray comment = makeCommentPdu(config);
    comment.chop(1);
    EntityId heartbeatEntity;

    CHECK_FALSE(heartbeatEntityId(comment, config.targetId, &heartbeatEntity));
    CHECK_FALSE(heartbeatEntityId(QByteArray(DisHeaderLength, '\0'), config.targetId, &heartbeatEntity));
}

} // namespace dispatch
