#!/bin/bash
# Linux GPU Runner Startup Script
#
# This script runs as root (GCE metadata scripts always run as root).
# The GitHub Actions runner must run as the user who owns the runner
# directory owner), not as root.
#
# Steps:
# 1. Removes any pre-existing runner service from the base image
# 2. Reads the JIT config from GCP instance metadata
# 3. Starts the GitHub Actions runner as the correct user
# 4. Shuts down the VM when the job completes

set -euo pipefail

# Find the runner directory and its owner
RUNNER_DIR=""
RUNNER_USER=""
for home in /home/*; do
  if [ -d "$home/actions-runner" ]; then
    RUNNER_DIR="$home/actions-runner"
    RUNNER_USER=$(stat -c '%U' "$home/actions-runner")
    break
  fi
done

if [ -z "$RUNNER_DIR" ]; then
  if [ -d "/actions-runner" ]; then
    RUNNER_DIR="/actions-runner"
    RUNNER_USER=$(stat -c '%U' "/actions-runner")
  else
    echo "ERROR: Cannot find actions-runner directory"
    shutdown -h now
    exit 1
  fi
fi

LOG_FILE="${RUNNER_DIR}/startup.log"

log() {
  local msg
  msg="$(date '+%Y-%m-%d %H:%M:%S') - $1"
  echo "$msg"
  echo "$msg" >>"$LOG_FILE"
}

log "=== Linux GPU Runner Startup ==="
log "Runner directory: $RUNNER_DIR"
log "Runner user: $RUNNER_USER"

# Step 0: Remove any pre-existing runner service from the base image.
log "Removing pre-existing runner service (if any)..."
if systemctl list-units --type=service --all 2>/dev/null | grep -q "actions.runner"; then
  cd "$RUNNER_DIR"
  ./svc.sh stop 2>&1 | while read -r line; do log "  $line"; done || true
  ./svc.sh uninstall 2>&1 | while read -r line; do log "  $line"; done || true
  log "  Service removed."
else
  log "  No existing runner service found."
fi

# Remove old runner config files
for f in .runner .credentials .credentials_rsaparams .runner_migrated; do
  if [ -f "$RUNNER_DIR/$f" ]; then
    rm -f "$RUNNER_DIR/$f"
    log "  Removed $f"
  fi
done

# Step 0.5: Ensure NVIDIA GPU devices are initialized.
# On fresh boot, the kernel module may not be loaded yet. Running nvidia-smi
# loads the module and creates /dev/nvidia* device files that the CI workflow
# mounts into Docker containers (--device /dev/nvidia-modeset, /dev/dri, etc.)
log "Initializing NVIDIA GPU..."
gpu_ready=false
for attempt in $(seq 1 10); do
  if nvidia-smi >/dev/null 2>&1; then
    log "  GPU initialized successfully."
    gpu_ready=true
    break
  fi
  log "  Attempt ${attempt}/10: nvidia-smi not ready, waiting..."
  sleep 5
done

if [ "$gpu_ready" != "true" ]; then
  log "ERROR: GPU initialization failed after 10 attempts"
  shutdown -h now
  exit 1
fi

# Create nvidia-modeset device if it doesn't exist (needed for Vulkan)
if [ ! -e /dev/nvidia-modeset ]; then
  log "  Creating /dev/nvidia-modeset..."
  nvidia-modprobe -m 2>/dev/null || modprobe nvidia-modeset 2>/dev/null || true
fi

# Enable GPU persistence mode to prevent NVML state corruption in containers.
# Without this, NVML can lose track of GPU processes when they exit inside Docker,
# causing "Failed to initialize NVML: Unknown Error".
log "  Enabling GPU persistence mode..."
if pm_out="$(nvidia-smi -pm 1 2>&1)"; then
  log "  GPU persistence mode enabled."
else
  log "WARNING: Failed to enable GPU persistence mode: ${pm_out}"
fi

# Create /dev/char symlinks for all NVIDIA device nodes. Recent runc versions
# with cgroup v2 require these symlinks to properly inject devices into
# containers. Without them, containers can intermittently lose GPU access
# with "Failed to initialize NVML: Unknown Error".
# See: https://github.com/NVIDIA/nvidia-docker/issues/1730
log "  Creating /dev/char symlinks..."
if ctk_out="$(nvidia-ctk system create-dev-char-symlinks --create-all 2>&1)"; then
  log "  /dev/char symlinks created."
else
  log "WARNING: Failed to create /dev/char symlinks: ${ctk_out}"
fi

# Verify GPU devices
log "  GPU devices:"
ls -la /dev/nvidia* 2>&1 | while read -r line; do log "    $line"; done || true
ls -la /dev/dri/* 2>&1 | while read -r line; do log "    $line"; done || true

# Step 1: Read JIT config from GCP instance metadata
log "Reading JIT config from instance metadata..."
METADATA_URL="http://metadata.google.internal/computeMetadata/v1/instance/attributes/jit-config"
MAX_RETRIES=10
JIT_CONFIG=""

for i in $(seq 1 "$MAX_RETRIES"); do
  JIT_CONFIG=$(curl -sf --max-time 10 --connect-timeout 5 -H "Metadata-Flavor: Google" "$METADATA_URL") && break
  log "  Attempt ${i}/${MAX_RETRIES}: Metadata not available yet, waiting..."
  sleep 5
done

if [ -z "$JIT_CONFIG" ]; then
  log "ERROR: Failed to read JIT config from metadata after $MAX_RETRIES attempts"
  shutdown -h now
  exit 1
fi

log "JIT config retrieved (${#JIT_CONFIG} chars)"

# Step 2: Log GPU and system info
log "=== System Information ==="
nvidia-smi 2>&1 | while read -r line; do log "  $line"; done || log "WARNING: nvidia-smi not available"
docker --version 2>&1 | while read -r line; do log "  $line"; done || log "WARNING: docker not available"

# Step 3: Run the GitHub Actions runner as the correct user
log "Starting runner as user '$RUNNER_USER' with JIT config..."
cd "$RUNNER_DIR"

# Run as the runner user, not root. The runner agent requires this.
EXIT_CODE=0
sudo -u "$RUNNER_USER" ./run.sh --jitconfig "$JIT_CONFIG" || EXIT_CODE=$?
log "Runner exited with code $EXIT_CODE"

# Step 4: Shut down the VM
log "=== Runner complete, shutting down VM ==="
shutdown -h now
