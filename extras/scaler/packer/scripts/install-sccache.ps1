# Install sccache and configure it with GCS backend
#
# Replaces the network-local ccache that the current static runners use.
# With ephemeral VMs, we need a remote cache that persists across VM
# lifecycles and is shared between runners.
#
# sccache sits between CMake and cl.exe, caching compilation results in
# the GCS bucket that's already used by Linux/macOS builds.

$ErrorActionPreference = "Stop"

$sccacheVersion = $env:SCCACHE_VERSION
if (-not $sccacheVersion) {
    $sccacheVersion = "0.7.4"
}

Write-Host "=== Installing sccache $sccacheVersion ==="

$sccacheUrl = "https://github.com/mozilla/sccache/releases/download/v${sccacheVersion}/sccache-v${sccacheVersion}-x86_64-pc-windows-msvc.tar.gz"
$archivePath = "$env:TEMP\sccache.tar.gz"

Write-Host "Downloading sccache..."
Invoke-WebRequest -Uri $sccacheUrl -OutFile $archivePath -UseBasicParsing

# Extract
Write-Host "Extracting..."
$extractDir = "$env:TEMP\sccache-extract"
New-Item -ItemType Directory -Path $extractDir -Force | Out-Null
tar -xzf $archivePath -C $extractDir

# Install to C:\sccache
$installDir = "C:\sccache"
New-Item -ItemType Directory -Path $installDir -Force | Out-Null
Copy-Item "$extractDir\sccache-v${sccacheVersion}-x86_64-pc-windows-msvc\sccache.exe" "$installDir\sccache.exe"

# Add to system PATH
[Environment]::SetEnvironmentVariable(
    "Path",
    [Environment]::GetEnvironmentVariable("Path", "Machine") + ";$installDir",
    "Machine"
)

# Configure sccache environment variables (system-wide)
# These are read by sccache at runtime to determine the cache backend.
$sccacheVars = @{
    "SCCACHE_GCS_BUCKET"     = "slang-ci-cache"
    "SCCACHE_GCS_RW_MODE"    = "READ_WRITE"
    "SCCACHE_CACHE_ZSTD_LEVEL" = "3"
    "SCCACHE_IDLE_TIMEOUT"   = "0"
    "SCCACHE_IGNORE_SERVER_IO_ERROR" = "1"
}

foreach ($kv in $sccacheVars.GetEnumerator()) {
    [Environment]::SetEnvironmentVariable($kv.Key, $kv.Value, "Machine")
    Write-Host "  $($kv.Key) = $($kv.Value)"
}

# GCS authentication: The VM uses its service account (set in the instance
# template) for GCS access. No explicit credentials file needed - sccache
# picks up Application Default Credentials automatically.

# Clean up
Remove-Item $archivePath -Force
Remove-Item $extractDir -Recurse -Force

# Verify
$env:Path += ";$installDir"
sccache --version

Write-Host "=== sccache installed and configured for GCS ==="
Write-Host "Note: GCS auth uses VM service account (Application Default Credentials)"
