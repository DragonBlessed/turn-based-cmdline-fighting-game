# Turn-Based Command-Line Fighting Game

A fast, hot-seat 1v1 fighting game for two people sharing one terminal. Both
players choose privately, then their actions resolve at the same time.

## Run the game

```bash
bash test_runner.sh
```

The game uses ANSI terminal escape codes to clear the screen between players.
Pass the terminal after selecting an action and do not use scrollback to keep
choices private.

## Rules

Each fighter starts with 12 health and 6 stamina. A fighter at 0 health loses;
if both reach 0 after the same round, the match is a draw.

| Action | Cost | Effect |
|---|---:|---|
| Light Attack | 1 stamina | Deals 2 damage unless blocked. |
| Heavy Attack | 3 stamina | Deals 5 damage unless blocked. |
| Guard | 0 stamina | Blocks attacks and restores 1 stamina. |
| Recover | 0 stamina | Restores 2 stamina but does not block damage. |

Stamina never exceeds 6. Actions that cost more stamina than a player has are
rejected before the round resolves.

## Test the combat rules

```bash
bash run_tests.sh
```

The tests cover simultaneous damage, guarding, recovery, stamina caps, and
double knockouts.

## Project structure

- `main.cpp` — terminal interaction and match loop
- `fighter.h` / `fighter.cpp` — deterministic combat rules
- `tests/fighter_tests.cpp` — unit tests for combat
- `specs/turn-based-fighter-plan.md` — approved specification and task plan
