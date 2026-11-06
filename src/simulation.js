// Pure deterministic gameplay, independent of rendering and browser input.
export const RULES = Object.freeze({
  arenaRadius: 160, coreRadius: 8, coreHeight: 4, coreMaxHp: 1200,
  buildRadius: 58, turretRange: 35, turretCost: 80, turretDamage: 24,
  turretCooldown: 0.42, turretSpacing: 4, startingCredits: 320,
  extractionRate: 3, spawnRadius: 142,
});

export function makeRandom(seed = 7731) {
  return () => { seed |= 0; seed = seed + 0x6d2b79f5 | 0; let t = Math.imul(seed ^ seed >>> 15, 1 | seed); t ^= t + Math.imul(t ^ t >>> 7, 61 | t); return ((t ^ t >>> 14) >>> 0) / 4294967296; };
}
export function createGame(seed = 7731) {
  return { time: 0, coreHp: RULES.coreMaxHp, credits: RULES.startingCredits,
    kills: 0, wave: 0, nextWave: 12, pendingSpawns: 0, spawnTimer: 0,
    enemies: [], turrets: [], events: [], nextId: 1, random: makeRandom(seed),
    paused: false, over: false, extracted: 0 };
}
export function canBuild(game, x, z) {
  if (!Number.isFinite(x) || !Number.isFinite(z)) return 'Wskaż miejsce na powierzchni';
  if (game.over) return 'Rdzeń został zniszczony';
  const d = Math.hypot(x, z);
  if (d < RULES.coreRadius + 3) return 'Zbyt blisko rdzenia';
  if (d > RULES.buildRadius) return 'Poza siecią zasilania';
  if (game.turrets.some(t => Math.hypot(t.x - x, t.z - z) < RULES.turretSpacing)) return 'Zbyt blisko innego działa';
  if (game.credits < RULES.turretCost) return 'Za mało surowców';
  return null;
}
export function buildTurret(game, x, z) {
  const error = canBuild(game, x, z);
  if (error) return { error };
  const turret = { id: game.nextId++, x, z, targetId: null, cooldown: 0, shots: 0 };
  game.turrets.push(turret); game.credits -= RULES.turretCost;
  game.events.push({ type: 'build', id: turret.id, x, z });
  return { turret };
}
export function acquireTarget(turret, enemies) {
  const inRange = e => e.hp > 0 && Math.hypot(e.x - turret.x, e.z - turret.z) <= RULES.turretRange;
  const locked = enemies.find(e => e.id === turret.targetId);
  if (locked && inRange(locked)) return locked;
  let best = null, distance = Infinity;
  for (const e of enemies) {
    const d = Math.hypot(e.x - turret.x, e.z - turret.z);
    if (inRange(e) && d < distance) { best = e; distance = d; }
  }
  turret.targetId = best?.id ?? null;
  return best;
}
export function spawnEnemy(game, options = {}) {
  const angle = game.random() * Math.PI * 2;
  const type = options.type ?? (game.wave >= 3 && game.random() < 0.22 ? 'brute' : game.wave >= 2 && game.random() < 0.3 ? 'runner' : 'crawler');
  const stats = type === 'brute' ? { hp: 160, speed: 3.1, damage: 18, size: 6, reward: 14 } : type === 'runner' ? { hp: 42, speed: 7, damage: 6, size: 3, reward: 7 } : { hp: 68, speed: 4.3, damage: 9, size: 4.2, reward: 9 };
  const hp = stats.hp * (1 + Math.max(0, game.wave - 1) * 0.11);
  const e = { id: game.nextId++, x: Math.sin(angle) * RULES.spawnRadius, z: Math.cos(angle) * RULES.spawnRadius, ...stats, hp, maxHp: hp, type, phase: game.random() * 6.28, ...options };
  game.enemies.push(e); return e;
}
export function startWave(game) {
  if (game.over || game.pendingSpawns > 0) return false;
  game.wave++; game.pendingSpawns = 12 + game.wave * 6;
  game.spawnTimer = 0; game.nextWave = game.time + 42;
  game.events.push({ type: 'wave', wave: game.wave }); return true;
}
export function stepGame(game, dt) {
  if (game.paused || game.over || !Number.isFinite(dt) || dt <= 0) return;
  dt = Math.min(dt, 0.1); game.time += dt;
  game.credits += RULES.extractionRate * dt; game.extracted += RULES.extractionRate * dt;
  if (game.time >= game.nextWave && game.pendingSpawns === 0) startWave(game);
  game.spawnTimer -= dt;
  if (game.pendingSpawns > 0 && game.spawnTimer <= 0) {
    spawnEnemy(game); game.pendingSpawns--; game.spawnTimer = Math.max(0.18, 0.72 - game.wave * 0.035);
  }
  for (const e of game.enemies) {
    if (e.hp <= 0) continue;
    const d = Math.hypot(e.x, e.z), stop = RULES.coreRadius + 1;
    if (d > stop) { const movement = Math.min(d - stop, e.speed * dt); e.x -= e.x / d * movement; e.z -= e.z / d * movement; }
    else game.coreHp = Math.max(0, game.coreHp - e.damage * dt);
  }
  for (const t of game.turrets) {
    t.cooldown = Math.max(0, t.cooldown - dt);
    const target = acquireTarget(t, game.enemies);
    if (target && t.cooldown <= 0) {
      // Hitscan: damage is guaranteed at fire time, no projectile miss or travel time.
      target.hp -= RULES.turretDamage; t.cooldown = RULES.turretCooldown; t.shots++;
      game.events.push({ type: 'shot', turretId: t.id, targetId: target.id, x: target.x, z: target.z, height: target.size * 0.5 });
      if (target.hp <= 0) {
        game.kills++; game.credits += target.reward;
        game.events.push({ type: 'kill', x: target.x, z: target.z, size: target.size });
      }
    }
  }
  game.enemies = game.enemies.filter(e => e.hp > 0);
  if (game.coreHp <= 0) { game.over = true; game.events.push({ type: 'gameover' }); }
}
