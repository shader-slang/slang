# run-tutorial.ps1 — execute every step of the Shader Execution Coverage
# tutorial (docs/user-guide/a1-06-shader-coverage.md) in one go, from
# this directory. Each step below is named after the chapter section it
# comes from, so you can follow along in the text.
#
# Prerequisites: slangc.exe (any Slang release, or a repo build), a C++
# compiler (a Visual Studio installation is found automatically via
# vswhere; clang++/g++ on PATH also work), and Python 3. genhtml (from
# the lcov package) is optional — without it the HTML report is
# rendered with the in-repo Python renderer.
#
# Usage:
#   ./run-tutorial.ps1
#   ./run-tutorial.ps1 -Slangc C:/path/to/slangc.exe
#   ./run-tutorial.ps1 -OpenReport   # also open the HTML report in the browser

param(
    [string]$Slangc = "",
    [switch]$OpenReport
)

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

# Stop at the first failing external command (PowerShell does not do
# this by default the way `set -e` does in bash).
function Invoke-Step
{
    param([string]$Exe, [string[]]$Arguments = @())
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0)
    {
        throw "command failed: $Exe $($Arguments -join ' ')"
    }
}

# --- Step 0: find the tools ---------------------------------------------------
# slangc is taken from -Slangc, then PATH, then a sibling repo build
# (convenient when running from a shader-slang/slang checkout). An old
# slangc without coverage support is skipped: releases that predate the
# feature reject -trace-coverage.
function Test-CoverageSupport
{
    param([string]$Exe)
    try { return ((& $Exe -h 2>&1 | Out-String) -match "trace-coverage") }
    catch { return $false }
}

if ($Slangc)
{
    if (-not (Get-Command $Slangc -ErrorAction SilentlyContinue))
    {
        throw "slangc not found at '$Slangc'"
    }
    if (-not (Test-CoverageSupport $Slangc))
    {
        throw "$Slangc does not support -trace-coverage; use a newer Slang release"
    }
}
else
{
    foreach ($candidate in "slangc", "../../build/Release/bin/slangc.exe", "../../build/Debug/bin/slangc.exe")
    {
        if (-not (Get-Command $candidate -ErrorAction SilentlyContinue)) { continue }
        if (Test-CoverageSupport $candidate) { $Slangc = $candidate; break }
    }
}
if (-not $Slangc)
{
    throw "no slangc with -trace-coverage support found on PATH or in ../../build; install a recent Slang release or pass -Slangc"
}
Write-Host "using slangc: $Slangc"

# The kernel is a shared library; the host program loads
# hello-coverage-kernel.dll on Windows and .so elsewhere. Non-Windows
# hosts use dlopen, which needs -ldl on older glibc.
$isWindowsHost = $IsWindows -or $env:OS -eq "Windows_NT"
$kernel = if ($isWindowsHost) { "hello-coverage-kernel.dll" } else { "hello-coverage-kernel.so" }
$dlLib = if ($isWindowsHost) { @() } else { @("-ldl") }

# --- Step 1: "Compiling with coverage" ----------------------------------------
# One flag, -trace-coverage, turns on line coverage. Two files appear:
# the compiled shader and the .coverage-manifest.json sidecar that maps
# counter slots back to source locations.
Invoke-Step $Slangc @("hello-coverage.slang", "-target", "spirv",
    "-stage", "compute", "-entry", "computeMain",
    "-trace-coverage", "-o", "hello-coverage.spv")
Write-Host "wrote hello-coverage.spv and hello-coverage.spv.coverage-manifest.json"

# --- Step 2: "Manifest structure" ---------------------------------------------
# Show the sidecar. Note the buffer block: on SPIR-V the hidden
# counter buffer binds at a descriptor (set, binding).
Get-Content hello-coverage.spv.coverage-manifest.json

# --- Step 3: "Dispatching the precompiled kernel" -----------------------------
# Compile the same shader once more, to a directly callable CPU shared
# library. slangc drives the system C++ compiler; the new sidecar
# reports uniform_offset instead of a descriptor location.
Invoke-Step $Slangc @("hello-coverage.slang", "-target", "shader-sharedlib",
    "-stage", "compute", "-entry", "computeMain",
    "-trace-coverage", "-o", $kernel)
Write-Host "wrote $kernel and its sidecar manifest"

# Build the host program — an ordinary C++ compile with no Slang SDK
# paths — preferring cl.exe, then clang++ or g++. When no compiler is
# on PATH, locate Visual Studio with vswhere and enter its developer
# shell, the same discovery slangc's shader-sharedlib compile just did;
# running from a developer prompt is then unnecessary.
if ($isWindowsHost -and
    -not (Get-Command cl -ErrorAction SilentlyContinue) -and
    -not (Get-Command clang++ -ErrorAction SilentlyContinue) -and
    -not (Get-Command g++ -ErrorAction SilentlyContinue))
{
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere)
    {
        $vsRoot = & $vswhere -latest -products * `
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
            -property installationPath
        if ($vsRoot)
        {
            # The dev-shell init scripts invoke vswhere.exe by bare name.
            $env:Path = "$(Split-Path $vswhere);$env:Path"
            Import-Module (Join-Path $vsRoot "Common7\Tools\Microsoft.VisualStudio.DevShell.dll")
            Enter-VsDevShell -VsInstallPath $vsRoot -SkipAutomaticLocation `
                -DevCmdArguments "-arch=x64" *> $null
        }
    }
}

if (Get-Command cl -ErrorAction SilentlyContinue)
{
    Invoke-Step "cl" @("/nologo", "/std:c++17", "/EHsc",
        "hello-coverage-host.cpp", "/Fe:hello-coverage-host.exe")
}
elseif (Get-Command clang++ -ErrorAction SilentlyContinue)
{
    Invoke-Step "clang++" (@("-std=c++17", "hello-coverage-host.cpp", "-o", "hello-coverage-host.exe") + $dlLib)
}
elseif (Get-Command g++ -ErrorAction SilentlyContinue)
{
    Invoke-Step "g++" (@("-std=c++17", "hello-coverage-host.cpp", "-o", "hello-coverage-host.exe") + $dlLib)
}
else
{
    throw "no C++ compiler found (cl, clang++, or g++) and no Visual Studio installation located"
}

# Dispatch. The host loads the precompiled kernel, binds the coverage
# buffer at the manifest-reported uniform_offset, runs one thread
# group, prints the outputs and the raw counter slots, and writes
# hello-coverage.counters.bin.
Invoke-Step "./hello-coverage-host.exe"

# --- Step 4: "Generating a report" --------------------------------------------
# The LCOV converter joins the raw counters with the manifest's source
# attribution. Expect varied counts: applyGain's branches split 3/1
# across the four threads, and the negative-input clamp never runs.
Invoke-Step "python" @("../../tools/shader-coverage/slang-coverage-to-lcov.py",
    "--manifest", "$kernel.coverage-manifest.json",
    "--counters", "hello-coverage.counters.bin",
    "--output", "hello-coverage.lcov")
Get-Content hello-coverage.lcov

# Guard the numbers the chapter publishes: the LCOV records combine the
# manifest's source attribution with the counter values, so this one
# comparison catches instrumentation, attribution, or converter drift.
# expected.lcov is the single checked-in copy both runner scripts use;
# the comparison is order-exact, matching the bash runner's diff.
$expectedText = ((Get-Content expected.lcov) -join "`n").TrimEnd()
$actualText = ((Get-Content hello-coverage.lcov) -join "`n").TrimEnd()
if ($expectedText -ne $actualText)
{
    throw "hello-coverage.lcov does not match expected.lcov"
}
Write-Host "LCOV records match the tutorial published values"

# Render HTML with genhtml (the de-facto LCOV tool) when installed;
# otherwise fall back to the repository's own Python renderer.
if (Get-Command genhtml -ErrorAction SilentlyContinue)
{
    Invoke-Step "genhtml" @("hello-coverage.lcov", "--output-directory", "coverage-html")
}
else
{
    Invoke-Step "python" @("../../tools/coverage-html/slang-coverage-html.py",
        "hello-coverage.lcov", "--output-dir", "coverage-html")
}
if ($OpenReport)
{
    Invoke-Item (Join-Path "coverage-html" "index.html")
}
else
{
    Write-Host "open coverage-html/index.html to see the annotated source (or rerun with -OpenReport)"
}
