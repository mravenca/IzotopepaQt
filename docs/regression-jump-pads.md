# Jump-pad regression checklist

- Player landing from above triggers one launch.
- Walking across a pad without landing does not cause repeated launches.
- The JSON `strength` value changes the launch height.
- Positive and negative `horizontal` values launch in the expected direction.
- A pushable box can be launched.
- A box and player can be queued during the same compression cycle.
- Pad animation returns to idle after cooldown.
- Launch particles and camera shake occur once per activation.
- Existing ladders, moving platforms, crates, barrels, and pushable boxes still work.
- Restarting a level restores all pads to idle state.
