#pragma once

#include <QPainter>
#include <QRectF>

class JumpPad {
public:
    JumpPad() = default;
    JumpPad(
        const QRectF &rect,
        double strength,
        double horizontalImpulse,
        double launchDelay,
        double cooldown);

    void update(double dt);
    bool requestTrigger();
    bool consumeLaunch();

    QRectF rect() const;
    QRectF triggerZone() const;
    double strength() const;
    double horizontalImpulse() const;
    bool coolingDown() const;

    void draw(QPainter &painter, double cameraX) const;

private:
    enum class State {
        Idle,
        Compressing,
        Releasing,
        Cooldown
    };

    QRectF rect_;
    double strength_ = 900.0;
    double horizontalImpulse_ = 0.0;
    double launchDelay_ = 0.08;
    double cooldown_ = 0.25;
    double timer_ = 0.0;
    bool launchReady_ = false;
    State state_ = State::Idle;
};
