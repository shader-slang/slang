# Packer template for Windows GPU Runner VM image
#
# Creates a GCP image with everything pre-installed for GitHub Actions
# GPU testing: NVIDIA driver, Visual Studio, sccache, GitHub runner agent.
#
# Build:
#   packer init packer/windows-gpu-runner.pkr.hcl
#   packer build -var 'project_id=slang-runners' packer/windows-gpu-runner.pkr.hcl

packer {
  required_plugins {
    googlecompute = {
      version = ">= 1.1.0"
      source  = "github.com/hashicorp/googlecompute"
    }
  }
}

variable "project_id" {
  type        = string
  description = "GCP project ID"
}

variable "zone" {
  type        = string
  default     = "us-west1-a"
  description = "GCP zone for building the image"
}

variable "nvidia_driver_version" {
  type        = string
  default     = "580.126.09"
  description = "NVIDIA driver version to install"
}

variable "runner_version" {
  type        = string
  default     = "2.321.0"
  description = "GitHub Actions runner version"
}

variable "sccache_version" {
  type        = string
  default     = "0.7.4"
  description = "sccache version"
}

source "googlecompute" "windows-gpu-runner" {
  project_id = var.project_id
  zone       = var.zone

  # Base image: Windows Server 2022 with GPU drivers
  source_image_family = "windows-2022"

  # Build VM with T4 GPU (needed to install/verify GPU driver)
  machine_type    = "n1-standard-8"
  accelerator_type = "nvidia-tesla-t4"
  accelerator_count = 1

  disk_size = 200
  disk_type = "pd-ssd"

  image_name        = "windows-gpu-runner-{{timestamp}}"
  image_family      = "windows-gpu-runner"
  image_description = "Windows Server 2022 with NVIDIA T4 GPU, VS 2022, sccache, GitHub runner"

  communicator   = "winrm"
  winrm_username = "packer"
  winrm_insecure = true
  winrm_use_ssl  = true

  metadata = {
    # Enable WinRM for Packer communication
    windows-startup-script-cmd = "winrm quickconfig -quiet & net user packer /add & net localgroup administrators packer /add & winrm set winrm/config/service/auth @{Basic=\"true\"}"
  }
}

build {
  sources = ["source.googlecompute.windows-gpu-runner"]

  # Install NVIDIA GPU driver
  provisioner "powershell" {
    script = "packer/scripts/install-nvidia-driver.ps1"
    environment_vars = [
      "NVIDIA_DRIVER_VERSION=${var.nvidia_driver_version}",
    ]
  }

  # Install Visual Studio 2022 Build Tools
  provisioner "powershell" {
    script = "packer/scripts/install-vs-buildtools.ps1"
  }

  # Install build dependencies (Git, Ninja, CMake, Python)
  provisioner "powershell" {
    script = "packer/scripts/install-build-deps.ps1"
  }

  # Install and configure sccache with GCS backend
  provisioner "powershell" {
    script = "packer/scripts/install-sccache.ps1"
    environment_vars = [
      "SCCACHE_VERSION=${var.sccache_version}",
    ]
  }

  # Install GitHub Actions runner agent
  provisioner "powershell" {
    script = "packer/scripts/install-runner-agent.ps1"
    environment_vars = [
      "RUNNER_VERSION=${var.runner_version}",
    ]
  }

  # Copy the startup script that reads JIT config and starts the runner
  provisioner "file" {
    source      = "packer/scripts/windows-runner-startup.ps1"
    destination = "C:\\actions-runner\\startup.ps1"
  }

  # Configure Windows for CI use
  provisioner "powershell" {
    script = "packer/scripts/configure-windows.ps1"
  }

  # Verify everything is installed
  provisioner "powershell" {
    inline = [
      "Write-Host '=== Verification ==='",
      "nvidia-smi",
      "git --version",
      "cmake --version",
      "ninja --version",
      "python --version",
      "sccache --version",
      "C:\\actions-runner\\run.cmd --version",
      "Write-Host '=== All tools verified ==='",
    ]
  }

  # Create GCP instance template from this image
  post-processor "shell-local" {
    inline = [
      "gcloud compute instance-templates create windows-gpu-runner --project=${var.project_id} --machine-type=n1-standard-4 --accelerator=type=nvidia-tesla-t4,count=1 --maintenance-policy=TERMINATE --image-family=windows-gpu-runner --image-project=${var.project_id} --boot-disk-size=200 --boot-disk-type=pd-ssd --metadata=windows-startup-script-ps1=C:\\actions-runner\\startup.ps1 --scopes=https://www.googleapis.com/auth/devstorage.read_write 2>/dev/null || echo 'Instance template already exists, update manually if needed'",
    ]
  }
}
