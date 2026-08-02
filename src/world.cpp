#include "world.h"

#include <QApplication>
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QSettings>
#include <QtMath>

#include <algorithm>

namespace {
constexpr double kPlatformCapWidth = 12.0;
constexpr double kLadderRailWidth = 5.0;
constexpr double kLadderRungSpacing = 14.0;

void drawStaticPlatform(QPainter &painter, const QRectF &rect)
{
    const bool ground = rect.height() >= 60.0;
    const QColor body = ground ? QColor(70, 110, 50) : QColor(110, 78, 45);
    const QColor top = ground ? QColor(120, 175, 75) : QColor(180, 135, 75);
    const QColor edge = ground ? QColor(52, 86, 39) : QColor(82, 55, 31);
    const QColor detail = ground ? QColor(83, 132, 59) : QColor(137, 96, 52);

    painter.fillRect(rect, body);
    painter.fillRect(QRectF(rect.left(), rect.top(), rect.width(), qMin(8.0, rect.height())), top);

    const double cap = qMin(kPlatformCapWidth, rect.width() / 2.0);
    painter.fillRect(QRectF(rect.left(), rect.top(), cap, rect.height()), edge);
    painter.fillRect(QRectF(rect.right() - cap, rect.top(), cap, rect.height()), edge);

    painter.setPen(QPen(detail, 2.0));
    for (double x = rect.left() + cap + 10.0; x < rect.right() - cap; x += 30.0) {
        painter.drawLine(QPointF(x, rect.top() + 13.0),
                         QPointF(x + 8.0, rect.top() + 13.0));
    }
}

void drawLadder(QPainter &painter, const QRectF &rect)
{
    if (rect.width() <= 0.0 || rect.height() <= 0.0) {
        return;
    }

    const QColor darkWood(95, 56, 27);
    const QColor wood(151, 93, 43);
    const QColor highlight(221, 165, 91);

    const double railInset = qMax(2.0, rect.width() * 0.15);
    const QRectF leftRail(rect.left() + railInset, rect.top(), kLadderRailWidth, rect.height());
    const QRectF rightRail(rect.right() - railInset - kLadderRailWidth,
                           rect.top(), kLadderRailWidth, rect.height());

    painter.fillRect(leftRail.adjusted(-1.0, 0.0, 1.0, 0.0), darkWood);
    painter.fillRect(rightRail.adjusted(-1.0, 0.0, 1.0, 0.0), darkWood);
    painter.fillRect(leftRail, wood);
    painter.fillRect(rightRail, wood);

    painter.setPen(QPen(darkWood, 6.0, Qt::SolidLine, Qt::SquareCap));
    for (double y = rect.top() + 7.0; y <= rect.bottom(); y += kLadderRungSpacing) {
        painter.drawLine(QPointF(leftRail.center().x(), y),
                         QPointF(rightRail.center().x(), y));
    }

    painter.setPen(QPen(highlight, 2.0, Qt::SolidLine, Qt::SquareCap));
    for (double y = rect.top() + 6.0; y <= rect.bottom(); y += kLadderRungSpacing) {
        painter.drawLine(QPointF(leftRail.center().x(), y),
                         QPointF(rightRail.center().x(), y));
    }
}

void drawMovingPlatform(QPainter &painter, const QRectF &rect)
{
    if (rect.width() <= 0.0 || rect.height() <= 0.0) {
        return;
    }

    const QColor outline(54, 45, 67);
    const QColor endCap(77, 62, 94);
    const QColor middle(111, 86, 132);
    const QColor top(188, 153, 220);
    const QColor seam(81, 62, 99);

    painter.fillRect(rect, outline);
    const QRectF inner = rect.adjusted(2.0, 2.0, -2.0, -2.0);
    painter.fillRect(inner, middle);

    const double cap = qMin(14.0, inner.width() / 2.0);
    painter.fillRect(QRectF(inner.left(), inner.top(), cap, inner.height()), endCap);
    painter.fillRect(QRectF(inner.right() - cap, inner.top(), cap, inner.height()), endCap);
    painter.fillRect(QRectF(inner.left(), inner.top(), inner.width(), qMin(6.0, inner.height())), top);

    painter.setPen(QPen(seam, 2.0));
    for (double x = inner.left() + cap + 18.0; x < inner.right() - cap; x += 28.0) {
        painter.drawLine(QPointF(x, inner.top() + 8.0),
                         QPointF(x, inner.bottom() - 4.0));
    }

    painter.setBrush(QColor(215, 190, 230));
    painter.setPen(QPen(outline, 1.0));
    painter.drawEllipse(QPointF(inner.left() + cap / 2.0, inner.center().y()), 2.5, 2.5);
    painter.drawEllipse(QPointF(inner.right() - cap / 2.0, inner.center().y()), 2.5, 2.5);
}
}

World::World(const SpriteSheet *playerSheet, const SpriteSheet *enemySheet)
    : playerSheet_(playerSheet),
      enemySheet_(enemySheet),
      player_(playerSheet)
{
}

bool World::loadLevel(int index)
{
    levelIndex_ = std::clamp(index, 0, 2);
    if (!level_.load(QString("level%1.json").arg(levelIndex_ + 1))) {
        return false;
    }

    player_.reset(level_.playerSpawn(), true);
    respawn_ = level_.playerSpawn();

    enemies_.clear();
    for (const auto &spawn : level_.enemies()) {
        enemies_ << Enemy(enemySheet_, spawn.kind, spawn.position,
                          spawn.leftLimit, spawn.rightLimit);
    }

    moving_.clear();
    for (const auto &spawn : level_.moving()) {
        moving_ << MovingPlatform{spawn.rect, spawn.minX, spawn.maxX,
                                  spawn.speedX, spawn.speedY, 1, {}};
    }

    coins_.clear();
    for (const auto &position : level_.coins()) {
        coins_ << Coin{QRectF(position.x(), position.y(), 24, 24),
                       false, position.x() * .01};
    }

    pickups_.clear();
    for (const auto &spawn : level_.pickups()) {
        pickups_ << Pickup{spawn.kind,
                           QRectF(spawn.position.x(), spawn.position.y(), 28, 28),
                           false};
    }

    doors_.clear();
    for (const auto &spawn : level_.doors()) {
        doors_ << Door{spawn.key, spawn.rect, false};
    }

    switches_.clear();
    for (const auto &spawn : level_.switches()) {
        switches_ << SwitchObj{spawn.key,
                               QRectF(spawn.position.x(), spawn.position.y(), 32, 25),
                               false};
    }

    keys_.clear();
    for (const auto &spawn : level_.keys()) {
        keys_ << KeyObj{spawn.key,
                        QRectF(spawn.position.x(), spawn.position.y(), 26, 18),
                        false};
    }

    crates_.clear();
    for (const auto &spawn : level_.crates()) {
        crates_ << Crate {
            QRectF(spawn.position.x(), spawn.position.y(), 48, 48),
            spawn.drop,
            spawn.hp,
            true
        };
    }

    projectiles_.clear();
    particles_.clear();
    completed_ = false;
    gameOver_ = false;
    message_ = level_.name();
    messageTime_ = 2.5;
    rebuildCollision();
    return true;
}

void World::resetFromCheckpoint()
{
    player_.reset(respawn_, false);
    gameOver_ = false;
    projectiles_.clear();
    message_ = "Checkpoint restart";
    messageTime_ = 1.5;
}

void World::rebuildCollision()
{
    collision_ = level_.platforms();
    for (const auto &platform : moving_) {
        collision_ << platform.rect;
    }
    for (const auto &door : doors_) {
        if (!door.open) {
            collision_ << door.rect;
        }
    }
    for (const auto &crate : crates_) {
        if (crate.alive) {
            collision_ << crate.rect;
        }
    }
}

void World::update(double dt)
{
    if (completed_ || gameOver_) {
        return;
    }
    animationTime_ += dt;
    shakeTime_ = std::max(0.0, shakeTime_ - dt);
    if (shakeTime_ <= 0.0) {
        shakeStrength_ = 0.0;
    }

    // Move platforms before ordinary player physics. A player standing on
    // a platform's previous top surface receives the same displacement.
    for (int index = 0; index < moving_.size(); ++index) {
        MovingPlatform &platform = moving_[index];

        const QRectF oldRect = platform.rect;
        const QPointF oldPosition = oldRect.topLeft();

        const QRectF playerFeet(
            player_.rect().left() + 5.0,
            player_.rect().bottom() - 4.0,
            player_.rect().width() - 10.0,
            8.0);

        const bool riding =
            playerFeet.intersects(oldRect)
            && player_.rect().bottom() <= oldRect.top() + 5.0;

        platform.rect.translate(
            platform.speedX * platform.dir * dt,
            platform.speedY * platform.dir * dt);

        const bool outsideHorizontal =
            platform.speedX != 0
            && (platform.rect.left() < platform.minX
                || platform.rect.left() > platform.maxX);

        const bool outsideVertical =
            platform.speedY != 0
            && (platform.rect.top() < 250
                || platform.rect.top() > 520);

        if (outsideHorizontal || outsideVertical) {
            platform.dir *= -1;
            platform.rect.moveTopLeft(oldPosition);
        }

        platform.delta =
            platform.rect.topLeft() - oldPosition;

        if (riding && !platform.delta.isNull()) {
            QVector<QRectF> blockers = level_.platforms();

            for (const Door &door : doors_) {
                if (!door.open) {
                    blockers << door.rect;
                }
            }

            for (int other = 0; other < moving_.size(); ++other) {
                if (other != index) {
                    blockers << moving_[other].rect;
                }
            }

            if (!player_.carryBy(platform.delta, blockers)) {
                player_.damage(platform.rect.center().x());
                platform.dir *= -1;
                platform.rect = oldRect;
                platform.delta = {};
            }
        }
    }
    rebuildCollision();
    player_.update(dt, collision_, level_.ladders(), level_.worldSize().width());
    for (auto &enemy : enemies_) {
        enemy.update(
            dt,
            collision_,
            player_.rect().center(),
            projectiles_);
    }

    for (int first = 0; first < enemies_.size(); ++first) {
        if (!enemies_[first].alive()) {
            continue;
        }

        for (int second = first + 1; second < enemies_.size(); ++second) {
            if (!enemies_[second].alive()) {
                continue;
            }

            const QRectF firstRect = enemies_[first].rect();
            const QRectF secondRect = enemies_[second].rect();

            enemies_[first].separateFrom(secondRect);
            enemies_[second].separateFrom(firstRect);
        }
    }

    for (auto &coin : coins_) {
        coin.phase += dt * 4;
        if (!coin.collected && player_.rect().intersects(coin.rect)) {
            coin.collected = true;
            player_.addScore(10);
            explode(coin.rect.center(), Qt::yellow);
            beep();
        }
    }

    for (auto &pickup : pickups_) {
        if (!pickup.taken && player_.rect().intersects(pickup.rect)) {
            pickup.taken = true;
            if (pickup.kind == "health") {
                player_.heal(2);
            } else {
                player_.addAmmo(12);
            }
            message_ = pickup.kind == "health" ? "Health restored" : "Ammo +12";
            messageTime_ = 1.5;
            beep();
        }
    }

    for (auto &key : keys_) {
        if (!key.taken && player_.rect().intersects(key.rect)) {
            key.taken = true;
            player_.addKey(key.key);
            message_ = key.key + " key acquired";
            messageTime_ = 2;
            beep();
        }
    }

    for (auto &door : doors_) {
        if (!door.open && player_.hasKey(door.key)
            && player_.rect().adjusted(-8, -8, 8, 8).intersects(door.rect)) {
            door.open = true;
            message_ = door.key + " door unlocked";
            messageTime_ = 2;
            beep();
            rebuildCollision();
        }
    }

    for (const auto &spike : level_.spikes()) {
        if (player_.rect().intersects(spike)) {
            player_.damage(spike.center().x());
        }
    }

    for (auto &projectile : projectiles_) {
        if (!projectile.alive) {
            continue;
        }

        projectile.life -= dt;
        projectile.rect.translate(projectile.velocity.x() * dt,
                                  projectile.velocity.y() * dt);

        bool hitCrate = false;

        if (!projectile.hostile) {
            for (auto &crate : crates_) {
                if (!crate.alive || !projectile.rect.intersects(crate.rect)) {
                    continue;
                }

                projectile.alive = false;
                hitCrate = true;
                --crate.hp;

                explode(projectile.rect.center(), QColor(225, 165, 80));

                if (crate.hp <= 0) {
                    crate.alive = false;
                    player_.addScore(25);

                    for (int fragment = 0; fragment < 3; ++fragment) {
                        explode(
                            crate.rect.center()
                                + QPointF((fragment - 1) * 10, -fragment * 4),
                            QColor(145, 85, 38));
                    }

                    const QPointF dropPosition(
                        crate.rect.center().x() - 12,
                        crate.rect.top() - 28);

                    if (crate.drop == "coin") {
                        coins_ << Coin {
                            QRectF(dropPosition.x(), dropPosition.y(), 24, 24),
                            false,
                            dropPosition.x() * .01
                        };
                    } else if (crate.drop == "health"
                               || crate.drop == "ammo") {
                        pickups_ << Pickup {
                            crate.drop,
                            QRectF(dropPosition.x(), dropPosition.y(), 28, 28),
                            false
                        };
                    }

                    message_ = crate.drop == "none"
                        ? "Crate destroyed"
                        : "Crate dropped " + crate.drop;
                    messageTime_ = 1.2;

                    rebuildCollision();
                    beep();
                }

                break;
            }
        }

        if (hitCrate) {
            continue;
        }

        if (projectile.life <= 0 || intersectsAny(projectile.rect, collision_)) {
            projectile.alive = false;
            explode(projectile.rect.center(), Qt::lightGray);
            continue;
        }

        if (projectile.hostile) {
            if (projectile.rect.intersects(player_.rect())) {
                player_.damage(projectile.rect.center().x());
                shakeTime_ = std::max(shakeTime_, 0.18);
                shakeStrength_ = std::max(shakeStrength_, 5.0);
                projectile.alive = false;
                explode(projectile.rect.center(), Qt::red);
            }
        } else {
            for (auto &enemy : enemies_) {
                if (enemy.alive() && projectile.rect.intersects(enemy.rect())) {
                    enemy.damage(1, projectile.rect.center().x());
                    shakeTime_ = std::max(shakeTime_, 0.10);
                    shakeStrength_ = std::max(shakeStrength_, 3.0);
                    projectile.alive = false;
                    explode(projectile.rect.center(), Qt::yellow);
                    if (!enemy.alive()) {
                        player_.addScore(enemy.reward());
                        explode(enemy.rect().center(), Qt::red);
                        explode(
                            enemy.rect().center() + QPointF(10, -12),
                            QColor(255, 170, 40));
                        shakeTime_ = std::max(shakeTime_, 0.30);
                        shakeStrength_ = std::max(shakeStrength_, 8.0);
                        beep();
                    }
                    break;
                }
            }
        }
    }

    for (auto &particle : particles_) {
        particle.life -= dt;
        particle.pos += particle.velocity.toPointF() * dt;
        particle.velocity.setY(particle.velocity.y() + 400 * dt);
    }

    projectiles_.erase(std::remove_if(projectiles_.begin(), projectiles_.end(),
                                      [](const Projectile &p) { return !p.alive; }),
                       projectiles_.end());
    particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
                                    [](const Particle &p) { return p.life <= 0; }),
                     particles_.end());

    if (level_.checkpoint().x() >= 0
        && player_.rect().center().x() > level_.checkpoint().x()
        && respawn_ != level_.checkpoint()) {
        respawn_ = level_.checkpoint();
        checkpoint();
    }

    if (player_.rect().intersects(level_.goal())) {
        completed_ = true;
        message_ = "Level complete!";
        QSettings settings("OpenAI", "Izotopepa");
        settings.setValue("unlockedLevel",
                          std::max(settings.value("unlockedLevel", 0).toInt(),
                                   levelIndex_ + 1));
        beep();
    }

    if (player_.dead() || player_.rect().top() > level_.worldSize().height() + 100) {
        gameOver_ = true;
    }

    if (messageTime_ > 0) {
        messageTime_ -= dt;
    } else {
        message_.clear();
    }
}

void World::setInput(bool left, bool right, bool up, bool down)
{
    player_.setLeft(left);
    player_.setRight(right);
    player_.setUp(up);
    player_.setDown(down);
}

void World::jump()
{
    if (!completed_ && !gameOver_) {
        player_.jump();
    }
}

void World::stopJump()
{
    if (!completed_ && !gameOver_) {
        player_.stopJump();
    }
}

void World::shoot()
{
    if (!completed_ && !gameOver_ && player_.canShoot()) {
        projectiles_ << player_.shoot();
        shakeTime_ = std::max(shakeTime_, 0.08);
        shakeStrength_ = std::max(shakeStrength_, 2.0);
        beep();
    }
}

void World::interact()
{
    for (auto &switchObject : switches_) {
        if (!switchObject.active
            && player_.rect().adjusted(-25, -10, 25, 10).intersects(switchObject.rect)) {
            switchObject.active = true;
            for (auto &door : doors_) {
                if (door.key == switchObject.key) {
                    door.open = true;
                }
            }
            message_ = "Switch activated";
            messageTime_ = 1.5;
            rebuildCollision();
            beep();
        }
    }
}

void World::explode(QPointF at, QColor color)
{
    for (int i = 0; i < 10; ++i) {
        const double angle = QRandomGenerator::global()->generateDouble() * 6.283;
        const double speed = 60 + QRandomGenerator::global()->bounded(170);
        particles_ << Particle{at,
                               QVector2D(qCos(angle) * speed, qSin(angle) * speed),
                               .35 + QRandomGenerator::global()->generateDouble() * .45,
                               3 + QRandomGenerator::global()->bounded(5),
                               color};
    }
}

void World::checkpoint()
{
    message_ = "Checkpoint reached";
    messageTime_ = 2;
    beep();
}

void World::beep() const
{
    if (sound_) {
        QApplication::beep();
    }
}

void World::draw(QPainter &painter, double cameraX) const
{
    painter.save();

    const QPointF shakeOffset(
        qSin(shakeTime_ * 91.0) * shakeStrength_,
        qCos(shakeTime_ * 73.0) * shakeStrength_ * 0.65);

    painter.translate(shakeOffset);

    for (const auto &platform : level_.platforms()) {
        drawStaticPlatform(painter, platform.translated(-cameraX, 0));
    }

    for (const auto &ladder : level_.ladders()) {
        drawLadder(painter, ladder.translated(-cameraX, 0));
    }

    for (const auto &platform : moving_) {
        drawMovingPlatform(painter, platform.rect.translated(-cameraX, 0));
    }

    for (const auto &spike : level_.spikes()) {
        const QRectF rect = spike.translated(-cameraX, 0);
        QPolygonF polygon;
        const double step = 20;
        for (double x = rect.left(); x < rect.right(); x += step) {
            polygon << QPointF(x, rect.bottom())
                    << QPointF(x + step / 2, rect.top())
                    << QPointF(x + step, rect.bottom());
        }
        painter.setBrush(Qt::darkGray);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(polygon);
    }

    for (const auto &coin : coins_) {
        if (coin.collected) {
            continue;
        }

        const QPointF center(coin.rect.center().x() - cameraX,
                             coin.rect.center().y() + qSin(coin.phase) * 4);
        const double halfWidth = 2.5 + 8.5 * qAbs(qCos(coin.phase));
        const QRectF coinRect(center.x() - halfWidth, center.y() - 11,
                              halfWidth * 2, 22);

        painter.setBrush(QColor(255, 220, 35));
        painter.setPen(QPen(QColor(180, 120, 0), 3));
        painter.drawEllipse(coinRect);

        if (halfWidth > 5.0) {
            painter.setPen(QPen(QColor(255, 245, 150), 2));
            painter.drawArc(coinRect.adjusted(4, 3, -4, -3), 70 * 16, 80 * 16);
        }
    }

    for (int index = 0; index < pickups_.size(); ++index) {
        const Pickup &pickup = pickups_[index];
        if (pickup.taken) {
            continue;
        }

        QRectF rect = pickup.rect.translated(-cameraX, 0);
        rect.translate(0, qSin(animationTime_ * 4.0 + index * 1.7) * 4.0);

        const QColor color = pickup.kind == "health"
            ? QColor(220, 45, 65)
            : QColor(40, 190, 235);

        QRadialGradient glow(rect.center(), 28);
        glow.setColorAt(0.0, QColor(color.red(), color.green(), color.blue(), 100));
        glow.setColorAt(1.0, QColor(color.red(), color.green(), color.blue(), 0));

        painter.setPen(Qt::NoPen);
        painter.setBrush(glow);
        painter.drawEllipse(rect.center(), 28, 28);

        painter.setBrush(color);
        painter.setPen(QPen(Qt::white, 2));
        painter.drawRoundedRect(rect, 6, 6);
        painter.drawText(rect, Qt::AlignCenter, pickup.kind == "health" ? "+" : "A");
    }
    for (int index = 0; index < keys_.size(); ++index) {
        const KeyObj &key = keys_[index];
        if (key.taken) {
            continue;
        }

        const QRectF rect = key.rect.translated(-cameraX, 0);
        const QPointF center(
            rect.center().x(),
            rect.center().y() + qSin(animationTime_ * 3.5 + index) * 4.0);

        const double widthScale =
            0.25 + 0.75 * qAbs(qCos(animationTime_ * 2.8 + index));
        const double halfLength = rect.width() * 0.5 * widthScale;

        painter.setPen(QPen(QColor(255, 220, 50), 4, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(
            QPointF(center.x() - halfLength, center.y()),
            QPointF(center.x() + halfLength, center.y()));
        painter.drawEllipse(
            QPointF(center.x() - halfLength + 3.0, center.y()),
            5.0 * widthScale + 1.0,
            5.0);
    }
    for (const auto &door : doors_) {
        if (door.open) {
            continue;
        }
        const QRectF rect = door.rect.translated(-cameraX, 0);
        painter.fillRect(rect, QColor(90, 55, 25));
        painter.setPen(QPen(QColor(220, 170, 70), 3));
        painter.drawRect(rect);
        painter.drawEllipse(QPointF(rect.right() - 12, rect.center().y()), 3, 3);
    }

    for (int index = 0; index < switches_.size(); ++index) {
        const SwitchObj &switchObject = switches_[index];
        const QRectF rect = switchObject.rect.translated(-cameraX, 0);
        const QColor color = switchObject.active
            ? QColor(60, 225, 90)
            : QColor(170, 45, 45);

        const double glowRadius = 22.0
            + qSin(animationTime_ * 5.0 + index) * 3.0;

        QRadialGradient glow(rect.center(), glowRadius);
        glow.setColorAt(0.0, QColor(color.red(), color.green(), color.blue(), 110));
        glow.setColorAt(1.0, QColor(color.red(), color.green(), color.blue(), 0));

        painter.setPen(Qt::NoPen);
        painter.setBrush(glow);
        painter.drawEllipse(rect.center(), glowRadius, glowRadius);

        painter.setBrush(QColor(45, 45, 55));
        painter.setPen(QPen(QColor(20, 20, 25), 2));
        painter.drawRoundedRect(rect, 4, 4);

        painter.setBrush(color);
        painter.setPen(QPen(Qt::white, 1));
        painter.drawRoundedRect(rect.adjusted(7, 5, -7, -5), 3, 3);
    }

    if (level_.checkpoint().x() >= 0.0) {
        const QPointF base(
            level_.checkpoint().x() - cameraX,
            level_.checkpoint().y());

        painter.setPen(QPen(QColor(65, 55, 45), 5));
        painter.drawLine(base, base + QPointF(0, -72));

        const double wave = qSin(animationTime_ * 4.0) * 4.0;
        QPolygonF flag;
        flag << base + QPointF(2, -70)
             << base + QPointF(42 + wave, -58)
             << base + QPointF(2, -46);

        painter.setBrush(QColor(70, 210, 245));
        painter.setPen(QPen(Qt::white, 2));
        painter.drawPolygon(flag);
    }

    {
        const QRectF goal = level_.goal().translated(-cameraX, 0);
        const QPointF pole(goal.left() + 10.0, goal.bottom());

        painter.setPen(QPen(QColor(45, 45, 50), 6));
        painter.drawLine(pole, QPointF(pole.x(), goal.top()));

        const double wave = qSin(animationTime_ * 4.5) * 5.0;
        QPolygonF flag;
        flag << QPointF(pole.x() + 3.0, goal.top() + 4.0)
             << QPointF(pole.x() + 48.0 + wave, goal.top() + 18.0)
             << QPointF(pole.x() + 3.0, goal.top() + 34.0);

        painter.setBrush(QColor(235, 70, 65));
        painter.setPen(QPen(QColor(255, 225, 180), 2));
        painter.drawPolygon(flag);
    }

    const QColor crateWood(154, 91, 42);
    const QColor crateDark(91, 52, 25);
    const QColor crateLight(213, 146, 72);

    for (const auto &crate : crates_) {
        if (!crate.alive) {
            continue;
        }

        const QRectF rect = crate.rect.translated(-cameraX, 0);

        painter.setPen(QPen(crateDark, 3));
        painter.setBrush(crateWood);
        painter.drawRect(rect);

        painter.setPen(QPen(crateLight, 4));
        painter.drawLine(
            rect.topLeft() + QPointF(6, 6),
            rect.bottomRight() - QPointF(6, 6));
        painter.drawLine(
            rect.topRight() + QPointF(-6, 6),
            rect.bottomLeft() + QPointF(6, -6));

        painter.setPen(QPen(crateDark, 3));
        painter.drawRect(rect.adjusted(6, 6, -6, -6));

        if (crate.hp > 1) {
            painter.setPen(Qt::white);
            painter.drawText(rect, Qt::AlignCenter, QString::number(crate.hp));
        }
    }

    for (const auto &enemy : enemies_) {
        enemy.draw(painter, cameraX);
    }

    for (const auto &projectile : projectiles_) {
        const QRectF projectileRect =
            projectile.rect.translated(-cameraX, 0);

        const double trailLength =
            std::clamp(
                std::abs(projectile.velocity.x()) * 0.035,
                12.0,
                28.0);

        QRectF trail = projectileRect;

        if (projectile.velocity.x() > 0.0f) {
            trail.setLeft(trail.left() - trailLength);
        } else {
            trail.setRight(trail.right() + trailLength);
        }

        QLinearGradient gradient(
            projectile.velocity.x() > 0.0f
                ? trail.topLeft()
                : trail.topRight(),
            projectile.velocity.x() > 0.0f
                ? trail.topRight()
                : trail.topLeft());

        const QColor core =
            projectile.hostile
                ? QColor(255, 70, 40)
                : QColor(255, 235, 80);

        gradient.setColorAt(
            0.0,
            QColor(
                core.red(),
                core.green(),
                core.blue(),
                0));
        gradient.setColorAt(1.0, core);

        painter.fillRect(trail, gradient);
        painter.fillRect(projectileRect, core);
    }

    for (const auto &particle : particles_) {
        painter.setBrush(particle.color);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(particle.pos.x() - cameraX, particle.pos.y()),
                            particle.size, particle.size);
    }

    player_.draw(painter, cameraX);
    painter.restore();
}

Player &World::player() { return player_; }
const Player &World::player() const { return player_; }
double World::width() const { return level_.worldSize().width(); }
QString World::levelName() const { return level_.name(); }
int World::levelIndex() const { return levelIndex_; }
bool World::completed() const { return completed_; }
bool World::gameOver() const { return gameOver_; }
void World::toggleSound() { sound_ = !sound_; }
bool World::soundEnabled() const { return sound_; }
QString World::message() const { return message_; }
