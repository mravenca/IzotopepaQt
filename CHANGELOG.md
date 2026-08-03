## M8.1 - Enemy Framework and Drone

### Added
- Reusable EnemyBrain state machine.
- Flying drone with waypoint patrol, perception, burst fire, knockback, animation, and F3 diagnostics.

## M7 — Factory Escape Showcase World

### Added
- Ten-level campaign covering all mechanics from M1 through M6.
- Final mixed-mechanics escape level and boss encounter.
- Showcase-world design and regression documentation.

### Changed
- Campaign progression now supports ten levels instead of three.
- Level-complete flow advances through level 10.
- All ten JSON levels are embedded as resource fallbacks.

## M6 — Frozen Depths

- Added ice surfaces and water environment volumes.
- Added swimming, buoyancy, drag, splashes and bubbles.
- Added water interaction for boxes, barrels, enemies and projectiles.

## M5 — The Floor Is Gone

### Added
- Falling platforms triggered by player, boxes, or enemies
- 0.15 second standing confirmation before activation
- Warning animation, falling physics, and safe automatic respawn
- Stone, wood, metal, and ice material behavior

## M4 — Trust Your Step

### Added
- Directional one-way platform collision.
- Down + Jump drop-through control.
- One-way support for players, enemies, and pushable boxes.
- JSON level definitions and dedicated vector rendering.

## M3 — Assembly Line

### Added
- Animated conveyor belts with configurable direction and speed.
- Conveyor transport for the player and pushable boxes.
- Conveyor documentation and regression checklist.

# Changelog

## Unreleased

### Added
- M1 Spring Awakening: animated JSON-configurable jump pads.
- Jump-pad launching for the player and pushable boxes.
- Jump-pad documentation and regression checklist.
## M2 - The Weight of Things

### Added
- Momentary pressure plates with configurable weight thresholds.
- Lightweight world event queue with named signal channels.
- Pressure-driven puzzle doors compatible with existing keys and switches.
- Pressure-plate animation, indicator light, documentation, and level examples.

