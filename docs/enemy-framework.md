# Enemy framework

M8.1 introduces `EnemyBrain`, a reusable state machine for perception, target memory, firing cooldowns, and debug state reporting. Existing ground enemies remain compatible while new enemy classes can adopt the framework incrementally.

States: Patrol, Alert, Attack, Search, Return.
