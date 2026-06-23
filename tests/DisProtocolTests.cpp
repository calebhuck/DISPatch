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
constexpr int StartResumeRequestIdOffset = 40;

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

TEST_CASE("Start/Resume PDU can use literal zero clock times for zero offsets")
{
    DisConfig config = testConfig();
    config.startUseLiteralZero = true;

    const QByteArray pdu = makeStartResumePdu(config, 1);

    REQUIRE(pdu.size() == StartResumePduLength);
    CHECK(readU32(pdu, StartResumeRealWorldHourOffset) == 0);
    CHECK(readU32(pdu, StartResumeRealWorldTimePastHourOffset) == 0);
    CHECK(readU32(pdu, StartResumeSimulationHourOffset) == 0);
    CHECK(readU32(pdu, StartResumeSimulationTimePastHourOffset) == 0);
}

TEST_CASE("Start/Resume PDU keeps non-zero offsets when useLiteralZero is enabled")
{
    DisConfig config = testConfig();
    config.startRealWorldTimeOffsetSeconds = 10;
    config.startSimulationTimeOffsetSeconds = 20;
    config.startUseLiteralZero = true;

    const QByteArray pdu = makeStartResumePdu(config, 1);

    REQUIRE(pdu.size() == StartResumePduLength);
    CHECK(readU32(pdu, StartResumeRealWorldHourOffset) > 0);
    CHECK(readU32(pdu, StartResumeSimulationHourOffset) > 0);
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

} // namespace dispatch
