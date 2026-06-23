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
    const QString path = directory.filePath(QStringLiteral("DISPatch_config.json"));
    QFile file(path);
    REQUIRE(file.open(QIODevice::WriteOnly));
    REQUIRE(file.write(json) == json.size());
    return path;
}

} // namespace

TEST_CASE("Config loads Start useLiteralZero and offsets")
{
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const QString path = writeConfig(directory, R"json({
  "commands": {
    "start": {
      "realWorldTimeOffsetSeconds": 12,
      "simulationTimeOffsetSeconds": 34,
      "useLiteralZero": true
    }
  }
})json");

    QStringList warnings;
    const AppConfig config = loadAppConfig(path, &warnings);

    CHECK(warnings.isEmpty());
    CHECK(config.configPath == path);
    CHECK(config.startRealWorldTimeOffsetSeconds == 12);
    CHECK(config.startSimulationTimeOffsetSeconds == 34);
    CHECK(config.startUseLiteralZero);
}

TEST_CASE("Config warns and falls back for invalid Start useLiteralZero")
{
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const QString path = writeConfig(directory, R"json({
  "commands": {
    "start": {
      "useLiteralZero": "yes"
    }
  }
})json");

    QStringList warnings;
    const AppConfig config = loadAppConfig(path, &warnings);

    CHECK_FALSE(config.startUseLiteralZero);
    REQUIRE_FALSE(warnings.isEmpty());
    CHECK(warnings.join(QLatin1Char('\n')).contains(QStringLiteral("config.commands.start.useLiteralZero")));
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

TEST_CASE("Config accepts stop/freeze reason labels")
{
    CHECK(stopFreezeReasonLabel(RecessReason) == QStringLiteral("Recess"));
    CHECK(stopFreezeReasonLabel(TerminationReason) == QStringLiteral("Termination"));
    CHECK(stopFreezeReasonLabel(StopForResetReason) == QStringLiteral("Stop For Reset"));
}

} // namespace dispatch
