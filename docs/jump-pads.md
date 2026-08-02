# Jump pads

Jump pads launch the player and pushable boxes by applying a velocity impulse.
They are defined in each JSON level under `jumpPads`.

```json
"jumpPads": [
  {
    "x": 800,
    "y": 550,
    "width": 72,
    "height": 20,
    "strength": 900,
    "horizontal": 0,
    "delay": 0.08,
    "cooldown": 0.25
  }
]
```

## Fields

- `x`, `y`: top-left world position.
- `width`, `height`: pad dimensions. Defaults are 72 by 20 pixels.
- `strength`: upward launch speed. The engine applies it as a negative Y velocity.
- `horizontal`: optional horizontal launch velocity. Positive values launch right.
- `delay`: compression time before launch, in seconds.
- `cooldown`: minimum time before the pad can trigger again.

## Behaviour

A descending player or pushable box touching the top trigger zone starts the
compression animation. When the delay expires, every queued body still near
the pad receives the configured impulse. A cooldown prevents repeated launches.

The launch produces particles, a short camera shake, and the existing sound
hook. Jump pads are not solid terrain; they should be placed directly on top of
a platform.
