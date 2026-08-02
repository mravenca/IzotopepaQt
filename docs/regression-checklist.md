# Regression checklist

Before committing an engine change, verify:

1. Both CMake targets compile.
2. All three levels load from JSON.
3. Walking, variable jumping, coyote time and jump buffering work.
4. Ladders can be entered from the top and bottom.
5. Moving platforms carry the player.
6. Walkers avoid edges; shooters and jumpers behave normally.
7. Player and hostile projectiles collide correctly.
8. Crates break and release their configured drops.
9. Barrels explode, damage actors and trigger chain reactions.
10. Keys, doors, switches, checkpoints and goals still function.
11. The level editor opens each JSON level.
