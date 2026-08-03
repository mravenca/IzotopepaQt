
## Developer Tools DT3 — Debug Overlay 2.0

### Added
- Structured F3 developer HUD with session, performance, player, enemy, combat, and camera diagnostics.
- Shift+F3 compact debug layout.
- Player movement-state and collected-key diagnostics.
- Shared world debug statistics for enemy and effect counts.
## Developer Tools 1.0 — DT2 Runtime Shortcuts

### Added
- F5 reloads the current level from disk.
- F6 reloads an external `--level-file` session.
- Ctrl+R restarts runtime state without reparsing JSON.
- Ctrl+PageUp and Ctrl+PageDown switch campaign levels.
- Short on-screen status confirmations for developer actions.

## Developer Tools 1.0 — DT1

### Added
- `--level 1-10` for direct campaign-level startup.
- `--level-file <path>` for loading an external JSON level.
- `--debug` for enabling the F3 overlay at startup.
- `--god` for disabling player damage.
- Clear developer-mode window titles.

## M8.5 - Combat Polish

### Added
- Shared combat impact and death particle recipes.
- Subtle hit-stop for major impacts and enemy deaths.
- Named audio-event hooks with the existing beep fallback.
- Expanded F3 combat diagnostics and balancing documentation.

## M8.4 - Shield Soldier

### Added
- Tactical Shield Soldier with frontal bullet blocking.
- Vulnerability from behind, above, explosions, and environmental hazards.
- Jump-pad, conveyor, water, ice, and falling-platform integration.
- F3 diagnostics and campaign placements.

## M8.3 - Charger

### Added
- Charger enemy with warning, charge, stun and recovery states.
- Interactions with boxes, crates, barrels, jump pads, conveyors, water, ice and falling platforms.
- Charger JSON configuration, diagnostics and campaign placements.

## M8.2 - Turret and Enemy Factory

- Added floor- and ceiling-mounted turrets with directional vision cones.
- Added sequential burst fire, rotation, recoil and muzzle flash.
- Added explosion vulnerability and debug diagnostics.
- Added `EnemyFactory` for new-framework enemy creation.

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

