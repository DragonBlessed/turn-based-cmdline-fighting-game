# Specification and Implementation Plan: Turn-Based Command-Line Fighter

## Objective

Build a two-player, hot-seat, command-line 1v1 fighting game. Each round, both
players secretly select an action, the terminal clears between selections, and
the actions resolve simultaneously. Matches should usually end in 5–8 rounds
and reward prediction, bluffing, and stamina management rather than a large
move list or random outcomes.

### Success criteria

- Two players can complete a match on one terminal without either player seeing
  the other player's choice during selection.
- Every player has health and stamina visible after each resolved round.
- The core action set is small, understandable, and no action is always best.
- Invalid input and actions that cost more stamina than available do not alter
  game state; the player is prompted again.
- A normal match ends in roughly 5–8 rounds under typical mixed play.
- Combat-resolution logic is covered by automated tests.

## Scope and boundaries

**Always:** validate input, keep combat rules deterministic, show a concise
round recap, test rules independently of terminal I/O.

**Ask first:** add external libraries, change build/CI configuration, add
platform-specific terminal dependencies, or expand the stretch goal beyond one
signature move per fighter.

**Never:** add online play, an AI opponent, real-time input, graphics, a large
roster, or hidden randomness that makes a sound choice fail unpredictably.

**Out of scope:** permanent progression, save files, network play, and an
elaborate UI.

## Technical approach

- Language: C++ using the standard library only.
- `Fighter` stores display name, health, maximum health, stamina, maximum
  stamina, and (later) a fighter identity.
- `Action` is an enum shared by the input and combat layers.
- A pure resolver accepts two fighters and two legal actions, then returns the
  next combat state plus a human-readable round result. It does not read stdin,
  write the terminal, or clear the screen.
- `main.cpp` owns the match loop: render state, collect secret choices, call the
  resolver, and print the recap.
- Screen clearing uses an ANSI escape sequence in a small wrapper. If the
  terminal does not support it, the game still plays correctly; secrecy is a
  social convention rather than a security guarantee.

## Project structure

```text
main.cpp                 terminal input, rendering, and the match loop
fighter.h                combat data types and the public rules contract
fighter.cpp              deterministic combat-rule implementation
tests/fighter_tests.cpp  small, dependency-free unit tests for combat rules
specs/                   requirements and implementation plans
```

## Code style

Use C++17 and the standard library only. Types use `PascalCase`; functions and
variables use `snake_case`; constants start with `k`. Keep terminal I/O in
`main.cpp` and make combat functions operate only on values passed to them.

```cpp
RoundResult resolve_round(Fighter first, Action first_action, Fighter second,
                          Action second_action) {
  // Apply both choices from the same pre-round state.
}
```

## Testing strategy

The combat resolver gets small unit tests because it is deterministic and has
no terminal I/O. Tests use the standard library and a simple assertion helper,
so no dependency is required. The match loop is verified manually by playing
matches that include invalid input, unaffordable actions, a normal win, and a
simultaneous knockout. Every rule or balance change must extend or update the
resolver tests before the behavior is changed.

## Core rules (proposed)

Starting state: **12 health, 6 stamina**, stamina capped at 6.

| Action | Stamina cost | Resolution |
|---|---:|---|
| Light attack | 1 | Deal 2 damage; blocked by Guard. |
| Heavy attack | 3 | Deal 5 damage. |
| Guard | 0 | Block the opponent's attacks; gain 1 stamina after resolution. |
| Recover | 0 | Gain 2 stamina; if hit, still take normal damage. |

Actions resolve from the pre-round state. Stamina costs are paid first; any
action that cannot be afforded is rejected during selection, rather than being
silently replaced. Damage is applied simultaneously, so a double knockout is a
draw. Stamina gains occur after damage, then are capped at 6.

### Design critique

- **Good:** four actions are few enough to learn in one match, while Guard and
  Recover give attacks distinct tradeoffs and prevent Heavy from dominating.
- **Risk:** Heavy may still be too strong despite its cost. Mitigation: tune its
  cost or damage after playtesting before adding a fifth action or exception.
- **Risk:** 12 health and the proposed damage numbers might produce matches
  outside the 5–8 round target. Mitigation: treat these as tuning values and
  playtest 10 matches before declaring balance complete.
- **Risk:** terminal clearing does not guarantee privacy. Mitigation: state the
  hot-seat etiquette before the first round and make the selection screen
  minimal.
- **Good constraint:** no combat randomness makes losses explainable and lets
  players adapt. The tradeoff is that repeated play can become solvable; varied
  human opponents and the stretch signature move are the intended answer, not
  random damage.

## Stretch goal: two fighter identities

After the shared core is balanced, add exactly two selectable fighters, each
with one signature action that follows the same resolver contract:

- **Brawler — Haymaker:** cost 4; deal 6 damage, but it is blocked by Guard and
  dodged completely.
- **Tactician — Feint:** cost 2; deal 1 damage and prevent Guard's stamina gain
  that round.

These proposals are intentionally modest. They supply asymmetry without adding
new resources, turn phases, or special-case input flows. Do not implement this
until the core game is playable and balance-tested.

## Commands

Run the current application:

```bash
bash test_runner.sh
```

Compile a planned unit-test executable (after test sources are added):

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic fighter.cpp fighter_tests.cpp -o fighter_tests && ./fighter_tests
```

## Implementation tasks

### Task 1: Establish the combat domain and tests

**Description:** Create the fighter/action data model and a pure combat resolver
with tests for each core interaction. Keep `main.cpp` unchanged apart from any
necessary includes until the rules work in isolation.

**Acceptance criteria:**

- [ ] Actions have named enum values and documented stamina costs.
- [ ] Simultaneous damage, stamina costs, stamina caps, and draw handling have
  deterministic behavior.
- [ ] Tests cover attack vs. attack, each attack vs. Guard, Recover while hit,
  simultaneous knockout, and cap/cost edge cases.

**Verification:** compile and run the unit-test command above.

**Dependencies:** None.

**Likely files:** `fighter.hpp`, `fighter.cpp`, `fighter_tests.cpp`.

**Estimated scope:** Medium (3 files).

### Task 2: Build the hot-seat match loop

**Description:** Add terminal presentation and validated player input on top of
the tested resolver. Each player receives a private selection screen, followed
by a clear and the other player's selection. Then render the outcome.

**Acceptance criteria:**

- [ ] The game displays both health and stamina before every selection phase.
- [ ] Invalid menu entries and unaffordable actions re-prompt without resolving
  the round.
- [ ] Choices are collected before either action is resolved.
- [ ] The match announces Player 1 win, Player 2 win, or draw and can cleanly
  exit.

**Verification:** compile `main.cpp` with `fighter.cpp`; manually play at least
two matches, including an invalid input and an insufficient-stamina attempt.

**Dependencies:** Task 1.

**Likely files:** `main.cpp`, `fighter.hpp`, `fighter.cpp`.

**Estimated scope:** Medium (3 files).

### Checkpoint: Core game playable

- [ ] Unit tests pass.
- [ ] A complete hot-seat match runs without a crash.
- [ ] Both choices are resolved simultaneously.
- [ ] A human review confirms the rules are understandable from the terminal.

### Task 3: Tune for the 5–8 round target

**Description:** Run at least 10 informal mixed-strategy matches and adjust only
health, stamina maximum, damage, costs, and recovery amounts as needed.

**Acceptance criteria:**

- [ ] Results and observed dominant patterns are recorded in the README.
- [ ] Most observed games finish in the target range, or any deliberate
  deviation is documented.
- [ ] Updated numeric rules retain full resolver test coverage.

**Verification:** run unit tests and record the ten match lengths.

**Dependencies:** Task 2.

**Likely files:** `fighter.cpp`, `fighter_tests.cpp`, `README.md`.

**Estimated scope:** Small (3 files).

### Task 4: Add the optional fighter signatures

**Description:** If the checkpoint indicates the core game is fun and balanced,
add two fighter identities and their one signature action each.

**Acceptance criteria:**

- [ ] Each player selects a fighter before the first round.
- [ ] Signature actions show their cost and effect in the existing action menu.
- [ ] Tests demonstrate both signature actions and their counters.
- [ ] Core actions continue to work identically for either fighter.

**Verification:** unit tests pass; manually play each matchup once.

**Dependencies:** Task 3 and explicit approval to take on the stretch scope.

**Likely files:** `fighter.hpp`, `fighter.cpp`, `fighter_tests.cpp`, `main.cpp`.

**Estimated scope:** Medium (4 files).

### Checkpoint: Complete

- [ ] Tests pass with compiler warnings enabled.
- [ ] README explains setup, actions, and hot-seat privacy convention.
- [ ] The game meets the confirmed core intent without requiring the stretch
  goal.

## Dependency map

```text
Task 1: pure rules + tests
          |
          v
Task 2: terminal match loop
          |
          v
Task 3: playtest and tune
          |
          v
Task 4: optional fighter signatures
```

## Open decisions to revisit during playtesting

- Tune numeric values based on observed match lengths, not intuition alone.
- Add the signature moves only if the base game is already strategically
  replayable; they are flavor, not a substitute for sound core rules.
