import { test, expect } from '@playwright/test';

test('loads assets, changes views, places turrets, fights, pauses and restarts', async ({ page }) => {
  const errors = []; page.on('pageerror', error => errors.push(error.message));
  const failed = []; page.on('response', r => { if (r.status() >= 400) failed.push(`${r.status()} ${r.url()}`); });
  await page.goto('/?test=1');
  await expect(page.locator('#start')).toBeEnabled();
  await page.screenshot({ path: 'artifacts/menu.png' });
  await page.locator('#start').click();
  await expect(page.locator('.status-panel')).toBeVisible();
  await page.keyboard.press('Tab');
  await expect(page.locator('#view-name')).toHaveText('WIDOK TAKTYCZNY');
  await expect(page.locator('#build-panel')).toBeVisible();
  // Project a valid world location to pixels and build through real mouse input.
  const point = await page.evaluate(async () => {
    const { Vector3 } = await import('/node_modules/three/build/three.module.js');
    const p = new Vector3(22, 0, 16).project(window.__game.world.camera);
    return { x: (p.x + 1) / 2 * innerWidth, y: (1 - p.y) / 2 * innerHeight };
  });
  await page.mouse.move(point.x, point.y); await page.waitForTimeout(200); await page.mouse.click(point.x, point.y);
  await expect.poll(() => page.evaluate(() => window.__game.state.turrets.length)).toBe(4);
  // Repeated click on occupied location is rejected.
  await page.mouse.click(point.x, point.y);
  await expect.poll(() => page.evaluate(() => window.__game.state.turrets.length)).toBe(4);
  await page.evaluate(() => {
    window.__game.state.nextWave = window.__game.state.time + 42;
    window.__game.state.wave = 1;
    for (let i = 0; i < 25; i++) { const a = i * Math.PI * 2 / 25; window.__game.spawn({ x: Math.cos(a) * (38 + i % 5), z: Math.sin(a) * (38 + i % 5), hp: 110, maxHp: 110 }); }
  });
  await expect.poll(() => page.evaluate(() => window.__game.state.turrets.reduce((sum, t) => sum + t.shots, 0))).toBeGreaterThan(4);
  await page.mouse.move(920, 420); await page.waitForTimeout(500);
  await page.screenshot({ path: 'artifacts/tactical.png' });
  await expect.poll(() => page.evaluate(() => window.__game.state.kills), { timeout: 15000 }).toBeGreaterThan(0);
  await page.keyboard.press('Tab'); await expect(page.locator('#view-name')).toHaveText('WIDOK OPERATORA');
  await page.screenshot({ path: 'artifacts/first-person.png' });
  await page.keyboard.down('KeyW'); await page.waitForTimeout(2000); await page.keyboard.up('KeyW');
  const r = await page.evaluate(() => Math.hypot(window.__game.player.x, window.__game.player.z));
  expect(r).toBeGreaterThanOrEqual(1.64); expect(r).toBeLessThanOrEqual(7.36);
  await page.keyboard.press('KeyP'); await expect(page.locator('#pause-overlay')).toBeVisible();
  const paused = await page.evaluate(() => window.__game.state.time); await page.waitForTimeout(300);
  expect(await page.evaluate(() => window.__game.state.time)).toBe(paused);
  await page.locator('#resume').click();
  await page.evaluate(() => { window.__game.state.coreHp = 1; window.__game.spawn({ x: 9, z: 0, damage: 500, hp: 10000 }); });
  await expect(page.locator('#gameover')).toBeVisible(); await page.locator('#restart').click();
  await expect(page.locator('#gameover')).toBeHidden();
  expect(await page.evaluate(() => window.__game.state.coreHp)).toBe(1200);
  expect(await page.evaluate(() => window.__game.state.turrets.length)).toBe(3);
  expect(errors).toEqual([]); expect(failed).toEqual([]);
});
