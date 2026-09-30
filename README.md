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

Before the match, each player publicly chooses a fighter. Both players may
choose the same fighter.

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

Each fighter also has one signature action:

| Fighter | Signature action | Cost | Effect |
|---|---|---:|---|
| Brawler | Haymaker | 4 stamina | Deals 6 damage unless blocked by Guard. |
| Tactician | Feint | 2 stamina | Deals 1 damage unless blocked; if the opponent used Guard, it prevents that Guard from regaining 1 stamina. |

All damage resolves simultaneously. A Guard blocks any attack, including a
signature action. Feint does not prevent Recover from restoring stamina.

## Test the combat rules

```bash
bash run_tests.sh
```

The tests cover simultaneous damage, guarding, recovery, stamina caps, and
double knockouts, fighter-specific move availability, signature actions, and
mirror-match symmetry.

## Balance validation

The signature-action values are initial targets, not a claim of completed
balance testing. The approved validation process requires 20 human-played
matches: 10 Brawler-versus-Tactician games in five paired sets with players
swapping fighters, plus five games of each mirror matchup. The targets are a
40–60% cross-fighter win rate and a 5–8 round median across all games.

No human-playtest results have been recorded yet. Use this table when running
the validation; tune only one numeric value at a time if a target is missed.

| Matchup | Games | Brawler wins | Tactician wins | Draws | Median rounds | Observation |
|---|---:|---:|---:|---:|---:|---|
| Brawler vs. Tactician | 10 | pending | pending | pending | pending | pending human playtest |
| Brawler vs. Brawler | 5 | n/a | n/a | pending | pending | pending human playtest |
| Tactician vs. Tactician | 5 | n/a | n/a | pending | pending | pending human playtest |

## Project structure

- `main.cpp` — terminal interaction and match loop
- `fighter.h` / `fighter.cpp` — deterministic combat rules
- `tests/fighter_tests.cpp` — unit tests for combat
- `specs/turn-based-fighter-plan.md` — approved specification and task plan
