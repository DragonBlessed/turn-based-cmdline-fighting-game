#include "../fighter.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void expect_equal(int actual, int expected, const std::string& message) {
  if (actual != expected) {
    std::cerr << "FAIL: " << message << " (expected " << expected << ", got "
              << actual << ")\n";
    std::exit(1);
  }
}

Fighter fresh_fighter(const std::string& name) {
  return Fighter{name, 12, 6};
}

void light_attacks_damage_both_fighters_simultaneously() {
  const RoundResult result = resolve_round(
      fresh_fighter("Player 1"), Action::LightAttack,
      fresh_fighter("Player 2"), Action::LightAttack);

  expect_equal(result.first.health, 10,
               "a Light Attack should damage the first fighter");
  expect_equal(result.second.health, 10,
               "a Light Attack should damage the second fighter");
  expect_equal(result.first.stamina, 5,
               "a Light Attack should cost the first fighter one stamina");
  expect_equal(result.second.stamina, 5,
               "a Light Attack should cost the second fighter one stamina");
}

void guard_blocks_a_heavy_attack_and_restores_stamina() {
  Fighter defender = fresh_fighter("Defender");
  defender.stamina = 4;
  const RoundResult result = resolve_round(
      fresh_fighter("Attacker"), Action::HeavyAttack, defender, Action::Guard);

  expect_equal(result.second.health, 12, "Guard should block Heavy Attack damage");
  expect_equal(result.second.stamina, 5,
               "Guard should restore one stamina after the round");
  expect_equal(result.first.stamina, 3,
               "Heavy Attack should cost three stamina");
}

void recover_restores_stamina_but_does_not_block_damage() {
  Fighter recovering = fresh_fighter("Recovering");
  recovering.stamina = 3;
  const RoundResult result = resolve_round(
      fresh_fighter("Attacker"), Action::LightAttack, recovering, Action::Recover);

  expect_equal(result.second.health, 10,
               "Recover should not prevent Light Attack damage");
  expect_equal(result.second.stamina, 5,
               "Recover should restore two stamina");
}

void stamina_recovery_never_exceeds_the_maximum() {
  Fighter recovering = fresh_fighter("Recovering");
  recovering.stamina = 5;
  const RoundResult result = resolve_round(
      fresh_fighter("Other"), Action::Guard, recovering, Action::Recover);

  expect_equal(result.second.stamina, 6,
               "Recover should cap stamina at the maximum");
}

void double_knockout_is_a_draw() {
  Fighter first = fresh_fighter("Player 1");
  Fighter second = fresh_fighter("Player 2");
  first.health = 2;
  second.health = 2;
  const RoundResult result = resolve_round(first, Action::LightAttack, second,
                                           Action::LightAttack);

  expect_equal(result.first.health, 0, "first fighter should be knocked out");
  expect_equal(result.second.health, 0, "second fighter should be knocked out");
  expect_equal(static_cast<int>(result.outcome), static_cast<int>(Outcome::Draw),
               "simultaneous knockouts should result in a draw");
}

}  // namespace

int main() {
  light_attacks_damage_both_fighters_simultaneously();
  guard_blocks_a_heavy_attack_and_restores_stamina();
  recover_restores_stamina_but_does_not_block_damage();
  stamina_recovery_never_exceeds_the_maximum();
  double_knockout_is_a_draw();
  std::cout << "All fighter tests passed.\n";
}
