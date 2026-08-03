# Turret regression checklist

- Floor and ceiling turrets load from JSON.
- The initial direction and rear blind spot work.
- A turret tracks only targets inside its configured cone and range.
- Rotation speed, burst size, reload and projectile speed respect JSON.
- Shots are emitted sequentially rather than simultaneously.
- Player bullets damage and destroy turrets.
- Explosions deal increased damage and award score once.
- F3 shows turret state when no living drone is selected.
- Existing drones and legacy enemies still behave normally.
- Campaign levels 1 through 10 still load and complete.
