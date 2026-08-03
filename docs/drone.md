# Drone

The drone is a flying ranged enemy configured through the normal `enemies` JSON array.

```json
{
  "kind": "drone",
  "x": 1800,
  "y": 260,
  "patrol": [[1600,260],[2200,260]],
  "speed": 140,
  "vision": 420,
  "health": 3,
  "burst": 3,
  "reload": 2.0
}
```

It patrols waypoints, remembers the player briefly after losing sight, fires aimed bursts, and receives explosion knockback.
