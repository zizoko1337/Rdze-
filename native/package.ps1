param([switch]$Rebuild)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
if ($Rebuild -or !(Test-Path -LiteralPath (Join-Path $PSScriptRoot 'build/rdzen-game.exe'))) {
  & (Join-Path $PSScriptRoot 'build.ps1') -GameOnly
}
$compiler = Join-Path $root '.toolchain/llvm-mingw-20261020-ucrt-x86_64/bin/clang++.exe'
$stage = Join-Path $root 'artifacts/package-work'
New-Item -ItemType Directory -Force -Path $stage | Out-Null
$instructions = @'
RDZEN - prototyp na Windows x64

Uruchom Rdzen.exe dwuklikiem. Mozesz przenosic sam plik EXE.
Gra zawiera grafiki i biblioteke SDL. Nie wymaga instalowania silnika,
Node.js ani przegladarki. Wymagany Windows 10/11 i Direct3D 11.

Pierwszy start zapisuje dane w %LOCALAPPDATA%\Rdzen\Prototyp.
Kolejne uruchomienia korzystaja z zapisanych plikow.

Sterowanie:
Po wejsciu na platforme uruchom wydobycie czerwonym przyciskiem za dzialem [E].
Lewy komputer: ulepszenia. Prawy: jednorazowe uslugi za punkty z niebieskich kart.
1-9 wybor jednego z 9 dzial manualnych, Enter start; WASD chodzenie; mysz rozgladanie.
E fotel / komputer / wyjscie z dziala. LPM strzelanie.
; laboratorium ulepszen: wyszukiwanie, Tab rzadkosc, Enter dodaj, Esc zamknij.
' bestiariusz: grafiki, statystyki, klikniecie przywoluje; x1/x10/x100. Czas stoi.
Tab widok z gory. LPM heks: dostawa dziala; LPM krysztal: wyslij drona.
C dron po najblizszy krysztal. P pauza. F1 pomoc. F11 pelny ekran.
R nowa ekspedycja. Alt+F4 wyjscie.
Radio wyjezdza po schowaniu przycisku. E przy radiu: utwory, pauza i glosnosc.

Uran daje wybor ulepszenia przy bocznym komputerze. Boss co 3 minuty.
Rdzen regeneruje 3 HP/s. Nanoboty regenerujace zwiekszaja regeneracje.
Pompa naprzeciw radia pokazuje wydobycie i stabilnosc (procent HP rdzenia).
Fabryka jezdzikow odblokowywana jest karta ulepszenia.
'@
$utf8 = New-Object System.Text.UTF8Encoding($false)
[IO.File]::WriteAllText((Join-Path $stage 'INSTRUKCJA.txt'), $instructions, $utf8)
$files = @(
  @{ Name='rdzen-game.exe'; Source=(Join-Path $PSScriptRoot 'build/rdzen-game.exe') },
  @{ Name='SDL3.dll'; Source=(Join-Path $PSScriptRoot 'build/SDL3.dll') },
  @{ Name='SDL-LICENSE.txt'; Source=(Join-Path $root '.toolchain/SDL3-3.4.18/LICENSE.txt') },
  @{ Name='INSTRUKCJA.txt'; Source=(Join-Path $stage 'INSTRUKCJA.txt') }
)
foreach ($asset in (Get-ChildItem -LiteralPath (Join-Path $root 'public/assets') -File | Sort-Object Name)) {
  $files += @{ Name=('public/assets/' + $asset.Name); Source=$asset.FullName }
}
foreach ($track in (Get-ChildItem -LiteralPath (Join-Path $root 'public/music') -Filter '*.mp3' -File | Sort-Object Name)) {
  $files += @{ Name=('public/music/' + $track.Name); Source=$track.FullName }
}
$stream = [IO.File]::Create((Join-Path $stage 'payload.bin'))
$writer = New-Object IO.BinaryWriter($stream)
$manifest = @()
try {
  $writer.Write([Text.Encoding]::ASCII.GetBytes('RDZENPK1'))
  $writer.Write([uint32]$files.Count)
  foreach ($file in $files) {
    $name = $utf8.GetBytes($file.Name)
    $bytes = [IO.File]::ReadAllBytes($file.Source)
    $writer.Write([uint32]$name.Length)
    $writer.Write([uint64]$bytes.LongLength)
    $writer.Write($name)
    $writer.Write($bytes)
    $manifest += @{ name=$file.Name; size=$bytes.LongLength; sha256=(Get-FileHash -LiteralPath $file.Source -Algorithm SHA256).Hash }
  }
} finally { $writer.Dispose() }
$bundleId = (Get-FileHash -LiteralPath (Join-Path $stage 'payload.bin') -Algorithm SHA256).Hash.Substring(0,16).ToLowerInvariant()
[IO.File]::WriteAllText((Join-Path $stage 'bundle_id.hpp'), ('inline constexpr wchar_t RDZEN_BUNDLE_ID[] = L"' + $bundleId + '";'), $utf8)
$assembly = @'
.section .rdata,"dr"
.p2align 4
.global rdzen_bundle_start
rdzen_bundle_start:
.incbin "payload.bin"
.global rdzen_bundle_end
rdzen_bundle_end:
'@
[IO.File]::WriteAllText((Join-Path $stage 'payload.S'), $assembly, $utf8)
$output = Join-Path $root 'Rdzen.exe'
Push-Location $stage
try {
  & $compiler '-std=c++20' '-O2' '-DNDEBUG' '-Wall' '-Wextra' '-static' '-municode' '-mwindows' '-I' $stage (Join-Path $PSScriptRoot 'portable_launcher.cpp') 'payload.S' '-lshell32' '-luser32' '-Wl,--strip-all' '-o' $output
  if ($LASTEXITCODE -ne 0) { throw 'Portable executable compilation failed.' }
} finally { Pop-Location }
Copy-Item -LiteralPath (Join-Path $stage 'INSTRUKCJA.txt') -Destination (Join-Path $root 'INSTRUKCJA_EXE.txt') -Force
$zip = Join-Path $root 'Rdzen-prototyp.zip'
for ($taskAttempt = 0; $taskAttempt -lt 5; $taskAttempt++) {
  try {
    # Antivirus scanning can briefly hold the freshly linked executable open.
    Compress-Archive -LiteralPath $output,(Join-Path $root 'INSTRUKCJA_EXE.txt'),(Join-Path $root 'Uruchom-Linux.sh'),(Join-Path $root 'LINUX.txt') -DestinationPath $zip -Force -ErrorAction Stop
    break
  } catch {
    if ($taskAttempt -eq 4) { throw }
    Start-Sleep -Seconds 1
  }
}
@{ bundle_id=$bundleId; exe_sha256=(Get-FileHash -LiteralPath $output -Algorithm SHA256).Hash; files=$manifest } |
  ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $root 'artifacts/package-manifest.json') -Encoding UTF8
Write-Output "READY: $output"
Write-Output "ZIP: $zip"
Write-Output "BUNDLE: $bundleId"
