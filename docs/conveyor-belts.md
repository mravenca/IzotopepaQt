# Conveyor belts

Conveyor belts are animated surface objects that move actors horizontally.

## JSON format

```json
"conveyors": [
  {
    "x": 880,
    "y": 552,
    "width": 240,
    "height": 18,
    "speed": 110
  }
]
```

Positive `speed` moves objects to the right. Negative values move them to the
left. The loader clamps speed to the range -500 through 500 pixels per second.

## Current interactions

- The player is carried while standing on the belt.
- Pushable boxes inherit the belt speed and can still be pushed manually.
- Multiple overlapping belts combine their speeds, with the final value
  clamped to the supported range.
- Conveyors are surfaces rather than solid geometry. A normal platform should
  exist directly below a conveyor in a level.

Crates and explosive barrels are currently static world props and therefore do
not move on conveyors. They can be converted to physics objects in a later
milestone without changing the conveyor JSON format.

## Design guidance

Use moderate speeds between 80 and 180 pixels per second for ordinary traversal.
Faster belts work best for short timing challenges. Place pressure plates or
jump pads near belt exits to combine mechanics.
