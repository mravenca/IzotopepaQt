# Izotopepa Complete Edition

This is the expanded Qt/C++ version of the game.

## Implemented gameplay

- Three distinct levels with saved progression
- Smooth camera and parallax background
- Player shooting, ammunition and hostile projectiles
- Walker, shooter, jumper and boss enemies
- Health/ammo pickups, coins and score
- Moving platforms, ladders and spike hazards
- Colored keys, locked doors and switches
- Checkpoints and checkpoint respawning
- Particle hit/explosion effects
- Start, help, settings, pause, game-over and completion screens
- Sound toggle and persistent unlocked-level state through `QSettings`

## Build

Open `CMakeLists.txt` in Qt Creator, select your Qt 6 Desktop kit, configure, then build and run.

## Controls

- A/D or Left/Right: move
- Space: jump
- W/Up and S/Down: climb ladders
- F or Ctrl: shoot
- E: activate nearby switches
- P or Esc: pause
- Arrow keys + Enter: menus

## Level format

Levels live under `assets/levels`. Supported records include `PLATFORM`, `MOVING`, `LADDER`, `SPIKE`, `ENEMY`, `COIN`, `PICKUP`, `KEY`, `DOOR`, `SWITCH`, `CHECKPOINT`, and `GOAL`.
