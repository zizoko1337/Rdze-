import { spawn } from 'node:child_process';
import { chromium } from '@playwright/test';
import assert from 'node:assert/strict';
import { setTimeout as delay } from 'node:timers/promises';

// Run the actual Windows launcher, only suppress its automatic visible browser.
const child = spawn('cmd.exe', ['/d', '/c', 'WEB_PROTOTYP.bat'], { windowsHide: true, env: { ...process.env, RDZEN_NO_OPEN: '1' }, stdio: ['ignore', 'pipe', 'pipe'] });
let output = ''; child.stdout.on('data', d => output += d); child.stderr.on('data', d => output += d);
let browser;
try {
  let url;
  for (let i = 0; i < 80; i++) { url = output.match(/http:\/\/127\.0\.0\.1:\d+/)?.[0]; if (url) break; await delay(100); }
  assert.ok(url, `Launcher failed: ${output}`);
  const response = await fetch(url + '/__rdzen_health'); assert.equal(response.status, 200); assert.equal((await response.json()).game, 'rdzen');
  browser = await chromium.launch({ channel: 'msedge', headless: true });
  const page = await browser.newPage({ viewport: { width: 1440, height: 900 } });
  const errors = []; page.on('pageerror', e => errors.push(e.message));
  const external = []; page.on('request', r => { if (!r.url().startsWith(url) && !r.url().startsWith('data:')) external.push(r.url()); });
  await page.goto(url);
  await page.locator('#start:enabled').waitFor(); await page.locator('#start').click();
  await page.keyboard.press('Tab'); await page.locator('#build-panel').waitFor();
  assert.equal(await page.locator('#view-name').textContent(), 'WIDOK TAKTYCZNY');
  await page.waitForTimeout(500); await page.screenshot({ path: 'artifacts/production-launch.png' });
  assert.deepEqual(errors, []); assert.deepEqual(external, []);
  console.log('WEB_PROTOTYP.bat OK. Production build loads all assets locally; FPS and tactical mode work.');
} finally {
  await browser?.close();
  if (child.pid) await new Promise(resolve => { const kill = spawn('taskkill', ['/pid', String(child.pid), '/t', '/f'], { windowsHide: true, stdio: 'ignore' }); kill.once('close', resolve); kill.once('error', resolve); });
}
