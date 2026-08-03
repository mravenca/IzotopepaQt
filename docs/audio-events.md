# Audio Event Hooks

M8.5 introduces named combat events. They currently use the existing beep
fallback and are intended for a future audio manager.

- `player.hit`
- `enemy.drone.destroy`
- `enemy.turret.destroy`
- `enemy.charger.destroy`
- `enemy.shield.block`
- `enemy.shield.destroy`
- `enemy.legacy.destroy`
- `environment.explosion`

Event names should remain stable so sound assets can be connected without
changing gameplay code.
