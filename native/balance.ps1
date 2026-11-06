$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$compiler = Join-Path $root '.toolchain/llvm-mingw-20261020-ucrt-x86_64/bin/clang++.exe'
& $compiler '-std=c++20' '-O3' '-DNDEBUG' '-Wall' '-Wextra' '-static' '-I' $PSScriptRoot (Join-Path $PSScriptRoot 'frontier_soak.cpp') '-o' (Join-Path $PSScriptRoot 'build/frontier-soak.exe')
if ($LASTEXITCODE -ne 0) { throw 'Balance runner compilation failed.' }
Push-Location $root
try {
  & (Join-Path $PSScriptRoot 'build/frontier-soak.exe')
  if ($LASTEXITCODE -ne 0) { throw 'Reference defence did not survive the pacing test.' }
} finally { Pop-Location }
