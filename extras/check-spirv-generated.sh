#!/bin/bash
# Script to verify that external/spirv-tools-generated/ files are up-to-date
# when external/spirv-tools or external/spirv-headers are modified.
#
# This is used by CI to ensure developers regenerate files after updating submodules.
# See docs/update_spirv.md for manual update instructions.

set -e

# Output helpers
echo_info() {
  echo "INFO: $1"
}

echo_success() {
  echo "SUCCESS: $1"
}

echo_error() {
  echo "ERROR: $1"
}

echo_warning() {
  echo "WARNING: $1"
}

echo_info "Verifying SPIRV-Tools generated files are up-to-date..."
echo ""

# 1. Initialize SPIRV-Tools submodule
echo_info "Initializing SPIRV-Tools submodule..."
git submodule update --init --recursive external/spirv-tools

# 2. Verify spirv-headers commit matches spirv-tools DEPS
echo_info "Verifying spirv-headers commit matches spirv-tools DEPS..."

# Get the expected commit from spirv-tools/DEPS
EXPECTED_COMMIT=$(grep spirv_headers_revision external/spirv-tools/DEPS | grep -o "'[a-f0-9]\{40\}'" | tr -d "'")

if [ -z "$EXPECTED_COMMIT" ]; then
  echo_error "Could not extract spirv_headers_revision from external/spirv-tools/DEPS"
  exit 1
fi

# Get the actual commit from spirv-headers submodule
ACTUAL_COMMIT=$(git submodule status external/spirv-headers | awk '{print $1}' | sed 's/^[+-]*//')

if [ -z "$ACTUAL_COMMIT" ]; then
  echo_error "Could not determine spirv-headers submodule commit"
  exit 1
fi

echo_info "Expected spirv-headers commit (from DEPS): $EXPECTED_COMMIT"
echo_info "Actual spirv-headers commit: $ACTUAL_COMMIT"

if [ "$EXPECTED_COMMIT" != "$ACTUAL_COMMIT" ]; then
  echo_error "spirv-headers commit mismatch!"
  echo_error "  Expected (from spirv-tools/DEPS): $EXPECTED_COMMIT"
  echo_error "  Actual (submodule):                $ACTUAL_COMMIT"
  echo ""
  echo "Please update external/spirv-headers to match what spirv-tools expects:"
  echo "  git -C external/spirv-headers fetch"
  echo "  git -C external/spirv-headers checkout $EXPECTED_COMMIT"
  echo "  git add external/spirv-headers"
  exit 1
fi

echo_success "spirv-headers commit matches spirv-tools DEPS"

# 3. Sync spirv-tools dependencies
echo_info "Syncing spirv-tools dependencies..."
cd external/spirv-tools

if [ ! -f "utils/git-sync-deps" ]; then
  echo_error "utils/git-sync-deps not found in external/spirv-tools"
  exit 1
fi

# git-sync-deps needs to be run to get additional dependencies
echo_info "Running git-sync-deps (this may take a moment)..."
if ! python3 utils/git-sync-deps; then
  echo_error "Failed to sync spirv-tools dependencies"
  echo_error "This may be due to network issues or missing git credentials"
  cd ../..
  exit 1
fi
echo_success "Dependencies synced successfully"

cd ../..

# 4. Build spirv-tools to generate files
BUILD_DIR="external/spirv-tools/build-ci-check"
echo_info "Building spirv-tools generation targets in $BUILD_DIR..."

mkdir -p "$BUILD_DIR"

# Configure CMake (suppress warnings from external project)
cmake -Wno-dev -B "$BUILD_DIR" external/spirv-tools

# Build only the generation targets (faster than full build)
echo_info "Building generation targets (this may take a few minutes)..."
if ! cmake --build "$BUILD_DIR" \
  --target spirv-tools-build-version \
  --target core_tables \
  --target extinst_tables; then
  echo_error "Failed to build spirv-tools generation targets"
  echo_error "Check the build output above for details"
  rm -rf "$BUILD_DIR"
  exit 1
fi
echo_success "Build completed successfully"

# 5. Compare generated files
echo_info "Comparing generated files..."
TEMP_DIR=$(mktemp -d)

# Copy generated files to temp directory (allow glob to expand to nothing)
shopt -s nullglob
cp "$BUILD_DIR"/*.inc "$TEMP_DIR/" 2>/dev/null
cp "$BUILD_DIR"/*.h "$TEMP_DIR/" 2>/dev/null
shopt -u nullglob

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

# 8. Save generated files for artifact upload (before cleanup)
if [ "$DIFF_FOUND" = true ]; then
  echo_info "Saving generated files for artifact upload..."
  ARTIFACT_DIR="external/spirv-tools/artifacts-for-upload"
  mkdir -p "$ARTIFACT_DIR"
  shopt -s nullglob
  cp "$BUILD_DIR"/*.inc "$ARTIFACT_DIR/" 2>/dev/null
  cp "$BUILD_DIR"/*.h "$ARTIFACT_DIR/" 2>/dev/null
  shopt -u nullglob

  ARTIFACT_COUNT=$(ls -1 "$ARTIFACT_DIR" 2>/dev/null | wc -l)
  echo_info "Saved $ARTIFACT_COUNT generated files for CI artifact upload"
fi

# 9. Cleanup
echo_info "Cleaning up build artifacts..."
rm -rf "$BUILD_DIR"
rm -rf "$TEMP_DIR"

# 10. Report results
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
  echo "=================================================="
  echo_info "How to fix this issue:"
  echo "=================================================="
  echo ""
  echo "Follow these steps to regenerate the files:"
  echo "  cd external/spirv-tools"
  echo "  python3 utils/git-sync-deps"
  echo "  cmake . -B build"
  echo "  cmake --build build --target spirv-tools-build-version --target core_tables --target extinst_tables"
  echo "  cd ../.."
  echo "  rm external/spirv-tools-generated/*.h external/spirv-tools-generated/*.inc"
  echo "  cp external/spirv-tools/build/*.h external/spirv-tools-generated/"
  echo "  cp external/spirv-tools/build/*.inc external/spirv-tools-generated/"
  echo "  git add external/spirv-tools-generated"
  echo ""
  echo "For more details, see: docs/update_spirv.md"
  echo ""
  exit 1
else
  echo_success "All generated files are up-to-date!"
  echo ""
  echo "All checks passed:"
  echo "  - All generated files are present"
  echo "  - All generated files match build output"
  echo "  - No orphaned files detected"
  echo ""
fi

exit 0
