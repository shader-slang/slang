#!/bin/bash
#
# Script to update SPIRV-Tools and SPIRV-Headers submodules
# and regenerate the generated files for a PR.
#
# Usage: bash extras/update-spirv-tools.sh [commit-hash]
#   commit-hash: Specific commit hash for SPIRV-Tools (default: origin/main)
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the repository root
if [ ! -f "CMakeLists.txt" ] || [ ! -d "external/spirv-tools" ]; then
    log_error "This script must be run from the Slang repository root."
    exit 1
fi

COMMIT_HASH="${1:-origin/main}"

log_info "Starting SPIRV update process..."
log_info "SPIRV-Tools target: $COMMIT_HASH"

# Step 1: Synchronize and update submodules
log_info "Synchronizing and updating submodules..."
git submodule sync
git submodule update --init --recursive

# Step 2: Fetch and resolve the commit hash
log_info "Fetching SPIRV-Tools..."
git -C external/spirv-tools fetch

log_info "Resolving commit hash..."
RESOLVED_COMMIT=$(git -C external/spirv-tools rev-parse "$COMMIT_HASH" 2>/dev/null || echo "")

if [ -z "$RESOLVED_COMMIT" ]; then
    log_error "Could not resolve commit hash: $COMMIT_HASH"
    exit 1
fi

# Extract last 6 characters of commit hash for branch suffix
COMMIT_SUFFIX="${RESOLVED_COMMIT: -6}"
BRANCH_NAME="update-spirv-${COMMIT_SUFFIX}"

log_info "Resolved commit: $RESOLVED_COMMIT"
log_info "Branch name: $BRANCH_NAME"

# Step 3: Create a branch for the update
log_info "Creating branch '$BRANCH_NAME'..."
git checkout -b "$BRANCH_NAME" || {
    log_warn "Branch '$BRANCH_NAME' may already exist. Switching to it..."
    git checkout "$BRANCH_NAME"
}

# Step 4: Update the SPIRV-Tools submodule to the specified version
log_info "Updating SPIRV-Tools to $RESOLVED_COMMIT..."
git -C external/spirv-tools checkout "$RESOLVED_COMMIT"

# Step 5: Build spirv-tools to generate files
log_info "Building SPIRV-Tools to generate files..."
cd external/spirv-tools

log_info "Running git-sync-deps to fetch dependencies..."
python3 utils/git-sync-deps

log_info "Configuring SPIRV-Tools with CMake..."
cmake . -B build

log_info "Building SPIRV-Tools (Release configuration)..."
cmake --build build --config Release

cd ../..

# Step 6: Update SPIRV-Headers to what SPIRV-Tools uses
log_info "Determining SPIRV-Headers revision from SPIRV-Tools DEPS..."
SPIRV_HEADERS_REV=$(grep -oP "spirv_headers_revision.*'\K[a-f0-9]+" external/spirv-tools/DEPS || \
                    grep "spirv_headers_revision" external/spirv-tools/DEPS | grep -oE "[a-f0-9]{40}")

if [ -z "$SPIRV_HEADERS_REV" ]; then
    log_error "Could not determine SPIRV-Headers revision from DEPS file."
    log_info "Contents of spirv_headers_revision line:"
    grep spirv_headers_revision external/spirv-tools/DEPS
    exit 1
fi

log_info "SPIRV-Headers revision: $SPIRV_HEADERS_REV"
log_info "Updating SPIRV-Headers submodule..."
git -C external/spirv-headers fetch
git -C external/spirv-headers checkout "$SPIRV_HEADERS_REV"

# Step 7: Copy the generated files from spirv-tools/build/ to spirv-tools-generated/
log_info "Copying generated files to spirv-tools-generated/..."

# Remove old generated files (but keep README.md)
find external/spirv-tools-generated -maxdepth 1 -name "*.h" -delete
find external/spirv-tools-generated -maxdepth 1 -name "*.inc" -delete

# Copy new generated files
cp external/spirv-tools/build/*.h external/spirv-tools-generated/ 2>/dev/null || true
cp external/spirv-tools/build/*.inc external/spirv-tools-generated/ 2>/dev/null || true

# Verify files were copied
H_COUNT=$(find external/spirv-tools-generated -maxdepth 1 -name "*.h" | wc -l)
INC_COUNT=$(find external/spirv-tools-generated -maxdepth 1 -name "*.inc" | wc -l)

log_info "Copied $H_COUNT .h files and $INC_COUNT .inc files"

if [ "$H_COUNT" -eq 0 ] && [ "$INC_COUNT" -eq 0 ]; then
    log_error "No generated files were copied. Check if SPIRV-Tools build succeeded."
    exit 1
fi

# Step 8: Stage the changes
log_info "Staging changes..."
git add external/spirv-headers
git add external/spirv-tools
git add external/spirv-tools-generated

# Show status
log_info "Current git status:"
git status --short external/spirv-headers external/spirv-tools external/spirv-tools-generated

echo ""
log_info "=========================================="
log_info "SPIRV update preparation complete!"
log_info "=========================================="
echo ""
log_info "Next steps:"
echo "  1. Review the staged changes with: git diff --cached"
echo "  2. Build and test Slang:"
echo "     rm -rf build"
echo "     cmake --preset vs2022  # or your preferred preset"
echo "     cmake --build --preset release"
echo "     export SLANG_RUN_SPIRV_VALIDATION=1"
echo "     build/Release/bin/slang-test -use-test-server -server-count 8"
echo "  3. Commit the changes:"
echo "     git commit -m \"Update SPIRV-Tools and SPIRV-Headers to latest versions\""
echo "  4. Push and create a PR:"
echo "     git push origin $BRANCH_NAME"
echo ""