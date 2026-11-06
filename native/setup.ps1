$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$toolsRoot = Join-Path $projectRoot '.toolchain'
New-Item -ItemType Directory -Force $toolsRoot | Out-Null
$packages = @(
  @{ File='llvm-mingw-20261020-ucrt-x86_64.zip'; Url='https://github.com/mstorsjo/llvm-mingw/releases/download/20261020/llvm-mingw-20261020-ucrt-x86_64.zip'; Hash='fd5462b7f5d93b2ff981fcd3f9c7e427e47f3203c64ea91c27bcc444eebd74cd'; Folder='llvm-mingw-20261020-ucrt-x86_64' },
  @{ File='SDL3-devel-3.4.18-mingw.zip'; Url='https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/SDL3-devel-3.4.18-mingw.zip'; Hash='cd98158c8d025a816f430a600b4e9526524e6219ee884ec72bd498b80fd3bf58'; Folder='SDL3-3.4.18' }
)
foreach ($package in $packages) {
  $archive = Join-Path $toolsRoot $package.File
  $destination = Join-Path $toolsRoot $package.Folder
  if (Test-Path -LiteralPath (Join-Path $destination '.verified')) { continue }
  if (!(Test-Path -LiteralPath $archive)) {
    Write-Output "Downloading $($package.File) from its official GitHub release..."
    & curl.exe --fail --location --retry 2 --output $archive $package.Url
    if ($LASTEXITCODE -ne 0) { throw 'Download failed.' }
  }
  if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant() -ne $package.Hash) { throw "SHA256 mismatch: $archive" }
  Write-Output "Verified SHA256. Extracting $($package.File)..."
  Expand-Archive -LiteralPath $archive -DestinationPath $toolsRoot -Force
  Set-Content -LiteralPath (Join-Path $destination '.verified') -Value $package.Hash
}
Write-Output 'Local C++ toolchain ready. No system PATH changes.'
