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
platform-specific terminal dependencies, add a third fighter, or give either
fighter more than one signature move.

**Never:** add online play, an AI opponent, real-time input, graphics, a large
roster, or hidden randomness that makes a sound choice fail unpredictably.

**Out of scope:** permanent progression, save files, network play, and an
elaborate UI.

## Technical approach

- Language: C++ using the standard library only.
- `Fighter` stores display name, health, stamina, and fighter identity.
- `FighterType` identifies the two selectable fighters. `Action` remains the
  shared action enum; a small helper determines whether a fighter may select a
  given signature action.
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

## Planned stretch feature: selectable fighters and signature moves

### Objective

Before the first round, each player chooses one of two fighters. Each fighter
has the four existing core actions plus one signature action that only that
fighter can select. This adds a small, visible strategic difference without
introducing randomness, new resources, or a separate combat phase.

### Assumptions to confirm during review

1. Each player chooses independently; mirror matchups (both choose the same
   fighter) are allowed.
2. Players know the two fighter names and signature-move rules before choosing.
3. Fighter selection is public and happens before the private action-selection
   loop; only round actions need hot-seat secrecy.
4. The numeric values below are initial balance targets, subject to playtesting.

### Fighter rules

| Fighter | Signature action | Cost | Effect | Counterplay |
|---|---|---:|---|---|
| Brawler | Haymaker | 4 stamina | Deals 6 damage. | Guard blocks it, as it blocks every attack. |
| Tactician | Feint | 2 stamina | Deals 1 damage. If the opponent chose Guard, that Guard does not restore its usual 1 stamina. | It is low damage and Guard still blocks its damage. |

Haymaker replaces the earlier undefined reference to being "dodged"; there is
no Dodge action in this game. Both signature actions resolve from the same
pre-round state as core actions. A signature action remains unavailable to the
other fighter even when the players share a fighter name.

### Success criteria

- The game presents Brawler and Tactician, including each signature move's
  cost and effect, before round one.
- Both players select a valid fighter before combat begins; invalid input
  re-prompts without changing match state.
- Each action menu displays only the player's legal signature action alongside
  the four unchanged core actions.
- Brawler's Haymaker costs 4 stamina, deals 6 damage, and is blocked by Guard.
- Tactician's Feint costs 2 stamina, deals 1 damage, and prevents Guard's
  one-stamina recovery while the Guard still blocks the Feint's damage.
- Standard actions retain exactly their current behavior for both fighters.
- Unit tests cover availability, affordability, signature effects, Guard
  interactions, simultaneous resolution, and a mirror matchup.
- In a mirror matchup, equivalent fighters using the same action from the same
  combat state produce equivalent health and stamina after the round. Neither
  player position receives a rules advantage.
- After paired playtesting, no fighter wins more than 60% of the recorded
  matches and the median match length remains within the existing 5–8 round
  target. A documented tuning decision is required if either target is missed.

### Implementation plan

1. Extend the combat model with `FighterType` and two signature `Action`
   values. Add a pure legality helper so the terminal code cannot offer a move
   that the resolver would reject.
2. Update the resolver's damage and stamina-gain logic to apply signature
   effects using both actions from the pre-round state. Preserve the existing
   pay-costs, apply-simultaneous-damage, then apply-capped-gains order.
3. Add resolver tests before changing the terminal interface: legal/illegal
   signature selection, Haymaker versus Guard and attack, Feint versus Guard,
   and unchanged core-action behavior.
4. Add a public, validated fighter-selection prompt before the match loop.
   Keep the existing private action prompts and show the selected fighter in
   status and action menus.
5. Run automated tests and manually play Brawler vs. Tactician plus one mirror
   matchup. Record any numeric tuning decision in this document and README.

### Mirror-matchup rules

- Each player may choose either fighter, including the fighter already chosen
  by the other player. Selection order therefore cannot lock a player out of a
  preferred fighter or create a first-picker advantage.
- The player number affects only display order and the winner label. It must
  not affect action availability, damage, stamina cost, stamina gain, or action
  resolution.
- In Brawler-vs.-Brawler and Tactician-vs.-Tactician matches, each player has
  access to the same five actions. Signature actions resolve simultaneously:
  two Haymakers deal damage to each other unless blocked, and two Feints deal
  damage to each other without interacting with Guard recovery.
- Resolver tests must assert symmetry by resolving a mirrored pair of fighters
  and actions, then comparing the resulting health, stamina, and outcome after
  swapping player positions.

### Balance-validation plan

The initial numbers are not declared balanced merely because they are
implemented. Use this repeatable post-implementation check before treating the
stretch feature as complete:

1. Play 10 Brawler-vs.-Tactician matches as five paired sets. In every set,
   the same two people play twice and swap fighters for the second match.
   This reduces the effect of one player simply being stronger.
2. Play five Brawler-vs.-Brawler and five Tactician-vs.-Tactician matches.
   Record round count, winner or draw, signature-action use, and any repeated
   dominant pattern.
3. Calculate win rate by fighter from the cross-fighter matches and the median
   round count across all 20 matches. The target is 40–60% wins for each
   fighter and a 5–8 round median. Draws count separately rather than as wins.
4. If a target is missed, change only one numeric value at a time (signature
   damage, stamina cost, or the Feint stamina-denial effect), add or update its
   resolver test, and repeat the affected matchup set. Do not add new actions,
   randomness, or resources to solve a numerical balance issue.

Record the results in a small table in the README:

| Matchup | Games | Brawler wins | Tactician wins | Draws | Median rounds | Observation |
|---|---:|---:|---:|---:|---:|---|
| Brawler vs. Tactician | 10 |  |  |  |  |  |
| Brawler vs. Brawler | 5 | n/a | n/a |  |  |  |
| Tactician vs. Tactician | 5 | n/a | n/a |  |  |  |

### Risks and mitigations

- **Haymaker may make Brawler dominant.** Its high stamina cost and Guard
  counter are deliberate; validate it in both cross-fighter and mirror matches
  before changing numbers or adding mechanics.
- **Feint's Guard interaction may be hard to notice.** The round recap must
  state when a Guard's stamina gain was denied.
- **Selection can leak an action choice in a hot-seat game.** Selection is
  intentionally public and occurs only once; action choices remain private.
- **Player skill can masquerade as fighter balance.** Paired Brawler-vs.-
  Tactician games require the same players to swap fighters.

## Commands

Run the current application:

```bash
bash test_runner.sh
```

Compile and run the unit tests:

```bash
bash run_tests.sh
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

### Task 4: Add selectable fighters and signature actions

**Description:** Add Brawler and Tactician selection before the match, then
implement their respective Haymaker and Feint actions in the pure combat layer
and terminal menus.

**Acceptance criteria:**

- [ ] Each player independently selects Brawler or Tactician before the first
  round; invalid input re-prompts.
- [ ] Haymaker is offered only to Brawler and Feint only to Tactician; each
  displays its cost and effect in the action menu.
- [ ] Tests demonstrate Haymaker's Guard counter, Feint's denied Guard stamina
  gain, legal/illegal signature availability, and a mirror matchup.
- [ ] Tests prove that mirrored states and actions produce position-independent
  results for Brawler-vs.-Brawler and Tactician-vs.-Tactician.
- [ ] Core actions continue to work identically for either fighter.
- [ ] Round recap explicitly reports a denied Guard stamina gain.
- [ ] The README records the 20-match balance check or explicitly documents why
  a stated balance target was not met.

**Verification:** `bash run_tests.sh`; complete the 20-match balance-validation
plan, including five paired Brawler-vs.-Tactician sets and five games of each
mirror matchup.

**Dependencies:** Task 2 and approval of this specification.

**Likely files:** `fighter.hpp`, `fighter.cpp`, `fighter_tests.cpp`, `main.cpp`.

**Estimated scope:** Medium (4 files).

#### Task 4.1: Extend the combat contract

**Description:** Add fighter identity and the two signature action values to
the public combat types. Define pure helpers for signature-action availability
and retain a single source of truth for action names and stamina costs.

**Acceptance criteria:**

- [ ] `Fighter` identifies Brawler or Tactician.
- [ ] The public API can determine whether a fighter may use any action.
- [ ] Haymaker and Feint have their specified names and stamina costs.
- [ ] Existing core-action names, costs, and affordability remain unchanged.

**Verify:** `bash run_tests.sh` after adding focused availability and cost tests.

**Files:** `fighter.h`, `fighter.cpp`, `tests/fighter_tests.cpp`.

#### Task 4.2: Implement and prove signature resolution

**Description:** Extend the pure resolver to handle Haymaker damage and
Feint's Guard-stamina denial while preserving simultaneous resolution and
capped stamina gains.

**Acceptance criteria:**

- [ ] Haymaker deals 6 damage, costs 4 stamina, and Guard blocks it.
- [ ] Feint deals 1 damage, costs 2 stamina, and suppresses only an opposing
  Guard's one-stamina gain.
- [ ] Feint does not suppress Recover, and Guard still blocks Feint damage.
- [ ] The resolver produces position-independent results for equivalent
  Brawler and Tactician mirror states.

**Verify:** `bash run_tests.sh` with new signature, Guard, and symmetry tests.

**Files:** `fighter.cpp`, `tests/fighter_tests.cpp`.

#### Checkpoint: combat rules complete

- [ ] All resolver tests pass with `-Wall -Wextra -Wpedantic`.
- [ ] Core actions retain their current tested behavior.
- [ ] No terminal I/O was added to the combat layer.

#### Task 4.3: Add fighter and action selection UI

**Description:** Add a public fighter-selection prompt before the match loop
and render the selected fighter plus only its legal signature action in each
player's existing private action menu.

**Acceptance criteria:**

- [ ] Both players can independently select Brawler or Tactician; invalid
  input re-prompts without changing state.
- [ ] Mirror matchups are possible without special-case code or turn-order
  restrictions.
- [ ] Action menus show five actions: the four core actions and the selected
  fighter's signature action, with accurate cost and effect text.
- [ ] An unaffordable signature move re-prompts like an unaffordable core move.

**Verify:** `bash run_tests.sh`; manually start Brawler-vs.-Tactician,
Brawler-vs.-Brawler, and Tactician-vs.-Tactician matches.

**Files:** `main.cpp`, `fighter.h`, `fighter.cpp`.

#### Task 4.4: Explain effects and validate balance

**Description:** Make the round recap report a denied Guard stamina gain and
record the specified paired and mirror-match playtest results in the README.

**Acceptance criteria:**

- [ ] A Feint-versus-Guard round visibly explains that Guard did not regain
  stamina.
- [ ] README documents fighter selection and both signature moves.
- [ ] README contains the completed 20-match balance table and any resulting
  single-variable tuning decision.

**Verify:** `bash run_tests.sh`; perform the 20-match balance-validation plan
and compare the resulting win rate and median-round count with its targets.

**Files:** `main.cpp`, `README.md`, `specs/turn-based-fighter-plan.md`.

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
Task 4: fighter selection + signature moves
```

## Open decisions to revisit during playtesting

- Tune numeric values based on observed match lengths, not intuition alone.
- Confirm whether mirror matchups are desired before implementation. This plan
  assumes they are allowed, because it avoids a turn-order advantage in fighter
  selection.
- Add no additional signature moves or fighters as part of this feature.
