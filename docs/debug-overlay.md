# Debug Overlay 2.0

Press `F3` to toggle the full developer HUD. Press `Shift+F3` to switch between the full and compact layouts.

## Session

The session section shows the current campaign level or external level file, whether developer options are active, and whether God Mode is enabled.

## Performance

- **FPS** is a smoothed frames-per-second value.
- **Frame** is the corresponding approximate frame duration in milliseconds.

## Player

The overlay displays position, velocity, current surface or movement state, health, ammunition, score, collected key count, and temporary invulnerability.

The surface value is one of:

- `ground`
- `air`
- `ladder`
- `water`
- `ice`

## Enemies

Enemy counts are split into legacy enemies, drones, turrets, chargers, and shield soldiers. The selected-enemy section displays the first living framework enemy's detailed diagnostics.

## Combat and camera

The overlay reports active projectiles, particles, camera position, look-ahead, shake strength, and whether hit-stop is active.

## Compact mode

Compact mode shows only FPS, level, total enemies, projectile count, effect count, and the most important player/session state. It is intended for normal playtesting when the full HUD would obstruct the level.
