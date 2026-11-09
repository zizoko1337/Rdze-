#include "upgrade_lab.hpp"
#include <iostream>
#include <stdexcept>
using namespace command;
void require(bool yes, const char* message) {
  if (!yes)
    throw std::runtime_error(message);
}
void tick(Session& s, float seconds) {
  for (int i = 0; i < int(seconds * 60); ++i)
    s.step(1.f / 60);
}
void mount(Session& s) {
  s.mode = Mode::Mounted;
  s.lift = s.hatch = 1;
  s.spawnTimer = s.frontier.nextBoss = s.frontier.nextElite = 100000;
  s.pitch = s.gunPitch = -.25f;
}
int main() {
  try {
    require(minimumRarity(Upgrade::PlasmaMortar) == Rarity::Legendary &&
                uniqueUpgrade(Upgrade::PlasmaMortar),
            "Plasma conversion is not unique Legendary");
    {
      Session s(true, true);
      mount(s);
      s.module.kind = Gun::Crank;
      s.expedition.manualLaser = false;
      require(!s.expedition.eligible(Upgrade::PlasmaMortar) &&
                  !s.debugGrant(Upgrade::PlasmaMortar, Rarity::Legendary),
              "Minigun receives laser-specific conversion");
      for (int i = 0; i < 1000; ++i) {
        s.expedition.addReward(RewardSource::BossCrystal);
        for (auto kind : s.expedition.rewards.back().choices)
          require(kind != Upgrade::PlasmaMortar, "Wrong manual weapon rolls laser conversion");
        s.expedition.rewards.clear();
      }
      std::cout << "PASS Legendary uniqueness and laser-only reward eligibility\n";
    }
    {
      Session s(true, true);
      mount(s);
      s.expedition.addReward();
      auto real = s.expedition.rewards.front();
      require(s.debugGrant(Upgrade::PlasmaMortar, Rarity::Common) && s.module.plasma,
              "Lab does not enforce Legendary minimum / apply actual conversion");
      require(s.expedition.rewards.size() == 1 &&
                  s.expedition.rewards.front().choices == real.choices &&
                  s.expedition.rewards.front().id == real.id &&
                  s.expedition.rewards.front().rarities == real.rarities,
              "Lab consumes or rerolls earned reward");
      require(!s.debugGrant(Upgrade::PlasmaMortar, Rarity::Legendary), "Unique conversion stacks");
      s.expedition.rewards.clear();
      Vec3 center = s.groundAim();
      s.fire();
      require(s.manualProjectiles.size() == 20 && s.shots == 1 && s.cooldown == 1,
              "Plasma volley is not twenty impulses at original laser cadence");
      float damage = 0;
      float slowest = 1000, fastest = 0;
      std::vector<int> targets;
      for (auto p : s.manualProjectiles) {
        require(p.plasma && p.damage == 150 &&
                    std::hypot(p.endpoint.x - center.x, p.endpoint.z - center.z) <= 10.001f,
                "Plasma impulse damage or dispersion radius wrong");
        float speed = std::max(60.f, length(p.endpoint - p.start)) / p.duration;
        slowest = std::min(slowest, speed);
        fastest = std::max(fastest, speed);
        require(speed >= 59.999f && speed <= 85.001f,
                "Plasma flight speed is outside the slower random range");
        damage += p.damage;
        int id = s.world.spawn(Crawler, p.endpoint.x, p.endpoint.z);
        s.world.foes[id].hp = s.world.foes[id].maxHp = 10000;
        s.world.foes[id].speed = 0;
        targets.push_back(id);
      }
      require(damage == 3000, "Volley nominal damage is not twenty times base laser");
      require(fastest - slowest > 10,
              "Plasma impulses do not have independently randomized speeds");
      s.world.grid.rebuild(s.world.foes, s.world.active);
      tick(s, .1f);
      for (int id : targets)
        require(s.world.foes[id].hp == 10000, "Plasma hits before its ballistic flight");
      Vec3 frozen = s.manualProjectiles[0].pos;
      s.upgradeLabOpen = true;
      tick(s, 1);
      require(length(s.manualProjectiles[0].pos - frozen) == 0,
              "Lab does not pause active plasma flight");
      s.upgradeLabOpen = false;
      tick(s, 5);
      require(s.manualProjectiles.empty() && s.impacts == 20,
              "Twenty independent plasma impacts missing");
      for (int id : targets)
        require(std::abs(s.world.foes[id].hp - 9850) < .01f, "Plasma applies wrong or area damage");
      s.module.damageMultiplier = 2;
      s.flatDamage = 7;
      s.module.rateMultiplier = 1.5f;
      s.fire();
      damage = 0;
      for (auto p : s.manualProjectiles)
        damage += p.damage;
      require(std::abs(damage - 6140) < .01f && std::abs(s.cooldown - 1.f / 1.5f) < .001f,
              "Plasma loses existing manual damage / rate bonuses");
      auto sample = s.manualProjectiles[0];
      s.manualProjectiles.resize(1010, sample);
      s.cooldown = 0;
      auto shots = s.shots;
      s.fire();
      require(s.shots == shots && s.manualProjectiles.size() == 1010,
              "Pool cap creates partial volley");
      std::cout << "PASS 20 independent ballistic lasers, 10m disk, 60-85m/s randomized speed, "
                   "3000 total damage, pause, "
                   "modifiers and bounded full volleys\n";
    }
    {
      Session s;
      int a = s.world.spawn(Crawler, 2.4f, 100), b = s.world.spawn(Crawler, -2.4f, 100);
      int outside = s.world.spawn(Crawler, 2.8f, 100);
      for (int id : {a, b, outside})
        s.world.foes[id].hp = s.world.foes[id].maxHp = 1000;
      s.world.grid.rebuild(s.world.foes, s.world.active);
      Vec3 end{0, .2f, 100};
      require(trace(s.world, end + Vec3{0, 60, 0}, {0, -1, 0}, 60).id == -1,
              "Thick-laser fixture is not outside the standard hitbox");
      ManualProjectile p{Gun::Pulse, end + Vec3{0, 10, 0}, {}, {}, end, {}};
      p.plasma = true;
      p.duration = .1f;
      p.damage = 150;
      s.manualProjectiles.push_back(p);
      s.stepManualProjectiles(.2f);
      require(s.impacts == 1 && s.world.foes[a].hp + s.world.foes[b].hp == 1850 &&
                  s.world.foes[outside].hp == 1000,
              "One-metre laser radius fails a grazing hit or applies area damage");
      mount(s);
      s.module.plasma = true;
      s.pitch = s.gunPitch = -.01f;
      s.fire();
      float firstDuration = s.manualProjectiles.front().duration;
      bool varied = false;
      for (const auto& shot : s.manualProjectiles) {
        require(shot.duration > 5,
                "Long-range plasma is incorrectly clamped to the old fast flight time");
        varied |= std::abs(shot.duration - firstDuration) > .2f;
      }
      require(varied, "Long-range plasma loses randomized speeds");
      int visible = 0;
      for (int i = 0; i < 20; ++i)
        visible += PlasmaSpec::visibleEmitter(i);
      require(visible == 17 && PlasmaSpec::visibleEmitter(0) && PlasmaSpec::visibleEmitter(4),
              "Wrong plasma emitters removed");
      for (int i = 1; i <= 3; ++i)
        require(length(s.plasmaMuzzle(i) - s.plasmaMuzzle(i + 5)) < .0001f,
                "A hidden front emitter still launches out of the steel housing");
      std::cout << "PASS one-metre grazing hitbox, single-target damage, long-range speed "
                   "variation and 17 visible emitters\n";
    }
    {
      UpgradeLab lab;
      require(lab.results().size() == size_t(Upgrade::Count), "Lab does not list all upgrades");
      lab.text("eksperymentalna");
      require(lab.results().size() == 1 && lab.results()[0] == Upgrade::PlasmaMortar,
              "Search cannot find experimental plasma technology");
      lab.query = "mozdzierz";
      auto results = lab.results();
      require(std::find(results.begin(), results.end(), Upgrade::MortarTower) != results.end() &&
                  std::find(results.begin(), results.end(), Upgrade::PlasmaMortar) != results.end(),
              "Search cannot match Polish accents without typing them");
      lab.query = "ż";
      lab.backspace();
      require(lab.query.empty(), "UTF-8 backspace breaks query");
      Session s(true, true);
      mount(s);
      for (int i = int(Upgrade::Sentry); i <= int(Upgrade::FlameTower); ++i)
        require(s.debugGrant(Upgrade(i), Rarity::Legendary),
                "Lab cannot test all combat turret technologies");
      require(s.sentryStock == 9 && s.expedition.chosenTypes(false) == 9,
              "Debug-only technology limit bypass did not grant actual inventory");
      require(s.debugGrant(Upgrade::ServiceCredit, Rarity::Legendary) &&
                  s.expedition.servicePoints == 1,
              "Lab service grant does not use actual service inventory");
      require(!s.debugGrant(Upgrade::Count, Rarity::Legendary), "Lab accepts invalid upgrade");
      std::cout << "PASS searchable full catalogue, accent folding, actual grants, preserved "
                   "economy and debug-only technology bypass\n";
    }
    std::cout << "ALL PLASMA AND UPGRADE LAB TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
