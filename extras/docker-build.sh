#!/usr/bin/env bash
#
# docker-build.sh - Build Slang in Linux GPU CI container
#
# This script builds Slang using the same Docker container environment as the
# GitHub Actions CI runners, making it easy to reproduce CI builds locally and
# debug build issues.
#
# Prerequisites: Docker installed and running
#
# Usage:
#   ./extras/docker-build.sh [options]
#
# Options:
#   --config <debug|release>        Build configuration (default: debug)
#   --llvm <disable|fetch|cached>   LLVM backend mode (default: disable)
#   --clean                         Remove build directory before building
#   --no-pull                       Skip pulling container image (use local)
#   --github-token <token>          GitHub token for API access
#   --help                          Show this help message
#
# LLVM Modes:
#   disable - No LLVM backend, fastest builds (matches GPU CI)
#   fetch   - Download prebuilt slang-llvm from GitHub releases
#   cached  - Use host's pre-built LLVM at build/llvm-project-install/
#
# Examples:
#   ./extras/docker-build.sh
#   ./extras/docker-build.sh --config release
#   ./extras/docker-build.sh --llvm fetch
#   ./extras/docker-build.sh --llvm cached --config release
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
LLVM_MODE="disable"
CLEAN=false
NO_PULL=false
GITHUB_TOKEN="${GITHUB_TOKEN:-}"
CONTAINER_IMAGE="ghcr.io/shader-slang/slang-linux-gpu-ci:v1.3.0"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
  --config)
    CONFIG="$2"
    shift 2
    ;;
  --llvm)
    LLVM_MODE="$2"
    shift 2
    ;;
  --clean)
    CLEAN=true
    shift
    ;;
  --no-pull)
    NO_PULL=true
    shift
    ;;
  --github-token)
    GITHUB_TOKEN="$2"
    shift 2
    ;;
  --help)
    head -n 40 "$0" | grep "^#" | sed 's/^# //' | sed 's/^#//'
    exit 0
    ;;
  *)
    echo -e "${RED}Unknown option: $1${NC}"
    echo "Use --help for usage information"
    exit 1
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

# Map configuration name to CMake preset and build directory
case "$CONFIG" in
debug)
  CMAKE_CONFIG="Debug"
  BUILD_PRESET="debug"
  ;;
release)
  CMAKE_CONFIG="Release"
  BUILD_PRESET="release"
  ;;
*)
  log_error "Invalid configuration: $CONFIG"
  log_info "Valid options: debug, release"
  exit 1
  ;;
esac

BUILD_DIR="build/$CMAKE_CONFIG"
BIN_DIR="$BUILD_DIR/bin"

log_info "Build configuration: $CMAKE_CONFIG"
log_info "Build directory: $BUILD_DIR"

# ============================================================================
# Step 3: Setup LLVM configuration
# ============================================================================

log_info "Configuring LLVM backend mode: $LLVM_MODE"

LLVM_CMAKE_ARGS=""
LLVM_DOCKER_MOUNTS=""
LLVM_ENV_VARS=""

case "$LLVM_MODE" in
disable)
  LLVM_CMAKE_ARGS="-DSLANG_SLANG_LLVM_FLAVOR=DISABLE"
  log_info "LLVM mode: DISABLE (fastest, no LLVM backend)"
  log_info "This matches the GPU CI configuration"
  ;;
fetch)
  LLVM_CMAKE_ARGS="-DSLANG_SLANG_LLVM_FLAVOR=FETCH_BINARY_IF_POSSIBLE"
  if [ -n "$GITHUB_TOKEN" ]; then
    LLVM_ENV_VARS="-e SLANG_GITHUB_TOKEN=$GITHUB_TOKEN"
    log_info "LLVM mode: FETCH (download prebuilt from GitHub releases, with token)"
  else
    log_info "LLVM mode: FETCH (download prebuilt from GitHub releases)"
    log_warning "No GitHub token provided, API rate limits may apply"
    log_info "Set GITHUB_TOKEN environment variable or use --github-token to avoid limits"
  fi
  ;;
cached)
  if [ ! -d "$REPO_ROOT/build/llvm-project-install" ]; then
    log_error "LLVM cache not found at: $REPO_ROOT/build/llvm-project-install"
    log_info "Please build LLVM first using:"
    log_info "  ./external/build-llvm.sh --install-prefix \$(pwd)/build/llvm-project-install"
    log_info "Or use --llvm fetch to download a prebuilt binary"
    exit 1
  fi
  LLVM_CMAKE_ARGS="-DSLANG_SLANG_LLVM_FLAVOR=USE_SYSTEM_LLVM"
  LLVM_DOCKER_MOUNTS="-v $REPO_ROOT/build/llvm-project-install:/workspace/build/llvm-project-install:ro"
  LLVM_ENV_VARS="-e LLVM_DIR=/workspace/build/llvm-project-install -e Clang_DIR=/workspace/build/llvm-project-install"
  log_info "LLVM mode: CACHED (using host's cached LLVM build)"
  log_success "Found LLVM cache at: build/llvm-project-install"
  ;;
*)
  log_error "Invalid LLVM mode: $LLVM_MODE"
  log_info "Valid options: disable, fetch, cached"
  exit 1
  ;;
esac

# ============================================================================
# Step 4: Clean build directory if requested
# ============================================================================

if [ "$CLEAN" = true ]; then
  if [ -d "$BUILD_DIR" ]; then
    log_warning "Cleaning build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
    log_success "Build directory cleaned"
  else
    log_info "Build directory does not exist, nothing to clean"
  fi
fi

# ============================================================================
# Step 5: Pull container image
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
# Step 6: Build Slang in container
# ============================================================================

log_info "Starting build in container..."
log_info "This may take 2-10 minutes depending on your system and LLVM mode"

start_time=$(date +%s)

# Build the docker run command
DOCKER_CMD="docker run --rm \
  --gpus all \
  --user $(id -u):$(id -g) \
  -v $REPO_ROOT:/workspace \
  -w /workspace \
  $LLVM_ENV_VARS \
  $LLVM_DOCKER_MOUNTS \
  $CONTAINER_IMAGE \
  bash -c 'cmake --preset default --fresh \
    -DSLANG_ENABLE_CUDA=ON \
    $LLVM_CMAKE_ARGS && \
  cmake --build --preset $BUILD_PRESET -j\$(nproc)'"

# Execute build
if eval "$DOCKER_CMD"; then
  end_time=$(date +%s)
  duration=$((end_time - start_time))
  minutes=$((duration / 60))
  seconds=$((duration % 60))

  log_success "Build completed in ${minutes}m ${seconds}s"

  # Verify output
  if [ -f "$BIN_DIR/slangc" ]; then
    log_success "Build artifacts created in: $BIN_DIR"

    # Show version
    log_info "Slangc version:"
    "$BIN_DIR/slangc" -version || log_warning "Failed to get slangc version"

    # List key binaries
    log_info "Key binaries built:"
    for binary in slangc slang-test slangi; do
      if [ -f "$BIN_DIR/$binary" ]; then
        echo "  - $BIN_DIR/$binary"
      fi
    done
  else
    log_error "Build artifacts not found in expected location: $BIN_DIR"
    log_error "Build may have failed silently"
    exit 1
  fi
else
  log_error "Build failed"
  log_info "Check the output above for error details"
  exit 1
fi

# ============================================================================
# Success!
# ============================================================================

echo ""
echo "================================================"
log_success "Slang build completed successfully"
echo "================================================"
echo ""
log_info "Build configuration: $CMAKE_CONFIG"
log_info "LLVM mode: $LLVM_MODE"
log_info "Build artifacts: $BUILD_DIR/"
echo ""
log_info "Next steps:"
log_info "  - Run quick tests: ./extras/docker-test.sh --category quick"
log_info "  - Run full tests:  ./extras/docker-test.sh --category full"
log_info "  - Use slangc:      $BIN_DIR/slangc -help"
echo ""
