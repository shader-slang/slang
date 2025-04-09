# Install Chocolatey if not already installed
if (!(Get-Command choco -ErrorAction SilentlyContinue)) {
    Set-ExecutionPolicy Bypass -Scope Process -Force
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
    Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
}

# Install required packages
Write-Host "Installing Visual Studio Build Tools..."
choco install visualstudio2022buildtools -y

Write-Host "Installing Visual C++ build tools workload..."
choco install visualstudio2022-workload-vctools -y

Write-Host "Installing Windows SDK..."
choco install windows-sdk-10.1 -y
choco install windows-sdk-10-version-2004-all -y

Write-Host "Installing CMake..."
choco install cmake --pre -y

Write-Host "Installing Git..."
choco install git -y

Write-Host "Installing Ninja..."
choco install ninja -y

Write-Host "Installing Python 3.11..."
choco install python311 -y

Write-Host "Installing CUDA..."
choco install cuda -y

# Refresh environment variables
Write-Host "Refreshing environment variables..."
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

# Verify installations
Write-Host "`nVerifying installations..."
Write-Host "CMake version:"
cmake --version

Write-Host "`nGit version:"

git --version

Write-Host "`nVisual Studio Build Tools version:"
& 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe' /version

Write-Host "`nWindows SDK version:"
if (Test-Path "C:\Program Files (x86)\Windows Kits\10\include") {
    $sdkVersions = Get-ChildItem "C:\Program Files (x86)\Windows Kits\10\include" | Where-Object { $_.PSIsContainer }
    Write-Host "Installed Windows SDK versions:"
    $sdkVersions | ForEach-Object { Write-Host "- $($_.Name)" }
} else {
    Write-Host "Windows SDK not found in expected location"
}

Write-Host "`nInstallation complete! Please restart your terminal to ensure all PATH changes take effect."

# Download NVIDIA drivers
$driverUrl = "https://storage.googleapis.com/nvidia-drivers-us-public/GRID/vGPU17.5/553.62_grid_win10_win11_server2019_server2022_dch_64bit_international.exe"
$driverPath = "$env:TEMP\nvidia_driver.exe"
Write-Host "Downloading NVIDIA drivers..."
Invoke-WebRequest -Uri $driverUrl -OutFile $driverPath

# Install NVIDIA drivers silently
Write-Host "Installing NVIDIA drivers..."
Start-Process -FilePath $driverPath -ArgumentList "-s" -Wait

# Verify NVIDIA driver installation
Write-Host "Verifying NVIDIA driver installation..."
$nvidiaDriver = Get-WmiObject Win32_VideoController | Where-Object { $_.Name -like "*NVIDIA*" }
if ($nvidiaDriver) {
  Write-Host "NVIDIA driver version: $($nvidiaDriver.DriverVersion)"
} else {
  Write-Host "Warning: NVIDIA driver not found after installation"
}

# Wait for installation to complete
Start-Sleep -Seconds 10

# Add Vulkan SDK to PATH
$vulkanPath = "C:\VulkanSDK\1.4.309.0\Bin"
if (Test-Path $vulkanPath) {
  Add-Content -Path $env:GITHUB_PATH -Value $vulkanPath
  $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
  
  # Verify Vulkan installation
  try {
    & "$vulkanPath\vulkaninfo.exe" --summary
  } catch {
    Write-Host "Error running vulkaninfo: $_"
    Write-Host "Please check NVIDIA driver installation and system restart if needed"
  }
} 