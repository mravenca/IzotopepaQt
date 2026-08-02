#pragma once

#include <QPainter>
#include <QRectF>
#include <QString>

enum class FallingPlatformState {
    Idle,
    Warning,
    Falling,
    WaitingToRespawn
};

struct FallingPlatformUpdate {
    bool armed = false;
    bool beganFalling = false;
    bool respawned = false;
};

class FallingPlatform {
public:
    FallingPlatform(
        const QRectF &rect = {},
        const QString &material = "stone",
        double confirmationTime = 0.15,
        double fallDelay = -1.0,
        double respawnDelay = 4.0);

    FallingPlatformUpdate update(
        double dt,
        bool occupied,
        bool respawnAreaClear,
        double worldHeight);

    void draw(QPainter &painter, double cameraX) const;

    const QRectF &rect() const;
    const QRectF &startRect() const;
    FallingPlatformState state() const;
    bool isSolid() const;
    QString material() const;
    double remainingTime() const;

private:
    static double defaultFallDelay(const QString &material);
    static double defaultGravity(const QString &material);
    double warningWindow() const;

    QRectF startRect_;
    QRectF rect_;
    QString material_ = "stone";
    FallingPlatformState state_ = FallingPlatformState::Idle;

    double confirmationTime_ = 0.15;
    double fallDelay_ = 8.0;
    double respawnDelay_ = 4.0;
    double gravity_ = 1500.0;

    double contactTime_ = 0.0;
    double remainingTime_ = 8.0;
    double verticalVelocity_ = 0.0;
    double respawnTimer_ = 0.0;
    double animationTime_ = 0.0;
};
