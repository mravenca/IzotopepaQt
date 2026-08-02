#include "leveldocument.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace {
QRectF readRect(const QJsonValue &value)
{
    const QJsonArray a = value.toArray();
    return a.size() >= 4
        ? QRectF(a[0].toDouble(), a[1].toDouble(),
                 a[2].toDouble(), a[3].toDouble())
        : QRectF();
}

QPointF readPoint(const QJsonValue &value)
{
    const QJsonArray a = value.toArray();
    return a.size() >= 2
        ? QPointF(a[0].toDouble(), a[1].toDouble())
        : QPointF();
}
}

bool LevelDocument::load(const QString &fileName, QString *error)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(file.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError
        || !document.isObject()) {
        if (error)
            *error = QString("JSON error at %1: %2")
                .arg(parseError.offset)
                .arg(parseError.errorString());
        return false;
    }

    platforms_.clear();
    moving_.clear();
    ladders_.clear();
    spikes_.clear();
    enemies_.clear();
    coins_.clear();
    pickups_.clear();
    keys_.clear();
    doors_.clear();
    switches_.clear();
    checkpoint_ = QPointF(-1, -1);

    const QJsonObject root = document.object();
    name_ = root.value("name").toString("Untitled");

    const QJsonObject world = root.value("world").toObject();
    worldSize_ = QSizeF(
        world.value("width").toDouble(960),
        world.value("height").toDouble(640));

    const QJsonObject player = root.value("player").toObject();
    playerSpawn_ = QPointF(
        player.value("x").toDouble(80),
        player.value("y").toDouble(450));

    for (const auto &value : root.value("platforms").toArray())
        platforms_ << readRect(value);

    for (const auto &value : root.value("movingPlatforms").toArray()) {
        const QJsonObject object = value.toObject();
        EditorMovingPlatform item;
        item.rect = readRect(object.value("rect"));
        item.minX = object.value("minX").toDouble(item.rect.left());
        item.maxX = object.value("maxX").toDouble(item.rect.left());
        item.speedX = object.value("speedX").toDouble();
        item.speedY = object.value("speedY").toDouble();
        moving_ << item;
    }

    for (const auto &value : root.value("ladders").toArray())
        ladders_ << readRect(value);

    for (const auto &value : root.value("spikes").toArray())
        spikes_ << readRect(value);

    for (const auto &value : root.value("enemies").toArray()) {
        const QJsonObject object = value.toObject();
        enemies_ << EditorEnemy {
            object.value("kind").toString("walker"),
            QPointF(object.value("x").toDouble(),
                    object.value("y").toDouble()),
            object.value("left").toDouble(),
            object.value("right").toDouble()
        };
    }

    for (const auto &value : root.value("coins").toArray())
        coins_ << readPoint(value);

    for (const auto &value : root.value("pickups").toArray()) {
        const QJsonObject object = value.toObject();
        pickups_ << EditorPickup {
            object.value("kind").toString("ammo"),
            QPointF(object.value("x").toDouble(),
                    object.value("y").toDouble())
        };
    }

    for (const auto &value : root.value("keys").toArray()) {
        const QJsonObject object = value.toObject();
        keys_ << EditorNamedPoint {
            object.value("key").toString(),
            QPointF(object.value("x").toDouble(),
                    object.value("y").toDouble())
        };
    }

    for (const auto &value : root.value("doors").toArray()) {
        const QJsonObject object = value.toObject();
        doors_ << EditorDoor {
            object.value("key").toString(),
            readRect(object.value("rect"))
        };
    }

    for (const auto &value : root.value("switches").toArray()) {
        const QJsonObject object = value.toObject();
        switches_ << EditorNamedPoint {
            object.value("key").toString(),
            QPointF(object.value("x").toDouble(),
                    object.value("y").toDouble())
        };
    }

    if (root.contains("checkpoint"))
        checkpoint_ = readPoint(root.value("checkpoint"));

    if (root.contains("goal"))
        goal_ = readRect(root.value("goal"));

    fileName_ = fileName;
    return true;
}
