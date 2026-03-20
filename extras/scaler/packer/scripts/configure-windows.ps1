# Configure Windows for CI runner use
#
# Tweaks Windows settings to improve CI performance and reliability.

$ErrorActionPreference = "Stop"

Write-Host "=== Configuring Windows for CI ==="

# Disable Windows Defender real-time scanning for build directories.
# This is a major performance win for compilation workloads.
Write-Host "Configuring Windows Defender exclusions..."
$exclusionPaths = @(
    "C:\actions-runner",
    "C:\a",               # GitHub Actions workspace
    "C:\sccache",
    "C:\hostedtoolcache"
)

foreach ($path in $exclusionPaths) {
    try {
        Add-MpPreference -ExclusionPath $path
        Write-Host "  Excluded: $path"
    }
    catch {
        Write-Host "  WARNING: Could not exclude $path : $_"
    }
}

# Exclude common build file extensions
$exclusionExtensions = @(".obj", ".pdb", ".lib", ".dll", ".exe", ".o", ".a")
foreach ($ext in $exclusionExtensions) {
    try {
        Add-MpPreference -ExclusionExtension $ext
        Write-Host "  Excluded extension: $ext"
    }
    catch {
        Write-Host "  WARNING: Could not exclude extension $ext : $_"
    }
}

# Disable Windows Search indexing (uses CPU and disk I/O)
Write-Host "Disabling Windows Search..."
Stop-Service WSearch -Force -ErrorAction SilentlyContinue
Set-Service WSearch -StartupType Disabled -ErrorAction SilentlyContinue

# Disable Windows Update (managed externally via new Packer builds)
Write-Host "Disabling Windows Update..."
Stop-Service wuauserv -Force -ErrorAction SilentlyContinue
Set-Service wuauserv -StartupType Disabled -ErrorAction SilentlyContinue

# Set power plan to High Performance
Write-Host "Setting power plan to High Performance..."
powercfg /setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c

# Disable Server Manager auto-start
Write-Host "Disabling Server Manager auto-start..."
$regPath = "HKLM:\SOFTWARE\Microsoft\ServerManager"
if (Test-Path $regPath) {
    Set-ItemProperty -Path $regPath -Name "DoNotOpenServerManagerAtLogon" -Value 1
}

# Configure startup script to run on boot via scheduled task
Write-Host "Registering startup task..."
$taskAction = New-ScheduledTaskAction -Execute "powershell.exe" -Argument "-NoProfile -ExecutionPolicy Bypass -File C:\actions-runner\startup.ps1"
$taskTrigger = New-ScheduledTaskTrigger -AtStartup
$taskSettings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries
$taskPrincipal = New-ScheduledTaskPrincipal -UserId "SYSTEM" -RunLevel Highest -LogonType ServiceAccount

Register-ScheduledTask -TaskName "GitHubRunnerStartup" `
    -Action $taskAction `
    -Trigger $taskTrigger `
    -Settings $taskSettings `
    -Principal $taskPrincipal `
    -Force

Write-Host "=== Windows configuration complete ==="
