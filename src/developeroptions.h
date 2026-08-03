#pragma once

#include <QString>

struct DeveloperOptions
{
    int level = -1;              // Zero-based campaign level, -1 when unused.
    QString levelFile;
    bool debugOverlay = false;
    bool godMode = false;

    bool hasDirectLevel() const
    {
        return level >= 0 || !levelFile.isEmpty();
    }

    bool developerMode() const
    {
        return hasDirectLevel() || debugOverlay || godMode;
    }
};

DeveloperOptions parseDeveloperOptions();
