# Falling platforms

Falling platforms remain solid until a supported object stands on them continuously for the confirmation time. After confirmation, the countdown cannot be cancelled by stepping away.

## JSON

```json
"fallingPlatforms": [
  {
    "rect": [900, 420, 180, 24],
    "material": "stone",
    "confirmationTime": 0.15,
    "fallDelay": 8.0,
    "respawnDelay": 4.0
  }
]
```

`fallDelay` is optional. Material defaults are:

- `stone`: 8 seconds, medium fall speed
- `wood`: 5 seconds, faster fall
- `metal`: 10 seconds, slower fall and warning lamps
- `ice`: 3 seconds, fast warning and icy rendering

The player, pushable boxes, and living enemies can trigger a platform. Contact must remain continuous for `confirmationTime`. A brief edge brush does not arm it.

After the platform leaves the world, it waits for `respawnDelay`. It only respawns when its original area is clear of the player, boxes, enemies, crates, and barrels.
