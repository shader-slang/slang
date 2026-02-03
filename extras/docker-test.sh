#!/usr/bin/env bash
#
# docker-test.sh - Run Slang tests in Linux GPU CI container
#
# This script runs Slang tests using the same Docker container environment as the
# GitHub Actions CI runners, making it easy to reproduce CI test results locally.
#
# Prerequisites: Docker installed, Slang built (run docker-build.sh first)
#
# Usage:
#   ./extras/docker-test.sh [options] [test-prefix]
#
# Options:
#   --config <debug|release>    Build configuration to test (default: debug)
#   --category <name>           Test category: quick, full, smoke (default: quick)
#   --no-gpu                    Skip GPU device mounts (for CPU-only tests)
#   --server-count <n>          Number of test servers (default: 4)
#   --show-adapter-info         Show detailed GPU adapter information
#   --no-pull                   Skip pulling container image (use local)
#   --help                      Show this help message
#
# Test Categories:
#   quick - Fast CPU tests, no GPU required (~5-10 min)
#   full  - Complete CI test suite with GPU (~30-60 min)
#   smoke - Minimal sanity checks (~1-2 min)
#
# Examples:
#   ./extras/docker-test.sh
#   ./extras/docker-test.sh --category full
#   ./extras/docker-test.sh tests/language-feature/lambda/
#   ./extras/docker-test.sh --config release --no-gpu
#

set -e # Exit on error
set -u # Exit on undefined variable

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script options with defaults
CONFIG="debug"
CATEGORY="quick"
NO_GPU=false
SERVER_COUNT="4"
SHOW_ADAPTER_INFO=false
NO_PULL=false
TEST_PREFIX=""
CONTAINER_IMAGE="ghcr.io/shader-slang/slang-linux-gpu-ci:v1.3.0"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
  --config)
    CONFIG="$2"
    shift 2
    ;;
  --category)
    CATEGORY="$2"
    shift 2
    ;;
  --no-gpu)
    NO_GPU=true
    shift
    ;;
  --server-count)
    SERVER_COUNT="$2"
    shift 2
    ;;
  --show-adapter-info)
    SHOW_ADAPTER_INFO=true
    shift
    ;;
  --no-pull)
    NO_PULL=true
    shift
    ;;
  --help)
    head -n 35 "$0" | grep "^#" | sed 's/^# //' | sed 's/^#//'
    exit 0
    ;;
  -*)
    echo -e "${RED}Unknown option: $1${NC}"
    echo "Use --help for usage information"
    exit 1
    ;;
  *)
    # Treat as test prefix
    TEST_PREFIX="$1"
    shift
    ;;
  esac
done

# Helper functions
log_info() {
  echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
  echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
  echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
  echo -e "${RED}[ERROR]${NC} $1"
}

# Detect repository root (go up until we find CMakeLists.txt)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT"

log_info "Repository root: $REPO_ROOT"

# Verify we're in the right place
if [ ! -f "CMakeLists.txt" ]; then
  log_error "This script must be run from the Slang repository root"
  exit 1
fi

# ============================================================================
# Step 1: Verify Docker is available
# ============================================================================

log_info "Checking Docker availability..."

if ! command -v docker &>/dev/null; then
  log_error "Docker not found. Please install Docker first."
  log_info "Visit: https://docs.docker.com/get-docker/"
  exit 1
fi

if ! docker ps &>/dev/null; then
  log_error "Docker daemon not running or permission denied"
  log_info "Make sure Docker is running and you have permission to use it"
  log_info "You may need to add yourself to the 'docker' group:"
  log_info "  sudo usermod -aG docker \$USER"
  log_info "Then log out and back in for the change to take effect"
  exit 1
fi

log_success "Docker is available"

# ============================================================================
# Step 2: Validate configuration
# ============================================================================

log_info "Validating configuration..."

# Map configuration name to build directory
case "$CONFIG" in
debug)
  CMAKE_CONFIG="Debug"
  ;;
release)
  CMAKE_CONFIG="Release"
  ;;
*)
  log_error "Invalid configuration: $CONFIG"
  log_info "Valid options: debug, release"
  exit 1
  ;;
esac

BUILD_DIR="build/$CMAKE_CONFIG"
BIN_DIR="$BUILD_DIR/bin"

# Verify build artifacts exist
if [ ! -f "$BIN_DIR/slang-test" ]; then
  log_error "Build artifacts not found: $BIN_DIR/slang-test"
  log_info "Please build Slang first using: ./extras/docker-build.sh --config $CONFIG"
  exit 1
fi

log_success "Found build artifacts in: $BIN_DIR"

# ============================================================================
# Step 3: Check GPU availability
# ============================================================================

HAS_GPU=false

if [ "$NO_GPU" = true ]; then
  log_info "CPU-only mode requested (--no-gpu), skipping GPU detection"
else
  log_info "Checking for NVIDIA GPU..."

  if command -v nvidia-smi &>/dev/null; then
    if nvidia-smi &>/dev/null; then
      HAS_GPU=true
      GPU_INFO=$(nvidia-smi --query-gpu=name,driver_version --format=csv,noheader 2>/dev/null || echo "unknown")
      log_success "NVIDIA GPU detected: $GPU_INFO"
    else
      log_warning "nvidia-smi found but failed to run (driver issue?)"
    fi
  else
    log_warning "No NVIDIA GPU detected (nvidia-smi not found)"
  fi

  # Auto-enable --no-gpu if no GPU detected
  if [ "$HAS_GPU" = false ]; then
    log_warning "Automatically enabling --no-gpu mode (no GPU available)"
    NO_GPU=true
  fi
fi

# ============================================================================
# Step 4: Configure test parameters
# ============================================================================

log_info "Configuring test parameters..."

# Configure test options based on category
SLANG_TEST_ARGS=""
case "$CATEGORY" in
quick)
  # Quick tests
  log_info "Test category: QUICK (quick tests)"
  SLANG_TEST_ARGS="-category quick"
  ;;
full)
  # Full CI test suite
  log_info "Test category: FULL (complete CI test suite)"
  SLANG_TEST_ARGS="-category full \
    -expected-failure-list tests/expected-failure-github.txt \
    -expected-failure-list tests/expected-failure-linux-gpu.txt \
    -skip-reference-image-generation \
    -ignore-abort-msg"

  if [ "$NO_GPU" = true ]; then
    log_error "Full tests require a GPU, but --no-gpu was specified"
    log_info "Remove --no-gpu flag or use --category quick for CPU-only tests"
    exit 1
  fi
  ;;
smoke)
  # Minimal sanity checks
  log_info "Test category: SMOKE (minimal sanity checks)"
  SLANG_TEST_ARGS="-category smoke"
  ;;
*)
  log_error "Invalid test category: $CATEGORY"
  log_info "Valid options: quick, full, smoke"
  exit 1
  ;;
esac

# In CPU-only mode, restrict to CPU API only and skip API detection
# Note: Even without GPU passthrough, the container has Vulkan libraries installed,
# so slang-test might detect GPU APIs as available. We explicitly restrict to CPU only
# and skip the detection phase to avoid attempting GPU initialization.
if [ "$NO_GPU" = true ]; then
  log_info "CPU-only mode: Restricting to CPU API only and skipping API detection"
  SLANG_TEST_ARGS="$SLANG_TEST_ARGS -api cpu -skip-api-detection"

  # Add no-GPU expected failure list
  if [ -f "tests/expected-failure-no-gpu.txt" ]; then
    SLANG_TEST_ARGS="$SLANG_TEST_ARGS -expected-failure-list tests/expected-failure-no-gpu.txt"
  fi
fi

# Add test server configuration
SLANG_TEST_ARGS="$SLANG_TEST_ARGS -use-test-server -server-count $SERVER_COUNT"

# Add adapter info if requested
if [ "$SHOW_ADAPTER_INFO" = true ]; then
  SLANG_TEST_ARGS="$SLANG_TEST_ARGS -show-adapter-info"
fi

# Add test prefix if specified
if [ -n "$TEST_PREFIX" ]; then
  log_info "Running tests matching: $TEST_PREFIX"
  SLANG_TEST_ARGS="$SLANG_TEST_ARGS $TEST_PREFIX"
fi

log_info "Test server count: $SERVER_COUNT"

# ============================================================================
# Step 5: Setup GPU device mounts
# ============================================================================

GPU_DOCKER_ARGS=""

if [ "$NO_GPU" = true ]; then
  log_info "CPU-only mode: Skipping GPU device mounts"
else
  log_info "Configuring GPU device mounts..."

  # Check for Vulkan ICD files
  VULKAN_ICD="/etc/vulkan/icd.d/nvidia_icd.json"
  EGL_VENDOR="/usr/share/glvnd/egl_vendor.d/10_nvidia.json"

  if [ ! -f "$VULKAN_ICD" ]; then
    log_warning "Vulkan ICD file not found: $VULKAN_ICD"
    log_warning "Vulkan tests may fail"
  fi

  if [ ! -f "$EGL_VENDOR" ]; then
    log_warning "EGL vendor file not found: $EGL_VENDOR"
    log_warning "EGL tests may fail"
  fi

  # Build GPU docker args
  GPU_DOCKER_ARGS="--gpus all \
    --device /dev/nvidia-modeset:/dev/nvidia-modeset \
    --device /dev/dri:/dev/dri \
    -e NVIDIA_DRIVER_CAPABILITIES=compute,utility,graphics"

  # Mount Vulkan files if they exist
  if [ -f "$VULKAN_ICD" ]; then
    GPU_DOCKER_ARGS="$GPU_DOCKER_ARGS -v $VULKAN_ICD:$VULKAN_ICD:ro"
  fi

  if [ -d "/usr/share/nvidia" ]; then
    GPU_DOCKER_ARGS="$GPU_DOCKER_ARGS -v /usr/share/nvidia:/usr/share/nvidia:ro"
  fi

  if [ -f "$EGL_VENDOR" ]; then
    GPU_DOCKER_ARGS="$GPU_DOCKER_ARGS -v $EGL_VENDOR:$EGL_VENDOR:ro"
  fi

  log_success "GPU device mounts configured"
fi

# ============================================================================
# Step 6: Pull container image
# ============================================================================

if [ "$NO_PULL" = true ]; then
  log_info "Skipping container pull (--no-pull specified)"
else
  log_info "Pulling container image: $CONTAINER_IMAGE"
  if docker pull "$CONTAINER_IMAGE"; then
    log_success "Container image ready"
  else
    log_error "Failed to pull container image"
    log_info "You can use --no-pull to use a local image if you have one"
    exit 1
  fi
fi

# ============================================================================
# Step 7: Run tests in container
# ============================================================================

log_info "Starting tests in container..."
log_info "Test configuration: $CMAKE_CONFIG ($CATEGORY)"
log_info "Test arguments: $SLANG_TEST_ARGS"

start_time=$(date +%s)

# Build the docker run command
DOCKER_CMD="docker run --rm \
  $GPU_DOCKER_ARGS \
  --user $(id -u):$(id -g) \
  -e SLANG_RUN_SPIRV_VALIDATION=1 \
  -e SLANG_USE_SPV_SOURCE_LANGUAGE_UNKNOWN=1 \
  -v $REPO_ROOT:/workspace \
  -w /workspace \
  $CONTAINER_IMAGE \
  bash -c './$BIN_DIR/slang-test $SLANG_TEST_ARGS'"

# Execute tests
echo ""
echo "================================================"
echo " Running Slang Tests"
echo "================================================"
echo ""

if eval "$DOCKER_CMD"; then
  TEST_EXIT_CODE=0
else
  TEST_EXIT_CODE=$?
fi

end_time=$(date +%s)
duration=$((end_time - start_time))
minutes=$((duration / 60))
seconds=$((duration % 60))

# ============================================================================
# Report results
# ============================================================================

echo ""
echo "================================================"
if [ $TEST_EXIT_CODE -eq 0 ]; then
  log_success "All tests passed in ${minutes}m ${seconds}s"
  echo "================================================"
  echo ""
  log_info "Test configuration: $CMAKE_CONFIG"
  log_info "Test category: $CATEGORY"
  if [ "$NO_GPU" = true ]; then
    log_info "Test mode: CPU-only"
  else
    log_info "Test mode: GPU-enabled"
  fi
  echo ""
else
  log_error "Tests failed with exit code $TEST_EXIT_CODE"
  echo "================================================"
  echo ""
  log_info "Duration: ${minutes}m ${seconds}s"
  log_info "Check the output above for failure details"
  echo ""
  exit $TEST_EXIT_CODE
fi
