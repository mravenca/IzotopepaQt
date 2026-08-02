# Pressure plates

Pressure plates are momentary world triggers. They publish a named signal when enough weight rests on them and clear that signal when the weight is removed.

## JSON format

```json
"pressurePlates": [
  {
    "x": 2030,
    "y": 552,
    "width": 84,
    "height": 18,
    "target": "bronze",
    "requiredWeight": 1
  }
]
```

`target` identifies the event channel. Doors whose `key` matches that channel open while the plate is pressed. A door opened permanently by a key or wall switch remains open after the plate is released.

`requiredWeight` defaults to `1`. The player, a pushable box, a crate, and a barrel each contribute one unit of weight.

## Runtime behavior

- The plate changes state only when the accumulated weight crosses its threshold.
- State changes are posted through `WorldEventQueue` rather than modifying doors directly.
- Door collision is rebuilt only when a signal actually changes a door state.
- The top plate compresses smoothly and its indicator changes from amber to green.

## Future extensions

The same event channel can later control elevators, lights, timed doors, enemy spawners, and scripted sequences.
