#include "worldevent.h"

void WorldEventQueue::postSignal(const QString &channel, bool active)
{
    if (channel.isEmpty()) {
        return;
    }

    events_ << WorldEvent {WorldEventType::SetSignal, channel, active};
}

QVector<WorldEvent> WorldEventQueue::takeAll()
{
    QVector<WorldEvent> result;
    result.swap(events_);
    return result;
}

void WorldEventQueue::clear()
{
    events_.clear();
}
