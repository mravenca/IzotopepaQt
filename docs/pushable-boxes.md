# Pushable boxes

Levels can define lightweight physics boxes with the `pushableBoxes` array:

```json
"pushableBoxes": [
  { "x": 500, "y": 522, "width": 48, "height": 48 }
]
```

The player pushes a box by walking into either vertical side. Boxes:

- fall under gravity;
- collide with static and moving platforms, closed doors, crates, barrels, and other boxes;
- can stack on each other;
- are carried by moving platforms;
- are pushed by explosions;
- are destroyed by spikes or by falling below the level.

`width` and `height` are optional and default to 48 pixels. Values smaller than 24 pixels are clamped to 24.
