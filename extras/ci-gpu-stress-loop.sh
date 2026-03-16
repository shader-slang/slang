#!/bin/bash
# ci-gpu-stress-loop.sh — Create ephemeral GCP VMs and run Linux GPU tests
# to reproduce intermittent driver crashes.
#
# Usage: ./extras/ci-gpu-stress-loop.sh <artifact-tarball> [iterations] [config]
#   artifact-tarball: path to slang-{debug,release}.tar.gz with CI artifacts
#   iterations: number of VMs to create and test (default: 20)
#   config: debug or release (default: debug)
#
# Each iteration creates a new GCP VM, runs the test suite, captures diagnostics,
# and deletes the VM. This exercises the GCP hardware lottery to hit bad GPUs.
#
# Requires: gcloud CLI, authenticated to slang-runners project

set -euo pipefail

ARTIFACT_TARBALL="${1:?Usage: $0 <artifact-tarball> [iterations] [config]}"
ITERATIONS="${2:-20}"
CONFIG="${3:-debug}"

# GHCR token for pulling container image (pass via env or get from gh CLI)
GHCR_TOKEN="${GHCR_TOKEN:-$(gh auth token 2>/dev/null || echo "")}"
if [ -z "$GHCR_TOKEN" ]; then
  echo "WARNING: No GHCR_TOKEN — container pull may fail if image is private"
fi

PROJECT="slang-runners"
ZONES=("us-west1-a" "us-central1-a" "us-east1-c")
MACHINE_TYPE="n1-standard-8"
IMAGE_FAMILY="linux-gpu-runner"
VM_PREFIX="gpu-stress-test"
RESULTS_DIR="./gpu-stress-results/$(date +%Y%m%d_%H%M%S)"

# Map config to cmake config directory name
if [ "$CONFIG" = "release" ]; then
  CMAKE_CONFIG="Release"
else
  CMAKE_CONFIG="Debug"
fi

mkdir -p "$RESULTS_DIR"

echo "=== GPU Stress Test Loop ==="
echo "Artifact: $ARTIFACT_TARBALL"
echo "Config: $CONFIG ($CMAKE_CONFIG)"
echo "Iterations: $ITERATIONS"
echo "Results: $RESULTS_DIR"
echo ""

# We also need the repo (for tests/ directory). Clone once locally.
REPO_TARBALL="$RESULTS_DIR/repo.tar.gz"
if [ ! -f "$REPO_TARBALL" ]; then
  echo "Creating repo tarball (tests + expected failures)..."
  TMPDIR_REPO="$(mktemp -d)"
  git clone --depth 1 https://github.com/shader-slang/slang.git "$TMPDIR_REPO/slang" 2>/dev/null
  tar czf "$REPO_TARBALL" -C "$TMPDIR_REPO" slang/tests slang/tools/slangc-test slang/extras/stress-test-gpu.sh 2>/dev/null ||
    tar czf "$REPO_TARBALL" -C "$TMPDIR_REPO" slang/tests slang/tools/slangc-test 2>/dev/null
  rm -rf "$TMPDIR_REPO"
  echo "Repo tarball: $(du -h "$REPO_TARBALL" | cut -f1)"
fi

echo "iteration,exit_code,duration_s,vm_name,zone,gpu_healthy_after" >"$RESULTS_DIR/results.csv"

cleanup_vm() {
  local vm_name="$1"
  local zone="$2"
  echo "  Deleting VM $vm_name..."
  gcloud compute instances delete "$vm_name" --zone="$zone" --project="$PROJECT" --quiet 2>/dev/null || true
}

for i in $(seq 1 "$ITERATIONS"); do
  # Rotate through zones to hit different hardware
  ZONE="${ZONES[$(((i - 1) % ${#ZONES[@]}))]}"
  VM_NAME="${VM_PREFIX}-${i}-$(date +%s)"
  ITER_DIR="$RESULTS_DIR/iter_$(printf '%03d' "$i")"
  mkdir -p "$ITER_DIR"

  echo ""
  echo "=== Iteration $i / $ITERATIONS — VM: $VM_NAME ($ZONE) ==="

  # Create VM
  echo "  Creating VM in $ZONE..."
  if ! gcloud compute instances create "$VM_NAME" \
    --project="$PROJECT" --zone="$ZONE" \
    --machine-type="$MACHINE_TYPE" \
    --accelerator=type=nvidia-tesla-t4,count=1 \
    --maintenance-policy=TERMINATE \
    --image-family="$IMAGE_FAMILY" --image-project="$PROJECT" \
    --boot-disk-size=512 --boot-disk-type=pd-ssd \
    --quiet 2>"$ITER_DIR/vm_create.log"; then
    echo "  FAILED to create VM (quota?). Skipping."
    cat "$ITER_DIR/vm_create.log"
    echo "$i,create_failed,0,$VM_NAME,$ZONE,unknown" >>"$RESULTS_DIR/results.csv"
    continue
  fi

  # Wait for SSH
  echo "  Waiting for SSH..."
  for attempt in $(seq 1 12); do
    if gcloud compute ssh "$VM_NAME" --zone="$ZONE" --project="$PROJECT" \
      --ssh-key-file=~/.ssh/google_compute_engine \
      --command="echo ready" 2>/dev/null; then
      break
    fi
    sleep 10
  done

  # GPU health check
  echo "  GPU health check..."
  if ! gcloud compute ssh "$VM_NAME" --zone="$ZONE" --project="$PROJECT" \
    --ssh-key-file=~/.ssh/google_compute_engine \
    --command="nvidia-smi --query-gpu=name,driver_version,memory.total --format=csv,noheader" \
    2>/dev/null >"$ITER_DIR/gpu_pre.txt"; then
    echo "  GPU health check FAILED at startup. Bad VM."
    echo "$i,gpu_check_failed,0,$VM_NAME,$ZONE,false" >>"$RESULTS_DIR/results.csv"
    cleanup_vm "$VM_NAME" "$ZONE"
    continue
  fi
  echo "  GPU: $(cat "$ITER_DIR/gpu_pre.txt")"

  # Transfer artifacts and repo
  echo "  Transferring artifacts..."
  gcloud compute scp "$ARTIFACT_TARBALL" "$VM_NAME":~/artifacts.tar.gz \
    --zone="$ZONE" --project="$PROJECT" --ssh-key-file=~/.ssh/google_compute_engine \
    2>/dev/null
  gcloud compute scp "$REPO_TARBALL" "$VM_NAME":~/repo.tar.gz \
    --zone="$ZONE" --project="$PROJECT" --ssh-key-file=~/.ssh/google_compute_engine \
    2>/dev/null

  # Setup and run test inside Docker container (matching CI exactly)
  echo "  Running tests ($CONFIG) inside container..."
  START_TIME="$(date +%s)"

  CONTAINER_IMAGE="ghcr.io/shader-slang/slang-linux-gpu-ci:v1.4.0"

  # Pre-authenticate docker on the VM for GHCR access
  if [ -n "$GHCR_TOKEN" ]; then
    gcloud compute ssh "$VM_NAME" --zone="$ZONE" --project="$PROJECT" \
      --ssh-key-file=~/.ssh/google_compute_engine \
      --command="echo '$GHCR_TOKEN' | docker login ghcr.io -u token --password-stdin" \
      2>/dev/null || true
  fi

  gcloud compute ssh "$VM_NAME" --zone="$ZONE" --project="$PROJECT" \
    --ssh-key-file=~/.ssh/google_compute_engine \
    --command="
set -e
cd ~

# Initialize GPU devices for Docker passthrough
sudo nvidia-smi > /dev/null 2>&1
sudo nvidia-modprobe -m 2>/dev/null || true

echo '=== Pre-test nvidia-smi ==='
nvidia-smi 2>&1

# Pull container image
if ! docker image inspect $CONTAINER_IMAGE > /dev/null 2>&1; then
    echo 'Pulling container image...'
    docker pull $CONTAINER_IMAGE 2>&1
fi

# Prepare workspace: extract repo and artifacts
mkdir -p workspace/slang && cd workspace/slang
tar xzf ~/repo.tar.gz --strip-components=1 2>/dev/null || true
tar xzf ~/artifacts.tar.gz --strip-components=1 2>/dev/null || true
chmod +x ${CMAKE_CONFIG}/bin/* 2>/dev/null || true
cd ~

echo '=== Starting test run in container ==='
# Run tests inside container with GPU passthrough (matching CI container options)
set +e
docker run --rm \
    --gpus all \
    --user root \
    --device /dev/nvidia-modeset:/dev/nvidia-modeset \
    --device /dev/dri:/dev/dri \
    -e NVIDIA_DRIVER_CAPABILITIES=compute,utility,graphics \
    -e SLANG_RUN_SPIRV_VALIDATION=1 \
    -e SLANG_USE_SPV_SOURCE_LANGUAGE_UNKNOWN=1 \
    -v /etc/vulkan/icd.d/nvidia_icd.json:/etc/vulkan/icd.d/nvidia_icd.json:ro \
    -v /usr/share/nvidia:/usr/share/nvidia:ro \
    -v /usr/share/glvnd/egl_vendor.d/10_nvidia.json:/usr/share/glvnd/egl_vendor.d/10_nvidia.json:ro \
    -v \$HOME/workspace/slang:/workspace \
    -w /workspace \
    $CONTAINER_IMAGE \
    bash -c '
        export LD_LIBRARY_PATH=/workspace/${CMAKE_CONFIG}/lib:\$LD_LIBRARY_PATH
        timeout 900 /workspace/${CMAKE_CONFIG}/bin/slang-test \
            -category full \
            -expected-failure-list tests/expected-failure-github.txt \
            -expected-failure-list tests/expected-failure-linux-gpu.txt \
            -skip-reference-image-generation \
            -show-adapter-info \
            -ignore-abort-msg \
            -use-test-server \
            -server-count 4 \
            2>&1
    '
SLANG_EXIT=\$?
set -e

echo ''
echo '=== EXIT_CODE: '\$SLANG_EXIT' ==='
echo ''
echo '=== Post-test nvidia-smi ==='
nvidia-smi 2>&1 || echo 'NVIDIA-SMI FAILED'
echo ''
echo '=== Post-test dmesg GPU ==='
sudo dmesg 2>/dev/null | grep -iE 'nvidia|nvrm|xid|gpu|fault' | tail -30 || echo '(no dmesg access)'
" 2>&1 | tee "$ITER_DIR/test_output.log"

  END_TIME="$(date +%s)"
  DURATION="$((END_TIME - START_TIME))"

  # Determine result
  if grep -q "Test run aborted: too many consecutive failures" "$ITER_DIR/test_output.log"; then
    EXIT_CODE="aborted"
    echo "  *** ABORT TRIGGERED — driver crash detected!"
  elif grep -q "CUDA_ERROR_NO_DEVICE\|NVIDIA-SMI FAILED\|CUDA_ERROR_NOT_PERMITTED" "$ITER_DIR/test_output.log"; then
    EXIT_CODE="gpu_crash"
    echo "  *** GPU CRASH detected (no early abort — check threshold)"
  elif grep -q "EXIT_CODE: 0" "$ITER_DIR/test_output.log"; then
    EXIT_CODE="pass"
  else
    EXIT_CODE="fail"
  fi

  # Post-test GPU health
  GPU_HEALTHY="unknown"
  if grep -q "NVIDIA-SMI FAILED" "$ITER_DIR/test_output.log"; then
    GPU_HEALTHY="false"
  elif grep -q "Post-test nvidia-smi" "$ITER_DIR/test_output.log"; then
    GPU_HEALTHY="true"
  fi

  echo "  Result: $EXIT_CODE (duration: ${DURATION}s, gpu_healthy_after: $GPU_HEALTHY)"
  echo "$i,$EXIT_CODE,$DURATION,$VM_NAME,$ZONE,$GPU_HEALTHY" >>"$RESULTS_DIR/results.csv"

  # Cleanup
  cleanup_vm "$VM_NAME" "$ZONE"
done

echo ""
echo "========================================"
echo "  GPU Stress Test Loop Complete"
echo "========================================"
echo ""
cat "$RESULTS_DIR/results.csv"
echo ""
TOTAL="$ITERATIONS"
PASS="$(grep -c ',pass,' "$RESULTS_DIR/results.csv" || echo 0)"
ABORT="$(grep -c ',aborted,' "$RESULTS_DIR/results.csv" || echo 0)"
CRASH="$(grep -c ',gpu_crash,' "$RESULTS_DIR/results.csv" || echo 0)"
FAIL="$(grep -c ',fail,' "$RESULTS_DIR/results.csv" || echo 0)"
echo "Total: $TOTAL, Pass: $PASS, Abort: $ABORT, GPU Crash: $CRASH, Other Fail: $FAIL"
echo "Results: $RESULTS_DIR"
echo "========================================"
