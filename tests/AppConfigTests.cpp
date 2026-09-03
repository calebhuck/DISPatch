#include <catch2/catch_test_macros.hpp>

#include <DISPatch/AppConfig.h>
#include <DISPatch/Constants.h>
#include <DISPatch/DisProtocol.h>

#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>

namespace dispatch {
namespace {

auto writeConfig(QTemporaryDir &directory, const QByteArray &json) -> QString
{
    const QString path = directory.filePath(QStringLiteral("dispatch.json"));
    QFile file(path);
    REQUIRE(file.open(QIODevice::WriteOnly));
    REQUIRE(file.write(json) == json.size());
    return path;
}

} // namespace

TEST_CASE("Config loads Start offsets")
{
    QTemporaryDir directory;
    REQUIRE(directory.isValid());

    const QString path = writeConfig(directory, R"json({
  "commands": {
    "start": {
      "realWorldTimeOffsetSeconds": 12,
      "simulationTimeOffsetSeconds": 34
    }
  }
})json");

    QStringList warnings;
    const AppConfig config = loadAppConfig(path, &warnings);

    CHECK(warnings.isEmpty());
    CHECK(config.configPath == path);
    CHECK(config.startRealWorldTimeOffsetSeconds == 12);
    CHECK(config.startSimulationTimeOffsetSeconds == 34);
}

TEST_CASE("Config accepts added theme names")
{
    QTemporaryDir directory;
    REQUIRE(directory.isValid());

    QStringList warnings;
    const AppConfig oneDark = loadAppConfig(writeConfig(directory, R"json({"theme": "onedark"})json"), &warnings);
    CHECK(warnings.isEmpty());
    CHECK(oneDark.theme == Theme::OneDark);

    warnings.clear();
    const AppConfig vsCode = loadAppConfig(writeConfig(directory, R"json({"theme": "vscode"})json"), &warnings);
    CHECK(warnings.isEmpty());
    CHECK(vsCode.theme == Theme::VsCodeDefault);

    warnings.clear();
    const AppConfig tokyoNight =
        loadAppConfig(writeConfig(directory, R"json({"theme": "tokyo-night"})json"), &warnings);
    CHECK(warnings.isEmpty());
    CHECK(tokyoNight.theme == Theme::TokyoNight);

    warnings.clear();
    const AppConfig dracula = loadAppConfig(writeConfig(directory, R"json({"theme": "dracula"})json"), &warnings);
    CHECK(warnings.isEmpty());
    CHECK(dracula.theme == Theme::Dracula);
}

TEST_CASE("Config accepts exercise ID zero")
{
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const QString path = writeConfig(directory, R"json({
  "dis": {
    "exerciseId": 0
  }
})json");

    QStringList warnings;
    const AppConfig config = loadAppConfig(path, &warnings);

    CHECK(warnings.isEmpty());
    CHECK(config.exerciseId == 0);
}

TEST_CASE("Config defaults multicast loopback off")
{
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const QString path = writeConfig(directory, R"json({})json");

    QStringList warnings;
    const AppConfig config = loadAppConfig(path, &warnings);

    CHECK(warnings.isEmpty());
    CHECK_FALSE(config.multicastLoopback);
}

TEST_CASE("Config loads multiple test federates")
{
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const QString path = writeConfig(directory, R"json({
  "testFederate": {
    "enabled": true,
    "entityIds": [
      {"site": 1, "application": 1, "entity": 2},
      {"site": 1, "application": 1, "entity": 3}
    ]
  }
})json");

    QStringList warnings;
    const AppConfig config = loadAppConfig(path, &warnings);

    CHECK(warnings.isEmpty());
    CHECK(config.testFederateEnabled);
    REQUIRE(config.testFederateIds.size() == 2);
    CHECK(config.testFederateIds.at(0).entity == 2);
    CHECK(config.testFederateIds.at(1).entity == 3);
}

TEST_CASE("Config rejects invalid test federate list")
{
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const QString path = writeConfig(directory, R"json({
  "testFederate": {
    "entityIds": "1:1:1"
  }
})json");

    QStringList warnings;
    const AppConfig config = loadAppConfig(path, &warnings);

    REQUIRE(config.testFederateIds.size() == 1);
    CHECK(config.testFederateIds.first().entity == 0);
    CHECK(warnings.join(QLatin1Char('\n')).contains(QStringLiteral("config.testFederate.entityIds")));
}

TEST_CASE("Config loads heartbeat settings")
{
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const QString path = writeConfig(directory, R"json({
  "heartbeat": {
    "enabled": true,
    "timeout": 12
  }
})json");

    QStringList warnings;
    const AppConfig config = loadAppConfig(path, &warnings);

    CHECK(warnings.isEmpty());
    CHECK(config.heartbeatEnabled);
    CHECK(config.heartbeatTimeoutSeconds == 12);
}

TEST_CASE("Config rejects an invalid heartbeat timeout")
{
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const QString path = writeConfig(directory, R"json({
  "heartbeat": {
    "enabled": true,
    "timeout": 0
  }
})json");

    QStringList warnings;
    const AppConfig config = loadAppConfig(path, &warnings);

    CHECK(config.heartbeatEnabled);
    CHECK(config.heartbeatTimeoutSeconds == DefaultHeartbeatTimeoutSeconds);
    CHECK(warnings.join(QLatin1Char('\n')).contains(QStringLiteral("config.heartbeat.timeout")));
}

TEST_CASE("Config rejects oversized integer values")
{
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const QString path = writeConfig(directory, R"json({
  "network": {
    "destinationPort": 1000000000000
  }
})json");

    QStringList warnings;
    const AppConfig config = loadAppConfig(path, &warnings);

    CHECK(config.destinationPort == 3000);
    CHECK(warnings.join(QLatin1Char('\n')).contains(QStringLiteral("config.network.destinationPort")));
}

TEST_CASE("Command line config path takes precedence and network flags override config")
{
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const QString selectedPath = writeConfig(directory, R"json({
  "network": {
    "destinationAddress": "192.0.2.1",
    "destinationPort": 3001,
    "listenPort": 3000,
    "multicastGroupAddress": "239.9.9.9",
    "shareAddress": true,
    "reuseAddress": false,
    "joinMulticast": true,
    "multicastLoopback": false
  }
})json");
    const QString ignoredPath = directory.filePath(QStringLiteral("ignored.json"));
    QFile ignoredFile(ignoredPath);
    REQUIRE(ignoredFile.open(QIODevice::WriteOnly));
    REQUIRE(ignoredFile.write(R"json({
  "network": {
    "destinationAddress": "198.51.100.9"
    }
})json") > 0);
    ignoredFile.close();

    QStringList warnings;
    const AppConfig config = loadAppConfig(QStringList{QStringLiteral("dispatch"),
                                                       QStringLiteral("--config"),
                                                       selectedPath,
                                                       QStringLiteral("--config=%1").arg(ignoredPath),
                                                       QStringLiteral("--multicast-group"),
                                                       QStringLiteral("239.2.3.4"),
                                                       QStringLiteral("--no-share-address"),
                                                       QStringLiteral("--reuse-address"),
                                                       QStringLiteral("--no-join-multicast"),
                                                       QStringLiteral("--multicast-loopback"),
                                                       QStringLiteral("--log-dir=%1").arg(directory.path())},
                                           &warnings);

    CHECK(config.configPath == ignoredPath);
    CHECK(config.destinationAddress == QStringLiteral("198.51.100.9"));
    CHECK(config.multicastGroupAddress == QStringLiteral("239.2.3.4"));
    CHECK_FALSE(config.shareAddress);
    CHECK(config.reuseAddress);
    CHECK_FALSE(config.joinMulticast);
    CHECK(config.multicastLoopback);
    CHECK(config.logDir == directory.path());
}

TEST_CASE("Config accepts stop/freeze reason labels")
{
    CHECK(stopFreezeReasonLabel(RecessReason) == QStringLiteral("Recess"));
    CHECK(stopFreezeReasonLabel(TerminationReason) == QStringLiteral("Termination"));
    CHECK(stopFreezeReasonLabel(StopForResetReason) == QStringLiteral("Stop For Reset"));
}

} // namespace dispatch
