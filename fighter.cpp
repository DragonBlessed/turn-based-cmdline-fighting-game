#include "fighter.h"

#include <algorithm>

namespace {

int attack_damage(Action action) {
  switch (action) {
    case Action::LightAttack:
      return 2;
    case Action::HeavyAttack:
      return 5;
    case Action::Haymaker:
      return 6;
    case Action::Feint:
      return 1;
    case Action::Guard:
    case Action::Recover:
      return 0;
  }
  return 0;
}

bool is_guarding(Action action) {
  return action == Action::Guard;
}

int stamina_gain(Action action, Action opposing_action) {
  switch (action) {
    case Action::Guard:
      return opposing_action == Action::Feint ? 0 : 1;
    case Action::Recover:
      return 2;
    case Action::LightAttack:
    case Action::HeavyAttack:
    case Action::Haymaker:
    case Action::Feint:
      return 0;
  }
  return 0;
}

Outcome determine_outcome(const Fighter& first, const Fighter& second) {
  if (first.health <= 0 && second.health <= 0) {
    return Outcome::Draw;
  }
  if (first.health <= 0) {
    return Outcome::SecondWins;
  }
  if (second.health <= 0) {
    return Outcome::FirstWins;
  }
  return Outcome::InProgress;
}

}  // namespace

int stamina_cost(Action action) {
  switch (action) {
    case Action::LightAttack:
      return 1;
    case Action::HeavyAttack:
      return 3;
    case Action::Haymaker:
      return 4;
    case Action::Feint:
      return 2;
    case Action::Guard:
    case Action::Recover:
      return 0;
  }
  return 0;
}

bool can_use_action(const Fighter& fighter, Action action) {
  switch (action) {
    case Action::Haymaker:
      return fighter.type == FighterType::Brawler;
    case Action::Feint:
      return fighter.type == FighterType::Tactician;
    case Action::LightAttack:
    case Action::HeavyAttack:
    case Action::Guard:
    case Action::Recover:
      return true;
  }
  return false;
}

bool can_afford(const Fighter& fighter, Action action) {
  return fighter.stamina >= stamina_cost(action);
}

const char* action_name(Action action) {
  switch (action) {
    case Action::LightAttack:
      return "Light Attack";
    case Action::HeavyAttack:
      return "Heavy Attack";
    case Action::Guard:
      return "Guard";
    case Action::Recover:
      return "Recover";
    case Action::Haymaker:
      return "Haymaker";
    case Action::Feint:
      return "Feint";
  }
  return "Unknown Action";
}

RoundResult resolve_round(Fighter first, Action first_action, Fighter second,
                          Action second_action) {
  first.stamina -= stamina_cost(first_action);
  second.stamina -= stamina_cost(second_action);

  const int first_damage = is_guarding(first_action)
                               ? 0
                               : attack_damage(second_action);
  const int second_damage = is_guarding(second_action)
                                ? 0
                                : attack_damage(first_action);
  first.health = std::max(0, first.health - first_damage);
  second.health = std::max(0, second.health - second_damage);

  first.stamina = std::min(kMaximumStamina,
                           first.stamina + stamina_gain(first_action, second_action));
  second.stamina = std::min(kMaximumStamina,
                            second.stamina + stamina_gain(second_action, first_action));

  return {first, second, determine_outcome(first, second)};
}
