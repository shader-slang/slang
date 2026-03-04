# Install build dependencies: Git, Ninja, CMake, Python, Chocolatey
#
# These are the tools needed by the Slang CI build and test workflows.

$ErrorActionPreference = "Stop"

Write-Host "=== Installing Build Dependencies ==="

# Install Chocolatey package manager
Write-Host "Installing Chocolatey..."
Set-ExecutionPolicy Bypass -Scope Process -Force
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

# Refresh PATH
$env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")

# Install tools via Chocolatey
Write-Host "Installing Git..."
choco install git -y --no-progress

Write-Host "Installing Ninja..."
choco install ninja -y --no-progress

Write-Host "Installing Python..."
choco install python3 --version=3.10.11 -y --no-progress

# Install CMake 3.30 (matching the Linux CI container)
Write-Host "Installing CMake 3.30..."
$cmakeUrl = "https://github.com/Kitware/CMake/releases/download/v3.30.0/cmake-3.30.0-windows-x86_64.msi"
$cmakeMsi = "$env:TEMP\cmake.msi"
Invoke-WebRequest -Uri $cmakeUrl -OutFile $cmakeMsi -UseBasicParsing
Start-Process msiexec.exe -ArgumentList "/i", $cmakeMsi, "/quiet", "/norestart", "ADD_CMAKE_TO_PATH=System" -Wait
Remove-Item $cmakeMsi -Force

# Add Git to system PATH (CI workflows expect it)
$gitPaths = @(
    "C:\Program Files\Git\bin",
    "C:\Program Files\Git\usr\bin"
)
foreach ($p in $gitPaths) {
    if (Test-Path $p) {
        [Environment]::SetEnvironmentVariable(
            "Path",
            [Environment]::GetEnvironmentVariable("Path", "Machine") + ";$p",
            "Machine"
        )
    }
}

# Configure git for CI
git config --global core.autocrlf false
git config --global --add safe.directory '*'

Write-Host "=== Build dependencies installed ==="
