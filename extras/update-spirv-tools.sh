#!/bin/bash
#
# Script to update SPIRV-Tools and SPIRV-Headers submodules
# and regenerate the generated files for a PR.
#
# Usage: bash extras/update-spirv-tools.sh [OPTIONS] [commit-hash]
#   commit-hash: Specific commit hash for SPIRV-Tools (default: origin/main)
#
# Options:
#   --test        Run test suite after update (for CI)
#   --create-pr   Create GitHub PR after successful update (requires gh CLI)
#

set -e

# Show help message
show_help() {
  cat << EOF
Update SPIRV-Tools and SPIRV-Headers submodules

Usage: $0 [OPTIONS] [commit-hash]

This script automates the SPIRV-Tools update process documented in docs/update_spirv.md.

Arguments:
  commit-hash          Specific commit hash for SPIRV-Tools (default: origin/main)

Options:
  --test               Run test suite after update
  --create-pr          Create GitHub PR after successful update
  --help               Show this help message

Examples:
  $0                                    # Update to latest, create branch, stage changes
  $0 abc123def                          # Update to specific commit
  $0 --test                             # Update and run test suite (CI mode, no branch)
  $0 --create-pr                        # Update and create PR (without tests)
  $0 --test --create-pr                 # Full workflow: update, test, PR

Reference: docs/update_spirv.md
EOF
}

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

# Parse options
RUN_TESTS=false
CREATE_PR=false
COMMIT_HASH=""

while [[ $# -gt 0 ]]; do
  case $1 in
    --test)
      RUN_TESTS=true
      shift
      ;;
    --create-pr)
      CREATE_PR=true
      shift
      ;;
    --help)
      show_help
      exit 0
      ;;
    -*)
      log_error "Unknown option: $1"
      echo ""
      show_help
      exit 1
      ;;
    *)
      COMMIT_HASH="$1"
      shift
      ;;
  esac
done

# Default to origin/main if no commit specified
COMMIT_HASH="${COMMIT_HASH:-origin/main}"

# Check if we're in the repository root
if [ ! -f "CMakeLists.txt" ] || [ ! -d "external/spirv-tools" ]; then
  log_error "This script must be run from the Slang repository root."
  exit 1
fi

# Check gh CLI if creating PR
if [ "$CREATE_PR" = true ]; then
  if ! command -v gh &> /dev/null; then
    log_error "gh CLI not found but --create-pr was specified"
    log_info "Install from https://cli.github.com/"
    exit 1
  fi
fi

log_info "Starting SPIRV update process..."
log_info "SPIRV-Tools target: $COMMIT_HASH"

# Capture current state for CI output and PR
OLD_SPIRV_COMMIT=$(git -C external/spirv-tools rev-parse HEAD)
OLD_SPIRV_SHORT=$(git -C external/spirv-tools rev-parse --short=8 HEAD)
OLD_HEADERS_COMMIT=$(git -C external/spirv-headers rev-parse HEAD)
OLD_HEADERS_SHORT=$(git -C external/spirv-headers rev-parse --short=8 HEAD)

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
NEW_SPIRV_SHORT=$(git -C external/spirv-tools rev-parse --short=8 "$RESOLVED_COMMIT")

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
SPIRV_HEADERS_REV=$(grep -oP "spirv_headers_revision.*'\K[a-f0-9]+" external/spirv-tools/DEPS ||
  grep "spirv_headers_revision" external/spirv-tools/DEPS | grep -oE "[a-f0-9]{40}")

if [ -z "$SPIRV_HEADERS_REV" ]; then
  log_error "Could not determine SPIRV-Headers revision from DEPS file."
  log_info "Contents of spirv_headers_revision line:"
  grep spirv_headers_revision external/spirv-tools/DEPS
  exit 1
fi

NEW_HEADERS_SHORT=$(git -C external/spirv-headers rev-parse --short=8 "$SPIRV_HEADERS_REV")

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

# Step 9: Run tests if requested
if [ "$RUN_TESTS" = true ]; then
  log_info "=========================================="
  log_info "Running test suite..."
  log_info "=========================================="

  # Clean build directory
  log_info "Cleaning build directory..."
  rm -rf build

  # Detect platform and configure
  log_info "Configuring build..."
  if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    cmake --preset vs2022
  else
    cmake --preset default
  fi

  log_info "Building Release configuration..."
  cmake --build --preset release

  if [ $? -ne 0 ]; then
    log_error "Slang build failed"
    exit 1
  fi

  log_info "Running test suite (this may take 10-30 minutes)..."
  export SLANG_RUN_SPIRV_VALIDATION=1
  ./build/Release/bin/slang-test \
    -expected-failure-list tests/expected-failure-github.txt \
    -use-test-server \
    -server-count 8

  if [ $? -ne 0 ]; then
    log_error "Tests failed"
    exit 1
  fi

  log_info "Tests passed!"
fi

# Step 10: Create PR if requested
if [ "$CREATE_PR" = true ]; then
  log_info "=========================================="
  log_info "Creating Pull Request..."
  log_info "=========================================="

  # Commit changes
  log_info "Committing changes..."
  if [ "$RUN_TESTS" = true ]; then
    TEST_STATUS="Tests passed"
  else
    TEST_STATUS="Tests not run (manual testing required)"
  fi

  git commit -m "Update SPIRV-Tools to $NEW_SPIRV_SHORT

- SPIRV-Tools: $OLD_SPIRV_SHORT -> $NEW_SPIRV_SHORT
- SPIRV-Headers: $OLD_HEADERS_SHORT -> $NEW_HEADERS_SHORT
- Generated files updated and validated
- $TEST_STATUS"

  # Push branch
  log_info "Pushing branch to origin..."
  git push -u origin "$BRANCH_NAME"

  # Create PR body
  if [ "$RUN_TESTS" = true ]; then
    TEST_RESULTS="Test suite passed with SPIRV validation enabled"
    SCRIPT_ARGS="--test --create-pr"
  else
    TEST_RESULTS="Tests not run locally before creating PR"
    SCRIPT_ARGS="--create-pr"
  fi

  PR_BODY="## SPIRV-Tools Update

**SPIRV-Tools**: [\`$OLD_SPIRV_SHORT\`](https://github.com/KhronosGroup/SPIRV-Tools/commit/$OLD_SPIRV_COMMIT) -> [\`$NEW_SPIRV_SHORT\`](https://github.com/KhronosGroup/SPIRV-Tools/commit/$RESOLVED_COMMIT)
**SPIRV-Headers**: [\`$OLD_HEADERS_SHORT\`](https://github.com/KhronosGroup/SPIRV-Headers/commit/$OLD_HEADERS_COMMIT) -> [\`$NEW_HEADERS_SHORT\`](https://github.com/KhronosGroup/SPIRV-Headers/commit/$SPIRV_HEADERS_REV)

### Changes
[View SPIRV-Tools changes](https://github.com/KhronosGroup/SPIRV-Tools/compare/$OLD_SPIRV_COMMIT...$RESOLVED_COMMIT)

### Test Results
$TEST_RESULTS

---
Generated by \`extras/update-spirv-tools.sh $SCRIPT_ARGS\`"

  # Create PR
  log_info "Creating draft PR..."
  gh pr create --title "Update SPIRV-Tools to $NEW_SPIRV_SHORT" \
               --body "$PR_BODY" \
               --draft

  log_info "Pull request created successfully!"
fi

# Output commit info for CI parsing
log_info "Outputting commit info for CI parsing..."
echo "OLD_SPIRV_COMMIT=$OLD_SPIRV_SHORT"
echo "NEW_SPIRV_COMMIT=$NEW_SPIRV_SHORT"

echo ""
log_info "=========================================="
log_info "SPIRV update preparation complete!"
log_info "=========================================="
echo ""

if [ "$RUN_TESTS" = false ] && [ "$CREATE_PR" = false ]; then
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
fi
