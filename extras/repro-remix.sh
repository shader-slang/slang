#!/usr/bin/env bash
#
# repro-remix.sh - Reproduce RTX Remix shader compilation workflow
#
# This script replicates the steps from .github/workflows/compile-rtx-remix-shaders-nightly.yml
# to help debug issues when the nightly workflow fails.
#
# Prerequisites: Slang must already be built in Debug configuration at build/Debug/bin/
#
# Usage:
#   ./extras/repro-remix.sh [--clean]
#
# Options:
#   --clean        Remove existing dxvk-remix clone and start fresh
#   --help         Show this help message
#

set -e # Exit on error
set -u # Exit on undefined variable

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script options
CLEAN=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
  --clean)
    CLEAN=true
    shift
    ;;
  --help)
    head -n 20 "$0" | grep "^#" | sed 's/^# //' | sed 's/^#//'
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
# Step 1: Verify Slang Debug Build
# ============================================================================

log_info "Verifying existing Slang Debug build..."

# Verify slangc exists in Debug build
SLANGC_PATH="$REPO_ROOT/build/Debug/bin/slangc.exe"
if [ ! -f "$SLANGC_PATH" ]; then
  log_error "slangc.exe not found at: $SLANGC_PATH"
  log_error "Please build Slang in Debug configuration first:"
  log_error "  cmake.exe --preset default"
  log_error "  cmake.exe --build --preset debug"
  exit 1
fi

log_success "Found slangc at: $SLANGC_PATH"
log_info "Slangc version:"
"$SLANGC_PATH" -version

# ============================================================================
# Step 2: Clone dxvk-remix
# ============================================================================

REMIX_DIR="$REPO_ROOT/external/dxvk-remix"

if [ "$CLEAN" = true ] && [ -d "$REMIX_DIR" ]; then
  log_warning "Cleaning existing dxvk-remix clone..."
  rm -rf "$REMIX_DIR"
fi

if [ -d "$REMIX_DIR" ]; then
  log_info "dxvk-remix already cloned at: $REMIX_DIR"
  cd "$REMIX_DIR"
  COMMIT=$(git rev-parse HEAD)
  log_info "Current commit: $COMMIT"
else
  log_info "Cloning dxvk-remix repository..."
  mkdir -p "$REPO_ROOT/external"
  cd "$REPO_ROOT/external"
  git clone --recursive --depth 1 https://github.com/NVIDIAGameWorks/dxvk-remix.git
  cd dxvk-remix
  COMMIT=$(git rev-parse HEAD)
  log_success "Cloned dxvk-remix at commit: $COMMIT"
fi

# ============================================================================
# Step 3: Comment out Slang in packman-external.xml
# ============================================================================

log_info "Commenting out Slang section in packman-external.xml..."
cd "$REMIX_DIR"

PACKMAN_XML="packman-external.xml"

# Check if file exists
if [ ! -f "$PACKMAN_XML" ]; then
  log_error "packman-external.xml not found at: $REMIX_DIR/$PACKMAN_XML"
  exit 1
fi

# Create backup
cp "$PACKMAN_XML" "$PACKMAN_XML.backup"
log_info "Created backup: $PACKMAN_XML.backup"

# Check if Slang section is already commented out
if grep -q "<!-- Slang section commented out by repro-remix.sh" "$PACKMAN_XML"; then
  log_info "Slang section already commented out (script was run before)"
else
  log_info "Commenting out Slang section..."
  # Comment out the Slang dependency section
  # This prevents packman from downloading/managing Slang
  awk '
    /<dependency name="slang"/ { print "    <!-- Slang section commented out by repro-remix.sh"; in_slang=1 }
    in_slang { print }
    /<\/dependency>/ && in_slang { print "    -->"; in_slang=0; next }
    !in_slang { print }
    ' "$PACKMAN_XML" >"$PACKMAN_XML.tmp"
  mv "$PACKMAN_XML.tmp" "$PACKMAN_XML"
  log_success "Slang section commented out in packman-external.xml"
fi

# ============================================================================
# Step 4: Download packman dependencies (excluding Slang)
# ============================================================================

log_info "Downloading packman dependencies (Slang excluded)..."

# Run packman (will skip Slang now)
log_info "Running packman pull..."
cmd.exe /C "scripts-common\\packman\\packman.cmd pull packman-external.xml"

log_success "Packman dependencies downloaded"

# ============================================================================
# Step 5: Create Slang directory with our Debug build
# ============================================================================

log_info "Setting up custom Slang directory with Debug build..."

REMIX_SLANG_DIR="$REMIX_DIR/external/slang"

# Create the directory (packman didn't create it since we commented it out)
mkdir -p "$REMIX_SLANG_DIR"
log_info "Created directory: $REMIX_SLANG_DIR"

# Copy our built Slang binaries
BUILT_SLANG_BIN="$REPO_ROOT/build/Debug/bin"
log_info "Copying Slang binaries from: $BUILT_SLANG_BIN"

# Copy all files recursively
cp -rf "$BUILT_SLANG_BIN"/* "$REMIX_SLANG_DIR"/

# Verify copy
log_info "Verifying Slang binaries..."

# Check required files
for file in slangc.exe slang-compiler.dll slang-glslang.dll; do
  if [ ! -f "$REMIX_SLANG_DIR/$file" ]; then
    log_error "Required file not found after copy: $file"
    exit 1
  fi
done

# Verify version
SLANGC="$REMIX_SLANG_DIR/slangc.exe"
log_info "Slangc version:"
SLANG_VERSION=$("$SLANGC" -version 2>&1)
echo "$SLANG_VERSION"

# Check that version looks valid
if [[ "$SLANG_VERSION" =~ ^[0-9]{10,}$ ]]; then
  log_error "Slangc version appears invalid (timestamp only: $SLANG_VERSION)"
  log_error "This suggests the build failed to detect Git version tags"
  exit 1
fi

log_success "Custom Slang directory ready - dxvk-remix will use Debug build"

# ============================================================================
# Step 6: Build shaders
# ============================================================================

log_info "Building RTX Remix shaders..."
log_info "SPIRV validation enabled via SLANG_RUN_SPIRV_VALIDATION=1"

cd "$REMIX_DIR"

# Export environment variables for shader compilation
export SLANG_RUN_SPIRV_VALIDATION=1
export SLANG_USE_SPV_SOURCE_LANGUAGE_UNKNOWN=1

# Run shader build script
log_info "Running build_shaders_only.ps1..."
start_time=$(date +%s)

# Run PowerShell script
#powershell.exe -ExecutionPolicy Bypass -File "./build_shaders_only.ps1"
powershell.exe ".\\build_shaders_only.ps1"
EXIT_CODE=$?

end_time=$(date +%s)
duration=$(((end_time - start_time) / 60))

if [ $EXIT_CODE -ne 0 ]; then
  log_error "Shader build failed with exit code $EXIT_CODE"
  log_error "Check logs at: $REMIX_DIR/_BuildShadersOnly/meson-logs/"
  exit $EXIT_CODE
fi

log_success "Shader build completed in $duration minutes"

# ============================================================================
# Step 7: Verify shader compilation output
# ============================================================================

log_info "Verifying shader compilation output..."

SHADER_OUTPUT="$REMIX_DIR/_BuildShadersOnly/src/dxvk/rtx_shaders"
if [ ! -d "$SHADER_OUTPUT" ]; then
  log_error "Shader output directory not found: $SHADER_OUTPUT"
  exit 1
fi

cd "$SHADER_OUTPUT"

# Count generated files (use find for Windows compatibility)
HEADERS=$(find . -maxdepth 1 -name "*.h" 2>/dev/null | wc -l | xargs)
SPV_FILES=$(find . -maxdepth 1 -name "*.spv" 2>/dev/null | wc -l | xargs)

log_info "Compiled shader headers (.h): $HEADERS"
log_info "SPIR-V binary files (.spv): $SPV_FILES"

# Validate output
if [ "${HEADERS:-0}" -eq 0 ] || [ "${SPV_FILES:-0}" -eq 0 ]; then
  log_error "No shader files generated"
  log_error "This indicates shader compilation failed completely"
  exit 1
fi

# Sanity check: RTX Remix should have at least some reasonable number of shaders
# Shader count is expected to stay stable or increase over time
if [ "${HEADERS:-0}" -lt 100 ] || [ "${SPV_FILES:-0}" -lt 100 ]; then
  log_error "Unexpectedly low shader count: $HEADERS headers, $SPV_FILES .spv files"
  log_error "Expected at least 100 shaders - this indicates a partial build failure"
  exit 1
fi

# ============================================================================
# Success!
# ============================================================================

echo ""
echo "================================================"
log_success "RTX Remix shader compilation test PASSED"
echo "================================================"
echo ""
log_info "Summary:"
log_info "  - Compiled shader headers: $HEADERS"
log_info "  - SPIR-V binary files: $SPV_FILES"
log_info "  - Output location: $SHADER_OUTPUT"
echo ""
log_success "Slang compiler is compatible with RTX Remix shaders"
