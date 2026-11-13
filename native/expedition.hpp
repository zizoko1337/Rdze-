#pragma once
#include "gameplay.hpp"
#include "turret_slots.hpp"
#include "manual_arsenal.hpp"
#include "coffee.hpp"
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
  PlasmaMortar,
  ExtremeCooling,
  Nanobots,
  UraniumReserve,
  TeslaAmplifier,
  PlasmaGlue,
  BouncyMortar,
  InstantLock,
  DoubleRangeBonus,
  DualFlame,
  CoffeeII,
  CoffeeIII,
  CoffeeIV,
  CoffeeV,
  CoffeeMachineII,
  CoffeeMachineIII,
  LaserDamage,
  LaserRate,
  LaserRange,
  MinigunDamage,
  MinigunRate,
  MinigunRange,
  TeslaDamage,
  TeslaRate,
  TeslaRange,
  SniperDamage,
  SniperRate,
  SniperRange,
  LanceDamage,
  LanceRate,
  LanceRange,
  VenomDamage,
  VenomRate,
  VenomRange,
  RocketDamage,
  RocketRate,
  RocketRange,
  MortarDamage,
  MortarRate,
  MortarRange,
  FlameDamage,
  FlameRate,
  FlameRange,
  Count
};
enum class Rarity { Common, Uncommon, Rare, Epic, Legendary };
inline constexpr bool isGlobalTowerUpgrade(Upgrade u) {
  return u == Upgrade::TowerDamage || u == Upgrade::TowerRate || u == Upgrade::TowerRange;
}
inline constexpr bool isTypedTowerUpgrade(Upgrade u) {
  return u >= Upgrade::LaserDamage && u <= Upgrade::FlameRange;
}
inline constexpr Weapon upgradedTower(Upgrade u) {
  return isTypedTowerUpgrade(u) ? Weapon((int(u) - int(Upgrade::LaserDamage)) / 3) : TowerTypeCount;
}
inline constexpr int towerStat(Upgrade u) {
  return (int(u) - int(Upgrade::LaserDamage)) % 3;
}
inline constexpr Upgrade typedTowerUpgrade(Weapon w, int stat) {
  return Upgrade(int(Upgrade::LaserDamage) + int(w) * 3 + stat);
}
inline constexpr int coffeeUpgradeTier(Upgrade u) {
  return u >= Upgrade::CoffeeII && u <= Upgrade::CoffeeV ? int(u) - int(Upgrade::CoffeeII) + 2 : 0;
}
inline constexpr int machineUpgradeTier(Upgrade u) {
  return u == Upgrade::CoffeeMachineII ? 2 : u == Upgrade::CoffeeMachineIII ? 3 : 0;
}
// -1 means that the card uses the normal minimum-rarity rules.
inline constexpr int fixedRarity(Upgrade u) {
  switch (u) {
  case Upgrade::Sentry:
  case Upgrade::MinigunTower:
  case Upgrade::VenomTower:
  case Upgrade::FlameTower:
    return int(Rarity::Common);
  case Upgrade::SniperTower:
  case Upgrade::RocketTower:
  case Upgrade::MortarTower:
  case Upgrade::RateRelayTower:
  case Upgrade::RangeRelayTower:
    return int(Rarity::Uncommon);
  case Upgrade::TeslaTower:
  case Upgrade::LanceTower:
  case Upgrade::FrostSupportTower:
    return int(Rarity::Rare);
  case Upgrade::CoffeeII:
    return int(Rarity::Common);
  case Upgrade::CoffeeIII:
  case Upgrade::CoffeeMachineII:
    return int(Rarity::Rare);
  case Upgrade::CoffeeIV:
    return int(Rarity::Epic);
  case Upgrade::CoffeeV:
  case Upgrade::CoffeeMachineIII:
    return int(Rarity::Legendary);
  default:
    return -1;
  }
}
inline constexpr Gun requiredManualGun(Upgrade u) {
  switch (u) {
  case Upgrade::PlasmaMortar:
    return Gun::Pulse;
  case Upgrade::ExtremeCooling:
    return Gun::Crank;
  case Upgrade::UraniumReserve:
    return Gun::Lance;
  case Upgrade::TeslaAmplifier:
    return Gun::Arc;
  case Upgrade::PlasmaGlue:
    return Gun::Toxin;
  case Upgrade::BouncyMortar:
    return Gun::Howitzer;
  case Upgrade::InstantLock:
    return Gun::Smart;
  case Upgrade::DoubleRangeBonus:
    return Gun::Rail;
  case Upgrade::DualFlame:
    return Gun::Flame;
  default:
    return Gun::Count;
  }
}
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
  if (fixedRarity(u) >= 0)
    return Rarity(fixedRarity(u));
  if (requiredManualGun(u) != Gun::Count)
    return Rarity::Legendary;
  switch (u) {
  case Upgrade::PlasmaMortar:
  case Upgrade::ExtremeCooling:
  case Upgrade::UraniumReserve:
    return Rarity::Legendary;
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
  case Upgrade::RateRelayTower:
  case Upgrade::RangeRelayTower:
  case Upgrade::RoverFactory:
    return Rarity::Uncommon;
  default:
    return Rarity::Common;
  }
}
inline constexpr bool uniqueUpgrade(Upgrade u) {
  if (coffeeUpgradeTier(u) || machineUpgradeTier(u))
    return true;
  if (requiredManualGun(u) != Gun::Count)
    return true;
  return u == Upgrade::Frost || u == Upgrade::Focus || u == Upgrade::Purge ||
         u == Upgrade::RoverFactory || u == Upgrade::PlasmaMortar || u == Upgrade::ExtremeCooling ||
         u == Upgrade::UraniumReserve;
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
     "Piorun trafia hordę w zasięgu. Mniej skuteczny na elity i bossów.",
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
     "Uruchamia produkcję: 5 jeździków co 12 s. Wybuch: 180 obrażeń w promieniu 4 m, mniej bossom.",
     {.3f, 1, .8f}},
    {"Większa seria", "PRODUKCJA JEŹDZIKÓW", "", {.5f, 1, .7f}},
    {"Szybsza linia", "PRODUKCJA JEŹDZIKÓW", "", {1, .75f, .3f}},
    {"Punkt usług",
     "JEDNORAZOWE WSPARCIE",
     "+1 punkt do wydania przy prawym komputerze na Brimstone, falę ogłuszającą lub przeciążenie.",
     {.3f, .62f, 1}},
    {"Laserowy moździerz",
     "EKSPERYMENTALNA TECHNOLOGIA PLAZMOWA",
     "Nielegalna technologia eksperymentalna. Nie została zatwierdzona, ale raczej nikt się nie "
     "dowie, że jej używasz. Salwa 20 impulsów lasera, rozrzut w promieniu 10 m.",
     {.2f, .95f, 1}},
    {"Extreme Cooling System",
     "LEGENDARNE DZIAŁO MANUALNE",
     "Kriogeniczne dysze chłodzą rozgrzane końcówki luf. Bazowa szybkostrzelność x5, rozrzut x2. "
     "Zachowuje dotychczasowe premie.",
     {.55f, .85f, 1}},
    {"Nanoboty regenerujące",
     "RDZEŃ / REGENERACJA",
     "Zwiększa stałą regenerację rdzenia.",
     {.3f, 1, .65f}},
    {"REZERWA URANOWA",
     "LEGENDARNA LANCA",
     "Po 3 s lanca przechodzi na zieloną wiązkę. Zużywa 5 uranu/s do puszczenia LPM lub "
     "wyczerpania paliwa.",
     {.3f, 1, .2f}},
    {"Tryb wzmacniania",
     "LEGENDARNA TESLA",
     "PPM: przełącz na wsparcie. Łańcuch wzmacnia do 10 wieżyczek: +100% szybkostrzelności na 10 "
     "s.",
     {.45f, .7f, 1}},
    {"Dodatek kleju plazmowego",
     "LEGENDARNA KATAPULTA",
     "Plamy toksyny unieruchamiają zamiast spowalniać. Działają też na bossów.",
     {.16f, .4f, .06f}},
    {"Naboje z kauczuku antygrawitacyjnego",
     "LEGENDARNY MOŹDZIERZ",
     "BOUNCY BOOM: dwa dodatkowe odbicia do przodu, łącznie trzy eksplozje pełnej mocy.",
     {1, .7f, .2f}},
    {"Instant namierzanie",
     "LEGENDARNE RAKIETY SMART",
     "Usuwa półsekundowy czas namierzania. Natychmiastowa blokada celu w okienku.",
     {1, .4f, .3f}},
    {"Podwójna premia dystansu",
     "LEGENDARNY SNAJPER",
     "+2% obrażeń za metr zamiast +1%. Dodatkowy ekran CRT z dystansem i potencjalnymi "
     "obrażeniami.",
     {.3f, 1, .8f}},
    {"Podwójny palnik",
     "LEGENDARNY MIOTACZ",
     "Druga lufa. Szeroki i skupiony strumień działają równocześnie. Zastępuje przełącznik trybów.",
     {1, .45f, .1f}},
    {"Frontier Roast / kawa II",
     "KAWA / TIER II",
     "Kolejna filiżanka usuwa senność na 6 minut.",
     {1, .5f, .12f}},
    {"Luna Nitro / kawa III",
     "KAWA / TIER III",
     "Kolejna filiżanka usuwa senność na 7 minut.",
     {.1f, .85f, 1}},
    {"Reactor Blend / kawa IV",
     "KAWA / TIER IV",
     "Kolejna filiżanka usuwa senność na 8 minut.",
     {.7f, 1, .08f}},
    {"Pure Caffeine Star Dust / kawa V",
     "KAWA / TIER V",
     "Kolejna filiżanka usuwa senność na 10 minut.",
     {.8f, .35f, 1}},
    {"Ekspres / podwójne ciśnienie",
     "EKSPRES / RARE",
     "Cały cykl kawy: 2 s. Drugi manometr, rurka i kranik. Picie: 0,5 s.",
     {.1f, .85f, 1}},
    {"Ekspres / Caffeine Gatling",
     "EKSPRES / LEGENDARY",
     "Cały cykl kawy: 1 s. Obrotowy minigun kawowy i reaktor. Picie: 0,5 s.",
     {1, .65f, .15f}},
    {"Laser / obrażenia",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Laser / szybkostrzelność",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Laser / zasięg",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Minigun / obrażenia",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Minigun / szybkostrzelność",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Minigun / zasięg",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Tesla / obrażenia",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Tesla / szybkostrzelność",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Tesla / zasięg",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Snajper / obrażenia",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Snajper / szybkostrzelność",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Snajper / zasięg",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Lanca / obrażenia",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Lanca / szybkostrzelność",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Lanca / zasięg",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Toksyna / obrażenia",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Toksyna / szybkostrzelność",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Toksyna / zasięg",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Rakiety / obrażenia",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Rakiety / szybkostrzelność",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Rakiety / zasięg",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Moździerz / obrażenia",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Moździerz / szybkostrzelność",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Moździerz / zasięg",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Palnik / obrażenia",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Palnik / szybkostrzelność",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
    {"Palnik / zasięg",
     "ROZWÓJ TYPU WIEŻYCZKI",
     "Kolejny poziom parametrów wybranego typu obrony.",
     {.4f, .8f, 1}},
};
static_assert(std::size(UPGRADES) == size_t(Upgrade::Count));
inline float typedTowerBonus(Upgrade u, Rarity rarity) {
  return RARITIES[int(rarity)].power * (towerStat(u) == 0 ? .30f : .20f);
}
inline std::string upgradeDescription(Upgrade u, Rarity rarity) {
  if (isTypedTowerUpgrade(u)) {
    static constexpr const char* stats[] = {"obrażeń", "szybkostrzelności", "zasięgu"};
    int percent = int(std::round(typedTowerBonus(u, rarity) * 100));
    return std::string(WEAPONS[upgradedTower(u)].name) + ": +" + std::to_string(percent) +
           "% bazowych " + stats[towerStat(u)] + ". Poziom " + std::to_string(int(rarity) + 1) +
           "/5. Dotyczy wszystkich obecnych i przyszłych sztuk tego typu. Premie sumują się.";
  }
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
  case Upgrade::Nanobots:
    return "+" + percent(1) + " HP/s stałej regeneracji rdzenia. Premie sumują się.";
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
  float regeneration = 3;
  int shipments = 0, earned = 0, awaitingPickup = 0, selected = 0, luck = 0;
  bool purgeUsed = false;
  bool miningStarted = false;
  bool manualLaser = true;
  bool manualMinigun = false;
  bool manualLance = false;
  Gun manualKind = Gun::Pulse;
  int servicePoints = 0;
  int coffeeTier = 1, coffeeMachineTier = 1;
  float activeTime = 0;
  int offersWithoutCombat = 0, uraniumOffers = 0;
  std::array<bool, TowerTypeCount> towerUnlocked{};
  uint64_t rewardSerial = 0;
  int crystalRewards = 0, roverBatch = 5;
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
    if (isGlobalTowerUpgrade(u))
      return levels[size_t(u)] < 5;
    if (isTypedTowerUpgrade(u)) {
      Weapon type = upgradedTower(u);
      bool known = towerUnlocked[type];
      for (int i = 0; i < int(Upgrade::Count); ++i)
        if (isTower(Upgrade(i)) && towerWeapon(Upgrade(i)) == type && levels[i] > 0)
          known = true;
      return known && levels[size_t(u)] < 5;
    }
    if (coffeeUpgradeTier(u))
      return coffeeUpgradeTier(u) > coffeeTier;
    if (machineUpgradeTier(u))
      return machineUpgradeTier(u) > coffeeMachineTier;
    if (requiredManualGun(u) != Gun::Count && requiredManualGun(u) != manualKind)
      return false;
    if (u == Upgrade::PlasmaMortar && !manualLaser)
      return false;
    if (u == Upgrade::ExtremeCooling && !manualMinigun)
      return false;
    if (u == Upgrade::UraniumReserve && !manualLance)
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
  Rarity effectiveRarity(Upgrade u, Rarity requested) const {
    if (isTypedTowerUpgrade(u))
      return Rarity(std::clamp(levels[size_t(u)], 0, 4));
    if (fixedRarity(u) >= 0)
      return Rarity(fixedRarity(u));
    if (u == Upgrade::Luck || u == Upgrade::FlatDamage)
      return Rarity::Common;
    if (u == Upgrade::ServiceCredit)
      return Rarity::Rare;
    return std::max(requested, minimumRarity(u));
  }
  bool eligible(Upgrade u, Rarity rarity) const {
    return eligible(u) && minimumRarity(u) <= rarity &&
           (!isTypedTowerUpgrade(u) || int(rarity) == levels[size_t(u)]) &&
           (fixedRarity(u) < 0 || fixedRarity(u) == int(rarity)) &&
           ((u != Upgrade::FlatDamage && u != Upgrade::Luck) || rarity == Rarity::Common) &&
           (u != Upgrade::ServiceCredit || rarity == Rarity::Rare);
  }
  std::string cardDescription(Upgrade u, Rarity rarity) const {
    std::string description = upgradeDescription(u, rarity);
    if (isGlobalTowerUpgrade(u))
      description += " Wybrano " + std::to_string(levels[size_t(u)]) +
                     "/5. Każda rzadkość zużywa jeden wybór.";
    return description;
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
    else
      ++uraniumOffers;
    std::array<Upgrade, WeaponCount> combat{};
    int combatCount = 0;
    for (int i = 0; i < int(Upgrade::Count); ++i) {
      auto u = Upgrade(i);
      if (isTower(u) && !isSupport(towerWeapon(u)) && eligible(u))
        combat[combatCount++] = u;
    }
    float chance = source == RewardSource::Uranium && activeTime < 600 ? .60f : .40f;
    bool first = source == RewardSource::Uranium && uraniumOffers == 1;
    bool forceCombat =
        combatCount > 0 && (first || offersWithoutCombat >= 2 || rewardRandom.unit() < chance);
    for (int card = 0; card < 3; ++card) {
      Rarity rarity = rollRarity(source);
      if (card == 0 && forceCombat) {
        auto u = combat[rewardRandom.next() % uint32_t(combatCount)];
        offer.choices[card] = u;
        offer.rarities[card] = Rarity(fixedRarity(u));
        continue;
      }
      std::array<Upgrade, size_t(Upgrade::Count)> pool{};
      int count = 0;
      for (int i = 0; i < int(Upgrade::Count); ++i) {
        auto u = Upgrade(i);
        if (eligible(u, rarity) &&
            std::find(offer.choices.begin(), offer.choices.end(), u) == offer.choices.end())
          pool[count++] = u;
      }
      offer.choices[card] = pool[rewardRandom.next() % uint32_t(count)];
      offer.rarities[card] = rarity;
    }
    bool hasCombat = std::any_of(offer.choices.begin(), offer.choices.end(), [](Upgrade u) {
      return isTower(u) && !isSupport(towerWeapon(u));
    });
    offersWithoutCombat = hasCombat || combatCount == 0 ? 0 : offersWithoutCombat + 1;
    rewards.push_back(offer);
  }
  void validateFront() {
    if (rewards.empty())
      return;
    auto& offer = rewards.front();
    auto& choices = offer.choices;
    for (int c = 0; c < 3; ++c) {
      auto& choice = choices[c];
      if (!eligible(choice, offer.rarities[c])) {
        // Keep the original rarity and prefer a combat delivery when repairing that category.
        bool preferCombat = isTower(choice) && !isSupport(towerWeapon(choice));
        for (int pass = 0; pass < 2; ++pass) {
          bool replaced = false;
          for (int i = 0; i < int(Upgrade::Count); ++i) {
            Upgrade replacement = Upgrade(i);
            if (eligible(replacement, offer.rarities[c]) &&
                (!preferCombat || pass > 0 ||
                 (isTower(replacement) && !isSupport(towerWeapon(replacement)))) &&
                std::find(choices.begin(), choices.end(), replacement) == choices.end()) {
              choice = replacement;
              replaced = true;
              break;
            }
          }
          if (replaced)
            break;
        }
      }
    }
  }
  void step(float dt, float damage) {
    if (!enabled || dead() || dt <= 0)
      return;
    if (miningStarted)
      activeTime += dt;
    hp = std::max(0.f, hp - std::max(0.f, damage));
    if (dead())
      return;
    hp = std::min(maxHp, hp + regeneration * dt);
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
