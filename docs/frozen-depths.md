# M6 — Frozen Depths

## Ice surfaces

```json
"iceSurfaces": [
  {"rect": [900, 552, 260, 18], "friction": 0.08}
]
```

Ice reduces horizontal acceleration and braking. The player and pushable boxes retain momentum, while conveyors and jump pads continue to work normally.

## Water zones

```json
"waterZones": [
  {"rect": [1400, 420, 500, 220], "buoyancy": 0.72, "drag": 0.55}
]
```

The player has unlimited breathing. Hold Jump or Up to swim upward and Down to descend. Pushable boxes and barrels float. Active barrel fuses are extinguished by water. Underwater barrel explosions use 55% of their normal radius. Enemies move slowly and sink; projectiles lose speed rapidly.

## Future extensions

The environment-volume design can later support lava, poison gas, healing pools, currents and low-gravity zones.
