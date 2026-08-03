#include "level.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QTextStream>
#include <QDebug>

#include <algorithm>

namespace {

QRectF rectFromArray(const QJsonValue &value)
{
    const QJsonArray a = value.toArray();
    if (a.size() < 4) {
        return {};
    }

    return QRectF(
        a[0].toDouble(),
        a[1].toDouble(),
        a[2].toDouble(),
        a[3].toDouble());
}

QPointF pointFromArray(const QJsonValue &value)
{
    const QJsonArray a = value.toArray();
    if (a.size() < 2) {
        return {};
    }

    return QPointF(a[0].toDouble(), a[1].toDouble());
}

QString resolveLevelPath(const QString &requested)
{
    if (requested.startsWith(":/")) {
        return requested;
    }

    const QString fileName = QFileInfo(requested).fileName();
    const QString appDir = QCoreApplication::applicationDirPath();

    const QStringList candidates {
        requested,
        QDir::current().filePath("assets/levels/" + fileName),
        QDir(appDir).filePath("assets/levels/" + fileName),
        QDir(appDir).filePath("levels/" + fileName),
        ":/levels/" + fileName
    };

    for (const QString &candidate : candidates) {
        if (QFile::exists(candidate)) {
            return candidate;
        }
    }

    return requested;
}

} // namespace

bool Level::load(const QString &requestedPath)
{
    const QString path = resolveLevelPath(requestedPath);
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open level:" << path;
        return false;
    }

    const QByteArray data = file.readAll();
    QJsonParseError error;
    const QJsonDocument document =
        QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError
        || !document.isObject()) {
        qWarning() << "Invalid JSON level" << path
                   << error.errorString()
                   << "at offset" << error.offset;
        return false;
    }

    platforms_.clear();
    moving_.clear();
    ladders_.clear();
    spikes_.clear();
    enemies_.clear();
    coins_.clear();
    pickups_.clear();
    doors_.clear();
    switches_.clear();
    keys_.clear();
    crates_.clear();
    barrels_.clear();
    pushBoxes_.clear();
    jumpPads_.clear();
    pressurePlates_.clear();
    conveyors_.clear();
    oneWayPlatforms_.clear();
    fallingPlatforms_.clear();
    iceSurfaces_.clear();
    waterZones_.clear();
    checkpoint_ = QPointF(-1, -1);

    const QJsonObject root = document.object();

    name_ = root.value("name").toString("Level");

    const QJsonObject world = root.value("world").toObject();
    world_ = QSizeF(
        world.value("width").toDouble(960),
        world.value("height").toDouble(640));

    const QJsonObject player = root.value("player").toObject();
    spawn_ = QPointF(
        player.value("x").toDouble(80),
        player.value("y").toDouble(450));

    for (const QJsonValue &value : root.value("platforms").toArray()) {
        platforms_ << rectFromArray(value);
    }

    for (const QJsonValue &value :
         root.value("movingPlatforms").toArray()) {
        const QJsonObject object = value.toObject();
        MovingSpawn moving;
        moving.rect = rectFromArray(object.value("rect"));
        moving.minX = object.value("minX").toDouble(moving.rect.left());
        moving.maxX = object.value("maxX").toDouble(moving.rect.left());
        moving.speedX = object.value("speedX").toDouble();
        moving.speedY = object.value("speedY").toDouble();
        moving_ << moving;
    }

    for (const QJsonValue &value : root.value("ladders").toArray()) {
        ladders_ << rectFromArray(value);
    }

    for (const QJsonValue &value : root.value("spikes").toArray()) {
        spikes_ << rectFromArray(value);
    }

    for (const QJsonValue &value : root.value("enemies").toArray()) {
        const QJsonObject object = value.toObject();
        EnemySpawn spawn;
        spawn.kind = object.value("kind").toString("walker");
        spawn.position = QPointF(
            object.value("x").toDouble(),
            object.value("y").toDouble());
        spawn.leftLimit = object.value("left").toDouble();
        spawn.rightLimit = object.value("right").toDouble();
        spawn.speed = object.value("speed").toDouble(140.0);
        spawn.vision = object.value("vision").toDouble(420.0);
        spawn.health = std::max(1, object.value("health").toInt(3));
        spawn.burst = std::clamp(object.value("burst").toInt(3), 1, 6);
        spawn.reload = std::max(0.1, object.value("reload").toDouble(2.0));
        spawn.mount = object.value("mount").toString("floor");
        const QString direction = object.value("direction").toString("left");
        spawn.direction = direction.compare("right", Qt::CaseInsensitive) == 0 ? 1 : -1;
        spawn.visionAngle = std::clamp(
            object.value("visionAngle").toDouble(100.0),
            20.0,
            300.0);
        spawn.rotationSpeed = std::max(
            20.0,
            object.value("rotationSpeed").toDouble(180.0));
        spawn.projectileSpeed = std::max(
            80.0,
            object.value("projectileSpeed").toDouble(420.0));
        spawn.warningTime = std::max(
            0.1,
            object.value("warning").toDouble(0.6));
        spawn.stunTime = std::max(
            0.2,
            object.value("stun").toDouble(2.5));
        spawn.contactDamage = std::max(
            1,
            object.value("damage").toInt(1));
        spawn.shieldAngle = std::clamp(
            object.value("shieldAngle").toDouble(130.0),
            45.0,
            180.0);
        spawn.fireCooldown = std::max(
            0.3,
            object.value("fireCooldown").toDouble(2.4));

        for (const QJsonValue &patrolValue : object.value("patrol").toArray()) {
            spawn.patrol << pointFromArray(patrolValue);
        }

        enemies_ << spawn;
    }

    for (const QJsonValue &value : root.value("coins").toArray()) {
        coins_ << pointFromArray(value);
    }

    for (const QJsonValue &value : root.value("pickups").toArray()) {
        const QJsonObject object = value.toObject();
        pickups_ << PickupSpawn {
            object.value("kind").toString("ammo"),
            QPointF(
                object.value("x").toDouble(),
                object.value("y").toDouble())
        };
    }

    for (const QJsonValue &value : root.value("keys").toArray()) {
        const QJsonObject object = value.toObject();
        keys_ << KeySpawn {
            object.value("key").toString(),
            QPointF(
                object.value("x").toDouble(),
                object.value("y").toDouble())
        };
    }

    for (const QJsonValue &value : root.value("doors").toArray()) {
        const QJsonObject object = value.toObject();
        doors_ << DoorSpawn {
            object.value("key").toString(),
            rectFromArray(object.value("rect"))
        };
    }

    for (const QJsonValue &value : root.value("switches").toArray()) {
        const QJsonObject object = value.toObject();
        switches_ << SwitchSpawn {
            object.value("key").toString(),
            QPointF(
                object.value("x").toDouble(),
                object.value("y").toDouble())
        };
    }

    for (const QJsonValue &value : root.value("crates").toArray()) {
        const QJsonObject object = value.toObject();
        crates_ << CrateSpawn {
            QPointF(
                object.value("x").toDouble(),
                object.value("y").toDouble()),
            object.value("drop").toString("none"),
            std::max(1, object.value("hp").toInt(1))
        };
    }

    for (const QJsonValue &value : root.value("barrels").toArray()) {
        const QJsonObject object = value.toObject();
        barrels_ << BarrelSpawn {
            QPointF(
                object.value("x").toDouble(),
                object.value("y").toDouble()),
            std::max(50.0, object.value("radius").toDouble(150.0)),
            std::max(1, object.value("damage").toInt(3))
        };
    }

    for (const QJsonValue &value :
         root.value("pushableBoxes").toArray()) {
        const QJsonObject object = value.toObject();
        pushBoxes_ << PushBoxSpawn {
            QPointF(
                object.value("x").toDouble(),
                object.value("y").toDouble()),
            std::max(24.0, object.value("width").toDouble(48.0)),
            std::max(24.0, object.value("height").toDouble(48.0))
        };
    }

    for (const QJsonValue &value : root.value("pressurePlates").toArray()) {
        const QJsonObject object = value.toObject();
        pressurePlates_ << PressurePlateSpawn {
            QPointF(
                object.value("x").toDouble(),
                object.value("y").toDouble()),
            std::max(32.0, object.value("width").toDouble(72.0)),
            std::max(10.0, object.value("height").toDouble(18.0)),
            object.value("target").toString(),
            std::max(0.1, object.value("requiredWeight").toDouble(1.0))
        };
    }

    for (const QJsonValue &value : root.value("conveyors").toArray()) {
        const QJsonObject object = value.toObject();
        conveyors_ << ConveyorSpawn {
            QRectF(
                object.value("x").toDouble(),
                object.value("y").toDouble(),
                std::max(32.0, object.value("width").toDouble(128.0)),
                std::max(12.0, object.value("height").toDouble(20.0))),
            std::clamp(object.value("speed").toDouble(120.0), -500.0, 500.0)
        };
    }

    for (const QJsonValue &value : root.value("oneWayPlatforms").toArray()) {
        QRectF rect;

        if (value.isArray()) {
            rect = rectFromArray(value);
        } else {
            const QJsonObject object = value.toObject();
            rect = QRectF(
                object.value("x").toDouble(),
                object.value("y").toDouble(),
                std::max(32.0, object.value("width").toDouble(128.0)),
                std::max(8.0, object.value("height").toDouble(16.0)));
        }

        if (!rect.isEmpty()) {
            oneWayPlatforms_ << OneWayPlatformSpawn {rect};
        }
    }

    for (const QJsonValue &value : root.value("jumpPads").toArray()) {
        const QJsonObject object = value.toObject();
        jumpPads_ << JumpPadSpawn {
            QPointF(
                object.value("x").toDouble(),
                object.value("y").toDouble()),
            std::max(32.0, object.value("width").toDouble(72.0)),
            std::max(12.0, object.value("height").toDouble(20.0)),
            std::max(100.0, object.value("strength").toDouble(900.0)),
            object.value("horizontal").toDouble(0.0),
            std::clamp(object.value("delay").toDouble(0.08), 0.0, 0.35),
            std::max(0.05, object.value("cooldown").toDouble(0.25))
        };
    }

    for (const QJsonValue &value : root.value("fallingPlatforms").toArray()) {
        const QJsonObject object = value.toObject();
        const QRectF rect = rectFromArray(object.value("rect"));

        if (rect.width() <= 0.0 || rect.height() <= 0.0) {
            continue;
        }

        fallingPlatforms_ << FallingPlatformSpawn {
            rect,
            object.value("material").toString("stone"),
            std::max(0.01, object.value("confirmationTime").toDouble(0.15)),
            object.contains("fallDelay")
                ? std::max(0.1, object.value("fallDelay").toDouble())
                : -1.0,
            std::max(0.0, object.value("respawnDelay").toDouble(4.0))
        };
    }

    for (const QJsonValue &value : root.value("iceSurfaces").toArray()) {
        const QJsonObject object = value.toObject();
        const QRectF rect = rectFromArray(object.value("rect"));
        if (!rect.isEmpty()) {
            iceSurfaces_ << IceSurfaceSpawn {
                rect,
                std::clamp(object.value("friction").toDouble(0.08), 0.01, 1.0)
            };
        }
    }

    for (const QJsonValue &value : root.value("waterZones").toArray()) {
        const QJsonObject object = value.toObject();
        const QRectF rect = rectFromArray(object.value("rect"));
        if (!rect.isEmpty()) {
            waterZones_ << WaterZoneSpawn {
                rect,
                std::clamp(object.value("buoyancy").toDouble(0.72), 0.0, 1.5),
                std::clamp(object.value("drag").toDouble(0.55), 0.0, 1.0)
            };
        }
    }

    if (root.contains("checkpoint")) {
        checkpoint_ =
            pointFromArray(root.value("checkpoint"));
    }

    goal_ = rectFromArray(root.value("goal"));

    if (platforms_.isEmpty()) {
        qWarning() << "Level contains no platforms:" << path;
        return false;
    }

    qInfo() << "Loaded level:" << path;
    return true;
}

QString Level::name() const { return name_; }
QSizeF Level::worldSize() const { return world_; }
QPointF Level::playerSpawn() const { return spawn_; }
const QVector<QRectF> &Level::platforms() const { return platforms_; }
const QVector<MovingSpawn> &Level::moving() const { return moving_; }
const QVector<QRectF> &Level::ladders() const { return ladders_; }
const QVector<QRectF> &Level::spikes() const { return spikes_; }
const QVector<EnemySpawn> &Level::enemies() const { return enemies_; }
const QVector<QPointF> &Level::coins() const { return coins_; }
const QVector<PickupSpawn> &Level::pickups() const { return pickups_; }
const QVector<DoorSpawn> &Level::doors() const { return doors_; }
const QVector<SwitchSpawn> &Level::switches() const { return switches_; }
const QVector<KeySpawn> &Level::keys() const { return keys_; }
const QVector<CrateSpawn> &Level::crates() const { return crates_; }
const QVector<BarrelSpawn> &Level::barrels() const { return barrels_; }
const QVector<PushBoxSpawn> &Level::pushBoxes() const { return pushBoxes_; }
const QVector<JumpPadSpawn> &Level::jumpPads() const { return jumpPads_; }
const QVector<PressurePlateSpawn> &Level::pressurePlates() const { return pressurePlates_; }
const QVector<ConveyorSpawn> &Level::conveyors() const { return conveyors_; }
const QVector<OneWayPlatformSpawn> &Level::oneWayPlatforms() const { return oneWayPlatforms_; }
const QVector<FallingPlatformSpawn> &Level::fallingPlatforms() const { return fallingPlatforms_; }
const QVector<IceSurfaceSpawn> &Level::iceSurfaces() const { return iceSurfaces_; }
const QVector<WaterZoneSpawn> &Level::waterZones() const { return waterZones_; }
QPointF Level::checkpoint() const { return checkpoint_; }
QRectF Level::goal() const { return goal_; }
