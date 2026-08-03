# Charger enemy

The Charger is a ground enemy that patrols until the player enters its forward detection range. It warns before charging, then rushes forward until it hits a wall or another solid gameplay object.

## JSON

```json
{
  "kind": "charger",
  "x": 1820,
  "y": 500,
  "left": 1650,
  "right": 2150,
  "vision": 500,
  "warning": 0.6,
  "speed": 520,
  "stun": 2.5,
  "health": 5,
  "damage": 1
}
```

## Interactions

- Walls and closed doors stun the Charger.
- Pushable boxes stop it and receive an impulse.
- Crates break on impact.
- Barrels ignite immediately.
- Jump pads can launch it.
- Water interrupts a charge.
- Conveyors modify charge speed.
- Ice reduces braking while stunned.
- Falling platforms can be triggered by it.
