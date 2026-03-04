# Install Visual Studio 2022 Build Tools with MSVC and Windows SDK
#
# Installs the components needed to build Slang:
# - MSVC v143 C++ toolset
# - Windows SDK
# - CMake (bundled with VS, but we also install standalone)

$ErrorActionPreference = "Stop"

Write-Host "=== Installing Visual Studio 2022 Build Tools ==="

$installerUrl = "https://aka.ms/vs/17/release/vs_buildtools.exe"
$installerPath = "$env:TEMP\vs_buildtools.exe"

Write-Host "Downloading VS Build Tools..."
Invoke-WebRequest -Uri $installerUrl -OutFile $installerPath -UseBasicParsing

Write-Host "Installing VS Build Tools (this takes 10-20 minutes)..."
$process = Start-Process -FilePath $installerPath -ArgumentList @(
    "--quiet",
    "--wait",
    "--norestart",
    "--nocache",
    "--add", "Microsoft.VisualStudio.Workload.VCTools",
    "--add", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
    "--add", "Microsoft.VisualStudio.Component.Windows11SDK.22621",
    "--includeRecommended"
) -Wait -PassThru

if ($process.ExitCode -notin 0, 3010) {
    throw "VS Build Tools installation failed with exit code $($process.ExitCode)"
}

# Clean up
Remove-Item $installerPath -Force

Write-Host "=== VS 2022 Build Tools installed ==="
