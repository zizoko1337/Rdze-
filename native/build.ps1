param([switch]$TestsOnly, [switch]$GameOnly)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$compiler = Join-Path $root '.toolchain/llvm-mingw-20261020-ucrt-x86_64/bin/clang++.exe'
$sdl = Join-Path $root '.toolchain/SDL3-3.4.18/x86_64-w64-mingw32'
if (!(Test-Path -LiteralPath $compiler)) { & (Join-Path $PSScriptRoot 'setup.ps1') }
$out = Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Force $out | Out-Null
$common = @('-std=c++20','-O3','-DNDEBUG','-Wall','-Wextra','-static','-I', $PSScriptRoot)
if (!$GameOnly) {
  & $compiler @common (Join-Path $PSScriptRoot 'tests.cpp') '-o' (Join-Path $out 'simulation-tests.exe')
  if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed.' }
  & (Join-Path $out 'simulation-tests.exe')
  if ($LASTEXITCODE -ne 0) { throw 'Simulation tests failed.' }
}
& $compiler @common (Join-Path $PSScriptRoot 'game_tests.cpp') '-o' (Join-Path $out 'arsenal-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Arsenal test compilation failed.' }
& (Join-Path $out 'arsenal-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Arsenal tests failed.' }
& $compiler @common (Join-Path $PSScriptRoot 'command_tests.cpp') '-o' (Join-Path $out 'command-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Command test compilation failed.' }
& (Join-Path $out 'command-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Command tests failed.' }
& $compiler @common (Join-Path $PSScriptRoot 'manual_tests.cpp') '-o' (Join-Path $out 'manual-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Manual arsenal test compilation failed.' }
& (Join-Path $out 'manual-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Manual arsenal tests failed.' }
& $compiler @common (Join-Path $PSScriptRoot 'manual_revision_tests.cpp') '-o' (Join-Path $out 'manual-revision-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Manual revision compilation failed.' }
& (Join-Path $out 'manual-revision-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Manual revision tests failed.' }
& $compiler @common (Join-Path $PSScriptRoot 'manual_legends_tests.cpp') '-o' (Join-Path $out 'manual-legends-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Manual legends compilation failed.' }
& (Join-Path $out 'manual-legends-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Manual legends tests failed.' }
& $compiler @common '-I' (Join-Path $sdl 'include') (Join-Path $PSScriptRoot 'plasma_tests.cpp') '-o' (Join-Path $out 'plasma-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Plasma / upgrade lab test compilation failed.' }
& (Join-Path $out 'plasma-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Plasma / upgrade lab tests failed.' }
& $compiler @common '-I' (Join-Path $sdl 'include') (Join-Path $PSScriptRoot 'cooling_tests.cpp') '-o' (Join-Path $out 'cooling-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Cooling test compilation failed.' }
& (Join-Path $out 'cooling-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Cooling tests failed.' }
& $compiler @common '-I' (Join-Path $sdl 'include') (Join-Path $PSScriptRoot 'enemy_tests.cpp') '-o' (Join-Path $out 'enemy-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Enemy test compilation failed.' }
& (Join-Path $out 'enemy-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Enemy tests failed.' }
& $compiler @common '-I' (Join-Path $sdl 'include') (Join-Path $PSScriptRoot 'core_tests.cpp') '-o' (Join-Path $out 'core-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Core test compilation failed.' }
& (Join-Path $out 'core-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Core tests failed.' }
& $compiler @common '-I' (Join-Path $sdl 'include') (Join-Path $PSScriptRoot 'base_revision_tests.cpp') '-o' (Join-Path $out 'base-revision-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Base revision test compilation failed.' }
& (Join-Path $out 'base-revision-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Base revision tests failed.' }
& $compiler @common (Join-Path $PSScriptRoot 'expedition_tests.cpp') '-o' (Join-Path $out 'expedition-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Expedition test compilation failed.' }
& (Join-Path $out 'expedition-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Expedition tests failed.' }
& $compiler @common (Join-Path $PSScriptRoot 'progression_tests.cpp') '-o' (Join-Path $out 'progression-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Progression test compilation failed.' }
& (Join-Path $out 'progression-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Progression tests failed.' }
& $compiler @common (Join-Path $PSScriptRoot 'support_tests.cpp') '-o' (Join-Path $out 'support-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Support test compilation failed.' }
& (Join-Path $out 'support-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Support tests failed.' }
& $compiler @common (Join-Path $PSScriptRoot 'frontier_tests.cpp') '-o' (Join-Path $out 'frontier-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Frontier test compilation failed.' }
& (Join-Path $out 'frontier-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Frontier tests failed.' }
& $compiler @common (Join-Path $PSScriptRoot 'services_tests.cpp') '-o' (Join-Path $out 'services-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Services test compilation failed.' }
& (Join-Path $out 'services-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Services tests failed.' }
Copy-Item -LiteralPath (Join-Path $sdl 'bin/SDL3.dll') -Destination (Join-Path $out 'SDL3.dll') -Force
& $compiler @common '-I' (Join-Path $sdl 'include') (Join-Path $PSScriptRoot 'music_tests.cpp') '-L' (Join-Path $sdl 'lib') '-lSDL3.dll' '-lole32' '-lmfplat' '-lmfreadwrite' '-lmfuuid' '-o' (Join-Path $out 'music-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Radio test compilation failed.' }
& (Join-Path $out 'music-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Radio tests failed.' }
if (!$TestsOnly) {
  $programs = @(@{ Source='game_main.cpp'; Output='rdzen-game.exe' }, @{ Source='render_tests.cpp'; Output='render-tests.exe' })
  if (!$GameOnly) { $programs += @{ Source='main.cpp'; Output='rdzen-native.exe' } }
  foreach ($program in $programs) {
    & $compiler @common '-I' (Join-Path $sdl 'include') (Join-Path $PSScriptRoot $program.Source) '-L' (Join-Path $sdl 'lib') '-lSDL3.dll' '-ld3d11' '-ldxgi' '-ld3dcompiler' '-lole32' '-lwindowscodecs' '-luuid' '-lpsapi' '-lgdi32' '-lmfplat' '-lmfreadwrite' '-lmfuuid' '-o' (Join-Path $out $program.Output)
    if ($LASTEXITCODE -ne 0) { throw "Native compilation failed: $($program.Source)" }
  }
  Copy-Item -LiteralPath (Join-Path $sdl 'bin/SDL3.dll') -Destination (Join-Path $out 'SDL3.dll') -Force
}
