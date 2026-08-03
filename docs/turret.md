# Turret enemy

Turrets are stationary area-control enemies introduced in M8.2.

## JSON

```json
{
  "kind": "turret",
  "x": 2450,
  "y": 510,
  "mount": "floor",
  "direction": "left",
  "vision": 400,
  "visionAngle": 100,
  "rotationSpeed": 180,
  "projectileSpeed": 420,
  "health": 4,
  "burst": 3,
  "reload": 2.2
}
```

`mount` accepts `floor` or `ceiling`. `direction` defines the centre of the
vision cone, so approaching from behind creates a blind spot. Turrets track a
visible player, rotate at the configured speed and fire sequential bursts.
Explosions deal double damage.

## Architecture

`EnemyFactory` centralises creation of new-framework enemies. Legacy walkers,
shooters, jumpers and the boss remain on the original `Enemy` implementation
until their later migration.
