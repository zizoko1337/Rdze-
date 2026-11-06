import http from 'node:http';
import { readFile, stat } from 'node:fs/promises';
import { resolve, dirname, extname, sep } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawn } from 'node:child_process';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '../dist');
const noOpen = process.argv.includes('--no-open') || process.env.RDZEN_NO_OPEN === '1';
const types = { '.html': 'text/html; charset=utf-8', '.js': 'text/javascript; charset=utf-8', '.css': 'text/css; charset=utf-8', '.png': 'image/png', '.svg': 'image/svg+xml', '.woff2': 'font/woff2', '.json': 'application/json' };
try { await stat(resolve(root, 'index.html')); } catch { console.error('Brak kompilacji gry. Uruchom START.bat lub npm run build.'); process.exit(1); }
const server = http.createServer(async (req, res) => {
  if (req.method !== 'GET' && req.method !== 'HEAD') { res.writeHead(405).end(); return; }
  if (req.url === '/__rdzen_health') { res.writeHead(200, { 'Content-Type': 'application/json' }).end('{"game":"rdzen","version":"0.1.0"}'); return; }
  try {
    const path = decodeURIComponent(new URL(req.url, 'http://localhost').pathname);
    const file = resolve(root, '.' + (path === '/' ? '/index.html' : path));
    if (!file.startsWith(root + sep)) { res.writeHead(403).end(); return; }
    const data = await readFile(file);
    res.writeHead(200, { 'Content-Type': types[extname(file)] ?? 'application/octet-stream', 'Cache-Control': 'no-cache', 'X-Content-Type-Options': 'nosniff' });
    res.end(req.method === 'HEAD' ? undefined : data);
  } catch { res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' }).end('Nie znaleziono pliku.'); }
});
let port = 4317;
for (; port < 4330; port++) {
  try {
    await new Promise((ok, fail) => { server.once('error', fail); server.listen(port, '127.0.0.1', () => { server.removeListener('error', fail); ok(); }); });
    break;
  } catch (error) { if (error.code !== 'EADDRINUSE') throw error; }
}
if (!server.listening) { console.error('Porty 4317–4329 są zajęte. Zamknij wcześniejsze okna gry.'); process.exit(1); }
const url = `http://127.0.0.1:${port}`;
console.log(`\n  RDZEŃ — Ostatni przyczółek\n\n  Gra działa pod adresem: ${url}\n  Pozostaw to okno otwarte podczas gry.\n  Zamknięcie okna lub Ctrl+C zatrzymuje serwer.\n`);
if (!noOpen) {
  const child = process.platform === 'win32'
    ? spawn('powershell.exe', ['-NoProfile', '-NonInteractive', '-Command', `Start-Process '${url}'`], { windowsHide: true, stdio: 'ignore' })
    : spawn(process.platform === 'darwin' ? 'open' : 'xdg-open', [url], { stdio: 'ignore' });
  child.on('error', () => console.log(`Otwórz przeglądarkę pod adresem ${url}`));
}
process.on('SIGINT', () => { server.close(); process.exit(0); });
