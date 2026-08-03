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
        // Debug overlay alone is also useful during ordinary campaign play.
        // Direct level selection and God Mode create an isolated developer
        // session that must never write campaign progress.
        return hasDirectLevel() || godMode;
    }
};

DeveloperOptions parseDeveloperOptions();
