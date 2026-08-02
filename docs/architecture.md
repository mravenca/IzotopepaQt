# Izotopepa architecture

## Executables

- `IzotopepaQtGameV3` is the game runtime.
- `IzotopepaLevelEditor` is the read-only JSON level viewer/editor foundation.

## Runtime modules

- `GameWidget` owns the Qt window, input dispatch, camera and top-level drawing.
- `World` owns the active level state, collisions, objects, combat and particles.
- `Player` and `Enemy` implement actor movement and combat behaviour.
- `Level` loads content from JSON files in `assets/levels`.
- `support.*` contains shared sprite, camera and collision helpers.

## Level loading

The runtime searches for external JSON levels first, then falls back to embedded resources. This allows level edits to be tested without rebuilding when the game is run from the repository root.

## Current interactive objects

- moving platforms
- ladders and spikes
- pickups, coins, keys, doors and switches
- breakable crates
- explosive barrels

## Cleanup rule

New object types should receive a dedicated data structure and helper methods. Avoid adding nested type declarations or relying on generated text-replacement patches.
