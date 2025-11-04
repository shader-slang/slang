#!/bin/bash
# Script to verify that external/spirv-tools-generated/ files are up-to-date
# when external/spirv-tools or external/spirv-headers are modified.
#
# This is used by CI to ensure developers regenerate files after updating submodules.
# See docs/update_spirv.md for manual update instructions.

set -e

# Color output helpers
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo_info() {
  echo -e "${BLUE}ℹ️  $1${NC}"
}

echo_success() {
  echo -e "${GREEN}✅ $1${NC}"
}

echo_error() {
  echo -e "${RED}❌ $1${NC}"
}

echo_warning() {
  echo -e "${YELLOW}⚠️  $1${NC}"
}

# 1. Check if spirv-tools or spirv-headers were modified
echo_info "Checking if SPIRV directories were modified..."

if [ -n "$GITHUB_BASE_REF" ]; then
  # Running in GitHub Actions
  BASE_REF="origin/$GITHUB_BASE_REF"
  echo_info "Running in CI mode, comparing against $BASE_REF"
elif [ -n "$1" ]; then
  # User provided base ref
  BASE_REF="$1"
  echo_info "Comparing against provided ref: $BASE_REF"
else
  # Try to detect default branch
  DEFAULT_BRANCH=$(git symbolic-ref refs/remotes/origin/HEAD 2>/dev/null | sed 's@^refs/remotes/origin/@@' || echo "master")
  BASE_REF="origin/$DEFAULT_BRANCH"
  echo_info "Comparing against detected default branch: $BASE_REF"
fi

# Ensure we have the base ref
if ! git rev-parse "$BASE_REF" >/dev/null 2>&1; then
  echo_warning "Base ref $BASE_REF not found, fetching..."
  git fetch origin
fi

# Check what files changed
CHANGED_FILES=$(git diff --name-only "$BASE_REF"...HEAD 2>/dev/null || echo "")

if [ -z "$CHANGED_FILES" ]; then
  echo_warning "Could not determine changed files, assuming SPIRV may have changed"
  SPIRV_CHANGED=true
else
  SPIRV_CHANGED=false
  if echo "$CHANGED_FILES" | grep -q "^external/spirv-tools"; then
    echo_info "Detected changes in external/spirv-tools"
    SPIRV_CHANGED=true
  fi
  if echo "$CHANGED_FILES" | grep -q "^external/spirv-headers"; then
    echo_info "Detected changes in external/spirv-headers"
    SPIRV_CHANGED=true
  fi
fi

if [ "$SPIRV_CHANGED" = false ]; then
  echo_success "No changes to external/spirv-tools or external/spirv-headers - skipping check"
  exit 0
fi

echo ""
echo_info "Changes detected in SPIRV directories - verifying generated files are up-to-date..."
echo ""

# 2. Initialize submodules
echo_info "Initializing SPIRV submodules..."
git submodule update --init --recursive external/spirv-tools
git submodule update --init --recursive external/spirv-headers

# 3. Sync spirv-tools dependencies
echo_info "Syncing spirv-tools dependencies..."
cd external/spirv-tools

if [ ! -f "utils/git-sync-deps" ]; then
  echo_error "utils/git-sync-deps not found in external/spirv-tools"
  exit 1
fi

# git-sync-deps needs to be run to get additional dependencies
python3 utils/git-sync-deps

cd ../..

# 4. Build spirv-tools to generate files
BUILD_DIR="external/spirv-tools/build-ci-check"
echo_info "Building spirv-tools generation targets in $BUILD_DIR..."

mkdir -p "$BUILD_DIR"

# Configure CMake (suppress warnings from external project)
cmake -Wno-dev -B "$BUILD_DIR" external/spirv-tools

# Build only the generation targets (faster than full build)
echo_info "Building generation targets (this may take a few minutes)..."
cmake --build "$BUILD_DIR" \
  --target spirv-tools-build-version \
  --target core_tables \
  --target extinst_tables

# 5. Compare generated files
echo_info "Comparing generated files..."
TEMP_DIR=$(mktemp -d)

# Copy generated files to temp directory
cp "$BUILD_DIR"/*.inc "$TEMP_DIR/" 2>/dev/null || true
cp "$BUILD_DIR"/*.h "$TEMP_DIR/" 2>/dev/null || true

# Count files for verification
GENERATED_COUNT=$(ls -1 "$TEMP_DIR" 2>/dev/null | wc -l)
echo_info "Found $GENERATED_COUNT generated files in build directory"

if [ "$GENERATED_COUNT" -eq 0 ]; then
  echo_error "No generated files found in $BUILD_DIR"
  echo_error "Build may have failed or generation targets did not produce output"
  rm -rf "$BUILD_DIR"
  rm -rf "$TEMP_DIR"
  exit 1
fi

# 6. Check for differences
DIFF_FOUND=false
MISSING_FILES=()
DIFFERENT_FILES=()

for file in "$TEMP_DIR"/*; do
  filename=$(basename "$file")

  # Skip if it's just the temp directory itself
  if [ ! -f "$file" ]; then
    continue
  fi

  TARGET_FILE="external/spirv-tools-generated/$filename"

  if [ ! -f "$TARGET_FILE" ]; then
    echo_error "Missing file in spirv-tools-generated: $filename"
    MISSING_FILES+=("$filename")
    DIFF_FOUND=true
  elif ! diff -q "$file" "$TARGET_FILE" >/dev/null 2>&1; then
    echo_error "File differs: $filename"
    DIFFERENT_FILES+=("$filename")
    # Show a snippet of the diff (first 20 lines)
    echo "--- Diff preview for $filename ---"
    diff -u "$TARGET_FILE" "$file" | head -20 || true
    echo "--- End diff preview ---"
    echo ""
    DIFF_FOUND=true
  else
    echo_success "File matches: $filename"
  fi
done

# 7. Check for orphaned files
ORPHANED_FILES=()
for file in external/spirv-tools-generated/*; do
  filename=$(basename "$file")

  # Skip README.md as it's documentation
  if [ "$filename" = "README.md" ]; then
    continue
  fi

  # Skip if it's a directory
  if [ ! -f "$file" ]; then
    continue
  fi

  if [ ! -f "$TEMP_DIR/$filename" ]; then
    echo_error "Orphaned file in spirv-tools-generated (should be removed): $filename"
    ORPHANED_FILES+=("$filename")
    DIFF_FOUND=true
  fi
done

# 8. Cleanup
echo_info "Cleaning up build artifacts..."
rm -rf "$BUILD_DIR"
rm -rf "$TEMP_DIR"

# 9. Report results
echo ""
echo "=================================================="
if [ "$DIFF_FOUND" = true ]; then
  echo_error "Generated files are out of sync!"
  echo ""

  if [ ${#MISSING_FILES[@]} -gt 0 ]; then
    echo_error "Missing files (need to be added):"
    for file in "${MISSING_FILES[@]}"; do
      echo "  - $file"
    done
    echo ""
  fi

  if [ ${#DIFFERENT_FILES[@]} -gt 0 ]; then
    echo_error "Files with differences (need to be updated):"
    for file in "${DIFFERENT_FILES[@]}"; do
      echo "  - $file"
    done
    echo ""
  fi

  if [ ${#ORPHANED_FILES[@]} -gt 0 ]; then
    echo_error "Orphaned files (need to be removed):"
    for file in "${ORPHANED_FILES[@]}"; do
      echo "  - $file"
    done
    echo ""
  fi

  echo ""
  echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
  echo_info "How to fix this issue:"
  echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
  echo ""
  echo "Option 1: Use the automated script (recommended):"
  echo "  ${GREEN}bash external/bump-glslang.sh${NC}"
  echo ""
  echo "Option 2: Manual steps:"
  echo "  ${GREEN}cd external/spirv-tools${NC}"
  echo "  ${GREEN}python3 utils/git-sync-deps${NC}"
  echo "  ${GREEN}cmake . -B build${NC}"
  echo "  ${GREEN}cmake --build build --target spirv-tools-build-version --target core_tables --target extinst_tables${NC}"
  echo "  ${GREEN}cd ../..${NC}"
  echo "  ${GREEN}rm external/spirv-tools-generated/*.h external/spirv-tools-generated/*.inc${NC}"
  echo "  ${GREEN}cp external/spirv-tools/build/*.h external/spirv-tools-generated/${NC}"
  echo "  ${GREEN}cp external/spirv-tools/build/*.inc external/spirv-tools-generated/${NC}"
  echo "  ${GREEN}git add external/spirv-tools-generated${NC}"
  echo ""
  echo "For more details, see: ${BLUE}docs/update_spirv.md${NC}"
  echo ""
  exit 1
else
  echo_success "All generated files are up-to-date!"
  echo ""
  echo "All checks passed:"
  echo "  ✓ All generated files are present"
  echo "  ✓ All generated files match build output"
  echo "  ✓ No orphaned files detected"
  echo ""
fi

exit 0
