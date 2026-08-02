# M4 — Trust Your Step: One-way platforms

One-way platforms are solid only while an actor is falling onto their top surface.
Actors can travel upward through them without colliding with their underside or sides.

## JSON

```json
"oneWayPlatforms": [
  [820, 410, 180, 16],
  {"x": 1490, "y": 300, "width": 180, "height": 16}
]
```

Both array and object forms are accepted.

## Player controls

- Jump upward through a platform normally.
- Land on its top while falling.
- Hold **Down** and press **Jump** to drop through.

## Physics interactions

- Pushable boxes pass upward through a one-way platform when launched by a jump pad.
- Boxes land on the platform while falling.
- Enemies use the same directional collision rule.
- One-way platforms do not block projectiles.
