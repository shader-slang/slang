<#
.SYNOPSIS
  Run tests with OpenCppCoverage instrumentation on Windows and export reports.

.DESCRIPTION
  Windows counterpart to tools/coverage/run-coverage.sh. Wraps slang-test with
  OpenCppCoverage, accumulates coverage across the main test pass, record-replay
  pass, and optional synthesis pass, then exports LCOV + HTML reports (full
  library and slangc compiler-only variants).

  OpenCppCoverage is required. Recommended local install (no admin):
    1. Download OpenCppCoverage-x64-*.zip from
       https://github.com/OpenCppCoverage/OpenCppCoverage/releases
    2. Extract to a directory of your choice
    3. Either add the directory to PATH, or set:
         $env:OpenCppCoverage = 'C:\path\to\OpenCppCoverage.exe'

  Alternative (system-wide, needs admin):
    winget install --id OpenCppCoverage.OpenCppCoverage --source winget

  See tools/coverage/README.md for environment variables and output layout.

.PARAMETER WithSynthesis
  Also run a -only-synthesized pass to exercise backend emit paths.

.PARAMETER ReportOnly
  Skip test execution; re-export reports from the existing merged .cov file.

.PARAMETER TestArgs
  Remaining positional arguments forwarded verbatim to slang-test.
#>

[CmdletBinding()]
param(
    [switch]$WithSynthesis,
    [switch]$ReportOnly,
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$TestArgs = @()
)

$ErrorActionPreference = 'Stop'

function Resolve-RepoPath([string]$Relative) {
    return (Join-Path $RepoRoot $Relative)
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot  = (Resolve-Path (Join-Path $ScriptDir '..\..')).Path
$Config    = if ($env:CONFIG)    { $env:CONFIG }    else { 'RelWithDebInfo' }

# Resolve the build directory. The vs2022-dev preset writes to
# build\windows-vs2022-dev\ while the default (Ninja) preset writes to build\.
# Prefer an explicit BUILD_DIR; otherwise probe both locations for the
# config-specific slang-test.exe and pick whichever has it.
if ($env:BUILD_DIR) {
    $BuildDir = (Resolve-Path $env:BUILD_DIR).Path
} else {
    $candidates = @(
        (Resolve-RepoPath 'build\windows-vs2022-dev')
        (Resolve-RepoPath 'build')
    )
    $BuildDir = $candidates[-1]  # default fallback
    foreach ($c in $candidates) {
        if (Test-Path (Join-Path $c "$Config\bin\slang-test.exe")) {
            $BuildDir = $c
            break
        }
    }
}

$SlangTest = Join-Path $BuildDir "$Config\bin\slang-test.exe"
# slang.dll on Windows is a thin proxy that forwards exports to
# slang-compiler.dll (see source/slang/CMakeLists.txt). The real compiler code
# we want to measure lives in slang-compiler.dll -- the Unix equivalent of
# libslang-compiler.so that the Linux/macOS coverage already targets.
$SlangDll  = Join-Path $BuildDir "$Config\bin\slang-compiler.dll"

$CoverageDir = if ($env:COVERAGE_DIR) { $env:COVERAGE_DIR } else { Join-Path $BuildDir 'coverage-data' }
$null = New-Item -ItemType Directory -Force -Path $CoverageDir | Out-Null

$MergedCov = Join-Path $CoverageDir 'slang-test.cov'

# Locate OpenCppCoverage. Explicit env var wins; then PATH; then default install dir.
$OCC = $env:OpenCppCoverage
if (-not $OCC) {
    $cmd = Get-Command OpenCppCoverage.exe -ErrorAction SilentlyContinue
    if ($cmd) { $OCC = $cmd.Source }
}
if (-not $OCC) {
    $default = 'C:\Program Files\OpenCppCoverage\OpenCppCoverage.exe'
    if (Test-Path $default) { $OCC = $default }
}
if (-not $OCC) {
    throw @"
OpenCppCoverage.exe not found. Install options:
  1. Portable (no admin): download OpenCppCoverage-x64-*.zip from
     https://github.com/OpenCppCoverage/OpenCppCoverage/releases
     extract it, then either add the folder to PATH or set
     `$env:OpenCppCoverage = 'C:\path\to\OpenCppCoverage.exe'
  2. winget (admin):
     winget install --id OpenCppCoverage.OpenCppCoverage --source winget
"@
}

# slangc compiler-only exclusion list. Mirror of tools/coverage/slangc-ignore-patterns.sh
# -- keep these in sync when either file is edited.
#
# These are case-insensitive substrings matched against each source file's
# absolute path (see ConvertTo-Lcov). A leading '\' anchors them to a path
# component boundary so e.g. '\external\' matches 'repos\slang\external\...'
# but not 'fooexternal\bar'. The patterns are agnostic to the build tree
# layout: '\source\slang-core-module\' matches both the Ninja-preset output
# (build\source\slang-core-module\) and the vs2022-dev preset output
# (build\windows-vs2022-dev\source\slang-core-module\). This is also the
# shape llvm-cov -ignore-filename-regex uses on Linux via substring matching.
$SlangcExcludePatterns = @(
    '\prelude\'
    '\source\slang\capability\'
    '\source\slang\fiddle\'
    '\source\slang\slang-lookup-tables\'
    '\source\slang-core-module\'
    '\external\'
    '\include\'
    '\source\slang-glslang\'
    '\source\slang-record-replay\'
    '\tools\'
    '\source\slang\slang-language-server'
    '\source\slang\slang-doc-markdown-writer'
    '\source\slang\slang-doc-ast'
    '\source\slang\slang-ast-dump'
    '\source\slang\slang-repro'
    '\source\slang\slang-workspace-version'
    '\source\slang\slang-ast-expr.h'
    '\source\slang\slang-ast-modifier.h'
    '\source\slang\slang-ast-stmt.h'
)
$SlangcExcluded = $SlangcExcludePatterns

function Invoke-OCC {
    param(
        [string[]]$OccArgs,
        [string[]]$ChildCommand = $null,
        [switch]$TolerateChildFailure
    )
    $full = @($OccArgs)
    if ($ChildCommand) {
        $full += '--'
        $full += $ChildCommand
    }
    Write-Host "+ $OCC $($full -join ' ')"
    # Run OCC with ErrorActionPreference temporarily relaxed so a non-zero
    # native exit sets $LASTEXITCODE rather than throwing a
    # NativeCommandExitException. Setting $PSNativeCommandUseErrorActionPreference
    # globally at script scope does NOT reliably propagate in pwsh 7 when the
    # script is invoked via `& $path` from another pwsh step -- the outer
    # scope's $true wins. Scoping the override to this function is robust.
    $prevEAP = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        & $OCC @full
    } finally {
        $ErrorActionPreference = $prevEAP
    }
    # OpenCppCoverage returns the child's exit code. For test runs we want to
    # tolerate non-zero (expected failures from missing GPU/CUDA drivers,
    # occasional illegal-instruction crashes on GitHub runners, etc.) as long
    # as coverage data was written. Callers that are strictly checking OCC
    # itself (e.g. re-export from --input_coverage) should leave the switch
    # off.
    if ($LASTEXITCODE -ne 0 -and -not $TolerateChildFailure) {
        throw "OpenCppCoverage exited with code $LASTEXITCODE"
    }
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "OpenCppCoverage child exited $LASTEXITCODE (tolerated). Coverage data still captured."
    }
}

function Run-WithCoverage {
    param(
        [string]$OutputCov,
        [string[]]$ChildArgs,
        [string]$InputCov = $null,
        [switch]$QuietRun
    )
    $args = @(
        '--sources', $RepoRoot
        '--modules', $SlangDll
        '--working_dir', $RepoRoot
        '--cover_children'
        '--export_type', "binary:$OutputCov"
    )
    if ($QuietRun) { $args += '--quiet' }
    if ($InputCov) { $args += @('--input_coverage', $InputCov) }
    # Test failures in the child are expected in environments missing GPU
    # drivers, CUDA, etc. Coverage is still written -- proceed with the run.
    Invoke-OCC -OccArgs $args -ChildCommand (@($SlangTest) + $ChildArgs) -TolerateChildFailure
}

if (-not $ReportOnly) {
    foreach ($p in @($SlangTest, $SlangDll)) {
        if (-not (Test-Path $p)) {
            throw "Required build output missing: $p. Build slang-test in $Config first."
        }
    }

    Write-Host "Cleaning previous coverage data..."
    Get-ChildItem $CoverageDir -Filter '*.cov' -ErrorAction SilentlyContinue | Remove-Item -Force

    $mainCov = Join-Path $CoverageDir 'main.cov'
    Write-Host "`nPass 1/3: main test run"
    Push-Location $RepoRoot
    try {
        Run-WithCoverage -OutputCov $mainCov -ChildArgs $TestArgs
    } finally {
        Pop-Location
    }

    $recordCov = Join-Path $CoverageDir 'record-replay.cov'
    Write-Host "`nPass 2/3: record-replay API tests (SLANG_RECORD_LAYER=1)"
    $recordDir = Join-Path $CoverageDir 'slang-record'
    $null = New-Item -ItemType Directory -Force -Path $recordDir | Out-Null
    $prevLayer = $env:SLANG_RECORD_LAYER
    $prevDir = $env:SLANG_RECORD_DIRECTORY
    $env:SLANG_RECORD_LAYER = '1'
    $env:SLANG_RECORD_DIRECTORY = $recordDir
    try {
        Push-Location $RepoRoot
        try {
            # Tolerate failures -- the main goal is coverage accumulation.
            # Forward -exclude-prefix / -exclude entries from the caller's
            # TestArgs so CI-level exclusions (e.g. tests that crash under
            # OCC instrumentation) apply here too. Other TestArgs are
            # dropped to keep the record-replay pass scoped to the
            # RecordReplayApi prefix.
            $recordArgs = @('slang-unit-test-tool/RecordReplayApi')
            for ($i = 0; $i -lt $TestArgs.Count; $i++) {
                if ($TestArgs[$i] -in @('-exclude', '-exclude-prefix')) {
                    $recordArgs += $TestArgs[$i]
                    if ($i + 1 -lt $TestArgs.Count) {
                        $recordArgs += $TestArgs[$i + 1]
                        $i++
                    }
                }
            }
            try {
                Run-WithCoverage -OutputCov $recordCov -InputCov $mainCov `
                    -ChildArgs $recordArgs
            } catch {
                Write-Warning "Record-replay pass failed: $_  (coverage from main pass retained)"
                Copy-Item -Force $mainCov $recordCov
            }
        } finally {
            Pop-Location
        }
    } finally {
        if ($null -ne $prevLayer) { $env:SLANG_RECORD_LAYER = $prevLayer } else { Remove-Item Env:SLANG_RECORD_LAYER -ErrorAction SilentlyContinue }
        if ($null -ne $prevDir)   { $env:SLANG_RECORD_DIRECTORY = $prevDir } else { Remove-Item Env:SLANG_RECORD_DIRECTORY -ErrorAction SilentlyContinue }
        Remove-Item -Recurse -Force $recordDir -ErrorAction SilentlyContinue
    }

    if ($WithSynthesis) {
        $synthCov = Join-Path $CoverageDir 'synthesis.cov'
        Write-Host "`nPass 3/3: synthesized compile-target tests"
        Push-Location $RepoRoot
        try {
            try {
                Run-WithCoverage -OutputCov $synthCov -InputCov $recordCov `
                    -ChildArgs (@($TestArgs) + @('-only-synthesized'))
            } catch {
                Write-Warning "Synthesis pass failed: $_  (coverage from earlier passes retained)"
                Copy-Item -Force $recordCov $synthCov
            }
        } finally {
            Pop-Location
        }
        Copy-Item -Force $synthCov $MergedCov
    } else {
        Write-Host "`nSkipping synthesis pass (--WithSynthesis not specified)"
        Copy-Item -Force $recordCov $MergedCov
    }

    Write-Host "`nMerged coverage data: $MergedCov"
} else {
    if (-not (Test-Path $MergedCov)) {
        throw "Report-only mode requested but $MergedCov not found. Run without -ReportOnly first."
    }
    Write-Host "Report-only mode: using existing coverage data at $MergedCov"
}

# Export reports -------------------------------------------------------------

$LcovFile     = if ($env:COVERAGE_LCOV_FILE) { $env:COVERAGE_LCOV_FILE } else { Join-Path $RepoRoot 'coverage.lcov' }
$HtmlDir      = if ($env:COVERAGE_HTML_DIR)  { $env:COVERAGE_HTML_DIR }  else { Join-Path $RepoRoot 'coverage-html' }
# Resolve relative paths against $RepoRoot. CI passes bare filenames like
# "coverage.lcov", and Split-Path -Parent on a bare filename returns "",
# which makes the Join-Path calls below throw "Cannot bind argument to
# parameter 'Path' because it is an empty string."
if (-not [System.IO.Path]::IsPathRooted($LcovFile)) { $LcovFile = Join-Path $RepoRoot $LcovFile }
if (-not [System.IO.Path]::IsPathRooted($HtmlDir))  { $HtmlDir  = Join-Path $RepoRoot $HtmlDir }
$LcovDir   = Split-Path -Parent $LcovFile
$LcovStem  = [IO.Path]::GetFileNameWithoutExtension($LcovFile)
$SlangcLcov = Join-Path $LcovDir ("{0}-slangc.lcov" -f $LcovStem)
$SlangcHtml   = "$HtmlDir-slangc"

# Cobertura is always emitted -- it's the source for both the summary parser
# and the LCOV conversion. LCOV and HTML are opt-in via env vars (matches
# run-coverage.sh semantics; the CI workflow and run-coverage-local.ps1 set
# both to "1").
$wantLcov = $env:COVERAGE_LCOV -eq '1'
$wantHtml = $env:COVERAGE_HTML -eq '1'

# OpenCppCoverage 0.9.9.0 does not support LCOV export -- only html, cobertura,
# and binary. We emit Cobertura XML and convert to LCOV here so downstream
# tooling (CI artifact upload, summary parsing) sees the same format Linux and
# macOS produce. Conversion is line-level only, which matches what Cobertura
# carries and what we need for the cross-platform summary.
function ConvertTo-Lcov {
    param(
        [string]$CoberturaPath,
        [string]$LcovPath,
        [string[]]$ExcludePrefixes = @()
    )
    [xml]$xml = Get-Content -LiteralPath $CoberturaPath
    # OpenCppCoverage's Cobertura output uses the drive letter (e.g. "C:") as
    # the <source> root and emits drive-relative paths in <class filename=...>.
    # Pick the first <source> that looks like a drive root and prepend it.
    $srcRoots = @($xml.coverage.sources.source)
    $driveRoot = $srcRoots | Where-Object { $_ -match '^[A-Za-z]:$' } | Select-Object -First 1
    $sb = [System.Text.StringBuilder]::new()
    foreach ($cls in $xml.coverage.packages.package.classes.class) {
        $file = $cls.filename
        if (-not [System.IO.Path]::IsPathRooted($file)) {
            if ($driveRoot) {
                $file = "$driveRoot\$file"
            } else {
                $file = Join-Path $RepoRoot $file
            }
        }
        # Post-hoc filter for the slangc-only report. OpenCppCoverage's
        # --excluded_sources is honored only at collection time, not on
        # re-export from --input_coverage, so we apply the slangc exclusion
        # list here to avoid running a second test pass just to filter.
        # Substring match (case-insensitive) mirrors llvm-cov
        # -ignore-filename-regex semantics and stays robust against preset-
        # specific build-tree layouts (see $SlangcExcludePatterns comment).
        $skip = $false
        foreach ($pat in $ExcludePrefixes) {
            if ($file.IndexOf($pat, [System.StringComparison]::OrdinalIgnoreCase) -ge 0) {
                $skip = $true
                break
            }
        }
        if ($skip) { continue }

        [void]$sb.AppendLine("SF:$file")
        $hit = 0
        $found = 0
        foreach ($ln in $cls.lines.line) {
            $n = [int]$ln.number
            $h = [int]$ln.hits
            [void]$sb.AppendLine("DA:$n,$h")
            $found++
            if ($h -gt 0) { $hit++ }
        }
        [void]$sb.AppendLine("LF:$found")
        [void]$sb.AppendLine("LH:$hit")
        [void]$sb.AppendLine("end_of_record")
    }
    [System.IO.File]::WriteAllText($LcovPath, $sb.ToString())
}

$FullCobertura = Join-Path $CoverageDir 'full.cobertura.xml'

# OpenCppCoverage applies --excluded_sources only at *collection* time, so we
# can't get a slangc-filtered HTML out of the same .cov without a second test
# run. We emit one Cobertura/HTML (the full library), and produce the
# slangc-only report by filtering inside ConvertTo-Lcov below. Net effect: full
# HTML is browseable; slangc-only summary metrics still feed the dashboard.
Write-Host "`nExporting full-library report..."
$fullArgs = @('--input_coverage', $MergedCov, '--sources', $RepoRoot)
$fullArgs += @('--export_type', "cobertura:$FullCobertura")
if ($wantHtml) {
    if (Test-Path $HtmlDir) { Remove-Item -Recurse -Force $HtmlDir }
    $fullArgs += @('--export_type', "html:$HtmlDir")
}
Invoke-OCC -OccArgs $fullArgs

if ($wantLcov) {
    Write-Host "`nConverting Cobertura -> LCOV (full)..."
    ConvertTo-Lcov -CoberturaPath $FullCobertura -LcovPath $LcovFile
    Write-Host "Converting Cobertura -> LCOV (slangc, post-hoc filter)..."
    ConvertTo-Lcov -CoberturaPath $FullCobertura -LcovPath $SlangcLcov `
        -ExcludePrefixes $SlangcExcluded
}

Write-Host "`nCoverage outputs:"
Write-Host "  Merged binary:    $MergedCov"
Write-Host "  Cobertura (full): $FullCobertura"
if ($wantLcov) {
    Write-Host "  LCOV (full):      $LcovFile"
    Write-Host "  LCOV (slangc):    $SlangcLcov"
}
if ($wantHtml) {
    Write-Host "  HTML (full):      $HtmlDir\index.html"
    Write-Host "  (slangc-only HTML is not produced on Windows -- see script comment)"
}

# Explicitly signal success. Without this, the script's implicit exit code is
# whatever $LASTEXITCODE the last native command left behind -- and Pass 1
# routinely leaves $LASTEXITCODE != 0 when slang-test has expected GPU-less
# failures (tolerated by Invoke-OCC but still visible in $LASTEXITCODE). A
# non-zero script exit triggers the caller's retry logic and doubles the
# runtime for no new data.
exit 0
