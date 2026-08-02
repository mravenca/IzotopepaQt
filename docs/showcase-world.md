# M7 — Factory Escape Showcase World

M7 replaces the original three test maps with a ten-level campaign. Each level introduces or combines one or more existing mechanics.

1. **Wake Up** — movement, jumping, ladders and basic enemies.
2. **Heavy Work** — pushable boxes.
3. **Spring Awakening** — jump pads and moving platforms.
4. **The Weight of Things** — pressure plates and doors.
5. **Assembly Line** — conveyor-belt puzzles.
6. **Trust Your Step** — one-way vertical traversal.
7. **The Floor Is Gone** — falling-platform timing.
8. **Frozen Depths** — ice and water.
9. **Systems Test** — combines the environmental systems.
10. **Factory Escape** — final mixed-mechanics level and boss encounter.

## Progression

Completing a level unlocks the next one. Progress is stored with `QSettings` under `unlockedLevel`, now clamped to the range 0–9. Completing level 10 restarts the campaign when the player chooses the first completion-menu option.

## Level files

The campaign is stored in `assets/levels/level1.json` through `level10.json`. External files remain preferred over Qt resources, so designers can edit and reload levels without recompiling.
