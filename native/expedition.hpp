#pragma once
#include "gameplay.hpp"
#include "turret_slots.hpp"
#include <deque>

namespace command {
using namespace arsenal;
enum class Upgrade {
  Damage,
  FireRate,
  Mining,
  Hull,
  Sentry,
  MinigunTower,
  TeslaTower,
  SniperTower,
  LanceTower,
  VenomTower,
  RocketTower,
  MortarTower,
  FlameTower,
  TowerDamage,
  TowerRate,
  TowerRange,
  FlatDamage,
  Luck,
  Radar,
  Frost,
  Focus,
  Purge,
  RateRelayTower,
  RangeRelayTower,
  FrostSupportTower,
  RoverFactory,
  RoverBatch,
  RoverRate,
  ServiceCredit,
  Count
};
enum class Rarity { Common, Uncommon, Rare, Epic, Legendary };
struct RarityInfo {
  const char* name;
  Color color;
  float power;
};
inline constexpr RarityInfo RARITIES[] = {{"COMMON", {.65f, .72f, .75f}, .5f},
                                          {"UNCOMMON", {.35f, .9f, .48f}, 1},
                                          {"RARE", {.3f, .62f, 1}, 1.5f},
                                          {"EPIC", {.78f, .4f, 1}, 2},
                                          {"LEGENDARY", {1, .7f, .18f}, 3}};
inline constexpr Weapon towerWeapon(Upgrade u) {
  if (u == Upgrade::Sentry)
    return Laser;
  if (u >= Upgrade::MinigunTower && u <= Upgrade::FlameTower)
    return Weapon(int(u) - int(Upgrade::MinigunTower) + 1);
  if (u >= Upgrade::RateRelayTower && u <= Upgrade::FrostSupportTower)
    return Weapon(int(RateRelay) + int(u) - int(Upgrade::RateRelayTower));
  return TowerTypeCount;
}
inline constexpr bool isTower(Upgrade u) {
  return towerWeapon(u) != TowerTypeCount;
}
inline constexpr Rarity minimumRarity(Upgrade u) {
  switch (u) {
  case Upgrade::LanceTower:
  case Upgrade::Purge:
    return Rarity::Epic;
  case Upgrade::TeslaTower:
  case Upgrade::SniperTower:
  case Upgrade::RocketTower:
  case Upgrade::MortarTower:
  case Upgrade::Frost:
  case Upgrade::FrostSupportTower:
  case Upgrade::ServiceCredit:
    return Rarity::Rare;
  case Upgrade::MinigunTower:
  case Upgrade::VenomTower:
  case Upgrade::FlameTower:
  case Upgrade::Radar:
  case Upgrade::RateRelayTower:
  case Upgrade::RangeRelayTower:
  case Upgrade::RoverFactory:
    return Rarity::Uncommon;
  default:
    return Rarity::Common;
  }
}
inline constexpr bool uniqueUpgrade(Upgrade u) {
  return u == Upgrade::Radar || u == Upgrade::Frost || u == Upgrade::Focus || u == Upgrade::Purge ||
         u == Upgrade::RoverFactory;
}
struct UpgradeInfo {
  const char* name;
  const char* category;
  const char* description;
  Color color;
};
inline constexpr UpgradeInfo UPGRADES[] = {
    {"Mocniejszy impuls",
     "DZIAŁO MANUALNE",
     "+25% bazowych obrażeń wybranego działa.",
     {.35f, .85f, 1}},
    {"Szybszy mechanizm",
     "DZIAŁO MANUALNE",
     "+20% bazowej szybkostrzelności działa.",
     {1, .7f, .24f}},
    {"Wydajniejsze wiertło",
     "WYDOBYCIE",
     "+25% bazowego wydobycia. Częstsze wypłaty.",
     {.35f, 1, .65f}},
    {"Wzmocnienie rdzenia", "RDZEŃ", "+250 maksymalnego HP i naprawa 250 HP.", {.65f, .85f, 1}},
    {"Automatyczny laser", "NOWA OBRONA", "Daje laser. Wybierz wolny heks z góry.", {1, .35f, .4f}},
    {"Minigun", "NOWA OBRONA", "Obracane lufy. Szybki ostrzał jednego celu.", {1, .69f, .22f}},
    {"Cewka Tesli",
     "NOWA OBRONA",
     "Piorun trafia wszystkich w zasięgu. Długa przerwa.",
     {.4f, .65f, 1}},
    {"Snajper", "NOWA OBRONA", "Potężny pojedynczy strzał. Daleki zasięg.", {.65f, .95f, 1}},
    {"Lanca termiczna",
     "NOWA OBRONA",
     "Ładuje 6 s, potem tnie ciągłą wiązką przez 3 s.",
     {1, .3f, .08f}},
    {"Rozpylacz toksyn", "NOWA OBRONA", "Zatrute pociski zostawiają chmury gazu.", {.45f, 1, .12f}},
    {"Wyrzutnia rakiet",
     "NOWA OBRONA",
     "Rakiety naprowadzane z eksplozją obszarową.",
     {1, .48f, .15f}},
    {"Moździerz", "NOWA OBRONA", "Pociski lecą łukiem i rażą duży obszar.", {1, .78f, .4f}},
    {"Miotacz ognia",
     "NOWA OBRONA",
     "Stożek płomieni podpala grupy przeciwników.",
     {1, .24f, .025f}},
    {"Wzmocnione emitery", "WSZYSTKIE WIEŻYCZKI", "", {1, .48f, .3f}},
    {"Szybsze serwomechanizmy", "WSZYSTKIE WIEŻYCZKI", "", {1, .75f, .3f}},
    {"Optyka dalekiego zasięgu", "WSZYSTKIE WIEŻYCZKI", "", {.4f, .8f, 1}},
    {"Kalibracja broni", "DZIAŁO MANUALNE", "+1 obrażenie na trafienie.", {.65f, .72f, .75f}},
    {"Szczęśliwy operator",
     "SZCZĘŚCIE",
     "+1% szczęścia. Większa szansa lepszych kart.",
     {.6f, 1, .5f}},
    {"Radar taktyczny",
     "MODUŁ RDZENIA",
     "Odblokowuje radar z kierunkami ataku bestii.",
     {.25f, 1, .8f}},
    {"Impulsy kriogeniczne",
     "MODUŁ RDZENIA",
     "Spowalnia bestie do 60% blisko rdzenia. Promień 85 m.",
     {.35f, .8f, 1}},
    {"Wspólny cel",
     "MODUŁ RDZENIA",
     "Trafienie manualne oznacza cel na 3 s. Wieżyczki skupiają na nim ogień, jeśli sięga ich "
     "zasięg.",
     {1, .55f, .3f}},
    {"Awaryjne oczyszczenie",
     "MODUŁ RDZENIA",
     "Raz na ekspedycję przy 50% HP zabija zwykłe bestie. Bossowie tracą 50% maksymalnego HP.",
     {1, .75f, .2f}},
    {"Przekaźnik tempa",
     "NOWE WSPARCIE",
     "+20% szybkostrzelności bojowych sąsiadów (maks. 6). Ten sam bonus nie kumuluje się.",
     {1, .7f, .2f}},
    {"Przekaźnik zasięgu",
     "NOWE WSPARCIE",
     "+20% zasięgu bojowych sąsiadów (maks. 6). Ten sam bonus nie kumuluje się.",
     {.4f, 1, .6f}},
    {"Generator śnieżycy",
     "NOWE WSPARCIE",
     "Spowalnia o 50% w promieniu 40 m. Łączy się z chłodzeniem rdzenia; nie z drugą śnieżycą.",
     {.45f, .8f, 1}},
    {"Fabryka jeździków",
     "JEDNOSTKI RDZENIA",
     "Uruchamia produkcję: 1 jeździk co 12 s. Kontakt zadaje 100 obrażeń jednemu celowi.",
     {.3f, 1, .8f}},
    {"Większa seria", "PRODUKCJA JEŹDZIKÓW", "", {.5f, 1, .7f}},
    {"Szybsza linia", "PRODUKCJA JEŹDZIKÓW", "", {1, .75f, .3f}},
    {"Punkt usług",
     "JEDNORAZOWE WSPARCIE",
     "+1 punkt do wydania przy prawym komputerze na Brimstone, falę ogłuszającą lub przeciążenie.",
     {.3f, .62f, 1}}};
static_assert(std::size(UPGRADES) == size_t(Upgrade::Count));
inline std::string upgradeDescription(Upgrade u, Rarity rarity) {
  float power = RARITIES[int(rarity)].power;
  auto percent = [&](float value) {
    std::string result = std::to_string(value * power);
    while (result.back() == '0')
      result.pop_back();
    if (result.back() == '.')
      result.pop_back();
    return result;
  };
  switch (u) {
  case Upgrade::Damage:
    return "+" + percent(25) + "% bazowych obrażeń działa manualnego.";
  case Upgrade::FireRate:
    return "+" + percent(20) + "% bazowej szybkostrzelności działa manualnego.";
  case Upgrade::Mining:
    return "+" + percent(25) + "% bazowego wydobycia uranu. Częstsze wypłaty.";
  case Upgrade::Hull:
    return "+" + percent(250) + " maksymalnego HP i tyle samo naprawy.";
  case Upgrade::TowerDamage:
    return "+" + percent(20) +
           "% bazowych obrażeń wszystkich wieżyczek, także przyszłych. Wzmacnia ogień i truciznę.";
  case Upgrade::TowerRate:
    return "+" + percent(15) +
           "% bazowej szybkostrzelności wieżyczek. Lanca szybciej ładuje wiązkę.";
  case Upgrade::TowerRange:
    return "+" + percent(15) + "% bazowego zasięgu wszystkich wieżyczek, także przyszłych.";
  case Upgrade::Focus:
    return "Trafienie oznacza na 3 s. Wieżyczki skupiają ogień. Oznaczony cel otrzymuje +" +
           std::to_string(int(rarity) * 10) + "% obrażeń ze wszystkich źródeł.";
  case Upgrade::RoverBatch:
    return "+" + std::to_string(int(rarity) + 1) + " jeździków w każdej serii produkcyjnej.";
  case Upgrade::RoverRate:
    return "+" + std::to_string((int(rarity) + 1) * 10) + "% bazowego tempa produkcji jeździków.";
  default:
    if (isTower(u))
      return std::string(UPGRADES[size_t(u)].description) + " Wybierz wolny heks.";
    return UPGRADES[size_t(u)].description;
  }
}
enum class RewardSource { Uranium, EliteCrystal, BossCrystal };
struct RewardOffer {
  uint64_t id;
  std::array<Upgrade, 3> choices;
  std::array<Rarity, 3> rarities{Rarity::Uncommon, Rarity::Uncommon, Rarity::Uncommon};
  RewardSource source = RewardSource::Uranium;
};
struct Expedition {
  static constexpr int COMBAT_TYPES = 4, SUPPORT_TYPES = 2;
  static constexpr float CAPACITY = 100, BASE_RATE = 2, FLIGHT_TIME = 10;
  static constexpr Vec3 DOCK{32, CORE_HEIGHT - 5.5f, -10.392305f};
  bool enabled = false, shipping = false;
  float hp = 1500, maxHp = 1500, uranium = 0, miningMultiplier = 1, flightAge = 0;
  int shipments = 0, earned = 0, awaitingPickup = 0, selected = 0, luck = 0;
  bool purgeUsed = false;
  bool miningStarted = false;
  int servicePoints = 0;
  std::array<bool, TowerTypeCount> towerUnlocked{};
  uint64_t rewardSerial = 0;
  int crystalRewards = 0, roverBatch = 1;
  float roverRate = 1;
  float roverInterval() const { return 12 / roverRate; }
  std::array<int, size_t(Upgrade::Count)> levels{};
  std::deque<RewardOffer> rewards;
  Random rewardRandom;
  explicit Expedition(bool active = false, bool running = false)
      : enabled(active), miningStarted(running) {
    rewardRandom.state = 79193;
  }
  bool dead() const { return enabled && hp <= 0; }
  float rate() const { return miningStarted ? BASE_RATE * miningMultiplier : 0; }
  const char* flightStatus() const {
    if (!shipping)
      return "WYDOBYCIE / ZAPEŁNIANIE ŁADUNKU";
    if (flightAge < 4)
      return "TRANSPORTOWIEC NADLATUJE";
    if (flightAge < 6)
      return "PRZEJMOWANIE URANU";
    return "TRANSPORT ODCHODZI / ŁADUNEK ODEBRANY";
  }
  Vec3 shipPosition() const {
    auto ease = [](float t) {
      t = std::clamp(t, 0.f, 1.f);
      return t * t * (3 - 2 * t);
    };
    Vec3 dock = DOCK;
    if (flightAge < 4) {
      float t = ease(flightAge / 4);
      return Vec3{170, 155, -190} * (1 - t) + dock * t;
    }
    if (flightAge < 6)
      return dock;
    float t = ease((flightAge - 6) / 4);
    return dock * (1 - t) + Vec3{210, 185, 100} * t;
  }
  int acquiredTowers() const {
    int count = 0;
    for (int i = 0; i < int(Upgrade::Count); ++i)
      if (isTower(Upgrade(i)))
        count += levels[i];
    return count;
  }
  int chosenTypes(bool support) const {
    int count = 0;
    for (int i = 0; i < int(Upgrade::Count); ++i)
      if (isTower(Upgrade(i)) && isSupport(towerWeapon(Upgrade(i))) == support &&
          (levels[i] > 0 || towerUnlocked[towerWeapon(Upgrade(i))]))
        ++count;
    return count;
  }
  bool eligible(Upgrade u) const {
    if (u < Upgrade::Damage || u >= Upgrade::Count)
      return false;
    if (isTower(u)) {
      bool support = isSupport(towerWeapon(u));
      return acquiredTowers() < SLOT_COUNT &&
             (levels[size_t(u)] > 0 || towerUnlocked[towerWeapon(u)] ||
              chosenTypes(support) < (support ? SUPPORT_TYPES : COMBAT_TYPES));
    }
    if (u == Upgrade::RoverBatch || u == Upgrade::RoverRate) {
      if (levels[size_t(Upgrade::RoverFactory)] == 0)
        return false;
      return u == Upgrade::RoverBatch ? roverBatch < 128 : roverRate < 24;
    }
    if (uniqueUpgrade(u))
      return levels[size_t(u)] == 0;
    return true;
  }
  void releaseTower(Weapon type) {
    for (int i = 0; i < int(Upgrade::Count); ++i)
      if (isTower(Upgrade(i)) && towerWeapon(Upgrade(i)) == type && levels[i] > 0) {
        towerUnlocked[type] = true;
        --levels[i];
        return;
      }
  }
  Rarity rollRarity(RewardSource source = RewardSource::Uranium) {
    // A common quantile makes each luck point monotonically improve the draw.
    float roll = rewardRandom.unit() / (1 + std::max(0, luck) * .01f);
    if (source == RewardSource::BossCrystal) {
      if (roll < .1f)
        return Rarity::Legendary;
      if (roll < .4f)
        return Rarity::Epic;
      return Rarity::Rare;
    }
    if (roll < .02f)
      return Rarity::Legendary;
    if (roll < .10f)
      return Rarity::Epic;
    if (roll < .28f)
      return Rarity::Rare;
    if (roll < .60f)
      return Rarity::Uncommon;
    return Rarity::Common;
  }
  void addReward(RewardSource source = RewardSource::Uranium) {
    RewardOffer offer{++rewardSerial, {Upgrade::Count, Upgrade::Count, Upgrade::Count}};
    offer.source = source;
    if (source != RewardSource::Uranium)
      ++crystalRewards;
    for (int card = 0; card < 3; ++card) {
      Rarity rarity = rollRarity(source);
      std::array<Upgrade, size_t(Upgrade::Count)> pool{};
      int count = 0;
      for (int i = 0; i < int(Upgrade::Count); ++i) {
        auto u = Upgrade(i);
        bool minor = u == Upgrade::FlatDamage || u == Upgrade::Luck;
        if (eligible(u) && minimumRarity(u) <= rarity && (!minor || rarity == Rarity::Common) &&
            (u != Upgrade::ServiceCredit || rarity == Rarity::Rare) &&
            std::find(offer.choices.begin(), offer.choices.end(), u) == offer.choices.end())
          pool[count++] = u;
      }
      offer.choices[card] = pool[rewardRandom.next() % uint32_t(count)];
      offer.rarities[card] = rarity;
    }
    // The first payment introduces placement, without waiting on a lucky tower draw.
    if (source == RewardSource::Uranium && earned == 1 && selected == 0 && acquiredTowers() == 0 &&
        std::none_of(offer.choices.begin(), offer.choices.end(),
                     [](Upgrade u) { return isTower(u) && !isSupport(towerWeapon(u)); })) {
      offer.choices[0] = Upgrade::Sentry;
    }
    rewards.push_back(offer);
  }
  void validateFront() {
    if (rewards.empty())
      return;
    auto& choices = rewards.front().choices;
    for (auto& choice : choices)
      if (!eligible(choice)) {
        for (int i = 0; i < 4; ++i) {
          Upgrade replacement = Upgrade(i);
          if (std::find(choices.begin(), choices.end(), replacement) == choices.end()) {
            choice = replacement;
            break;
          }
        }
      }
  }
  void step(float dt, float damage) {
    if (!enabled || dead() || dt <= 0)
      return;
    hp = std::max(0.f, hp - std::max(0.f, damage));
    if (dead())
      return;
    uranium += rate() * dt;
    // Each full bar pays immediately; physical collection has its own queue.
    while (uranium >= CAPACITY) {
      uranium -= CAPACITY;
      ++earned;
      ++awaitingPickup;
      addReward();
    }
    if (shipping) {
      flightAge += dt;
      if (flightAge >= FLIGHT_TIME) {
        shipping = false;
        flightAge = FLIGHT_TIME;
        ++shipments;
      }
    }
    if (!shipping && awaitingPickup > 0) {
      --awaitingPickup;
      shipping = true;
      flightAge = 0;
    }
  }
};
} // namespace command
