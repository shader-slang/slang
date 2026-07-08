# run-tutorial.ps1 — execute every step of the Shader Execution Coverage
# tutorial (docs/user-guide/a1-06-shader-coverage.md) in one go, from
# this directory. Each step below is named after the chapter section it
# comes from, so you can follow along in the text.
#
# Prerequisites: slangc.exe (any Slang release, or a repo build), a C++
# compiler (cl.exe from a Visual Studio developer prompt, or clang++/g++),
# and Python 3. genhtml (from the lcov package) is optional — the HTML
# step is skipped with a note when it is missing.
#
# Usage:
#   ./run-tutorial.ps1
#   ./run-tutorial.ps1 -Slangc C:/path/to/slangc.exe

param(
    [string]$Slangc = ""
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

# --- Step 0: find the tools -------------------------------------------------
# slangc is taken from -Slangc, then PATH, then a sibling repo build
# (convenient when running from a shader-slang/slang checkout).
if (-not $Slangc)
{
    if (Get-Command slangc -ErrorAction SilentlyContinue)
    {
        $Slangc = "slangc"
    }
    else
    {
        foreach ($candidate in "../../build/Release/bin/slangc.exe", "../../build/Debug/bin/slangc.exe")
        {
            if (Test-Path $candidate) { $Slangc = $candidate; break }
        }
    }
}
if (-not $Slangc)
{
    throw "slangc not found; put it on PATH or pass -Slangc"
}
Write-Host "using slangc: $Slangc"

# The kernel is a shared library; the host program loads
# hello-coverage-kernel.dll on Windows and .so elsewhere.
$kernel = if ($IsWindows -or $env:OS -eq "Windows_NT") { "hello-coverage-kernel.dll" }
else { "hello-coverage-kernel.so" }

# --- Step 1: "Your first coverage build" ------------------------------------
# One flag, -trace-coverage, turns on line coverage. Two files appear:
# the compiled shader and the .coverage-manifest.json sidecar that maps
# counter slots back to source locations.
Invoke-Step $Slangc @("hello-coverage.slang", "-target", "spirv",
    "-stage", "compute", "-entry", "computeMain",
    "-trace-coverage", "-o", "hello-coverage.spv")
Write-Host "wrote hello-coverage.spv and hello-coverage.spv.coverage-manifest.json"

# --- Step 2: "Reading the manifest" -----------------------------------------
# Pretty-print the sidecar. Note the buffer block: on SPIR-V the hidden
# counter buffer binds at a descriptor (set, binding).
python -m json.tool hello-coverage.spv.coverage-manifest.json | Select-Object -First 14

# --- Step 3: "Running for real: dispatching the precompiled kernel" ---------
# Compile the same shader once more, to a directly callable CPU shared
# library. slangc drives the system C++ compiler; the new sidecar
# reports uniform_offset instead of a descriptor location.
Invoke-Step $Slangc @("hello-coverage.slang", "-target", "shader-sharedlib",
    "-stage", "compute", "-entry", "computeMain",
    "-trace-coverage", "-o", $kernel)
Write-Host "wrote $kernel and its sidecar manifest"

# Build the host program — an ordinary C++ compile with no Slang SDK
# paths — preferring cl.exe (Visual Studio developer prompt), then
# clang++ or g++.
if (Get-Command cl -ErrorAction SilentlyContinue)
{
    Invoke-Step "cl" @("/nologo", "/std:c++17", "/EHsc",
        "hello-coverage-host.cpp", "/Fe:hello-coverage-host.exe")
}
elseif (Get-Command clang++ -ErrorAction SilentlyContinue)
{
    Invoke-Step "clang++" @("-std=c++17", "hello-coverage-host.cpp", "-o", "hello-coverage-host.exe")
}
elseif (Get-Command g++ -ErrorAction SilentlyContinue)
{
    Invoke-Step "g++" @("-std=c++17", "hello-coverage-host.cpp", "-o", "hello-coverage-host.exe")
}
else
{
    throw "no C++ compiler found (cl, clang++, or g++); run from a Visual Studio developer prompt"
}

# Dispatch. The host loads the precompiled kernel, binds the coverage
# buffer at the manifest-reported uniform_offset, runs one thread
# group, prints the raw counter slots, and writes
# hello-coverage.counters.bin.
Invoke-Step "./hello-coverage-host.exe"

# --- Step 4: "From counters to a report" ------------------------------------
# The LCOV converter joins the raw counters with the manifest's source
# attribution. Expect two zero-count lines: the negative-input clamp
# and the applyGain fallthrough, which these inputs never reach.
Invoke-Step "python" @("../../tools/shader-coverage/slang-coverage-to-lcov.py",
    "--manifest", "$kernel.coverage-manifest.json",
    "--counters", "hello-coverage.counters.bin",
    "--output", "hello-coverage.lcov")
Get-Content hello-coverage.lcov

# Render HTML when genhtml (lcov package) is installed.
if (Get-Command genhtml -ErrorAction SilentlyContinue)
{
    Invoke-Step "genhtml" @("hello-coverage.lcov", "--output-directory", "coverage-html")
    Write-Host "open coverage-html/index.html to see the annotated source"
}
else
{
    Write-Host "genhtml not found - skipping HTML report (install the lcov package to enable it)"
}
