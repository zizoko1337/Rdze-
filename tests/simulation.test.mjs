import test from 'node:test';
import assert from 'node:assert/strict';
import { RULES, createGame, buildTurret, canBuild, acquireTarget, stepGame, spawnEnemy, startWave } from '../src/simulation.js';

test('building checks power radius, core collision, spacing and credits without charging on failure', () => {
  const game = createGame();
  for (const [x, z] of [[0, 0], [90, 0], [NaN, 5], [Infinity, 0]]) assert.ok(canBuild(game, x, z));
  assert.equal(game.credits, 320);
  const { turret } = buildTurret(game, 20, 0); assert.ok(turret); assert.equal(game.credits, 240);
  assert.ok(buildTurret(game, 22, 0).error); assert.equal(game.credits, 240);
  game.credits = 79; assert.ok(buildTurret(game, -20, 0).error);
  game.credits = 80; assert.ok(buildTurret(game, -20, 0).turret); assert.equal(game.credits, 0);
});
test('targets nearest living enemy, then retains target even when another comes closer', () => {
  const t = { x: 20, z: 0, targetId: null };
  const far = { id: 1, x: 40, z: 0, hp: 100 }, close = { id: 2, x: 30, z: 0, hp: 100 };
  assert.equal(acquireTarget(t, [far, close]).id, 2);
  far.x = 21; assert.equal(acquireTarget(t, [far, close]).id, 2);
  close.hp = 0; assert.equal(acquireTarget(t, [far, close]).id, 1);
});
test('target unlocks outside range and ignores dead enemies', () => {
  const t = { x: 20, z: 0, targetId: 1 };
  const a = { id: 1, x: 56, z: 0, hp: 100 }, b = { id: 2, x: 25, z: 0, hp: 0 };
  assert.equal(acquireTarget(t, [a, b]), null); assert.equal(t.targetId, null);
  a.x = 55; assert.equal(acquireTarget(t, [a, b]).id, 1);
});
test('hitscan always applies damage immediately and rewards a kill once', () => {
  const g = createGame(); g.nextWave = Infinity;
  const { turret } = buildTurret(g, 20, 0);
  const e = spawnEnemy(g, { x: 30, z: 0, hp: 24, speed: 0 });
  stepGame(g, 1 / 60);
  assert.equal(g.enemies.length, 0); assert.equal(g.kills, 1); assert.equal(turret.shots, 1);
  assert.equal(g.events.filter(x => x.type === 'shot' && x.targetId === e.id).length, 1);
  stepGame(g, 1 / 60); assert.equal(g.kills, 1);
});
test('multiple turrets cannot kill or reward the same corpse twice', () => {
  const g = createGame(); g.nextWave = Infinity;
  buildTurret(g, 15, 0); buildTurret(g, 20, 0);
  spawnEnemy(g, { x: 25, z: 0, hp: 1, speed: 0 }); stepGame(g, 1 / 60);
  assert.equal(g.kills, 1); assert.equal(g.events.filter(e => e.type === 'kill').length, 1);
});
test('enemies move toward center, stop at cylinder and damage core over time', () => {
  const g = createGame(); g.nextWave = Infinity;
  const e = spawnEnemy(g, { x: 10, z: 0, speed: 10, damage: 9 });
  stepGame(g, 0.1); assert.equal(e.x, 9); assert.equal(g.coreHp, RULES.coreMaxHp);
  stepGame(g, 0.1); assert.ok(Math.abs(g.coreHp - (RULES.coreMaxHp - 0.9)) < 1e-8); assert.equal(e.x, 9);
});
test('pause freezes movement, extraction and wave timers', () => {
  const g = createGame(); const e = spawnEnemy(g, { x: 100, z: 0 }); g.paused = true;
  stepGame(g, 0.1); assert.equal(g.time, 0); assert.equal(g.credits, 320); assert.equal(e.x, 100);
});
test('core death ends simulation and prevents building', () => {
  const g = createGame(); g.coreHp = 1; spawnEnemy(g, { x: 9, z: 0, damage: 100 });
  stepGame(g, 0.1); assert.equal(g.over, true); assert.equal(g.coreHp, 0);
  const t = g.time; stepGame(g, 0.1); assert.equal(g.time, t); assert.ok(buildTurret(g, 20, 20).error);
});
test('waves spawn on horizon and extraction provides credits', () => {
  const g = createGame(); startWave(g); assert.equal(g.wave, 1);
  stepGame(g, 1 / 60); assert.equal(g.enemies.length, 1); assert.equal(g.pendingSpawns, 17);
  assert.ok(Math.hypot(g.enemies[0].x, g.enemies[0].z) > 141); assert.ok(g.credits > 320);
});
test('seeded simulations replay identically over five minutes', () => {
  function run() { const g = createGame(42); for (let i = 0; i < 4; i++) buildTurret(g, Math.cos(i * Math.PI / 2) * 20, Math.sin(i * Math.PI / 2) * 20); for (let i = 0; i < 18000; i++) { stepGame(g, 1 / 60); g.events.length = 0; } return { hp: g.coreHp, wave: g.wave, kills: g.kills, time: g.time }; }
  const a = run(), b = run(); assert.deepEqual(a, b); assert.ok(a.wave >= 3); assert.ok(a.kills > 20);
});
