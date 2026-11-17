#pragma once
#include "expedition.hpp"

namespace command {
enum class EnemyMutation {
  Health,
  Quantity,
  Speed,
  Damage,
  Haste,
  RunnerSprint,
  BruteArmor,
  BruteRegen,
  SpitterCadence,
  RunnerDodge,
  FloaterShield,
  BloodRush,
  BossCompanion,
  DoubleMutation,
  EliteBrood,
  SaboteurWave,
  NextBossHealth,
  NextBossSpeed,
  Count
};
struct MutationNotice {
  uint64_t id = 0;
  EnemyMutation kind;
  Rarity rarity;
  std::string title, description;
};
struct EnemyMutationSystem {
  static constexpr float NOTICE_TIME = 9;
  Random random{0xa9327d61};
  float nextMutation = 180, noticeAge = 0;
  uint64_t count = 0;
  std::array<int, 5> rarityCounts{};
  std::deque<MutationNotice> notices;
  std::deque<MutationNotice> history;
  const MutationNotice* current() const { return notices.empty() ? nullptr : &notices.front(); }
  static bool mouthOpen(float age) {
    if (age < .25f || age > 7.5f)
      return false;
    int syllable = int(age * 9);
    return syllable % 7 != 0 && syllable % 3 != 0;
  }
  static float interval(const Game& g) { return std::max(60.f, 180 - g.enemyGenome.mutationHaste); }
  static int fixedRarity(EnemyMutation kind) {
    switch (kind) {
    case EnemyMutation::NextBossHealth:
      return int(Rarity::Common);
    case EnemyMutation::NextBossSpeed:
    case EnemyMutation::RunnerSprint:
    case EnemyMutation::BruteArmor:
      return int(Rarity::Uncommon);
    case EnemyMutation::BruteRegen:
    case EnemyMutation::SpitterCadence:
    case EnemyMutation::SaboteurWave:
      return int(Rarity::Rare);
    case EnemyMutation::RunnerDodge:
    case EnemyMutation::FloaterShield:
    case EnemyMutation::BloodRush:
      return int(Rarity::Epic);
    case EnemyMutation::BossCompanion:
    case EnemyMutation::DoubleMutation:
    case EnemyMutation::EliteBrood:
      return int(Rarity::Legendary);
    default:
      return -1;
    }
  }
  static bool eligible(const Game& g, EnemyMutation kind) {
    const auto& m = g.enemyGenome;
    switch (kind) {
    case EnemyMutation::NextBossHealth:
      return !m.nextBossHealth;
    case EnemyMutation::NextBossSpeed:
      return !m.nextBossSpeed;
    case EnemyMutation::Health:
      return true;
    case EnemyMutation::Quantity:
      return m.quantityBonus < 3;
    case EnemyMutation::Speed:
      return m.speedBonus < 1.5f;
    case EnemyMutation::Damage:
      return m.damageBonus < 3;
    case EnemyMutation::Haste:
      return interval(g) > 60;
    case EnemyMutation::RunnerSprint:
      return g.time >= 60 && !m.runnerSprint;
    case EnemyMutation::BruteArmor:
      return g.time >= 120 && !m.bruteArmor;
    case EnemyMutation::BruteRegen:
      return g.time >= 120 && !m.bruteRegen;
    case EnemyMutation::SpitterCadence:
      return g.time >= 180 && !m.spitterCadence;
    case EnemyMutation::SaboteurWave:
      return g.time >= 135 && g.time >= m.saboteurWaveUntil;
    case EnemyMutation::RunnerDodge:
      return g.time >= 60 && !m.runnerDodge;
    case EnemyMutation::FloaterShield:
      return g.time >= 240 && !m.floaterShield;
    case EnemyMutation::BloodRush:
      return !m.bloodRush;
    case EnemyMutation::BossCompanion:
      return m.extraBosses < 7;
    case EnemyMutation::DoubleMutation:
      return !m.doubleMutation;
    case EnemyMutation::EliteBrood:
      return !m.eliteBrood;
    default:
      return false;
    }
  }
  Rarity rollRarity() {
    int roll = int(random.next() % 10000);
    return roll < 200    ? Rarity::Legendary
           : roll < 1000 ? Rarity::Epic
           : roll < 2800 ? Rarity::Rare
           : roll < 6000 ? Rarity::Uncommon
                         : Rarity::Common;
  }
  bool apply(Game& g, EnemyMutation kind, Rarity rarity) {
    if (!eligible(g, kind) || (fixedRarity(kind) >= 0 && fixedRarity(kind) != int(rarity)))
      return false;
    int r = int(rarity);
    if (r < 0 || r >= 5)
      return false;
    constexpr float hp[] = {.10f, .15f, .22f, .32f, .45f};
    constexpr float quantity[] = {.10f, .15f, .25f, .35f, .50f};
    constexpr float speed[] = {.03f, .05f, .08f, .12f, .18f};
    constexpr float damage[] = {.08f, .12f, .18f, .25f, .40f};
    constexpr float haste[] = {5, 8, 12, 18, 25};
    auto& m = g.enemyGenome;
    std::string title, description;
    auto percent = [](float p) { return std::to_string(int(std::round(p * 100))); };
    switch (kind) {
    case EnemyMutation::NextBossHealth:
      m.nextBossHealth = true;
      title = "Przerośnięty przywódca";
      description = "Tylko następny boss: 200% bazowego HP (x2).";
      break;
    case EnemyMutation::NextBossSpeed:
      m.nextBossSpeed = true;
      title = "Pęd przywódcy";
      description = "Tylko następny boss będzie poruszał się 2 razy szybciej.";
      break;
    case EnemyMutation::Health:
      m.hpBonus += hp[r];
      title = "Wzmocniony genom";
      description = "Wszystkie jednostki, także bossowie: +" + percent(hp[r]) + "% bazowego HP.";
      break;
    case EnemyMutation::Quantity: {
      float gain = std::min(quantity[r], 3 - m.quantityBonus);
      m.quantityBonus += gain;
      title = "Namnażanie roju";
      description = "Zwykłe serie i hordy: +" + percent(gain) + "% jednostek.";
      break;
    }
    case EnemyMutation::Speed: {
      float gain = std::min(speed[r], 1.5f - m.speedBonus);
      m.speedBonus += gain;
      title = "Przyspieszony metabolizm";
      description = "Wszystkie jednostki: +" + percent(gain) + "% bazowej prędkości.";
      break;
    }
    case EnemyMutation::Damage: {
      float gain = std::min(damage[r], 3 - m.damageBonus);
      m.damageBonus += gain;
      title = "Agresja kolonii";
      description = "Ataki przeciwników na rdzeń: +" + percent(gain) + "% bazowych obrażeń.";
      break;
    }
    case EnemyMutation::Haste: {
      float gain = std::min(haste[r], 120 - m.mutationHaste);
      m.mutationHaste += gain;
      title = "Przyspieszona ewolucja";
      description = "Odstęp między mutacjami krótszy o " + std::to_string(int(gain)) +
                    " s. Teraz " + std::to_string(int(interval(g))) + " s.";
      break;
    }
    case EnemyMutation::RunnerSprint:
      m.runnerSprint = true;
      title = "Szybki zwiad";
      description = "Łowcy poruszają się o 20% szybciej.";
      break;
    case EnemyMutation::BruteArmor:
      m.bruteArmor = true;
      title = "Chitynowa tarcza";
      description = "Tarany redukują otrzymywane obrażenia o 20%.";
      break;
    case EnemyMutation::BruteRegen:
      m.bruteRegen = true;
      title = "Samonaprawa tkanek";
      description = "Tarany regenerują 1% maksymalnego HP na sekundę.";
      break;
    case EnemyMutation::SpitterCadence:
      m.spitterCadence = true;
      title = "Gruczoły kwasowe";
      description = "Zwykłe i oblężnicze Plujce mają o 30% krótszy czas między atakami.";
      break;
    case EnemyMutation::SaboteurWave:
      m.saboteurWaveUntil = g.time + 60;
      title = "Inwazja sabotażystów";
      description = "Przez 60 s każda nowa elita będzie Sabotażystą niszczącym wieżyczki.";
      break;
    case EnemyMutation::RunnerDodge:
      m.runnerDodge = true;
      title = "Odruch zwiadowców";
      description =
          "Łowcy mają 50% szans na unik bezpośredniego trafienia. Obszar i DOT nadal działają.";
      break;
    case EnemyMutation::FloaterShield:
      m.floaterShield = true;
      title = "Pole widmowe";
      description = "Widma dalej niż 80 m od rdzenia redukują obrażenia o 35%.";
      break;
    case EnemyMutation::BloodRush:
      m.bloodRush = true;
      title = "Szaleństwo rannych";
      description = "Zwykłe jednostki i elity poniżej 30% HP biegną o 50% szybciej.";
      break;
    case EnemyMutation::BossCompanion:
      ++m.extraBosses;
      title = "Sygnał przywódców";
      description = "Kolejny boss w każdej przyszłej serii. Teraz " +
                    std::to_string(1 + m.extraBosses) + " bossów.";
      break;
    case EnemyMutation::DoubleMutation:
      m.doubleMutation = true;
      title = "Podwójna adaptacja";
      description = "Od następnego cyklu przeciwnicy losują dwie mutacje naraz.";
      break;
    case EnemyMutation::EliteBrood:
      m.eliteBrood = true;
      title = "Elitarne potomstwo";
      description = "Nowe zwykłe jednostki i hordy mają 5% szans na odmianę elitarną.";
      break;
    default:
      return false;
    }
    if (kind == EnemyMutation::Health || kind == EnemyMutation::Speed ||
        kind == EnemyMutation::RunnerSprint)
      g.refreshEnemyGenome();
    MutationNotice notice{++count, kind, rarity, title, description};
    if (notices.empty())
      noticeAge = 0;
    if (notices.size() >= 16)
      notices.pop_front();
    notices.push_back(notice);
    if (history.size() >= 64)
      history.pop_front();
    history.push_back(notice);
    ++rarityCounts[r];
    ++g.mutationAlerts;
    return true;
  }
  bool roll(Game& g) {
    Rarity rarity = rollRarity();
    std::array<EnemyMutation, size_t(EnemyMutation::Count)> pool{};
    int count = 0;
    for (int n = 0; n < int(EnemyMutation::Count); ++n) {
      auto kind = EnemyMutation(n);
      if (eligible(g, kind) && (fixedRarity(kind) < 0 || fixedRarity(kind) == int(rarity)))
        pool[count++] = kind;
    }
    return count > 0 && apply(g, pool[random.next() % uint32_t(count)], rarity);
  }
  void step(Game& g, float dt) {
    if (g.paused || dt <= 0)
      return;
    if (!notices.empty()) {
      noticeAge += dt;
      if (noticeAge >= NOTICE_TIME) {
        notices.pop_front();
        noticeAge = 0;
      }
    }
    if (g.time >= nextMutation) {
      int draws = g.enemyGenome.doubleMutation ? 2 : 1;
      for (int n = 0; n < draws; ++n)
        roll(g);
      nextMutation = g.time + interval(g);
    }
  }
};
} // namespace command
