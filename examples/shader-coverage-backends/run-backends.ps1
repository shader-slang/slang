# run-backends.ps1 — build (if needed) and run the shader-coverage-backends
# example on one or more backends, from this directory.
#
# Usage:
#   ./run-backends.ps1                   # run every backend: cpu cuda vulkan metal
#   ./run-backends.ps1 cpu cuda          # run a subset
#
# A backend that this machine or build cannot run (no CUDA toolkit, no
# Vulkan loader, not an Apple platform) fails its own run with a clear
# message from the example; the wrapper keeps going and reports a
# per-backend summary at the end.

param([string[]]$Backends = @())

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

if ($Backends.Count -eq 0)
{
    $Backends = @("cpu", "cuda", "vulkan", "metal")
}

# Find an existing binary in any CMake configuration, or build the
# release one. Rebuild manually after source changes:
#   cmake --build --preset release --target shader-coverage-backends
$root = Join-Path $PSScriptRoot "../.."
$bin = $null
foreach ($config in "Release", "RelWithDebInfo", "Debug")
{
    $candidate = Join-Path $root "build/examples/shader-coverage-backends/$config/shader-coverage-backends.exe"
    if (Test-Path $candidate) { $bin = $candidate; break }
}
if (-not $bin)
{
    Write-Host "binary not found; building shader-coverage-backends (release)..."
    Push-Location $root
    cmake --build --preset release --target shader-coverage-backends
    $buildOk = ($LASTEXITCODE -eq 0)
    Pop-Location
    if (-not $buildOk)
    {
        throw "build failed; configure first with: cmake --preset default"
    }
    $bin = Join-Path $root "build/examples/shader-coverage-backends/Release/shader-coverage-backends.exe"
}
Write-Host "using binary: $bin"

# Run each backend, collecting exit codes rather than stopping at the
# first failure.
$summary = @()
foreach ($b in $Backends)
{
    Write-Host ""
    Write-Host "=== --backend=$b ==="
    & $bin --backend=$b
    if ($LASTEXITCODE -eq 0) { $summary += "${b}: ok" }
    else { $summary += "${b}: exit $LASTEXITCODE" }
}

Write-Host ""
Write-Host "summary:"
$summary | ForEach-Object { Write-Host "  $_" }
