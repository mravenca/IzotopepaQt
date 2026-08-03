#include "developeroptions.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>

#include <cstdlib>

DeveloperOptions parseDeveloperOptions()
{
    QCommandLineParser parser;
    parser.setApplicationDescription(
        "Izotopepa Complete Edition\n\n"
        "Developer options can be combined, except --level and --level-file.");
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption levelOption(
        {"l", "level"},
        "Start campaign level 1-10 directly.",
        "number");
    const QCommandLineOption levelFileOption(
        "level-file",
        "Load an external JSON level directly.",
        "path");
    const QCommandLineOption debugOption(
        "debug",
        "Start with the F3 debug overlay enabled.");
    const QCommandLineOption godOption(
        "god",
        "Start with player damage disabled.");

    parser.addOption(levelOption);
    parser.addOption(levelFileOption);
    parser.addOption(debugOption);
    parser.addOption(godOption);
    parser.process(*QCoreApplication::instance());

    if (parser.isSet(levelOption) && parser.isSet(levelFileOption)) {
        parser.showHelp(2);
    }

    DeveloperOptions options;
    options.debugOverlay = parser.isSet(debugOption);
    options.godMode = parser.isSet(godOption);

    if (parser.isSet(levelOption)) {
        bool valid = false;
        const int oneBasedLevel = parser.value(levelOption).toInt(&valid);
        if (!valid || oneBasedLevel < 1 || oneBasedLevel > 10) {
            parser.showHelp(2);
        }
        options.level = oneBasedLevel - 1;
    }

    if (parser.isSet(levelFileOption)) {
        options.levelFile = parser.value(levelFileOption);
        if (options.levelFile.trimmed().isEmpty()) {
            parser.showHelp(2);
        }
    }

    return options;
}
