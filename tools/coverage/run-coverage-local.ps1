<#
.SYNOPSIS
  Wrapper for running Slang coverage locally on Windows.

.DESCRIPTION
  Configures, builds (RelWithDebInfo), runs slang-test under OpenCppCoverage,
  and emits LCOV + HTML reports. Counterpart to tools/coverage/run-coverage-local.sh.

  Requires OpenCppCoverage. Recommended: download the portable zip from
  https://github.com/OpenCppCoverage/OpenCppCoverage/releases and either add
  the folder to PATH or set $env:OpenCppCoverage to the full exe path.
  Alternative (admin): winget install --id OpenCppCoverage.OpenCppCoverage

.PARAMETER SkipBuild
  Skip the CMake configure + build steps.

.PARAMETER SkipTest
  Skip running tests (use existing coverage data).

.PARAMETER SkipReport
  Skip generating coverage reports.

.PARAMETER ServerCount
  Number of parallel slang-test servers (default: 8).

.PARAMETER WithSynthesis
  Also run the -only-synthesized pass.
#>

[CmdletBinding()]
param(
    [switch]$SkipBuild,
    [switch]$SkipTest,
    [switch]$SkipReport,
    [int]$ServerCount = 8,
    [switch]$WithSynthesis
)

$ErrorActionPreference = 'Stop'

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot  = (Resolve-Path (Join-Path $ScriptDir '..\..')).Path
Set-Location $RepoRoot

Write-Host "========================================" -ForegroundColor Blue
Write-Host "Slang Coverage Report - Local Run (Windows)" -ForegroundColor Blue
Write-Host "========================================" -ForegroundColor Blue

if (-not $SkipBuild) {
    Write-Host "`nStep 1-2: Configure + build slang-test (RelWithDebInfo)..." -ForegroundColor Green
    Write-Host "Note: this can take 10-20 minutes depending on your machine." -ForegroundColor Yellow
    # Use the sandbox build helper -- it handles vcvarsall, local dep caching,
    # and the vs2022-dev preset. Set SLANG_ENABLE_COVERAGE=ON so
    # cmake/CompilerFlags.cmake emits coverage-friendly flags on MSVC (/Ob0,
    # keeping every function body distinct in the PDB). OpenCppCoverage itself
    # needs no compile-time instrumentation; this just tunes the build for
    # accurate PDB line mappings.
    $env:SLANG_ENABLE_COVERAGE = 'ON'
    try {
        & cmd.exe /c "$(Join-Path $RepoRoot 'extras\win-sandbox-build.bat')" releaseWithDebugInfo x64 slang-test
        if ($LASTEXITCODE -ne 0) { throw "win-sandbox-build.bat failed ($LASTEXITCODE)" }
    } finally {
        Remove-Item Env:SLANG_ENABLE_COVERAGE -ErrorAction SilentlyContinue
    }
}

if (-not $SkipTest) {
    Write-Host "`nStep 3: Running tests with OpenCppCoverage..." -ForegroundColor Green

    $testArgs = @(
        '-expected-failure-list', 'tests/expected-failure-github.txt'
        '-expected-failure-list', 'tests/expected-failure-no-gpu.txt'
        '-skip-reference-image-generation'
        '-show-adapter-info'
        '-enable-debug-layers', 'true'
    )
    if ($ServerCount -gt 1) {
        $testArgs += @('-use-test-server', '-server-count', "$ServerCount")
    }

    $env:CONFIG = 'RelWithDebInfo'
    $env:COVERAGE_LCOV = '1'
    $env:COVERAGE_HTML = '1'
    $occScript = Join-Path $ScriptDir 'run-coverage-windows.ps1'
    $occArgs = @()
    if ($WithSynthesis) { $occArgs += '-WithSynthesis' }
    $occArgs += $testArgs
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $occScript @occArgs
    if ($LASTEXITCODE -ne 0) { throw "coverage run failed" }
}

if (-not $SkipReport) {
    # run-coverage-windows.ps1 already exports both LCOV and HTML when no
    # COVERAGE_* env vars are set. In report-only mode we re-export from the
    # cached .cov.
    if ($SkipTest) {
        Write-Host "`nStep 4: Regenerating reports from existing coverage data..." -ForegroundColor Green
        $env:CONFIG = 'RelWithDebInfo'
        $env:COVERAGE_LCOV = '1'
        $env:COVERAGE_HTML = '1'
        & powershell.exe -NoProfile -ExecutionPolicy Bypass `
            -File (Join-Path $ScriptDir 'run-coverage-windows.ps1') -ReportOnly
        if ($LASTEXITCODE -ne 0) { throw "report generation failed" }
    }
}

$htmlIndex = Join-Path $RepoRoot 'coverage-html\index.html'
Write-Host "`n========================================" -ForegroundColor Blue
Write-Host "Coverage run completed." -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Blue
Write-Host "  HTML (full):    $htmlIndex"
Write-Host "  HTML (slangc):  $(Join-Path $RepoRoot 'coverage-html-slangc\index.html')"
Write-Host "  LCOV (full):    $(Join-Path $RepoRoot 'coverage.lcov')"

if (Test-Path $htmlIndex) {
    Start-Process $htmlIndex
}
