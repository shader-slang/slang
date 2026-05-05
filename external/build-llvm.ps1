# PowerShell script to fetch, build, and install LLVM for Slang

# Set script to fail on errors
$ErrorActionPreference = "Stop"

function Show-Help {
    Write-Host "Fetch, build and install LLVM for Slang

Options:
  --repo: The source git repo, default: $repo
  --branch: The branch (or tag) to fetch, default: $branch
  --source-dir: Unpack and build in this directory: default $source_dir
  --config: The configuration to build, default $config
  --install-prefix: Install under this prefix
  --targets: Semicolon-separated LLVM target architectures, default: $llvmTargets
  --: Any following arguments will be passed to the CMake configuration command
"
}

function Msg {
    Write-Host "$args" -ForegroundColor Yellow
}

function Fail {
    Msg "$args"
    exit 1
}

function New-TemporaryDirectory {
  $tmp = [System.IO.Path]::GetTempPath() # Not $env:TEMP, see https://stackoverflow.com/a/946017
  $name = (New-Guid).ToString("N")
  New-Item -ItemType Directory -Path (Join-Path $tmp $name)
}

# Check if required programs are available
$requiredPrograms = "cmake", "git", "ninja"
foreach ($prog in $requiredPrograms) {
    if (-not (Get-Command $prog -ErrorAction SilentlyContinue)) {
        Msg "This script needs $prog, but it isn't in PATH"
        $missingBin = $true
    }
}
if ($missingBin) {
    exit 1
}

# Temp directory with cleanup on exit
$tempDir = New-TemporaryDirectory
$cleanup = {
    if ($tempDir) {
        Remove-Item -Recurse -Force $tempDir
    }
    exit $lastExitCode
}
$null = Register-EngineEvent PowerShell.Exiting -Action $cleanup

# Default values
$repo = "https://github.com/llvm/llvm-project"
$branch = "llvmorg-21.1.2"
$sourceDir = $tempDir.FullName
$installPrefix = ""
$config = "Release"
$llvmTargets = "X86;ARM;AArch64"
$extraArguments = @()

# Argument parsing
for ($i = 0; $i -lt $args.Length; $i++) {
    switch ($args[$i]) {
        "--help" {
            Show-Help
            exit
        }
        "--repo" {
            $repo = $args[++$i]
        }
        "--branch" {
            $branch = $args[++$i]
        }
        "--source-dir" {
            $sourceDir = $args[++$i]
        }
        "--config" {
            $config = $args[++$i]
        }
        "--install-prefix" {
            $installPrefix = $args[++$i]
        }
        "--targets" {
            $llvmTargets = $args[++$i]
        }
        "--" {
            $extraArguments = $args[$i + 1..$args.Length]
            break
        }
        default {
            Msg "Unknown parameter passed: $($args[$i])"
            Show-Help
            exit 1
        }
    }
}

if (-not $repo) { Fail "please set --repo" }
if (-not $branch) { Fail "please set --branch" }
if (-not $sourceDir) { Fail "please set --source-dir" }
if (-not $config) { Fail "please set --config" }
if (-not $installPrefix) { Fail "please set --install-prefix" }

# Fetch LLVM from the repo
Msg "##########################################################"
Msg "# Fetching LLVM from $repo at $branch"
Msg "##########################################################"
git clone --depth 1 --branch $branch $repo $sourceDir

# Configure LLVM with CMake
Msg "##########################################################"
Msg "# Configuring LLVM in $sourceDir"
Msg "##########################################################"
$msvcRuntimeLib = "MultiThreaded"
if ($config -eq 'Debug')
{
    $msvcRuntimeLib = "MultiThreadedDebug"
}
$cmakeArgumentsForSlang = @(
    # Don't build unnecessary things
    "-DLLVM_BUILD_LLVM_C_DYLIB=0"
    "-DLLVM_INCLUDE_BENCHMARKS=0"
    "-DLLVM_INCLUDE_DOCS=0"
    "-DLLVM_INCLUDE_EXAMPLES=0"
    "-DLLVM_INCLUDE_TESTS=0"
    "-DLLVM_ENABLE_TERMINFO=0"
    "-DLLVM_ENABLE_DIA_SDK=0"
    "-DCLANG_BUILD_TOOLS=0"
    "-DCLANG_ENABLE_STATIC_ANALYZER=0"
    "-DCLANG_ENABLE_ARCMT=0"
    "-DCLANG_INCLUDE_DOCS=0"
    "-DCLANG_INCLUDE_TESTS=0"
    # Requirements for Slang
    "-DLLVM_ENABLE_PROJECTS=clang"
    "-DLLVM_TARGETS_TO_BUILD=$llvmTargets"
    "-DLLVM_BUILD_TOOLS=0"
    # Narrow the distribution to just the libraries/headers Slang links against.
    # Using install-distribution (below) with this list avoids `ninja all`, which
    # would otherwise compile the Clang Static Analyzer and other unused pieces
    # despite CLANG_ENABLE_STATIC_ANALYZER=0 (llvm/llvm-project#117705).
    "-DLLVM_DISTRIBUTION_COMPONENTS=clang-libraries;clang-headers;clang-cmake-exports;llvm-libraries;llvm-headers;cmake-exports"
    # Get LLVM to use the static linked version of the msvc runtime.
    # CMAKE_MSVC_RUNTIME_LIBRARY is resolved at configure time with the
    # single-config generator, so we set it from $msvcRuntimeLib directly
    # rather than via a $<CONFIG:Debug> generator expression.
    "-DCMAKE_MSVC_RUNTIME_LIBRARY=$msvcRuntimeLib"
    "-DLLVM_USE_CRT_RELEASE=MT"
    "-DLLVM_USE_CRT_DEBUG=MTd"
)

$buildDir = Join-Path $sourceDir "build"
New-Item -Path $buildDir -ItemType Directory -Force
$myScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$toolchainFile = Join-Path $myScriptDir "WindowsToolchain\Windows.MSVC.toolchain.cmake"
# Use the single-config Ninja generator rather than Ninja Multi-Config because
# LLVM_DISTRIBUTION_COMPONENTS (used above) is not compatible with
# multi-configuration generators.
cmake -S $sourceDir\llvm -B $buildDir `
    -G "Ninja" `
    "-DCMAKE_BUILD_TYPE=$config" `
    "-DCMAKE_INSTALL_PREFIX=$installPrefix" `
    --toolchain $toolchainFile `
    @cmakeArgumentsForSlang `
    @extraArguments

# Build and install LLVM
Msg "##########################################################"
Msg "# Building and installing LLVM into $installPrefix"
Msg "##########################################################"
# install-distribution builds and installs exactly the components listed in
# LLVM_DISTRIBUTION_COMPONENTS (set at configure time). This is LLVM's
# supported mechanism for producing a trimmed toolchain — see
# https://llvm.org/docs/BuildingADistribution.html. It avoids `ninja all`,
# which would otherwise compile the Clang Static Analyzer and other unused
# pieces despite CLANG_ENABLE_STATIC_ANALYZER=0
# (llvm/llvm-project#117705).
cmake --build $buildDir -j --target install-distribution

# Sanity-check that the install tree actually contains the per-target
# codegen libraries we asked for. Mirrors the equivalent check in
# build-llvm.sh: install-distribution only installs what the named
# components transitively pull in, so if upstream LLVM ever regroups
# components and drops LLVM<TARGET>CodeGen out of the libraries
# component, fail loudly here rather than at first use.
Msg "##########################################################"
Msg "# Verifying installed LLVM codegen libraries"
Msg "##########################################################"
$missingCodegen = @()
foreach ($target in ($llvmTargets -split ';')) {
    $hit = Get-ChildItem -Path $installPrefix -Recurse -File -ErrorAction SilentlyContinue `
        -Include "LLVM${target}CodeGen.lib", "libLLVM${target}CodeGen.*" |
        Select-Object -First 1
    if (-not $hit) {
        $missingCodegen += $target
    }
}
if ($missingCodegen.Count -gt 0) {
    Fail "LLVM install at $installPrefix is missing codegen libraries for: $($missingCodegen -join ', '). The LLVM_DISTRIBUTION_COMPONENTS list in this script likely needs updating."
}

Msg "##########################################################"
Msg "LLVM installed in $installPrefix"
Msg "Please add $installPrefix to CMAKE_PREFIX_PATH"
Msg "##########################################################"
