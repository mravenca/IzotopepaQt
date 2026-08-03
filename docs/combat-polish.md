# M8.5 Combat Polish

M8.5 standardizes combat presentation without changing the core damage model.

## Feedback rules

- Ordinary bullet hits produce a short yellow or orange impact burst.
- Metal targets use brighter, faster sparks.
- Shield blocks use blue sparks and a small hit-stop.
- Enemy deaths use a larger two-stage burst and approximately 40 ms of hit-stop.
- Explosions use the strongest particle burst and camera impulse.

Hit-stop freezes world simulation for a very short interval. It is intentionally
limited to major feedback events and must remain subtle.

## Architecture

`CombatFeedback` owns reusable particle recipes. `World` remains responsible
for score, camera shake, hit-stop and audio-event dispatch because those are
world-level effects.

The audio event hook currently maps to the existing application beep. The
string event names are stable integration points for a future audio manager.
