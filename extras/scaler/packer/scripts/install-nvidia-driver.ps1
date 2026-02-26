# Install NVIDIA GPU driver for Windows Server 2022
#
# Uses the NVIDIA driver version specified via NVIDIA_DRIVER_VERSION env var.
# GCP Windows images may have a driver pre-installed, but we install a
# specific version for consistency with our Linux runners.

$ErrorActionPreference = "Stop"

$driverVersion = $env:NVIDIA_DRIVER_VERSION
if (-not $driverVersion) {
    $driverVersion = "580.126.09"
}

Write-Host "=== Installing NVIDIA Driver $driverVersion ==="

# GCP provides NVIDIA drivers via a special bucket
$driverUrl = "https://storage.googleapis.com/nvidia-drivers-us-public/GRID/vGPU17.5/580.126.09_grid_win10_win11_server2022_dch_64bit_international.exe"

# Download driver
$installerPath = "$env:TEMP\nvidia-driver.exe"
Write-Host "Downloading driver..."
Invoke-WebRequest -Uri $driverUrl -OutFile $installerPath -UseBasicParsing

# Silent install
Write-Host "Installing driver (silent)..."
$process = Start-Process -FilePath $installerPath -ArgumentList "-s", "-noreboot" -Wait -PassThru
if ($process.ExitCode -ne 0) {
    Write-Warning "Driver installer exited with code $($process.ExitCode)"
    # Exit code 1 is common for "reboot required" which is fine
    if ($process.ExitCode -ne 1) {
        throw "NVIDIA driver installation failed with exit code $($process.ExitCode)"
    }
}

# Clean up
Remove-Item $installerPath -Force

# Verify
Write-Host "Verifying driver installation..."
nvidia-smi
Write-Host "=== NVIDIA driver installed ==="
