#include "command_game.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace command;
void require(bool ok, const char* message) {
  if (!ok)
    throw std::runtime_error(message);
}
int main() {
  try {
    {
      auto g = std::make_unique<Game>(false);
      g->autoWaves = false;
      Frontier f;
      f.mutations.nextMutation = f.nextElite = f.nextHorde = 100000;
      float timer = 100000;
      constexpr int times[] = {180, 330, 450, 540, 600, 630, 660, 690};
      for (int n = 0; n < 8; ++n) {
        g->time = float(times[n] - 10);
        f.stepWaves(*g, .01f, timer);
        require(f.bossWarning, "Missing boss warning");
        g->time = float(times[n]);
        f.stepWaves(*g, .01f, timer);
        require(f.bossesSpawned == n + 1 && f.bossWaves == n + 1,
                "Boss series arrives at wrong time");
        require(f.nextBoss == times[n] + Frontier::bossInterval(n + 1),
                "Boss interval progression wrong");
      }
      require(Frontier::bossInterval(100) == 30, "Boss interval drops below 30 seconds");
      std::cout << "PASS 3:00, 5:30, 7:30, 9:00, 10:00, 10:30 boss schedule and 30s floor\n";
    }
    {
      auto g = std::make_unique<Game>(false);
      g->autoWaves = false;
      g->time = 180;
      Frontier f;
      f.nextElite = f.nextHorde = f.mutations.nextMutation = 100000;
      require(f.mutations.apply(*g, EnemyMutation::BossCompanion, Rarity::Legendary),
              "Legendary boss mutation rejected");
      float timer = 100000;
      f.stepWaves(*g, .01f, timer);
      require(f.bossesSpawned == 1 && f.bossBurstRemaining == 1 && f.nextBoss == 330,
              "Extra boss changes series interval");
      g->time = 182;
      f.stepWaves(*g, 2, timer);
      require(f.bossesSpawned == 2 && f.spawned[Titan] == 1 && f.spawned[Queen] == 1,
              "Extra boss does not join series");
      for (int n = 0; n < 6; ++n)
        require(f.mutations.apply(*g, EnemyMutation::BossCompanion, Rarity::Legendary),
                "Extra boss stack rejected early");
      require(!f.mutations.apply(*g, EnemyMutation::BossCompanion, Rarity::Legendary) &&
                  g->enemyGenome.extraBosses == 7,
              "Boss series cap missing");
      std::cout
          << "PASS extra bosses in a series, preserved rotation, two-second staggering and cap\n";
    }
    {
      auto g = std::make_unique<Game>(false);
      g->autoWaves = false;
      g->time = 180;
      int id = g->spawn(Crawler, 200, 0);
      g->foes[id].hp = 50;
      EnemyMutationSystem m;
      require(m.apply(*g, EnemyMutation::Health, Rarity::Rare), "Health mutation rejected");
      require(std::abs(g->foes[id].maxHp - 122) < .001 && std::abs(g->foes[id].hp - 61) < .001,
              "Live HP mutation heals or multiplies incorrectly");
      m.apply(*g, EnemyMutation::Health, Rarity::Legendary);
      require(std::abs(g->foes[id].maxHp - 167) < .001 && std::abs(g->foes[id].hp - 83.5f) < .001,
              "HP bonuses do not add from base");
      Frontier f;
      int next = f.spawnFoe(*g, Crawler, 220, 0);
      require(std::abs(g->foes[next].maxHp - 167) < .001, "New foe misses existing HP mutation");
      float speed = g->foes[next].speed;
      m.apply(*g, EnemyMutation::Speed, Rarity::Legendary);
      require(std::abs(g->foes[next].speed - speed * 1.18f) < .001, "Live speed bonus missing");
      m.apply(*g, EnemyMutation::Quantity, Rarity::Common);
      int total = 0;
      for (int n = 0; n < 100; ++n)
        total += f.swarmCount(*g, 1);
      require(total == 110, "Fractional quantity bonus rounds every one-unit wave upward");
      require(f.swarmCount(*g, 120, true) == 132, "Quantity mutation misses hordes");
      for (int n = 0; n < 5; ++n)
        m.apply(*g, EnemyMutation::Haste, Rarity::Legendary);
      require(EnemyMutationSystem::interval(*g) == 60 &&
                  !m.apply(*g, EnemyMutation::Haste, Rarity::Common),
              "Mutation interval floor missing");
      std::cout << "PASS additive base HP, living/new foes, preserved health ratio, speed, exact "
                   "quantities and haste floor\n";
    }
    {
      auto g = std::make_unique<Game>(false);
      g->autoWaves = false;
      g->time = 300;
      EnemyMutationSystem m;
      int id = g->spawn(Runner, 200, 0);
      require(!m.apply(*g, EnemyMutation::RunnerDodge, Rarity::Common) && m.count == 0,
              "Special mutation accepts wrong rarity");
      require(m.apply(*g, EnemyMutation::RunnerDodge, Rarity::Epic) &&
                  !m.apply(*g, EnemyMutation::RunnerDodge, Rarity::Epic),
              "Unique dodge mutation duplicates");
      g->foes[id].hp = g->foes[id].maxHp = 1000000;
      for (int n = 0; n < 20000; ++n)
        g->hurt(id, 1, Laser);
      require(g->enemyDodges > 9400 && g->enemyDodges < 10600, "Runner dodge is not 50 percent");
      auto count = g->enemyDodges;
      float hp = g->foes[id].hp;
      for (auto type : {DamageClass::Persistent, DamageClass::Area, DamageClass::Collision,
                        DamageClass::Service})
        g->hurt(id, 100, Venom, false, type);
      require(g->enemyDodges == count && g->foes[id].hp == hp - 400,
              "Dodge avoids DOT, explosions, contact or services");
      int brute = g->spawn(Brute, 200, 0);
      m.apply(*g, EnemyMutation::BruteArmor, Rarity::Uncommon);
      float before = g->foes[brute].hp;
      g->hurt(brute, 100, Laser);
      require(g->foes[brute].hp == before - 80, "Brute armor has no real damage reduction");
      m.apply(*g, EnemyMutation::BruteRegen, Rarity::Rare);
      before = g->foes[brute].hp;
      g->step(.1f);
      require(std::abs(g->foes[brute].hp - before - g->foes[brute].maxHp * .001f) < .002,
              "Brute regeneration missing");
      int floater = g->spawn(Floater, 200, 0);
      m.apply(*g, EnemyMutation::FloaterShield, Rarity::Epic);
      before = g->foes[floater].hp;
      g->hurt(floater, 100, Laser);
      require(std::abs(g->foes[floater].hp - before + 65) < .001, "Distant floater shield missing");
      g->foes[floater].x = 60;
      before = g->foes[floater].hp;
      g->hurt(floater, 10, Laser);
      require(g->foes[floater].hp == before - 10, "Floater shield persists inside 80 metres");
      std::cout << "PASS 50% direct-hit dodge, unavoidable DOT/AOE, armor, regeneration and "
                   "range-based shield\n";
    }
    {
      auto g = std::make_unique<Game>(false);
      g->autoWaves = false;
      g->time = 180;
      EnemyMutationSystem m;
      m.apply(*g, EnemyMutation::SpitterCadence, Rarity::Rare);
      m.apply(*g, EnemyMutation::Damage, Rarity::Rare);
      int id = g->spawn(Spitter, 50, 0);
      g->foes[id].ability = 0;
      g->step(.01f);
      require(std::abs(g->foes[id].ability - 2.1f) < .001 && std::abs(g->coreDamage - 29.5f) < .001,
              "Mutated acid cadence or core damage bonus missing");
      Frontier f;
      m.apply(*g, EnemyMutation::EliteBrood, Rarity::Legendary);
      int elites = 0;
      for (int n = 0; n < 4000; ++n) {
        int foe = f.spawnFoe(*g, Crawler, 300, 0);
        elites += g->foes[foe].elite;
      }
      require(elites > 145 && elites < 260, "Elite brood is not five percent");
      int runner = g->spawn(Runner, 150, 0);
      g->foes[runner].speed = 10;
      g->foes[runner].hp = g->foes[runner].maxHp * .2f;
      m.apply(*g, EnemyMutation::BloodRush, Rarity::Epic);
      float x = g->foes[runner].x;
      g->step(.1f);
      require(std::abs(x - g->foes[runner].x - 1.5f) < .002, "Wounded unit does not sprint");
      g->addTower(Laser,42,0);g->towers.back().cooldown=100000;
      int saboteur=g->spawn(Saboteur,100,0);
      g->foes[saboteur].speed=10;g->foes[saboteur].hp=g->foes[saboteur].maxHp*.2f;
      g->step(.1f);
      require(std::abs(g->foes[saboteur].x-98.5f)<.002,
              "Wounded saboteur loses sprint when targeting a turret");
      std::cout
          << "PASS faster enemy attacks, multiplied core damage, elite brood and wounded sprint\n";
    }
    {
      auto g = std::make_unique<Game>(false);
      g->autoWaves = false;
      EnemyMutationSystem m;
      g->time = 179.9f;
      m.step(*g, .1f);
      require(m.count == 0, "Early mutation before three minutes");
      g->time = 180;
      m.step(*g, .01f);
      require(m.count == 1 && m.current() && g->mutationAlerts == 1,
              "First mutation or comms notice missing");
      auto age = m.noticeAge;
      g->paused = true;
      g->time = m.nextMutation;
      m.step(*g, 1);
      require(m.count == 1 && m.noticeAge == age, "Pause advances mutations/comms");
      g->paused = false;
      m.apply(*g, EnemyMutation::DoubleMutation, Rarity::Legendary);
      auto before = m.count;
      m.step(*g, .01f);
      require(m.count == before + 2, "Double adaptation does not draw two mutations");
      require(!EnemyMutationSystem::mouthOpen(0) && EnemyMutationSystem::mouthOpen(.5f) &&
                  !EnemyMutationSystem::mouthOpen(8),
              "Captain mouth frames do not alternate");
      EnemyMutationSystem odds;
      std::array<int, 5> counts{};
      for (int n = 0; n < 100000; ++n)
        ++counts[int(odds.rollRarity())];
      constexpr int expected[] = {40000, 32000, 18000, 8000, 2000};
      for (int n = 0; n < 5; ++n)
        require(std::abs(counts[n] - expected[n]) < 1100,
                "Mutation rarity probabilities incorrect");
      std::cout << "PASS active-clock mutations, independent rarity odds, queued captain notices, "
                   "mouth frames, pause and double draws\n";
    }
    {
      auto s=std::make_unique<Session>(true,true);s->mode=Mode::Walking;
      s->world.time=179.98f;s->spawnTimer=100000;s->frontier.nextElite=100000;
      s->step(.04f);
      require(s->frontier.mutations.count==1 && s->frontier.mutations.current() &&
                  s->frontier.bossWaves==1 && s->frontier.bossesSpawned==1,
              "Real expedition does not deliver first mutation, boss and captain notice together");
      auto time=s->world.time;auto age=s->frontier.mutations.noticeAge;
      s->terminalOpen=true;s->step(.05f);s->terminalOpen=false;
      require(s->world.time==time && s->frontier.mutations.noticeAge==age,
              "Terminal does not freeze mutation communication");
      auto before=s->frontier.mutations.count;
      s->world.time=s->frontier.mutations.nextMutation-.02f;s->step(.04f);
      require(s->frontier.mutations.count>before,"Real expedition never rolls its next mutation");
      auto g=std::make_unique<Game>(false);g->autoWaves=false;g->time=600;
      for(int n=0;n<Frontier::MAX_ACTIVE;++n) g->spawn(Crawler,300,0);
      g->spawn(Titan,225,0);
      Frontier f;f.nextBoss=f.nextElite=f.nextHorde=f.mutations.nextMutation=100000;
      f.pendingHorde=12;float timer=100000;f.stepWaves(*g,.01f,timer);
      require(f.pendingHorde==12 && g->active.size()==6001,
              "Boss above population cap makes queued horde grow through negative spawn count");
      std::cout<<"PASS real expedition mutation/boss/comms integration, terminal pause and overloaded population cap\n";
    }
    {
      auto g = std::make_unique<Game>(false);
      g->autoWaves = false;
      g->time = 180;
      Frontier f;
      f.nextBoss = f.nextHorde = f.mutations.nextMutation = 100000;
      int existing = f.spawnFoe(*g, Brute, 300, 0, true);
      require(!f.mutations.apply(*g, EnemyMutation::SaboteurWave, Rarity::Epic),
              "Saboteur invasion accepts the wrong rarity");
      require(f.mutations.apply(*g, EnemyMutation::SaboteurWave, Rarity::Rare) &&
                  g->enemyGenome.saboteurWaveUntil == 240 &&
                  g->foes[existing].type == Brute &&
                  f.mutations.current()->kind == EnemyMutation::SaboteurWave &&
                  f.mutations.current()->rarity == Rarity::Rare &&
                  f.mutations.current()->description.find("60 s") != std::string::npos,
              "Rare invasion duration, untouched living elites or captain notice wrong");
      require(!f.mutations.apply(*g, EnemyMutation::SaboteurWave, Rarity::Rare),
              "Active invasion can stack or replace itself");
      float timer = 100000;
      f.nextElite = 180;
      for (float time : {180.f, 225.f}) {
        g->time = time;
        f.stepWaves(*g, .01f, timer);
        auto& spawned = g->foes[g->active.back()];
        require(spawned.type == Saboteur && spawned.elite &&
                    spawned.maxHp == Frontier::spawnHp(Saboteur, time, true) &&
                    spawned.size == SPECIES[Saboteur].size,
                "Scheduled elite is not a normally scaled Saboteur");
      }
      require(f.nextElite == 270, "Invasion changes elite cadence");
      int ordinary = f.spawnFoe(*g, Runner, 300, 0);
      int boss = f.spawnFoe(*g, Titan, 300, 0);
      require(g->foes[ordinary].type == Runner && !g->foes[ordinary].elite &&
                  g->foes[boss].type == Titan,
              "Invasion converts regular enemies or bosses");
      g->enemyGenome.eliteBrood = true;
      int promoted = 0;
      for (int n = 0; n < 500; ++n) {
        int id = f.spawnFoe(*g, Crawler, 300, 0);
        if (g->foes[id].elite) {
          ++promoted;
          require(g->foes[id].type == Saboteur, "Elite brood bypasses Saboteur invasion");
        } else {
          require(g->foes[id].type == Crawler, "Unpromoted brood unit changes species");
        }
      }
      require(promoted > 0, "Elite brood fixture produces no elites");
      g->time = 239.99f;
      int last = f.spawnFoe(*g, Runner, 300, 0, true);
      require(g->foes[last].type == Saboteur, "Invasion expires early");
      g->time = 240;
      int normal = f.spawnFoe(*g, Runner, 300, 0, true);
      require(g->foes[normal].type == Runner && g->foes[normal].elite &&
                  g->foes[last].type == Saboteur && g->foes[last].hp > 0,
              "Expiry fails to restore new elite types or removes existing Saboteurs");
      require(f.mutations.apply(*g, EnemyMutation::SaboteurWave, Rarity::Rare) &&
                  g->enemyGenome.saboteurWaveUntil == 300,
              "Expired invasion never returns to the Rare pool");
      std::cout << "PASS Rare 60s Saboteur invasion, scheduled/brood elites, normal cadence, expiry and repeat\n";
    }
    {
      auto s = std::make_unique<Session>(true, true);
      s->mode = Mode::Walking;
      s->world.time = 180;
      s->spawnTimer = s->frontier.nextElite = s->frontier.nextBoss =
          s->frontier.nextHorde = s->frontier.mutations.nextMutation = 100000;
      s->frontier.mutations.apply(s->world, EnemyMutation::SaboteurWave, Rarity::Rare);
      s->world.paused = true;
      s->step(1);
      s->world.paused = false;
      s->terminalOpen = true;
      s->step(1);
      require(s->world.time == 180 && s->world.enemyGenome.saboteurWaveUntil == 240,
              "Pause or terminal consumes invasion duration");
      s->terminalOpen = false;
      s->step(.05f);
      require(s->world.time > 180 && s->world.enemyGenome.saboteurWaveUntil == 240,
              "Invasion does not use active game time");
      s = std::make_unique<Session>(true, true); // Restart creates a new session in the app.
      s->start(Gun::Pulse);
      require(s->world.enemyGenome.saboteurWaveUntil == 0,
              "Invasion leaks into a new expedition");
      auto g = std::make_unique<Game>(false);
      g->time = 180;
      EnemyMutationSystem m;
      bool rolled = false;
      for (int n = 0; n < 1000 && !rolled; ++n) {
        g->enemyGenome = {};
        m.roll(*g);
        rolled = m.history.back().kind == EnemyMutation::SaboteurWave;
      }
      require(rolled && m.history.back().rarity == Rarity::Rare,
              "Natural mutation draw omits Rare invasion");
      std::cout << "PASS invasion natural roll, active-clock pause and new-expedition reset\n";
    }
    std::cout << "ALL ENEMY MUTATION TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
