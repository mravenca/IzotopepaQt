#pragma once

#include "drone.h"
#include "level.h"
#include "turret.h"
#include "charger.h"

class EnemyFactory {
public:
    static bool isDrone(const EnemySpawn &spawn);
    static bool isTurret(const EnemySpawn &spawn);
    static bool isCharger(const EnemySpawn &spawn);
    static Drone createDrone(const EnemySpawn &spawn);
    static Turret createTurret(const EnemySpawn &spawn);
    static Charger createCharger(const EnemySpawn &spawn);
};
