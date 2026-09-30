#include "fighter.h"

#include <iostream>
#include <sstream>
#include <string>

namespace {

void clear_screen() {
  std::cout << "\033[2J\033[H";
}

const char* fighter_type_name(FighterType type) {
  switch (type) {
    case FighterType::Brawler:
      return "Brawler";
    case FighterType::Tactician:
      return "Tactician";
  }
  return "Unknown Fighter";
}

Action signature_action(const Fighter& fighter) {
  return fighter.type == FighterType::Brawler ? Action::Haymaker : Action::Feint;
}

const char* action_effect(Action action) {
  switch (action) {
    case Action::LightAttack:
      return "2 damage";
    case Action::HeavyAttack:
      return "5 damage";
    case Action::Guard:
      return "block attacks, regain 1 stamina";
    case Action::Recover:
      return "regain 2 stamina";
    case Action::Haymaker:
      return "6 damage";
    case Action::Feint:
      return "1 damage; deny Guard's stamina gain";
  }
  return "unknown effect";
}

void show_fighter(const Fighter& fighter) {
  std::cout << fighter.name << " (" << fighter_type_name(fighter.type)
            << ") — Health: " << fighter.health
            << ", Stamina: " << fighter.stamina << '/' << kMaximumStamina
            << '\n';
}

void show_status(const Fighter& first, const Fighter& second) {
  std::cout << "\n=== Current status ===\n";
  show_fighter(first);
  show_fighter(second);
}

bool wait_for_enter(const std::string& message) {
  std::cout << message << '\n';
  std::string line;
  return static_cast<bool>(std::getline(std::cin, line));
}

bool choose_fighter(Fighter& fighter) {
  while (true) {
    std::cout << '\n' << fighter.name << ", choose a fighter:\n"
              << "  1. Brawler — Haymaker (4 stamina, 6 damage)\n"
              << "  2. Tactician — Feint (2 stamina, 1 damage; denies Guard's "
                 "stamina gain)\n"
              << "> ";

    std::string line;
    if (!std::getline(std::cin, line)) {
      return false;
    }

    std::istringstream input(line);
    int choice = 0;
    char extra = '\0';
    if (!(input >> choice) || (input >> extra)) {
      std::cout << "Enter 1 for Brawler or 2 for Tactician.\n";
      continue;
    }

    switch (choice) {
      case 1:
        fighter.type = FighterType::Brawler;
        return true;
      case 2:
        fighter.type = FighterType::Tactician;
        return true;
      default:
        std::cout << "Enter 1 for Brawler or 2 for Tactician.\n";
        continue;
    }
  }
}

bool choose_action(const Fighter& fighter, Action& selected) {
  const Action signature = signature_action(fighter);
  while (true) {
    std::cout << '\n' << fighter.name << ", choose an action:\n"
              << "  1. " << action_name(Action::LightAttack) << " ("
              << stamina_cost(Action::LightAttack) << " stamina, "
              << action_effect(Action::LightAttack) << ")\n"
              << "  2. " << action_name(Action::HeavyAttack) << " ("
              << stamina_cost(Action::HeavyAttack) << " stamina, "
              << action_effect(Action::HeavyAttack) << ")\n"
              << "  3. " << action_name(Action::Guard) << " ("
              << action_effect(Action::Guard) << ")\n"
              << "  4. " << action_name(Action::Recover) << " ("
              << action_effect(Action::Recover) << ")\n"
              << "  5. " << action_name(signature) << " ("
              << stamina_cost(signature) << " stamina, " << action_effect(signature)
              << ")\n"
              << "> ";

    std::string line;
    if (!std::getline(std::cin, line)) {
      return false;
    }

    std::istringstream input(line);
    int choice = 0;
    char extra = '\0';
    if (!(input >> choice) || (input >> extra)) {
      std::cout << "Enter a number from 1 to 4.\n";
      continue;
    }

    switch (choice) {
      case 1:
        selected = Action::LightAttack;
        break;
      case 2:
        selected = Action::HeavyAttack;
        break;
      case 3:
        selected = Action::Guard;
        break;
      case 4:
        selected = Action::Recover;
        break;
      case 5:
        selected = signature;
        break;
      default:
        std::cout << "Enter a number from 1 to 5.\n";
        continue;
    }

    if (!can_use_action(fighter, selected) || !can_afford(fighter, selected)) {
      std::cout << "You do not have enough stamina for " << action_name(selected)
                << ".\n";
      continue;
    }
    return true;
  }
}

void announce_outcome(Outcome outcome) {
  switch (outcome) {
    case Outcome::FirstWins:
      std::cout << "\nPlayer 1 wins!\n";
      break;
    case Outcome::SecondWins:
      std::cout << "\nPlayer 2 wins!\n";
      break;
    case Outcome::Draw:
      std::cout << "\nDouble knockout — the match is a draw!\n";
      break;
    case Outcome::InProgress:
      break;
  }
}

void announce_special_effects(Action first_action, Action second_action) {
  if (first_action == Action::Feint && second_action == Action::Guard) {
    std::cout << "Player 2's Guard did not regain stamina because of Feint.\n";
  }
  if (second_action == Action::Feint && first_action == Action::Guard) {
    std::cout << "Player 1's Guard did not regain stamina because of Feint.\n";
  }
}

}  // namespace

int main() {
  Fighter first{"Player 1"};
  Fighter second{"Player 2"};
  Outcome outcome = Outcome::InProgress;

  clear_screen();
  std::cout << "=== Turn-Based Fighting Game ===\n"
            << "Choose privately, then pass the terminal. Both actions resolve "
               "at once.\n";

  std::cout << "\nFighter selection is public. Both players may choose the same "
               "fighter.\n";
  if (!choose_fighter(first) || !choose_fighter(second)) {
    std::cout << "\nMatch ended before fighter selection was complete.\n";
    return 0;
  }
  clear_screen();

  while (outcome == Outcome::InProgress) {
    show_status(first, second);
    if (!wait_for_enter("\nPlayer 1: pass the terminal and press Enter when ready.")) {
      break;
    }
    clear_screen();

    Action first_action = Action::Guard;
    if (!choose_action(first, first_action)) {
      break;
    }
    clear_screen();

    if (!wait_for_enter("Player 2: pass the terminal and press Enter when ready.")) {
      break;
    }
    clear_screen();

    Action second_action = Action::Guard;
    if (!choose_action(second, second_action)) {
      break;
    }
    clear_screen();

    const RoundResult result =
        resolve_round(first, first_action, second, second_action);
    first = result.first;
    second = result.second;
    outcome = result.outcome;

    std::cout << "Player 1 chose " << action_name(first_action) << ".\n"
              << "Player 2 chose " << action_name(second_action) << ".\n";
    announce_special_effects(first_action, second_action);
    show_status(first, second);
    announce_outcome(outcome);
  }

  if (outcome == Outcome::InProgress) {
    std::cout << "\nMatch ended before a winner was decided.\n";
  }
  return 0;
}
