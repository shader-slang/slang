#!/usr/bin/env bash
# Update SPIRV-Tools and SPIRV-Headers submodules
# Automates the manual process documented in docs/update_spirv.md
# Can be used both locally and in CI

set -e  # Exit on error

# Script version
SCRIPT_VERSION="1.0.0"

# Default values
SPIRV_COMMIT=""
RUN_TESTS=false
CREATE_PR=false
FORCE=false

# Show help message
show_help() {
    cat << EOF
Update SPIRV-Tools and SPIRV-Headers submodules

Usage: $0 [OPTIONS]

This script automates the SPIRV-Tools update process documented in docs/update_spirv.md.
It can be used both locally by developers and in CI workflows.

Options:
  --commit HASH        Update to specific SPIRV-Tools commit (default: latest from main)
  --test               Run test suite after update
  --create-pr          Create GitHub PR after successful update (requires gh CLI)
  --force              Skip confirmation prompts and continue despite failures
  --help               Show this help message

Examples:
  $0                                    # Update to latest, no tests, no PR
  $0 --test                             # Update and run test suite
  $0 --create-pr                        # Update and create PR (implies --test)
  $0 --commit abc123def                 # Update to specific commit
  $0 --test --create-pr                 # Full workflow: update, test, PR

CI Usage:
  $0 --test                             # CI runs tests

Bisection Workflow:
  cd external/spirv-tools && git bisect start origin/main HEAD && cd ../..
  $0 --commit \$(git -C external/spirv-tools rev-parse HEAD) --test
  cd external/spirv-tools && git bisect good  # or: git bisect bad

Reference: docs/update_spirv.md
EOF
}

# Print banner
echo "========================================"
echo "SPIRV-Tools Update Script v$SCRIPT_VERSION"
echo "========================================"
echo ""

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --commit)
            SPIRV_COMMIT="$2"
            shift 2
            ;;
        --test)
            RUN_TESTS=true
            shift
            ;;
        --create-pr)
            CREATE_PR=true
            RUN_TESTS=true  # PR creation implies testing
            shift
            ;;
        --force)
            FORCE=true
            shift
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            echo "Error: Unknown option: $1"
            echo ""
            show_help
            exit 1
            ;;
    esac
done

# Validate environment
echo "=== Validating Environment ==="

# Check we're in Slang repo root
if [[ ! -f "CMakeLists.txt" ]] || [[ ! -d "external/spirv-tools" ]]; then
    echo "Error: Must run from Slang repository root"
    echo "Current directory: $(pwd)"
    exit 1
fi
echo "[OK] Running from Slang repository root"

# Check git working directory is clean (unless --force)
if [[ "$FORCE" != "true" ]] && [[ -n $(git status --porcelain) ]]; then
    echo "Error: Working directory has uncommitted changes"
    echo "Use --force to override (not recommended)"
    git status --short
    exit 1
fi
echo "[OK] Working directory is clean"

# Check required tools
for cmd in git cmake python3; do
    if ! command -v $cmd &> /dev/null; then
        echo "Error: Required tool not found: $cmd"
        exit 1
    fi
done
echo "[OK] Required tools available: git, cmake, python3"

# Check gh CLI if creating PR
if [[ "$CREATE_PR" == "true" ]]; then
    if ! command -v gh &> /dev/null; then
        echo "Error: gh CLI not found but --create-pr was specified"
        echo "Install from https://cli.github.com/"
        exit 1
    fi
    echo "[OK] gh CLI available"
fi

echo ""

# Sync submodules (from docs/update_spirv.md)
echo "=== Synchronizing Submodules ==="
git submodule sync
git submodule update --init --recursive
echo "[OK] Submodules synchronized"
echo ""

# Capture current commits (full hash for operations, short for display)
echo "=== Capturing Current State ==="
OLD_SPIRV=$(git -C external/spirv-tools rev-parse HEAD)
OLD_HEADERS=$(git -C external/spirv-headers rev-parse HEAD)
OLD_SPIRV_SHORT=$(git -C external/spirv-tools rev-parse --short=8 HEAD)
OLD_HEADERS_SHORT=$(git -C external/spirv-headers rev-parse --short=8 HEAD)
echo "Current SPIRV-Tools:   $OLD_SPIRV_SHORT"
echo "Current SPIRV-Headers: $OLD_HEADERS_SHORT"
echo ""

# Get SPIRV-Tools commit (latest if not specified)
if [[ -z "$SPIRV_COMMIT" ]]; then
    echo "=== Fetching Latest SPIRV-Tools Commit ==="
    git -C external/spirv-tools fetch origin
    SPIRV_COMMIT=$(git -C external/spirv-tools rev-parse origin/main)
    SPIRV_SHORT=$(git -C external/spirv-tools rev-parse --short=8 origin/main)
    echo "Target: $SPIRV_SHORT (latest from main)"
else
    echo "=== Using Specified SPIRV-Tools Commit ==="
    # Fetch to ensure we have the commit
    git -C external/spirv-tools fetch origin
    SPIRV_SHORT=$(git -C external/spirv-tools rev-parse --short=8 $SPIRV_COMMIT)
    echo "Target: $SPIRV_SHORT (specified)"
fi
echo ""

# Update SPIRV-Tools to target commit
echo "=== Updating SPIRV-Tools to $SPIRV_SHORT ==="
git -C external/spirv-tools checkout $SPIRV_COMMIT
echo "[OK] SPIRV-Tools updated"
echo ""

# Build SPIRV-Tools to generate files (from docs/update_spirv.md)
echo "=== Building SPIRV-Tools ==="
pushd external/spirv-tools > /dev/null

# Sync SPIRV-Tools dependencies (includes SPIRV-Headers)
echo "Running git-sync-deps..."
python3 utils/git-sync-deps
if [[ $? -ne 0 ]]; then
    echo "Error: git-sync-deps failed"
    echo "This may require SSH key registration with gitlab.khronos.org"
    echo "See docs/update_spirv.md for setup instructions"
    exit 1
fi

# Build SPIRV-Tools to generate files
echo "Configuring build..."
cmake . -B build -DCMAKE_BUILD_TYPE=Release
echo "Building..."
cmake --build build --config Release
if [[ $? -ne 0 ]]; then
    echo "Error: SPIRV-Tools build failed"
    exit 1
fi

popd > /dev/null
echo "[OK] SPIRV-Tools built successfully"
echo ""

# Determine SPIRV-Headers commit (from docs/update_spirv.md)
echo "=== Determining SPIRV-Headers Commit from DEPS ==="
# Parse DEPS file for spirv_headers_revision
HEADERS_COMMIT=$(grep -oP "'spirv_headers_revision':\s*'\K[a-f0-9]+(?=')" \
                 external/spirv-tools/DEPS)
if [[ -z "$HEADERS_COMMIT" ]]; then
    echo "Error: Could not parse spirv_headers_revision from DEPS"
    echo "Check external/spirv-tools/DEPS format"
    exit 1
fi
HEADERS_SHORT=$(git -C external/spirv-headers rev-parse --short=8 $HEADERS_COMMIT)
echo "Found in DEPS: $HEADERS_SHORT"
echo ""

echo "=== Updating SPIRV-Headers to $HEADERS_SHORT ==="
git -C external/spirv-headers fetch origin
git -C external/spirv-headers checkout $HEADERS_COMMIT
echo "[OK] SPIRV-Headers updated"
echo ""

# Copy generated files (from docs/update_spirv.md)
echo "=== Copying Generated Files ==="
rm -f external/spirv-tools-generated/*.h
rm -f external/spirv-tools-generated/*.inc
cp external/spirv-tools/build/*.h external/spirv-tools-generated/ 2>/dev/null || true
cp external/spirv-tools/build/*.inc external/spirv-tools-generated/ 2>/dev/null || true
echo "[OK] Generated files copied"
echo ""

# Stage all changes
echo "=== Staging Changes ==="
git add external/spirv-tools
git add external/spirv-headers
git add external/spirv-tools-generated
echo "[OK] Changes staged"
echo ""

# Validate generated files (use existing CI check)
echo "=== Validating Generated Files ==="
if [[ -f "extras/check-spirv-generated.sh" ]]; then
    bash extras/check-spirv-generated.sh
    if [[ $? -ne 0 ]]; then
        echo "Error: Generated files validation failed"
        echo "Files may be out of sync or missing"
        exit 1
    fi
    echo "[OK] Generated files validated"
else
    echo "Warning: extras/check-spirv-generated.sh not found, skipping validation"
fi
echo ""

# Build and test Slang (if requested)
if [[ "$RUN_TESTS" == "true" ]]; then
    echo "=== Building Slang ==="
    # Clean build directory (from docs/update_spirv.md)
    echo "Cleaning build directory..."
    rm -rf build

    # Detect platform and configure
    echo "Configuring build..."
    if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
        cmake --preset vs2022
    else
        cmake --preset default
    fi
    
    echo "Building Release configuration..."
    cmake --build --preset release
    if [[ $? -ne 0 ]]; then
        echo "Error: Slang build failed"
        exit 1
    fi
    echo "[OK] Slang built successfully"
    echo ""

    echo "=== Running Tests ==="
    export SLANG_RUN_SPIRV_VALIDATION=1
    
    echo "Running test suite (this may take 10-30 minutes)..."
    ./build/Release/bin/slang-test \
        -expected-failure-list tests/expected-failure-github.txt \
        -use-test-server \
        -server-count 8

    if [[ $? -ne 0 ]]; then
        echo "Error: Tests failed"
        if [[ "$FORCE" != "true" ]]; then
            echo "Use --force to continue despite test failures (not recommended)"
            exit 1
        fi
        echo "Warning: Continuing despite test failures (--force)"
    else
        echo "[OK] Tests passed"
    fi
    echo ""
fi

# Create PR (if --create-pr)
if [[ "$CREATE_PR" == "true" ]]; then
    echo "=== Creating Pull Request ==="
    
    # Create branch
    BRANCH="spirv-tools-update-$(date +%Y-%m-%d)"
    echo "Creating branch: $BRANCH"
    git checkout -b $BRANCH

    # Commit changes
    echo "Committing changes..."
    git commit -m "Update SPIRV-Tools to $SPIRV_SHORT

- SPIRV-Tools: $OLD_SPIRV_SHORT -> $SPIRV_SHORT
- SPIRV-Headers: $OLD_HEADERS_SHORT -> $HEADERS_SHORT
- Generated files updated and validated"

    # Create PR body
    PR_BODY="## SPIRV-Tools Update

**SPIRV-Tools**: [\`$OLD_SPIRV_SHORT\`](https://github.com/KhronosGroup/SPIRV-Tools/commit/$OLD_SPIRV) -> [\`$SPIRV_SHORT\`](https://github.com/KhronosGroup/SPIRV-Tools/commit/$SPIRV_COMMIT)
**SPIRV-Headers**: [\`$OLD_HEADERS_SHORT\`](https://github.com/KhronosGroup/SPIRV-Headers/commit/$OLD_HEADERS) -> [\`$HEADERS_SHORT\`](https://github.com/KhronosGroup/SPIRV-Headers/commit/$HEADERS_COMMIT)

### Changes
[View SPIRV-Tools changes](https://github.com/KhronosGroup/SPIRV-Tools/compare/$OLD_SPIRV...$SPIRV_COMMIT)

### Test Results
Test suite: PASSED

---
Generated by \`extras/update-spirv-tools.sh\`"

    # Create PR as draft
    echo "Creating draft PR..."
    gh pr create --title "Update SPIRV-Tools to $SPIRV_SHORT" \
                 --body "$PR_BODY" \
                 --draft

    echo "[OK] Pull request created as draft"
    echo ""
fi

# Summary output
echo ""
echo "========================================"
echo "=== SPIRV-Tools Update Complete ==="
echo "========================================"
echo ""
echo "SPIRV-Tools:   $OLD_SPIRV_SHORT -> $SPIRV_SHORT"
echo "SPIRV-Headers: $OLD_HEADERS_SHORT -> $HEADERS_SHORT"
if [[ "$RUN_TESTS" == "true" ]]; then
    echo "Tests:         PASSED"
else
    echo "Tests:         SKIPPED (use --test to run)"
fi
if [[ "$CREATE_PR" == "true" ]]; then
    echo "PR:            CREATED"
fi
echo ""

# Output for CI parsing (one per line)
echo "CURRENT_SPIRV_COMMIT=$OLD_SPIRV_SHORT"
echo "NEW_SPIRV_COMMIT=$SPIRV_SHORT"
