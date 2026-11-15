function Get-GameSourceFingerprint([string]$NativeRoot = $PSScriptRoot) {
  $files = Get-ChildItem -LiteralPath $NativeRoot -File | Where-Object {
    $_.Extension -in @('.h', '.hpp', '.inl') -or
    $_.Name -in @('game_main.cpp', 'build.ps1', 'source_state.ps1')
  } | Sort-Object Name
  $entries = foreach ($file in $files) {
    $file.Name + ':' + (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash
  }
  $algorithm = [Security.Cryptography.SHA256]::Create()
  try {
    return [BitConverter]::ToString($algorithm.ComputeHash([Text.Encoding]::UTF8.GetBytes(($entries -join "`n")))).Replace('-', '')
  } finally { $algorithm.Dispose() }
}

function Test-GameBuildCurrent([string]$NativeRoot = $PSScriptRoot) {
  $program = Join-Path $NativeRoot 'build/rdzen-game.exe'
  $stamp = Join-Path $NativeRoot 'build/game-source.sha256'
  if (!(Test-Path -LiteralPath $program) -or !(Test-Path -LiteralPath $stamp) -or
      !(Test-Path -LiteralPath (Join-Path $NativeRoot 'build/SDL3.dll'))) { return $false }
  $lines = [IO.File]::ReadAllLines($stamp)
  return $lines.Length -eq 2 -and $lines[0] -eq (Get-GameSourceFingerprint $NativeRoot) -and
    $lines[1] -eq (Get-FileHash -LiteralPath $program -Algorithm SHA256).Hash
}

function Write-GameBuildState([string]$NativeRoot = $PSScriptRoot) {
  $program = Join-Path $NativeRoot 'build/rdzen-game.exe'
  [IO.File]::WriteAllLines((Join-Path $NativeRoot 'build/game-source.sha256'), [string[]]@(
    (Get-GameSourceFingerprint $NativeRoot),
    (Get-FileHash -LiteralPath $program -Algorithm SHA256).Hash
  ))
}
