#pragma once

#include <QString>
#include <QVector>

enum class WorldEventType {
    SetSignal
};

struct WorldEvent {
    WorldEventType type = WorldEventType::SetSignal;
    QString channel;
    bool active = false;
};

class WorldEventQueue {
public:
    void postSignal(const QString &channel, bool active);
    QVector<WorldEvent> takeAll();
    void clear();

private:
    QVector<WorldEvent> events_;
};
