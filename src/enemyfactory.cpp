#include "enemyfactory.h"

bool EnemyFactory::isDrone(const EnemySpawn &spawn)
{
    return spawn.kind.compare("drone", Qt::CaseInsensitive) == 0;
}

bool EnemyFactory::isTurret(const EnemySpawn &spawn)
{
    return spawn.kind.compare("turret", Qt::CaseInsensitive) == 0;
}

bool EnemyFactory::isCharger(const EnemySpawn &spawn)
{
    return spawn.kind.compare("charger", Qt::CaseInsensitive) == 0;
}

Drone EnemyFactory::createDrone(const EnemySpawn &spawn)
{
    DroneConfig config;
    config.position = spawn.position;
    config.patrol = spawn.patrol;
    config.speed = spawn.speed;
    config.vision = spawn.vision;
    config.health = spawn.health;
    config.burst = spawn.burst;
    config.reload = spawn.reload;
    return Drone(config);
}

Turret EnemyFactory::createTurret(const EnemySpawn &spawn)
{
    TurretConfig config;
    config.position = spawn.position;
    config.mount = spawn.mount;
    config.direction = spawn.direction;
    config.vision = spawn.vision;
    config.visionAngle = spawn.visionAngle;
    config.rotationSpeed = spawn.rotationSpeed;
    config.projectileSpeed = spawn.projectileSpeed;
    config.health = spawn.health;
    config.burst = spawn.burst;
    config.reload = spawn.reload;
    return Turret(config);
}

Charger EnemyFactory::createCharger(const EnemySpawn &spawn)
{
    ChargerConfig config;
    config.position = spawn.position;
    config.leftLimit = spawn.leftLimit;
    config.rightLimit = spawn.rightLimit;
    config.vision = spawn.vision;
    config.warningTime = spawn.warningTime;
    config.chargeSpeed = spawn.speed;
    config.stunTime = spawn.stunTime;
    config.health = spawn.health;
    config.contactDamage = spawn.contactDamage;
    return Charger(config);
}
