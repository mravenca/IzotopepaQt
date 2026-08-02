#include "world.h"

#include <QApplication>
#include <QPainter>
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
    if (!level_.load(QString(":/levels/level%1.txt").arg(levelIndex_ + 1))) {
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
}

void World::update(double dt)
{
    if (completed_ || gameOver_) {
        return;
    }

    for (auto &platform : moving_) {
        const QPointF old = platform.rect.topLeft();
        platform.rect.translate(platform.speedX * platform.dir * dt,
                                platform.speedY * platform.dir * dt);

        const bool outsideHorizontal = platform.speedX != 0
            && (platform.rect.left() < platform.minX || platform.rect.left() > platform.maxX);
        const bool outsideVertical = platform.speedY != 0
            && (platform.rect.top() < 250 || platform.rect.top() > 520);

        if (outsideHorizontal || outsideVertical) {
            platform.dir *= -1;
            platform.rect.moveTopLeft(old);
        }
        platform.delta = platform.rect.topLeft() - old;
    }

    rebuildCollision();
    player_.update(dt, collision_, level_.ladders(), level_.worldSize().width());
    for (auto &enemy : enemies_) {
        enemy.update(dt, collision_, player_.rect().center(), projectiles_);
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

        if (projectile.life <= 0 || intersectsAny(projectile.rect, collision_)) {
            projectile.alive = false;
            explode(projectile.rect.center(), Qt::lightGray);
            continue;
        }

        if (projectile.hostile) {
            if (projectile.rect.intersects(player_.rect())) {
                player_.damage(projectile.rect.center().x());
                projectile.alive = false;
                explode(projectile.rect.center(), Qt::red);
            }
        } else {
            for (auto &enemy : enemies_) {
                if (enemy.alive() && projectile.rect.intersects(enemy.rect())) {
                    enemy.damage(1);
                    projectile.alive = false;
                    explode(projectile.rect.center(), Qt::yellow);
                    if (!enemy.alive()) {
                        player_.addScore(enemy.reward());
                        explode(enemy.rect().center(), Qt::red);
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

void World::shoot()
{
    if (!completed_ && !gameOver_ && player_.canShoot()) {
        projectiles_ << player_.shoot();
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

    for (const auto &pickup : pickups_) {
        if (pickup.taken) {
            continue;
        }
        const QRectF rect = pickup.rect.translated(-cameraX, 0);
        painter.setBrush(pickup.kind == "health" ? Qt::red : Qt::cyan);
        painter.setPen(Qt::white);
        painter.drawRoundedRect(rect, 5, 5);
        painter.drawText(rect, Qt::AlignCenter, pickup.kind == "health" ? "+" : "A");
    }

    for (const auto &key : keys_) {
        if (key.taken) {
            continue;
        }
        const QRectF rect = key.rect.translated(-cameraX, 0);
        painter.setPen(QPen(Qt::yellow, 4));
        painter.drawLine(rect.left(), rect.center().y(), rect.right(), rect.center().y());
        painter.drawEllipse(QPointF(rect.left() + 4, rect.center().y()), 5, 5);
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

    for (const auto &switchObject : switches_) {
        const QRectF rect = switchObject.rect.translated(-cameraX, 0);
        painter.fillRect(rect, switchObject.active ? Qt::green : Qt::darkRed);
    }

    for (const auto &enemy : enemies_) {
        enemy.draw(painter, cameraX);
    }

    for (const auto &projectile : projectiles_) {
        painter.fillRect(projectile.rect.translated(-cameraX, 0),
                         projectile.hostile ? Qt::red : Qt::yellow);
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
