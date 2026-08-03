# Developer Session Regression Checklist

1. Start normally, complete a level, and verify campaign progress is saved.
2. Start with `--level 7`, complete it, restart normally, and verify progress did not change.
3. Start with `--level-file`, complete it, and verify progress did not change.
4. Start with `--god` and verify Developer Mode starts automatically.
5. Start with `--debug` only and verify the overlay appears but saving remains enabled.
6. Confirm the Developer Mode watermark and window title appear only for isolated sessions.
7. Confirm F3 reports saving as disabled in Developer Mode and enabled normally.
8. Verify F5, F6, Ctrl+R, and level-switch shortcuts preserve the session type.
