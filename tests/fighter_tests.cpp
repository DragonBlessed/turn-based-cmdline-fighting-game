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

void unaffordable_actions_are_rejected_before_resolution() {
  Fighter tired = fresh_fighter("Tired");
  tired.stamina = 2;

  expect_equal(can_afford(tired, Action::HeavyAttack), 0,
               "a fighter with two stamina cannot use Heavy Attack");
  expect_equal(can_afford(tired, Action::LightAttack), 1,
               "a fighter with two stamina can use Light Attack");
}

void signature_actions_are_limited_to_their_fighter() {
  Fighter brawler = fresh_fighter("Brawler");
  brawler.type = FighterType::Brawler;
  Fighter tactician = fresh_fighter("Tactician");
  tactician.type = FighterType::Tactician;

  expect_equal(can_use_action(brawler, Action::Haymaker), 1,
               "Brawler should be able to use Haymaker");
  expect_equal(can_use_action(brawler, Action::Feint), 0,
               "Brawler should not be able to use Feint");
  expect_equal(can_use_action(tactician, Action::Feint), 1,
               "Tactician should be able to use Feint");
  expect_equal(can_use_action(tactician, Action::Haymaker), 0,
               "Tactician should not be able to use Haymaker");
  expect_equal(stamina_cost(Action::Haymaker), 4,
               "Haymaker should cost four stamina");
  expect_equal(stamina_cost(Action::Feint), 2,
               "Feint should cost two stamina");
}

void haymaker_deals_six_damage_and_guard_blocks_it() {
  Fighter brawler = fresh_fighter("Brawler");
  brawler.type = FighterType::Brawler;
  const RoundResult attack_result =
      resolve_round(brawler, Action::Haymaker, fresh_fighter("Target"),
                    Action::Recover);

  expect_equal(attack_result.second.health, 6,
               "Haymaker should deal six damage when not blocked");

  Fighter guarder = fresh_fighter("Guarder");
  guarder.stamina = 4;

  const RoundResult result =
      resolve_round(brawler, Action::Haymaker, guarder, Action::Guard);

  expect_equal(result.second.health, 12, "Guard should block Haymaker damage");
  expect_equal(result.first.stamina, 2, "Haymaker should cost four stamina");
  expect_equal(result.second.stamina, 5,
               "Guard should still regain stamina against Haymaker");
}

void feint_denies_guard_stamina_without_bypassing_guard() {
  Fighter tactician = fresh_fighter("Tactician");
  tactician.type = FighterType::Tactician;
  Fighter guarder = fresh_fighter("Guarder");
  guarder.stamina = 4;

  const RoundResult result =
      resolve_round(tactician, Action::Feint, guarder, Action::Guard);

  expect_equal(result.second.health, 12, "Guard should block Feint damage");
  expect_equal(result.first.stamina, 4, "Feint should cost two stamina");
  expect_equal(result.second.stamina, 4,
               "Feint should deny Guard's stamina gain");
}

void signature_actions_resolve_symmetrically_in_mirror_matches() {
  Fighter first_brawler = fresh_fighter("First Brawler");
  first_brawler.type = FighterType::Brawler;
  Fighter second_brawler = fresh_fighter("Second Brawler");
  second_brawler.type = FighterType::Brawler;
  const RoundResult brawler_result =
      resolve_round(first_brawler, Action::Haymaker, second_brawler,
                    Action::Haymaker);

  expect_equal(brawler_result.first.health, brawler_result.second.health,
               "matching Haymakers should affect both Brawlers equally");
  expect_equal(brawler_result.first.stamina, brawler_result.second.stamina,
               "matching Haymakers should cost both Brawlers equally");

  Fighter first_tactician = fresh_fighter("First Tactician");
  first_tactician.type = FighterType::Tactician;
  Fighter second_tactician = fresh_fighter("Second Tactician");
  second_tactician.type = FighterType::Tactician;
  const RoundResult tactician_result =
      resolve_round(first_tactician, Action::Feint, second_tactician,
                    Action::Feint);

  expect_equal(tactician_result.first.health, tactician_result.second.health,
               "matching Feints should affect both Tacticians equally");
  expect_equal(tactician_result.first.stamina, tactician_result.second.stamina,
               "matching Feints should cost both Tacticians equally");
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
  unaffordable_actions_are_rejected_before_resolution();
  signature_actions_are_limited_to_their_fighter();
  haymaker_deals_six_damage_and_guard_blocks_it();
  feint_denies_guard_stamina_without_bypassing_guard();
  signature_actions_resolve_symmetrically_in_mirror_matches();
  double_knockout_is_a_draw();
  std::cout << "All fighter tests passed.\n";
}
