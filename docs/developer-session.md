# Developer Session

Developer sessions isolate testing from normal campaign progression.

## Activation

A developer session starts automatically when any of these options is used:

```text
--level <1-10>
--level-file <path>
--god
```

`--debug` by itself only enables the F3 overlay and does not disable campaign saving.

## Save isolation

While Developer Mode is active:

- level completion does not update `unlockedLevel`;
- the in-memory unlocked-level value is not advanced;
- future progression and achievement writes should use the same session check.

The existing save data may still be read for settings and normal menu state.

## Identification

Developer Mode is visible through:

- the application window title;
- a persistent upper-right watermark;
- the F3 developer HUD, which reports progress saving as disabled.
