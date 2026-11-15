param([Parameter(ValueFromRemainingArguments = $true)][string[]]$GameArguments)
$ErrorActionPreference = 'Stop'
try {
  Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
  . (Join-Path $PSScriptRoot 'source_state.ps1')
  if (!(Test-GameBuildCurrent $PSScriptRoot)) {
    Write-Host 'Kod gry zmienil sie lub brakuje aktualnej kompilacji. Przebudowywanie...'
    & (Join-Path $PSScriptRoot 'build.ps1') -GameOnly
    if ($LASTEXITCODE -ne 0) { throw 'Nie udalo sie przebudowac gry.' }
    if (!(Test-GameBuildCurrent $PSScriptRoot)) { throw 'Kompilacja nie odpowiada aktualnym zrodlom.' }
  }
  & (Join-Path $PSScriptRoot 'build/rdzen-game.exe') @GameArguments
  exit $LASTEXITCODE
} catch {
  Write-Host $_.Exception.Message -ForegroundColor Red
  exit 1
}
