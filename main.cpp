#include "fighter.h"

#include <iostream>
#include <sstream>
#include <string>

namespace {

void clear_screen() {
  std::cout << "\033[2J\033[H";
}

void show_fighter(const Fighter& fighter) {
  std::cout << fighter.name << " — Health: " << fighter.health
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

bool choose_action(const Fighter& fighter, Action& selected) {
  while (true) {
    std::cout << '\n' << fighter.name << ", choose an action:\n"
              << "  1. Light Attack (1 stamina, 2 damage)\n"
              << "  2. Heavy Attack (3 stamina, 5 damage)\n"
              << "  3. Guard (block attacks, regain 1 stamina)\n"
              << "  4. Recover (regain 2 stamina)\n"
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
      default:
        std::cout << "Enter a number from 1 to 4.\n";
        continue;
    }

    if (!can_afford(fighter, selected)) {
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

}  // namespace

int main() {
  Fighter first{"Player 1"};
  Fighter second{"Player 2"};
  Outcome outcome = Outcome::InProgress;

  clear_screen();
  std::cout << "=== Turn-Based Fighting Game ===\n"
            << "Choose privately, then pass the terminal. Both actions resolve "
               "at once.\n";

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
    show_status(first, second);
    announce_outcome(outcome);
  }

  if (outcome == Outcome::InProgress) {
    std::cout << "\nMatch ended before a winner was decided.\n";
  }
  return 0;
}
