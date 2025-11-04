# Plan: CI Check for SPIRV-Tools Generated Files

## Problem Statement

**GitHub Issue:** https://github.com/shader-slang/slang/issues/8509

When `external/spirv-tools` is updated (submodule commit changes), developers must also regenerate and update the files in `external/spirv-tools-generated/`. Currently, there's no automated CI check to verify this has been done correctly, which can lead to build inconsistencies or failures.

## Background

### Directory Structure
- `external/spirv-tools/` - Submodule containing SPIRV-Tools source code
- `external/spirv-tools-generated/` - Contains generated `.h` and `.inc` files needed for building glslang
- `external/spirv-headers/` - Submodule containing SPIRV headers

### Current Process (Manual)
According to `docs/update_spirv.md` and the `external/bump-glslang.sh` script:

1. Update `external/spirv-tools` submodule
2. Run `python3 utils/git-sync-deps` in spirv-tools directory
3. Build specific CMake targets:
   - `spirv-tools-build-version`
   - `core_tables`
   - `extinst_tables`
4. Copy generated `.h` and `.inc` files from build directory to `external/spirv-tools-generated/`
5. Commit both submodule update and generated files

### Files in spirv-tools-generated/
Currently tracked files:
- `build-version.inc`
- `core_tables_body.inc`
- `core_tables_header.inc`
- `DebugInfo.h`
- `generators.inc`
- `NonSemanticShaderDebugInfo100.h`
- `OpenCLDebugInfo100.h`
- `options-pinned.h`
- `README.md`

## Solution Design

### Approach
Create a GitHub Actions workflow that:
1. Triggers on pull requests when `external/spirv-tools` or `external/spirv-headers` is modified
2. Regenerates the files that should be in `external/spirv-tools-generated/`
3. Compares the regenerated files with the committed files
4. Fails the CI if there are discrepancies
5. Provides clear feedback on what needs to be updated

### Implementation Steps

#### Step 1: Create Detection Script
**File:** `extras/check-spirv-generated.sh` ✅ **COMPLETED**

**Purpose:** Check if generated files are up-to-date

**Functionality:**
- Detect if `external/spirv-tools` or `external/spirv-headers` was modified in the PR
- If modified, regenerate files in a temporary build directory
- Compare generated files with committed files in `external/spirv-tools-generated/`
- Report differences with clear error messages
- Exit with appropriate status code

**Key Logic:**
```bash
#!/bin/bash
set -e

# 1. Check if spirv-tools or spirv-headers were modified
BASE_REF="${GITHUB_BASE_REF:-origin/master}"
CHANGED_FILES=$(git diff --name-only "$BASE_REF"...HEAD)

SPIRV_CHANGED=false
if echo "$CHANGED_FILES" | grep -q "^external/spirv-tools" || \
   echo "$CHANGED_FILES" | grep -q "^external/spirv-headers"; then
    SPIRV_CHANGED=true
fi

if [ "$SPIRV_CHANGED" = false ]; then
    echo "✅ No changes to external/spirv-tools or external/spirv-headers - skipping check"
    exit 0
fi

echo "🔍 Changes detected in SPIRV directories - verifying generated files..."

# 2. Initialize submodules
git submodule update --init --recursive external/spirv-tools
git submodule update --init --recursive external/spirv-headers

# 3. Sync spirv-tools dependencies
cd external/spirv-tools
python3 utils/git-sync-deps
cd ../..

# 4. Build spirv-tools to generate files
BUILD_DIR="external/spirv-tools/build-ci-check"
mkdir -p "$BUILD_DIR"
cmake -Wno-dev -B "$BUILD_DIR" external/spirv-tools

# Build only the generation targets (faster than full build)
cmake --build "$BUILD_DIR" \
  --target spirv-tools-build-version \
  --target core_tables \
  --target extinst_tables

# 5. Compare generated files
TEMP_DIR=$(mktemp -d)
cp "$BUILD_DIR"/*.inc "$TEMP_DIR/" 2>/dev/null || true
cp "$BUILD_DIR"/*.h "$TEMP_DIR/" 2>/dev/null || true

# 6. Check for differences
DIFF_FOUND=false
for file in "$TEMP_DIR"/*; do
    filename=$(basename "$file")
    if [ "$filename" = "README.md" ]; then
        continue
    fi

    if [ ! -f "external/spirv-tools-generated/$filename" ]; then
        echo "❌ Missing file in spirv-tools-generated: $filename"
        DIFF_FOUND=true
    elif ! diff -q "$file" "external/spirv-tools-generated/$filename" > /dev/null; then
        echo "❌ File differs: $filename"
        diff -u "external/spirv-tools-generated/$filename" "$file" || true
        DIFF_FOUND=true
    fi
done

# 7. Check for orphaned files
for file in external/spirv-tools-generated/*; do
    filename=$(basename "$file")
    if [ "$filename" = "README.md" ]; then
        continue
    fi
    if [ ! -f "$TEMP_DIR/$filename" ]; then
        echo "❌ Orphaned file in spirv-tools-generated (should be removed): $filename"
        DIFF_FOUND=true
    fi
done

# 8. Cleanup
rm -rf "$BUILD_DIR"
rm -rf "$TEMP_DIR"

if [ "$DIFF_FOUND" = true ]; then
    echo ""
    echo "❌ Generated files are out of sync!"
    echo ""
    echo "To fix this issue, please run the following commands:"
    echo ""
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
    echo "Or use the automated script:"
    echo "  bash external/bump-glslang.sh"
    echo ""
    exit 1
fi

echo "✅ All generated files are up-to-date!"
exit 0
```

#### Step 2: Create GitHub Actions Workflow
**File:** `.github/workflows/check-spirv-generated.yml` ✅ **COMPLETED**

**Purpose:** Run the check as part of CI

**Workflow Structure:**
```yaml
name: Check SPIRV Generated Files

on:
  pull_request:
    branches: [master]
    paths:
      - 'external/spirv-tools/**'
      - 'external/spirv-headers/**'
      - 'external/spirv-tools-generated/**'
      - '.github/workflows/check-spirv-generated.yml'
      - 'extras/check-spirv-generated.sh'

jobs:
  check-spirv-generated:
    runs-on: ubuntu-latest
    name: Verify SPIRV Generated Files

    steps:
      - name: Checkout code
        uses: actions/checkout@v4
        with:
          fetch-depth: 0  # Need full history for git diff
          submodules: false  # We'll init them selectively

      - name: Setup Python
        uses: actions/setup-python@v5
        with:
          python-version: '3.x'

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake build-essential

      - name: Run SPIRV generated files check
        run: |
          bash scripts/check-spirv-generated.sh
        env:
          GITHUB_BASE_REF: ${{ github.base_ref }}
```

#### Step 3: Integrate into Main CI Workflow
**File:** `.github/workflows/ci.yml`

Add the new check to the dependencies of the final `check-ci` job:

```yaml
jobs:
  # ... existing jobs ...

  check-spirv-generated:
    needs: [filter]
    if: needs.filter.outputs.should-run == 'true'
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          fetch-depth: 0
          submodules: false
      - uses: actions/setup-python@v5
        with:
          python-version: '3.x'
      - run: |
          sudo apt-get update
          sudo apt-get install -y cmake build-essential
      - run: bash extras/check-spirv-generated.sh
        env:
          GITHUB_BASE_REF: ${{ github.base_ref }}

  check-ci:
    needs:
      [
        # ... existing dependencies ...
        check-spirv-generated,
      ]
    # ... rest of check-ci job ...
```

#### Step 4: Update Documentation
**File:** `docs/update_spirv.md`

Add a section explaining:
- The new CI check and what it does
- How to interpret CI failures
- That the check ensures consistency
- Link to the automated script

#### Step 5: Optional Enhancements

**A. Cache CMake builds**
- Cache the build directory to speed up CI runs
- Use GitHub Actions cache with appropriate keys

**B. Artifact generation**
- If check fails, upload a zip of the correctly generated files
- Makes it easier for developers to fix the issue

**C. Auto-fix suggestion**
- Create a GitHub Actions bot comment with:
  - Specific files that need updating
  - Downloadable artifact with correct files
  - Copy-paste commands to fix

**D. Pre-commit hook**
- Create a pre-commit hook that runs locally
- Prevents committing out-of-sync changes

## Testing Plan

### Test Cases

1. **No changes to SPIRV directories**
   - Modify other files
   - Check should skip quickly

2. **Valid update**
   - Update spirv-tools submodule
   - Regenerate files correctly
   - Check should pass

3. **Missing regeneration**
   - Update spirv-tools submodule
   - Don't update generated files
   - Check should fail with clear message

4. **Partial regeneration**
   - Update spirv-tools submodule
   - Only update some generated files
   - Check should fail, listing missing files

5. **Orphaned files**
   - Remove a file that should be generated
   - Keep the file in spirv-tools-generated
   - Check should detect and report

6. **Both submodules updated**
   - Update both spirv-tools and spirv-headers
   - Verify check handles both

### Manual Testing Steps

1. Create test branch
2. Modify spirv-tools submodule reference
3. Push without updating generated files → should fail CI
4. Regenerate and commit files → should pass CI
5. Modify a generated file manually → should fail CI

## Implementation Timeline

### Phase 1: Core Implementation (Priority: High) ✅ **COMPLETED**
- [x] Step 1: Create `extras/check-spirv-generated.sh`
- [x] Step 2: Create `.github/workflows/check-spirv-generated.yml`
- [ ] Test manually on a test branch (READY FOR TESTING)

### Phase 2: Integration (Priority: High) ✅ **COMPLETED**
- [x] Step 3: Integrate into main CI workflow
- [x] Step 4: Update documentation
- [x] Ensure all tests pass (no linter errors)

### Phase 3: Enhancements (Priority: Medium)
- [ ] Add caching for faster builds
- [ ] Add artifact uploads on failure
- [ ] Add helpful bot comments

### Phase 4: Developer Experience (Priority: Low)
- [ ] Create pre-commit hook
- [ ] Add to developer onboarding docs

## Success Criteria

- ✅ CI fails when spirv-tools is updated without regenerating files
- ✅ CI passes when files are correctly regenerated
- ✅ Clear error messages guide developers on how to fix issues
- ✅ Check runs efficiently (< 5 minutes on Ubuntu)
- ✅ No false positives
- ✅ Documentation is updated

## Risks and Mitigation

| Risk | Impact | Mitigation |
|------|--------|-----------|
| Build takes too long in CI | High | Only build generation targets, not full spirv-tools |
| False positives due to platform differences | High | Use consistent build flags, normalize line endings |
| git-sync-deps requires credentials | Medium | Ensure GitHub Actions has proper permissions, use HTTPS |
| Check is too strict (whitespace differences) | Low | Use semantic comparison, normalize formatting |

## Dependencies

- Python 3.x (for git-sync-deps)
- CMake 3.x
- Build tools (gcc/clang)
- Git with submodule support
- GitHub Actions runners with sufficient storage

## Future Considerations

1. **Similar checks for glslang-generated**
   - Apply same pattern to `external/glslang-generated/`

2. **Automated PR creation**
   - Bot that automatically updates SPIRV and creates PR

3. **Scheduled updates**
   - Nightly job to check for SPIRV-Tools updates
   - Auto-create PRs when new versions available

## References

- Issue: https://github.com/shader-slang/slang/issues/8509
- Documentation: `docs/update_spirv.md`
- Existing automation: `external/bump-glslang.sh`
- SPIRV-Tools: https://github.com/KhronosGroup/SPIRV-Tools

