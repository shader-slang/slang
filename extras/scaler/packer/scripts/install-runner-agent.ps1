# Install GitHub Actions runner agent
#
# Pre-installs the runner at C:\actions-runner so the VM is ready to
# accept jobs immediately on boot. The actual runner configuration
# happens at boot time via the startup script using JIT config.

$ErrorActionPreference = "Stop"

$runnerVersion = $env:RUNNER_VERSION
if (-not $runnerVersion) {
    $runnerVersion = "2.321.0"
}

Write-Host "=== Installing GitHub Actions Runner $runnerVersion ==="

$runnerDir = "C:\actions-runner"
New-Item -ItemType Directory -Path $runnerDir -Force | Out-Null

$runnerUrl = "https://github.com/actions/runner/releases/download/v${runnerVersion}/actions-runner-win-x64-${runnerVersion}.zip"
$zipPath = "$env:TEMP\actions-runner.zip"

Write-Host "Downloading runner..."
Invoke-WebRequest -Uri $runnerUrl -OutFile $zipPath -UseBasicParsing

Write-Host "Extracting..."
Expand-Archive -Path $zipPath -DestinationPath $runnerDir -Force

# Clean up
Remove-Item $zipPath -Force

# Verify
& "$runnerDir\run.cmd" --version

Write-Host "=== GitHub Actions Runner installed at $runnerDir ==="
