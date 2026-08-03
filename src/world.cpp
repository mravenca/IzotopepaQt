#include "world.h"

#include <QApplication>
#include <QPainter>
#include <QLinearGradient>
#include <QLineF>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QSettings>
#include <QtMath>

#include <algorithm>
#include <cmath>

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
    levelIndex_ = std::clamp(index, 0, 9);
    if (!level_.load(QString("level%1.json").arg(levelIndex_ + 1))) {
        return false;
    }

    return initializeLoadedLevel();
}

bool World::loadLevelFile(const QString &fileName)
{
    levelIndex_ = -1;
    if (!level_.load(fileName)) {
        return false;
    }

    return initializeLoadedLevel();
}

bool World::restartLevel()
{
    return initializeLoadedLevel();
}

bool World::initializeLoadedLevel()
{
    player_.reset(level_.playerSpawn(), true);
    respawn_ = level_.playerSpawn();

    enemies_.clear();
    drones_.clear();
    turrets_.clear();
    chargers_.clear();
    shieldSoldiers_.clear();

    for (const auto &spawn : level_.enemies()) {
        if (EnemyFactory::isDrone(spawn)) {
            drones_ << EnemyFactory::createDrone(spawn);
        } else if (EnemyFactory::isTurret(spawn)) {
            turrets_ << EnemyFactory::createTurret(spawn);
        } else if (EnemyFactory::isCharger(spawn)) {
            chargers_ << EnemyFactory::createCharger(spawn);
        } else if (EnemyFactory::isShieldSoldier(spawn)) {
            shieldSoldiers_ << EnemyFactory::createShieldSoldier(spawn);
        } else {
            enemies_ << Enemy(
                enemySheet_,
                spawn.kind,
                spawn.position,
                spawn.leftLimit,
                spawn.rightLimit);
        }
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
        doors_ << Door{spawn.key, spawn.rect, false, false, false};
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

    barrels_.clear();
    for (const auto &spawn : level_.barrels()) {
        barrels_ << Barrel {
            QRectF(spawn.position.x(), spawn.position.y(), 40, 52),
            QVector2D(),
            spawn.radius,
            spawn.damage,
            -1.0,
            true
        };
    }

    pushBoxes_.clear();
    for (const auto &spawn : level_.pushBoxes()) {
        pushBoxes_ << PushBox {
            QRectF(
                spawn.position.x(),
                spawn.position.y(),
                spawn.width,
                spawn.height),
            QVector2D(),
            true
        };
    }

    jumpPads_.clear();
    jumpPadActivations_.clear();
    for (const auto &spawn : level_.jumpPads()) {
        jumpPads_ << JumpPad(
            QRectF(
                spawn.position.x(),
                spawn.position.y(),
                spawn.width,
                spawn.height),
            spawn.strength,
            spawn.horizontalImpulse,
            spawn.launchDelay,
            spawn.cooldown);
        jumpPadActivations_ << JumpPadActivation {};
    }

    conveyors_.clear();
    for (const auto &spawn : level_.conveyors()) {
        conveyors_ << Conveyor(spawn.rect, spawn.speed);
    }

    oneWayPlatforms_.clear();
    for (const auto &spawn : level_.oneWayPlatforms()) {
        oneWayPlatforms_ << OneWayPlatform(spawn.rect);
    }
    fallingPlatforms_.clear();
    for (const auto &spawn : level_.fallingPlatforms()) {
        fallingPlatforms_ << FallingPlatform(
            spawn.rect,
            spawn.material,
            spawn.confirmationTime,
            spawn.fallDelay,
            spawn.respawnDelay);
    }

    iceSurfaces_.clear();
    for (const auto &spawn : level_.iceSurfaces()) {
        iceSurfaces_ << IceSurface(spawn.rect, spawn.friction);
    }

    waterZones_.clear();
    for (const auto &spawn : level_.waterZones()) {
        waterZones_ << WaterZone(spawn.rect, spawn.buoyancy, spawn.drag);
    }
    playerWasInWater_ = false;

    pressurePlates_.clear();
    worldEvents_.clear();
    for (const auto &spawn : level_.pressurePlates()) {
        pressurePlates_ << PressurePlate(
            QRectF(
                spawn.position.x(),
                spawn.position.y(),
                spawn.width,
                spawn.height),
            spawn.target,
            spawn.requiredWeight);
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
    for (const auto &barrel : barrels_) {
        if (barrel.alive) {
            collision_ << barrel.rect;
        }
    }
    for (const auto &box : pushBoxes_) {
        if (box.alive) {
            collision_ << box.rect;
        }
    }
    for (const FallingPlatform &platform : fallingPlatforms_) {
        if (platform.isSolid()) {
            collision_ << platform.rect();
        }
    }
}

void World::update(double dt)
{
    if (completed_ || gameOver_) {
        return;
    }

    if (hitStop_ > 0.0) {
        hitStop_ = std::max(0.0, hitStop_ - dt);
        return;
    }

    animationTime_ += dt;
    oneWayDropTimer_ = std::max(0.0, oneWayDropTimer_ - dt);
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
    updateWaterObjects(dt);

    for (int barrelIndex = 0;
         barrelIndex < barrels_.size();
         ++barrelIndex) {
        Barrel &barrel = barrels_[barrelIndex];

        if (!barrel.alive || barrel.fuse < 0.0) {
            continue;
        }

        barrel.fuse -= dt;

        if (barrel.fuse <= 0.0) {
            const bool underwater = waterZoneFor(barrel.rect) != nullptr;
            const ExplosionEvent event {
                barrel.rect.center(),
                barrel.radius * (underwater ? 0.55 : 1.0),
                barrel.damage
            };

            barrel.alive = false;
            rebuildCollision();
            applyExplosion(event);
        }
    }

    updateFallingPlatforms(dt);
    rebuildCollision();
    updatePushBoxes(dt);
    rebuildCollision();

    const WaterZone *playerWater = waterZoneFor(player_.rect());
    const bool playerInWater = playerWater != nullptr;
    player_.setEnvironment(
        playerInWater,
        playerWater ? playerWater->drag() : 0.55,
        playerWater ? playerWater->buoyancy() : 0.72,
        iceFrictionBelow(player_.rect()));

    if (playerInWater != playerWasInWater_) {
        createSplash(QPointF(
            player_.rect().center().x(),
            playerWater ? playerWater->rect().top()
                        : player_.rect().bottom()));
        playerWasInWater_ = playerInWater;
    }

    player_.update(
        dt,
        collision_,
        oneWayRects(),
        level_.ladders(),
        level_.worldSize().width(),
        oneWayDropTimer_ > 0.0);
    updateConveyors(dt);
    updateJumpPads(dt);
    updatePressurePlates(dt);
    processWorldEvents();
    refreshDoorStates();
    for (auto &enemy : enemies_) {
        const WaterZone *enemyWater = waterZoneFor(enemy.rect());
        enemy.update(
            dt,
            collision_,
            oneWayRects(),
            player_.rect().center(),
            projectiles_,
            enemyWater != nullptr,
            enemyWater ? enemyWater->drag() : 0.55,
            enemyWater ? enemyWater->buoyancy() : 0.72);
    }

    for (Drone &drone : drones_) {
        drone.update(
            dt,
            player_.rect().center(),
            collision_,
            projectiles_);
    }

    for (Turret &turret : turrets_) {
        turret.update(
            dt,
            player_.rect().center(),
            projectiles_);
    }

    for (Charger &charger : chargers_) {
        const WaterZone *chargerWater = waterZoneFor(charger.rect());
        QVector<QRectF> chargerSolids = collision_;
        for (const PushBox &box : pushBoxes_) {
            if (box.alive) {
                chargerSolids.removeAll(box.rect);
            }
        }
        for (const Crate &crate : crates_) {
            if (crate.alive) {
                chargerSolids.removeAll(crate.rect);
            }
        }
        for (const Barrel &barrel : barrels_) {
            if (barrel.alive) {
                chargerSolids.removeAll(barrel.rect);
            }
        }

        charger.update(
            dt,
            player_.rect().center(),
            chargerSolids,
            oneWayRects(),
            chargerWater != nullptr,
            conveyorSpeedBelow(charger.rect()),
            iceFrictionBelow(charger.rect()));

        if (!charger.alive()) {
            continue;
        }

        if (charger.charging()
            && charger.rect().intersects(player_.rect())) {
            player_.damage(charger.rect().center().x());
            charger.stun();
            shakeTime_ = std::max(shakeTime_, 0.22);
            shakeStrength_ = std::max(shakeStrength_, 7.0);
        }

        if (charger.charging()) {
            for (PushBox &box : pushBoxes_) {
                if (box.alive && charger.rect().intersects(box.rect)) {
                    box.velocity.setX(
                        static_cast<float>(charger.direction() * 320.0));
                    box.velocity.setY(-90.0f);
                    charger.stun();
                    break;
                }
            }
        }

        if (charger.charging()) {
            for (Crate &crate : crates_) {
                if (crate.alive && charger.rect().intersects(crate.rect)) {
                    destroyCrate(crate);
                    charger.stun();
                    rebuildCollision();
                    break;
                }
            }
        }

        if (charger.charging()) {
            for (Barrel &barrel : barrels_) {
                if (barrel.alive && charger.rect().intersects(barrel.rect)) {
                    barrel.fuse = 0.01;
                    charger.stun();
                    break;
                }
            }
        }
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
        if (!door.latchedOpen && player_.hasKey(door.key)
            && player_.rect().adjusted(-8, -8, 8, 8).intersects(door.rect)) {
            door.latchedOpen = true;
            refreshDoorStates();
            message_ = door.key + " door unlocked";
            messageTime_ = 2;
            beep();
        }
    }

    for (const auto &spike : level_.spikes()) {
        if (player_.rect().intersects(spike)) {
            player_.damage(spike.center().x());
        }
    }

    for (ShieldSoldier &soldier : shieldSoldiers_) {
        const WaterZone *soldierWater = waterZoneFor(soldier.rect());
        soldier.update(
            dt,
            player_.rect().center(),
            collision_,
            oneWayRects(),
            soldierWater != nullptr,
            conveyorSpeedBelow(soldier.rect()),
            iceFrictionBelow(soldier.rect()),
            projectiles_);
    }

    for (auto &projectile : projectiles_) {
        if (!projectile.alive) {
            continue;
        }

        projectile.life -= dt;

        if (waterZoneFor(projectile.rect)) {
            const float dragFactor = static_cast<float>(std::exp(-2.8 * dt));
            projectile.velocity *= dragFactor;
            projectile.life -= dt * 0.35;
        }

        projectile.rect.translate(projectile.velocity.x() * dt,
                                  projectile.velocity.y() * dt);

        bool hitBarrel = false;

        if (!projectile.hostile) {
            for (auto &barrel : barrels_) {
                if (!barrel.alive
                    || !projectile.rect.intersects(barrel.rect)) {
                    continue;
                }

                projectile.alive = false;
                hitBarrel = true;

                if (waterZoneFor(barrel.rect)) {
                    const ExplosionEvent event {
                        barrel.rect.center(),
                        barrel.radius * 0.55,
                        barrel.damage
                    };
                    barrel.alive = false;
                    rebuildCollision();
                    applyExplosion(event);
                } else if (barrel.fuse < 0.0) {
                    barrel.fuse = 0.12;
                    message_ = "Barrel fuse lit";
                    messageTime_ = 0.7;
                }

                explode(
                    projectile.rect.center(),
                    QColor(255, 170, 35));
                break;
            }
        }

        if (hitBarrel) {
            continue;
        }

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
                requestCombatFeedback(0.18, 5.0, 0.018);
                projectile.alive = false;
                combatImpact(
                    projectile.rect.center(),
                    CombatImpact::Bullet,
                    1.15,
                    QColor(235, 65, 65));
                audioEvent("player.hit");
            }
        } else {
            bool hitDrone = false;
            for (Drone &drone : drones_) {
                if (drone.alive()
                    && projectile.rect.intersects(drone.rect())) {
                    drone.damage(1, projectile.rect.center());
                    projectile.alive = false;
                    hitDrone = true;
                    combatImpact(
                        projectile.rect.center(),
                        CombatImpact::Metal,
                        1.0);
                    requestCombatFeedback(0.10, 3.0, 0.010);

                    if (!drone.alive()) {
                        enemyDeath(
                            drone.rect().center(),
                            QColor(255, 95, 40),
                            drone.reward(),
                            "enemy.drone.destroy");
                    }
                    break;
                }
            }

            if (hitDrone) {
                continue;
            }

            bool hitTurret = false;
            for (Turret &turret : turrets_) {
                if (turret.alive()
                    && projectile.rect.intersects(turret.rect())) {
                    turret.damage(1, projectile.rect.center());
                    projectile.alive = false;
                    hitTurret = true;
                    combatImpact(
                        projectile.rect.center(),
                        CombatImpact::Metal,
                        1.0);
                    requestCombatFeedback(0.10, 3.0, 0.010);

                    if (!turret.alive()) {
                        enemyDeath(
                            turret.rect().center(),
                            QColor(255, 130, 35),
                            turret.reward(),
                            "enemy.turret.destroy");
                    }
                    break;
                }
            }

            if (hitTurret) {
                continue;
            }

            bool hitCharger = false;
            for (Charger &charger : chargers_) {
                if (charger.alive()
                    && projectile.rect.intersects(charger.rect())) {
                    charger.damage(1, projectile.rect.center());
                    projectile.alive = false;
                    hitCharger = true;
                    combatImpact(
                        projectile.rect.center(),
                        CombatImpact::Metal,
                        1.0);
                    requestCombatFeedback(0.10, 3.0, 0.010);

                    if (!charger.alive()) {
                        enemyDeath(
                            charger.rect().center(),
                            QColor(255, 95, 40),
                            charger.reward(),
                            "enemy.charger.destroy");
                    }
                    break;
                }
            }

            if (hitCharger) {
                continue;
            }

            bool hitShieldSoldier = false;
            for (ShieldSoldier &soldier : shieldSoldiers_) {
                if (!soldier.alive()
                    || !projectile.rect.intersects(soldier.rect())) {
                    continue;
                }

                const ShieldBulletResult result =
                    soldier.receiveBullet(
                        1,
                        projectile.rect.center()
                            - projectile.velocity.toPointF() * 0.05);
                projectile.alive = false;
                hitShieldSoldier = true;

                if (result == ShieldBulletResult::Blocked) {
                    combatImpact(
                        projectile.rect.center(),
                        CombatImpact::Shield,
                        1.25);
                    requestCombatFeedback(0.07, 2.0, 0.012);
                    audioEvent("enemy.shield.block");
                    message_ = "Shield blocked the shot";
                    messageTime_ = 0.55;
                } else {
                    combatImpact(
                        projectile.rect.center(),
                        CombatImpact::Bullet,
                        1.0);
                    requestCombatFeedback(0.10, 3.0, 0.010);
                }

                if (result == ShieldBulletResult::Destroyed) {
                    enemyDeath(
                        soldier.rect().center(),
                        QColor(80, 175, 230),
                        soldier.reward(),
                        "enemy.shield.destroy");
                }
                break;
            }

            if (hitShieldSoldier) {
                continue;
            }

            for (auto &enemy : enemies_) {
                if (enemy.alive() && projectile.rect.intersects(enemy.rect())) {
                    enemy.damage(1, projectile.rect.center().x());
                    requestCombatFeedback(0.10, 3.0, 0.010);
                    projectile.alive = false;
                    combatImpact(
                        projectile.rect.center(),
                        CombatImpact::Bullet,
                        1.0);
                    if (!enemy.alive()) {
                        enemyDeath(
                            enemy.rect().center(),
                            QColor(235, 70, 55),
                            enemy.reward(),
                            "enemy.legacy.destroy");
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

QVector<QRectF> World::pushBoxBlockers(int excludedIndex) const
{
    QVector<QRectF> blockers = level_.platforms();

    for (const auto &platform : moving_) {
        blockers << platform.rect;
    }

    for (const auto &door : doors_) {
        if (!door.open) {
            blockers << door.rect;
        }
    }

    for (const auto &crate : crates_) {
        if (crate.alive) {
            blockers << crate.rect;
        }
    }

    for (const auto &barrel : barrels_) {
        if (barrel.alive) {
            blockers << barrel.rect;
        }
    }

    for (int index = 0; index < pushBoxes_.size(); ++index) {
        if (index != excludedIndex && pushBoxes_[index].alive) {
            blockers << pushBoxes_[index].rect;
        }
    }

    for (const FallingPlatform &platform : fallingPlatforms_) {
        if (platform.isSolid()) {
            blockers << platform.rect();
        }
    }

    return blockers;
}

bool World::standingOnFallingPlatform(
    const QRectF &object,
    const QRectF &platform) const
{
    const QRectF feet(
        object.left() + 5.0,
        object.bottom() - 5.0,
        std::max(1.0, object.width() - 10.0),
        10.0);

    return feet.intersects(platform)
        && object.bottom() <= platform.top() + 7.0;
}

bool World::fallingPlatformRespawnClear(const QRectF &rect) const
{
    const QRectF safetyArea = rect.adjusted(-3.0, -6.0, 3.0, 3.0);

    if (player_.rect().intersects(safetyArea)) {
        return false;
    }

    for (const PushBox &box : pushBoxes_) {
        if (box.alive && box.rect.intersects(safetyArea)) {
            return false;
        }
    }

    for (const Enemy &enemy : enemies_) {
        if (enemy.alive() && enemy.rect().intersects(safetyArea)) {
            return false;
        }
    }

    for (const Charger &charger : chargers_) {
        if (charger.alive() && charger.rect().intersects(safetyArea)) {
            return false;
        }
    }
    for (const ShieldSoldier &soldier : shieldSoldiers_) {
        if (soldier.alive() && soldier.rect().intersects(safetyArea)) {
            return false;
        }
    }

    for (const Crate &crate : crates_) {
        if (crate.alive && crate.rect.intersects(safetyArea)) {
            return false;
        }
    }

    for (const Barrel &barrel : barrels_) {
        if (barrel.alive && barrel.rect.intersects(safetyArea)) {
            return false;
        }
    }

    return true;
}

void World::updateFallingPlatforms(double dt)
{
    bool collisionChanged = false;

    for (FallingPlatform &platform : fallingPlatforms_) {
        bool occupied = false;

        if (platform.isSolid()) {
            occupied = standingOnFallingPlatform(
                player_.rect(), platform.rect());

            if (!occupied) {
                for (const PushBox &box : pushBoxes_) {
                    if (box.alive
                        && standingOnFallingPlatform(
                            box.rect, platform.rect())) {
                        occupied = true;
                        break;
                    }
                }
            }

            if (!occupied) {
                for (const Enemy &enemy : enemies_) {
                    if (enemy.alive()
                        && standingOnFallingPlatform(
                            enemy.rect(), platform.rect())) {
                        occupied = true;
                        break;
                    }
                }
            }

            if (!occupied) {
                for (const Charger &charger : chargers_) {
                    if (charger.alive()
                        && standingOnFallingPlatform(
                            charger.rect(), platform.rect())) {
                        occupied = true;
                        break;
                    }
                }
            }
            if (!occupied) {
                for (const ShieldSoldier &soldier : shieldSoldiers_) {
                    if (soldier.alive()
                        && standingOnFallingPlatform(
                            soldier.rect(), platform.rect())) {
                        occupied = true;
                        break;
                    }
                }
            }
        }

        const bool wasSolid = platform.isSolid();
        const FallingPlatformUpdate result = platform.update(
            dt,
            occupied,
            fallingPlatformRespawnClear(platform.startRect()),
            level_.worldSize().height());

        if (wasSolid != platform.isSolid()) {
            collisionChanged = true;
        }

        if (result.armed) {
            message_ = platform.material() + " platform triggered";
            messageTime_ = 1.2;
            beep();
        }

        if (result.beganFalling) {
            explode(platform.rect().center(), QColor(155, 140, 115));
            shakeTime_ = std::max(shakeTime_, 0.20);
            shakeStrength_ = std::max(shakeStrength_, 5.0);
        }

        if (result.respawned) {
            explode(platform.rect().center(), QColor(190, 205, 215));
        }
    }

    if (collisionChanged) {
        rebuildCollision();
    }
}

void World::updatePressurePlates(double dt)
{
    for (PressurePlate &plate : pressurePlates_) {
        const QRectF trigger = plate.triggerZone();
        double weight = 0.0;

        const QRectF playerFeet(
            player_.rect().left() + 5.0,
            player_.rect().bottom() - 5.0,
            player_.rect().width() - 10.0,
            10.0);

        if (playerFeet.intersects(trigger)) {
            weight += 1.0;
        }

        for (const PushBox &box : pushBoxes_) {
            if (!box.alive) {
                continue;
            }

            const QRectF boxFeet(
                box.rect.left() + 4.0,
                box.rect.bottom() - 5.0,
                box.rect.width() - 8.0,
                10.0);

            if (boxFeet.intersects(trigger)) {
                weight += 1.0;
            }
        }

        for (const Crate &crate : crates_) {
            if (crate.alive
                && crate.rect.adjusted(4.0, 35.0, -4.0, 5.0)
                       .intersects(trigger)) {
                weight += 1.0;
            }
        }

        for (const Barrel &barrel : barrels_) {
            if (barrel.alive
                && barrel.rect.adjusted(4.0, 38.0, -4.0, 5.0)
                       .intersects(trigger)) {
                weight += 1.0;
            }
        }

        plate.update(weight, dt, worldEvents_);
    }
}

void World::processWorldEvents()
{
    for (const WorldEvent &event : worldEvents_.takeAll()) {
        if (event.type != WorldEventType::SetSignal) {
            continue;
        }

        for (Door &door : doors_) {
            if (door.key == event.channel) {
                door.signalActive = event.active;
            }
        }

        message_ = event.active
            ? event.channel + " pressure plate activated"
            : event.channel + " pressure plate released";
        messageTime_ = 1.0;
        beep();
    }
}

void World::refreshDoorStates()
{
    bool collisionChanged = false;

    for (Door &door : doors_) {
        bool nextOpen = door.latchedOpen || door.signalActive;

        if (!nextOpen && door.open) {
            bool obstructed = player_.rect().intersects(door.rect);

            for (const PushBox &box : pushBoxes_) {
                if (box.alive && box.rect.intersects(door.rect)) {
                    obstructed = true;
                    break;
                }
            }

            if (obstructed) {
                nextOpen = true;
            }
        }

        if (door.open != nextOpen) {
            door.open = nextOpen;
            collisionChanged = true;
        }
    }

    if (collisionChanged) {
        rebuildCollision();
    }
}

double World::conveyorSpeedBelow(const QRectF &rect) const
{
    const QRectF feet(
        rect.left() + 5.0,
        rect.bottom() - 4.0,
        std::max(1.0, rect.width() - 10.0),
        9.0);

    double result = 0.0;

    for (const Conveyor &conveyor : conveyors_) {
        if (feet.intersects(conveyor.surfaceZone())
            && rect.bottom() <= conveyor.rect().top() + 7.0) {
            result += conveyor.speed();
        }
    }

    return std::clamp(result, -500.0, 500.0);
}

void World::updateConveyors(double dt)
{
    for (Conveyor &conveyor : conveyors_) {
        conveyor.update(dt);
    }

    const double playerSpeed = conveyorSpeedBelow(player_.rect());

    if (!qFuzzyIsNull(playerSpeed)) {
        QVector<QRectF> blockers = collision_;

        // The player may be touching a push box that is also being carried.
        // Excluding boxes prevents artificial crush damage between two
        // objects travelling at the same belt speed.
        for (const PushBox &box : pushBoxes_) {
            if (box.alive) {
                blockers.removeAll(box.rect);
            }
        }

        player_.carryBy(QPointF(playerSpeed * dt, 0.0), blockers);
    }
}

void World::updateJumpPads(double dt)
{
    for (int index = 0; index < jumpPads_.size(); ++index) {
        JumpPad &pad = jumpPads_[index];
        JumpPadActivation &activation = jumpPadActivations_[index];

        pad.update(dt);

        const QRectF trigger = pad.triggerZone();
        const QRectF playerFeet(
            player_.rect().left() + 5.0,
            player_.rect().bottom() - 5.0,
            player_.rect().width() - 10.0,
            10.0);

        if (!activation.player
            && player_.velocity().y() >= -10.0f
            && playerFeet.intersects(trigger)) {
            const bool compressionCycleActive =
                !activation.pushBoxes.isEmpty();

            if (pad.requestTrigger() || compressionCycleActive) {
                activation.player = true;
            }
        }

        for (int boxIndex = 0; boxIndex < pushBoxes_.size(); ++boxIndex) {
            PushBox &box = pushBoxes_[boxIndex];

            if (!box.alive
                || activation.pushBoxes.contains(boxIndex)
                || box.velocity.y() < -10.0f) {
                continue;
            }

            const QRectF boxFeet(
                box.rect.left() + 4.0,
                box.rect.bottom() - 5.0,
                box.rect.width() - 8.0,
                10.0);

            if (!boxFeet.intersects(trigger)) {
                continue;
            }

            const bool compressionCycleActive =
                activation.player || !activation.pushBoxes.isEmpty();

            if (pad.requestTrigger() || compressionCycleActive) {
                activation.pushBoxes << boxIndex;
            }
        }

        for (int chargerIndex = 0; chargerIndex < chargers_.size(); ++chargerIndex) {
            Charger &charger = chargers_[chargerIndex];
            if (!charger.alive()
                || activation.chargers.contains(chargerIndex)
                || charger.velocity().y() < -10.0f) {
                continue;
            }

            const QRectF feet(
                charger.rect().left() + 5.0,
                charger.rect().bottom() - 5.0,
                charger.rect().width() - 10.0,
                10.0);
            if (!feet.intersects(trigger)) {
                continue;
            }

            const bool compressionCycleActive =
                activation.player
                || !activation.pushBoxes.isEmpty()
                || !activation.chargers.isEmpty();
            if (pad.requestTrigger() || compressionCycleActive) {
                activation.chargers << chargerIndex;
            }
        }

        for (int soldierIndex = 0;
             soldierIndex < shieldSoldiers_.size();
             ++soldierIndex) {
            ShieldSoldier &soldier = shieldSoldiers_[soldierIndex];
            if (!soldier.alive()
                || activation.shieldSoldiers.contains(soldierIndex)
                || soldier.velocity().y() < -10.0f) {
                continue;
            }

            const QRectF feet(
                soldier.rect().left() + 5.0,
                soldier.rect().bottom() - 5.0,
                soldier.rect().width() - 10.0,
                10.0);
            if (!feet.intersects(trigger)) {
                continue;
            }

            const bool compressionCycleActive =
                activation.player
                || !activation.pushBoxes.isEmpty()
                || !activation.chargers.isEmpty()
                || !activation.shieldSoldiers.isEmpty();
            if (pad.requestTrigger() || compressionCycleActive) {
                activation.shieldSoldiers << soldierIndex;
            }
        }

        if (pad.consumeLaunch()) {
            launchFromPad(index);
        }
    }
}

void World::launchFromPad(int padIndex)
{
    if (padIndex < 0 || padIndex >= jumpPads_.size()) {
        return;
    }

    JumpPad &pad = jumpPads_[padIndex];
    JumpPadActivation &activation = jumpPadActivations_[padIndex];
    bool launched = false;

    if (activation.player) {
        const QRectF expandedTrigger =
            pad.triggerZone().adjusted(-14.0, -18.0, 14.0, 12.0);

        if (player_.rect().intersects(expandedTrigger)) {
            const float launchX = qFuzzyIsNull(pad.horizontalImpulse())
                ? player_.velocity().x()
                : static_cast<float>(pad.horizontalImpulse());
            player_.launch(QVector2D(
                launchX,
                static_cast<float>(-pad.strength())));
            launched = true;
        }
    }

    for (const int boxIndex : activation.pushBoxes) {
        if (boxIndex < 0 || boxIndex >= pushBoxes_.size()) {
            continue;
        }

        PushBox &box = pushBoxes_[boxIndex];
        if (!box.alive) {
            continue;
        }

        const QRectF expandedTrigger =
            pad.triggerZone().adjusted(-12.0, -18.0, 12.0, 12.0);

        if (!box.rect.intersects(expandedTrigger)) {
            continue;
        }

        const float launchX = qFuzzyIsNull(pad.horizontalImpulse())
            ? box.velocity.x()
            : static_cast<float>(pad.horizontalImpulse());
        box.velocity = QVector2D(
            launchX,
            static_cast<float>(-pad.strength()));
        launched = true;
    }

    for (const int chargerIndex : activation.chargers) {
        if (chargerIndex < 0 || chargerIndex >= chargers_.size()) {
            continue;
        }
        Charger &charger = chargers_[chargerIndex];
        if (!charger.alive()) {
            continue;
        }
        const QRectF expandedTrigger =
            pad.triggerZone().adjusted(-12.0, -18.0, 12.0, 12.0);
        if (!charger.rect().intersects(expandedTrigger)) {
            continue;
        }
        const float launchX = qFuzzyIsNull(pad.horizontalImpulse())
            ? charger.velocity().x()
            : static_cast<float>(pad.horizontalImpulse());
        charger.launch(QVector2D(
            launchX,
            static_cast<float>(-pad.strength())));
        launched = true;
    }

    for (const int soldierIndex : activation.shieldSoldiers) {
        if (soldierIndex < 0 || soldierIndex >= shieldSoldiers_.size()) {
            continue;
        }
        ShieldSoldier &soldier = shieldSoldiers_[soldierIndex];
        if (!soldier.alive()) {
            continue;
        }
        const QRectF expandedTrigger =
            pad.triggerZone().adjusted(-12.0, -18.0, 12.0, 12.0);
        if (!soldier.rect().intersects(expandedTrigger)) {
            continue;
        }
        const float launchX = qFuzzyIsNull(pad.horizontalImpulse())
            ? soldier.velocity().x()
            : static_cast<float>(pad.horizontalImpulse());
        soldier.launch(QVector2D(
            launchX,
            static_cast<float>(-pad.strength())));
        launched = true;
    }

    activation = JumpPadActivation {};

    if (!launched) {
        return;
    }

    const QPointF burstCenter(
        pad.rect().center().x(),
        pad.rect().top());

    for (int burst = 0; burst < 3; ++burst) {
        explode(
            burstCenter + QPointF((burst - 1) * 13.0, -3.0),
            burst == 1
                ? QColor(255, 245, 120)
                : QColor(255, 150, 35));
    }

    shakeTime_ = std::max(shakeTime_, 0.16);
    shakeStrength_ = std::max(shakeStrength_, 4.0);
    message_ = "Jump pad!";
    messageTime_ = 0.7;
    beep();
}

void World::updatePushBoxes(double dt)
{
    constexpr double kGravity = 1900.0;
    constexpr double kPushSpeed = 135.0;
    constexpr double kContactTolerance = 7.0;

    bool collisionChanged = false;

    for (int index = 0; index < pushBoxes_.size(); ++index) {
        PushBox &box = pushBoxes_[index];

        if (!box.alive) {
            continue;
        }

        QVector<QRectF> blockers = pushBoxBlockers(index);

        // Carry boxes standing on a moving platform by its per-frame delta.
        for (const auto &platform : moving_) {
            const QRectF previousPlatform =
                platform.rect.translated(-platform.delta);
            const QRectF boxFeet(
                box.rect.left() + 4.0,
                box.rect.bottom() - 3.0,
                box.rect.width() - 8.0,
                7.0);

            if (!boxFeet.intersects(previousPlatform)
                || box.rect.bottom() > previousPlatform.top() + 5.0) {
                continue;
            }

            QRectF carried = box.rect.translated(platform.delta);
            bool blocked = false;

            for (const QRectF &blocker : blockers) {
                if (carried.intersects(blocker)) {
                    blocked = true;
                    break;
                }
            }

            if (!blocked) {
                box.rect = carried;
            }
            break;
        }

        const QRectF playerRect = player_.rect();
        const bool verticalOverlap =
            playerRect.bottom() > box.rect.top() + 6.0
            && playerRect.top() < box.rect.bottom() - 6.0;

        const double gapOnLeft = box.rect.left() - playerRect.right();
        const double gapOnRight = playerRect.left() - box.rect.right();

        int pushDirection = 0;

        if (inputRight_
            && verticalOverlap
            && gapOnLeft >= -2.0
            && gapOnLeft <= kContactTolerance) {
            pushDirection = 1;
        } else if (inputLeft_
                   && verticalOverlap
                   && gapOnRight >= -2.0
                   && gapOnRight <= kContactTolerance) {
            pushDirection = -1;
        }

        const double beltSpeed = conveyorSpeedBelow(box.rect);
        const WaterZone *water = waterZoneFor(box.rect);
        const double iceFriction = iceFrictionBelow(box.rect);

        if (pushDirection != 0) {
            const double pushScale = water ? 0.60 : 1.0;
            box.velocity.setX(static_cast<float>(
                pushDirection * kPushSpeed * pushScale + beltSpeed));
        } else if (!qFuzzyIsNull(beltSpeed)) {
            const float target = static_cast<float>(beltSpeed);
            box.velocity.setX(
                box.velocity.x()
                + (target - box.velocity.x())
                    * static_cast<float>(std::min(1.0, dt * 12.0)));
        } else if (water) {
            box.velocity.setX(box.velocity.x() * static_cast<float>(
                std::exp(-water->drag() * 2.0 * dt)));
        } else if (iceFriction < 0.99) {
            box.velocity.setX(box.velocity.x() * static_cast<float>(
                std::exp(-iceFriction * 2.4 * dt)));
        } else {
            box.velocity.setX(box.velocity.x() * 0.72f);
        }

        if (qAbs(box.velocity.x()) < 1.0f) {
            box.velocity.setX(0.0f);
        }

        if (water) {
            const double submerged = water->submergedFraction(box.rect);
            const double acceleration =
                kGravity - water->buoyancy() * 2450.0 * submerged;
            box.velocity.setY(static_cast<float>(
                box.velocity.y() + acceleration * dt));
            box.velocity.setY(box.velocity.y() * static_cast<float>(
                std::exp(-water->drag() * 2.5 * dt)));
            box.velocity.setY(std::clamp(box.velocity.y(), -230.0f, 280.0f));
        } else {
            box.velocity.setY(
                std::min(
                    box.velocity.y()
                        + static_cast<float>(kGravity * dt),
                    950.0f));
        }

        moveAndCollideOneWay(
            box.rect,
            box.velocity,
            dt,
            blockers,
            oneWayRects());

        if (box.rect.left() < 0.0) {
            box.rect.moveLeft(0.0);
            box.velocity.setX(0.0f);
        }

        if (box.rect.right() > level_.worldSize().width()) {
            box.rect.moveRight(level_.worldSize().width());
            box.velocity.setX(0.0f);
        }

        bool destroyed =
            box.rect.top() > level_.worldSize().height() + 120.0;

        if (!destroyed) {
            for (const QRectF &spike : level_.spikes()) {
                if (box.rect.intersects(spike)) {
                    destroyed = true;
                    break;
                }
            }
        }

        if (destroyed) {
            box.alive = false;
            collisionChanged = true;
            explode(box.rect.center(), QColor(120, 125, 130));
            message_ = "Push box destroyed";
            messageTime_ = 0.8;
        }
    }

    if (collisionChanged) {
        rebuildCollision();
    }
}

const WaterZone *World::waterZoneFor(const QRectF &rect) const
{
    for (const WaterZone &zone : waterZones_) {
        if (zone.overlaps(rect)) {
            return &zone;
        }
    }
    return nullptr;
}

double World::iceFrictionBelow(const QRectF &rect) const
{
    for (const IceSurface &surface : iceSurfaces_) {
        if (surface.supports(rect)) {
            return surface.friction();
        }
    }
    return 1.0;
}

void World::createSplash(const QPointF &position)
{
    for (int i = 0; i < 3; ++i) {
        explode(
            position + QPointF((i - 1) * 9.0, 0.0),
            QColor(150, 225, 255));
    }
}

void World::updateWaterObjects(double dt)
{
    constexpr double gravity = 1900.0;

    for (Barrel &barrel : barrels_) {
        if (!barrel.alive) {
            continue;
        }

        const WaterZone *water = waterZoneFor(barrel.rect);
        if (water) {
            if (barrel.fuse >= 0.0) {
                barrel.fuse = -1.0;
                message_ = "Barrel fuse extinguished";
                messageTime_ = 0.8;
                createSplash(QPointF(barrel.rect.center().x(), water->rect().top()));
            }

            const double submerged = water->submergedFraction(barrel.rect);
            const double acceleration =
                gravity - water->buoyancy() * 2650.0 * submerged;
            barrel.velocity.setY(static_cast<float>(
                barrel.velocity.y() + acceleration * dt));
            barrel.velocity *= static_cast<float>(
                std::exp(-water->drag() * 2.8 * dt));
            barrel.velocity.setY(std::clamp(barrel.velocity.y(), -210.0f, 260.0f));
        } else {
            barrel.velocity.setY(std::min(
                barrel.velocity.y() + static_cast<float>(gravity * dt),
                900.0f));
            barrel.velocity.setX(barrel.velocity.x() * 0.85f);
        }

        QVector<QRectF> blockers = level_.platforms();
        for (const MovingPlatform &platform : moving_) {
            blockers << platform.rect;
        }
        for (const Door &door : doors_) {
            if (!door.open) {
                blockers << door.rect;
            }
        }
        for (const Crate &crate : crates_) {
            if (crate.alive) {
                blockers << crate.rect;
            }
        }

        moveAndCollideOneWay(
            barrel.rect,
            barrel.velocity,
            dt,
            blockers,
            oneWayRects());
    }
}

QVector<QRectF> World::oneWayRects() const
{
    QVector<QRectF> rects;
    rects.reserve(oneWayPlatforms_.size());

    for (const OneWayPlatform &platform : oneWayPlatforms_) {
        rects << platform.rect();
    }

    return rects;
}

bool World::playerStandingOnOneWay() const
{
    const QRectF feet(
        player_.rect().left() + 5.0,
        player_.rect().bottom() - 3.0,
        player_.rect().width() - 10.0,
        8.0);

    for (const OneWayPlatform &platform : oneWayPlatforms_) {
        const QRectF &rect = platform.rect();
        if (feet.intersects(rect)
            && player_.rect().bottom() <= rect.top() + 5.0) {
            return true;
        }
    }

    return false;
}

void World::setInput(bool left, bool right, bool up, bool down)
{
    inputLeft_ = left;
    inputRight_ = right;
    inputDown_ = down;

    player_.setLeft(left);
    player_.setRight(right);
    player_.setUp(up);
    player_.setDown(down);
}

void World::jump()
{
    if (completed_ || gameOver_) {
        return;
    }

    if (inputDown_ && playerStandingOnOneWay()) {
        oneWayDropTimer_ = 0.24;
        player_.dropThroughOneWay();
        message_ = "Drop through";
        messageTime_ = 0.45;
        return;
    }

    player_.jump();
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
                    door.latchedOpen = true;
                }
            }
            refreshDoorStates();
            message_ = "Switch activated";
            messageTime_ = 1.5;
            rebuildCollision();
            beep();
        }
    }
}

void World::destroyCrate(Crate &crate)
{
    if (!crate.alive) {
        return;
    }

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
}

void World::applyExplosion(const ExplosionEvent &event)
{
    const auto inside =
        [&event](const QPointF &point) {
            const double dx = point.x() - event.center.x();
            const double dy = point.y() - event.center.y();
            return dx * dx + dy * dy <= event.radius * event.radius;
        };

    if (inside(player_.rect().center())) {
        player_.damage(event.center.x());
    }

    for (auto &enemy : enemies_) {
        if (!enemy.alive() || !inside(enemy.rect().center())) {
            continue;
        }

        const bool wasAlive = enemy.alive();
        enemy.damage(event.damage, event.center.x());

        if (wasAlive && !enemy.alive()) {
            enemyDeath(
                enemy.rect().center(),
                QColor(235, 70, 55),
                enemy.reward(),
                "enemy.legacy.destroy");
        }
    }

    for (Drone &drone : drones_) {
        if (!drone.alive() || !inside(drone.rect().center())) {
            continue;
        }

        const bool wasAlive = drone.alive();
        drone.applyExplosion(event.center, event.damage);
        if (wasAlive && !drone.alive()) {
            enemyDeath(
                drone.rect().center(),
                QColor(255, 95, 40),
                drone.reward(),
                "enemy.drone.destroy");
        }
    }

    for (Turret &turret : turrets_) {
        if (!turret.alive() || !inside(turret.rect().center())) {
            continue;
        }

        const bool wasAlive = turret.alive();
        turret.applyExplosion(event.center, event.damage);
        if (wasAlive && !turret.alive()) {
            enemyDeath(
                turret.rect().center(),
                QColor(255, 130, 35),
                turret.reward(),
                "enemy.turret.destroy");
        }
    }

    for (Charger &charger : chargers_) {
        if (!charger.alive() || !inside(charger.rect().center())) {
            continue;
        }

        const bool wasAlive = charger.alive();
        charger.applyExplosion(event.center, event.damage);
        if (wasAlive && !charger.alive()) {
            enemyDeath(
                charger.rect().center(),
                QColor(255, 95, 40),
                charger.reward(),
                "enemy.charger.destroy");
        }
    }
    for (ShieldSoldier &soldier : shieldSoldiers_) {
        if (!soldier.alive() || !inside(soldier.rect().center())) {
            continue;
        }

        const bool wasAlive = soldier.alive();
        soldier.applyExplosion(event.center, event.damage);
        if (wasAlive && !soldier.alive()) {
            enemyDeath(
                soldier.rect().center(),
                QColor(80, 175, 230),
                soldier.reward(),
                "enemy.shield.destroy");
        }
    }

    bool collisionChanged = false;

    for (auto &crate : crates_) {
        if (crate.alive && inside(crate.rect.center())) {
            destroyCrate(crate);
            collisionChanged = true;
        }
    }

    for (auto &barrel : barrels_) {
        if (barrel.alive
            && barrel.fuse < 0.0
            && inside(barrel.rect.center())) {
            barrel.fuse = 0.10;
        }
    }

    for (auto &box : pushBoxes_) {
        if (!box.alive || !inside(box.rect.center())) {
            continue;
        }

        QVector2D impulse(
            box.rect.center() - event.center);

        if (impulse.lengthSquared() < 1.0f) {
            impulse = QVector2D(1.0f, -0.4f);
        } else {
            impulse.normalize();
        }

        const double distance =
            QLineF(event.center, box.rect.center()).length();
        const double strength =
            std::max(0.2, 1.0 - distance / event.radius);

        box.velocity += impulse
            * static_cast<float>(420.0 * strength);
        box.velocity.setY(
            std::min(box.velocity.y(), -180.0f));
    }

    for (int ring = 0; ring < 4; ++ring) {
        const QColor color =
            ring == 0 ? QColor(255, 245, 150)
            : ring == 1 ? QColor(255, 165, 35)
            : ring == 2 ? QColor(220, 65, 20)
                        : QColor(85, 75, 70);

        explode(
            event.center
                + QPointF((ring - 1.5) * 7.0, -ring * 3.0),
            color);
    }

    combatImpact(
        event.center,
        CombatImpact::Explosion,
        1.35,
        QColor(255, 105, 25));
    requestCombatFeedback(0.36, 10.0, 0.025);
    audioEvent("environment.explosion");
    message_ = "Explosion!";
    messageTime_ = 0.8;

    if (collisionChanged) {
        rebuildCollision();
    }

}

void World::combatImpact(
    const QPointF &position,
    CombatImpact impact,
    double intensity,
    QColor accent)
{
    CombatFeedback::appendImpact(
        particles_,
        position,
        impact,
        intensity,
        accent);
}

void World::enemyDeath(
    const QPointF &position,
    QColor accent,
    int reward,
    const QString &audioEventName)
{
    player_.addScore(reward);
    combatImpact(
        position,
        CombatImpact::EnemyDeath,
        1.20,
        accent);
    combatImpact(
        position + QPointF(8, -9),
        CombatImpact::Explosion,
        0.75,
        QColor(255, 185, 55));
    requestCombatFeedback(0.28, 7.0, 0.040);
    audioEvent(audioEventName);
}

void World::requestCombatFeedback(
    double shakeDuration,
    double shakeStrength,
    double hitStop)
{
    shakeTime_ = std::max(shakeTime_, shakeDuration);
    shakeStrength_ = std::max(shakeStrength_, shakeStrength);
    hitStop_ = std::max(hitStop_, hitStop);
}

void World::audioEvent(const QString &name) const
{
    Q_UNUSED(name);
    beep();
}

void World::explode(QPointF at, QColor color)
{
    for (int i = 0; i < 10; ++i) {
        const double angle = QRandomGenerator::global()->generateDouble() * 6.283;
        const double speed = 60 + QRandomGenerator::global()->bounded(170);
        particles_ << Particle{at,
                               QVector2D(qCos(angle) * speed, qSin(angle) * speed),
                               .35 + QRandomGenerator::global()->generateDouble() * .45,
                               3.0 + QRandomGenerator::global()->bounded(5),
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

    for (const WaterZone &zone : waterZones_) {
        zone.drawBack(painter, cameraX, animationTime_);
    }

    for (const auto &platform : level_.platforms()) {
        drawStaticPlatform(painter, platform.translated(-cameraX, 0));
    }

    for (const auto &ladder : level_.ladders()) {
        drawLadder(painter, ladder.translated(-cameraX, 0));
    }

    for (const auto &platform : moving_) {
        drawMovingPlatform(painter, platform.rect.translated(-cameraX, 0));
    }

    for (const FallingPlatform &platform : fallingPlatforms_) {
        platform.draw(painter, cameraX);
    }

    for (const OneWayPlatform &platform : oneWayPlatforms_) {
        platform.draw(painter, cameraX);
    }

    for (const Conveyor &conveyor : conveyors_) {
        conveyor.draw(painter, cameraX);
    }

    for (const IceSurface &surface : iceSurfaces_) {
        surface.draw(painter, cameraX, animationTime_);
    }

    for (const JumpPad &pad : jumpPads_) {
        pad.draw(painter, cameraX);
    }

    for (const PressurePlate &plate : pressurePlates_) {
        plate.draw(painter, cameraX);
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

    const QColor boxBody(105, 115, 125);
    const QColor boxEdge(48, 55, 63);
    const QColor boxHighlight(175, 188, 198);

    for (const auto &box : pushBoxes_) {
        if (!box.alive) {
            continue;
        }

        const QRectF rect =
            box.rect.translated(-cameraX, 0);

        painter.setBrush(boxBody);
        painter.setPen(QPen(boxEdge, 3));
        painter.drawRoundedRect(rect, 4, 4);

        painter.setPen(QPen(boxHighlight, 3));
        painter.drawLine(
            rect.topLeft() + QPointF(7, 7),
            rect.topRight() + QPointF(-7, 7));

        painter.setPen(QPen(boxEdge, 3));
        painter.drawLine(
            rect.topLeft() + QPointF(8, 8),
            rect.bottomRight() - QPointF(8, 8));
        painter.drawLine(
            rect.topRight() + QPointF(-8, 8),
            rect.bottomLeft() + QPointF(8, -8));

        painter.setPen(QPen(QColor(235, 210, 90), 2));
        painter.drawText(rect, Qt::AlignCenter, "PUSH");
    }

    const QColor barrelRed(165, 42, 35);
    const QColor barrelDark(75, 25, 23);
    const QColor barrelBand(215, 180, 95);

    for (const auto &barrel : barrels_) {
        if (!barrel.alive) {
            continue;
        }

        const QRectF rect =
            barrel.rect.translated(-cameraX, 0);

        painter.setPen(QPen(barrelDark, 3));
        painter.setBrush(barrelRed);
        painter.drawRoundedRect(rect, 8, 8);

        painter.setBrush(barrelBand);
        painter.setPen(Qt::NoPen);
        painter.drawRect(
            QRectF(rect.left(), rect.top() + 8,
                   rect.width(), 5));
        painter.drawRect(
            QRectF(rect.left(), rect.bottom() - 13,
                   rect.width(), 5));

        painter.setPen(QPen(QColor(255, 230, 120), 3));
        painter.drawLine(
            rect.center() + QPointF(-7, -5),
            rect.center() + QPointF(7, 5));
        painter.drawLine(
            rect.center() + QPointF(7, -5),
            rect.center() + QPointF(-7, 5));

        if (barrel.fuse >= 0.0) {
            const double pulse =
                4.0 + qSin(animationTime_ * 28.0) * 2.0;

            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 225, 70, 220));
            painter.drawEllipse(
                rect.topRight() + QPointF(-5, 3),
                pulse,
                pulse);
        }
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

    for (const Drone &drone : drones_) {
        drone.draw(painter, cameraX, animationTime_);
    }

    for (const Turret &turret : turrets_) {
        turret.draw(painter, cameraX, animationTime_);
    }

    for (const Charger &charger : chargers_) {
        charger.draw(painter, cameraX, animationTime_);
    }
    for (const ShieldSoldier &soldier : shieldSoldiers_) {
        soldier.draw(painter, cameraX, animationTime_);
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

    for (const WaterZone &zone : waterZones_) {
        zone.drawFront(painter, cameraX, animationTime_);
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

void World::setGodMode(bool enabled)
{
    player_.setGodMode(enabled);
}

bool World::godMode() const
{
    return player_.godMode();
}

void World::toggleSound() { sound_ = !sound_; }
bool World::soundEnabled() const { return sound_; }
QString World::message() const { return message_; }

QString World::enemyDebugText() const
{
    int legacyAlive = 0;
    int dronesAlive = 0;
    int turretsAlive = 0;
    int chargersAlive = 0;
    int shieldsAlive = 0;

    for (const Enemy &enemy : enemies_) {
        legacyAlive += enemy.alive() ? 1 : 0;
    }
    for (const Drone &drone : drones_) {
        dronesAlive += drone.alive() ? 1 : 0;
    }
    for (const Turret &turret : turrets_) {
        turretsAlive += turret.alive() ? 1 : 0;
    }
    for (const Charger &charger : chargers_) {
        chargersAlive += charger.alive() ? 1 : 0;
    }
    for (const ShieldSoldier &soldier : shieldSoldiers_) {
        shieldsAlive += soldier.alive() ? 1 : 0;
    }

    QString selected = "No framework enemy alive";

    for (const Drone &drone : drones_) {
        if (drone.alive()) {
            selected = drone.debugText();
            break;
        }
    }

    if (selected == "No framework enemy alive") {
        for (const Turret &turret : turrets_) {
            if (turret.alive()) {
                selected = turret.debugText();
                break;
            }
        }
    }

    if (selected == "No framework enemy alive") {
        for (const Charger &charger : chargers_) {
            if (charger.alive()) {
                selected = charger.debugText();
                break;
            }
        }
    }

    if (selected == "No framework enemy alive") {
        for (const ShieldSoldier &soldier : shieldSoldiers_) {
            if (soldier.alive()) {
                selected = soldier.debugText();
                break;
            }
        }
    }

    return QString(
        "%1\nAlive L/D/T/C/S: %2/%3/%4/%5/%6\n"
        "Projectiles: %7  Particles: %8\nShake: %9  Hit-stop: %10")
        .arg(selected)
        .arg(legacyAlive)
        .arg(dronesAlive)
        .arg(turretsAlive)
        .arg(chargersAlive)
        .arg(shieldsAlive)
        .arg(projectiles_.size())
        .arg(particles_.size())
        .arg(shakeStrength_, 0, 'f', 1)
        .arg(hitStop_ > 0.0 ? "ON" : "off");
}
