$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$build = Join-Path $root "build-windows"

cmake -S $root -B $build -A x64
cmake --build $build --config Release --parallel

$source = Join-Path $build "Release\keyhunt.exe"
$destination = Join-Path $root "keyhunt.exe"
Copy-Item -LiteralPath $source -Destination $destination -Force

Write-Host "Built: $destination"
