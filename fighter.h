#ifndef FIGHTER_H
#define FIGHTER_H

#include <string>

constexpr int kStartingHealth = 12;
constexpr int kMaximumStamina = 6;

enum class Action {
  LightAttack,
  HeavyAttack,
  Guard,
  Recover,
};

enum class Outcome {
  InProgress,
  FirstWins,
  SecondWins,
  Draw,
};

struct Fighter {
  std::string name;
  int health = kStartingHealth;
  int stamina = kMaximumStamina;
};

struct RoundResult {
  Fighter first;
  Fighter second;
  Outcome outcome = Outcome::InProgress;
};

int stamina_cost(Action action);
bool can_afford(const Fighter& fighter, Action action);
const char* action_name(Action action);
RoundResult resolve_round(Fighter first, Action first_action, Fighter second,
                          Action second_action);

#endif
