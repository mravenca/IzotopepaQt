#include "jumppad.h"

#include <QColor>
#include <QLinearGradient>
#include <QPen>
#include <QtMath>

#include <algorithm>

JumpPad::JumpPad(
    const QRectF &rect,
    double strength,
    double horizontalImpulse,
    double launchDelay,
    double cooldown)
    : rect_(rect),
      strength_(std::max(100.0, strength)),
      horizontalImpulse_(horizontalImpulse),
      launchDelay_(std::clamp(launchDelay, 0.0, 0.35)),
      cooldown_(std::max(0.05, cooldown))
{
}

void JumpPad::update(double dt)
{
    if (state_ == State::Idle) {
        return;
    }

    timer_ = std::max(0.0, timer_ - dt);

    if (timer_ > 0.0) {
        return;
    }

    switch (state_) {
    case State::Compressing:
        launchReady_ = true;
        state_ = State::Releasing;
        timer_ = 0.11;
        break;
    case State::Releasing:
        state_ = State::Cooldown;
        timer_ = cooldown_;
        break;
    case State::Cooldown:
        state_ = State::Idle;
        break;
    case State::Idle:
        break;
    }
}

bool JumpPad::requestTrigger()
{
    if (state_ != State::Idle) {
        return false;
    }

    state_ = State::Compressing;
    timer_ = launchDelay_;
    launchReady_ = launchDelay_ <= 0.0;

    if (launchReady_) {
        state_ = State::Releasing;
        timer_ = 0.11;
    }

    return true;
}

bool JumpPad::consumeLaunch()
{
    if (!launchReady_) {
        return false;
    }

    launchReady_ = false;
    return true;
}

QRectF JumpPad::rect() const
{
    return rect_;
}

QRectF JumpPad::triggerZone() const
{
    return QRectF(
        rect_.left() + 5.0,
        rect_.top() - 8.0,
        std::max(1.0, rect_.width() - 10.0),
        rect_.height() + 14.0);
}

double JumpPad::strength() const
{
    return strength_;
}

double JumpPad::horizontalImpulse() const
{
    return horizontalImpulse_;
}

bool JumpPad::coolingDown() const
{
    return state_ != State::Idle;
}

void JumpPad::draw(QPainter &painter, double cameraX) const
{
    QRectF body = rect_.translated(-cameraX, 0.0);

    double compression = 0.0;
    if (state_ == State::Compressing) {
        const double duration = std::max(0.001, launchDelay_);
        compression = 1.0 - std::clamp(timer_ / duration, 0.0, 1.0);
    } else if (state_ == State::Releasing) {
        compression = std::clamp(timer_ / 0.11, 0.0, 1.0);
    }

    const double squash = 7.0 * compression;
    body.setTop(body.top() + squash);

    QLinearGradient gradient(body.topLeft(), body.bottomLeft());
    gradient.setColorAt(0.0, QColor(255, 225, 70));
    gradient.setColorAt(0.45, QColor(235, 120, 35));
    gradient.setColorAt(1.0, QColor(125, 50, 25));

    painter.save();
    painter.setPen(QPen(QColor(65, 35, 25), 3));
    painter.setBrush(gradient);
    painter.drawRoundedRect(body, 6, 6);

    painter.setPen(QPen(QColor(255, 250, 175), 3));
    const double arrowY = body.top() + 5.0;
    const double arrowStep = body.width() / 5.0;
    for (int index = 1; index <= 4; ++index) {
        const double x = body.left() + arrowStep * index;
        painter.drawLine(
            QPointF(x - 5.0, arrowY + 7.0),
            QPointF(x, arrowY + 2.0));
        painter.drawLine(
            QPointF(x, arrowY + 2.0),
            QPointF(x + 5.0, arrowY + 7.0));
    }

    if (state_ == State::Compressing) {
        const int alpha = 90 + int(100.0 * compression);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 235, 90, alpha));
        painter.drawEllipse(body.center(), 22.0, 8.0);
    }

    painter.restore();
}
