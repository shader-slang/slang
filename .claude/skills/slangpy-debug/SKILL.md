---
name: slangpy-debug
description: Debug slangpy compatibility issues by building slangpy from source with a local Slang build. Covers cloning, building, installing, and testing slangpy against your local Slang changes.
argument-hint: "[build-type: debug|release]"
allowed-tools:
  - Bash
  - Read
  - Write
  - Edit
  - Grep
  - Glob
---

# Debugging with slangpy repro steps

When debugging slangpy compatibility issues or testing Slang changes against slangpy, follow these steps to build slangpy from source using your local Slang build.

**Prerequisites:**
- Python 3.10+ installed
- Slang built in Debug or Release configuration
- Git with submodules support

## Step 1: Build Slang

```bash
# From Slang repository root
cmake --preset default
cmake --build --preset debug    # or --preset release
```

## Step 2: Clone slangpy Repository

```bash
# Clone with submodules (required for slangpy build)
git clone https://github.com/shader-slang/slangpy.git external/slangpy
cd external/slangpy
git submodule update --init --recursive
```

Read `external/slangpy/CLAUDE.md` for general information about the repo.

## Step 3: Build and Install slangpy with Local Slang

On Windows:
```bash
cd external/slangpy
SET CMAKE_ARGS=-DSGL_LOCAL_SLANG=ON -DSGL_LOCAL_SLANG_DIR=../.. -DSGL_LOCAL_SLANG_BUILD_DIR=build/Debug
python.exe -m pip install -e .
```

On Linux/macOS:
```bash
cd external/slangpy
CMAKE_ARGS="-DSGL_LOCAL_SLANG=ON -DSGL_LOCAL_SLANG_DIR=../.. -DSGL_LOCAL_SLANG_BUILD_DIR=build/Debug" python -m pip install -e .
```

On Windows WSL:
```bash
cd external/slangpy
WSLENV+=:CMAKE_ARGS CMAKE_ARGS='-DSGL_LOCAL_SLANG=ON -DSGL_LOCAL_SLANG_DIR=../.. -DSGL_LOCAL_SLANG_BUILD_DIR=build/Debug' python.exe -m pip install -e .
```

## Step 4: Install Test Dependencies

```bash
# From external/slangpy directory
python -m pip install -r requirements-dev.txt --user
python -m pip install pytest-xdist --user
```

## Step 5: Run Tests

Write a test case under `external/slangpy/slangpy/tests` and run it with pytest.

The following example is to run all of the existing slangpy tests:
```bash
# From external/slangpy directory
python -m pytest slangpy/tests -ra -n auto --maxprocesses=3
python tools/ci.py unit-test-python
```

## Notes
- Use `-DSGL_LOCAL_SLANG_BUILD_DIR=build/Release` for Release builds
- The `-e` flag installs in editable mode, allowing you to modify slangpy source
- Tests run in parallel with `-n auto` for faster execution
- Some tests may skip if GPU hardware is not available

## Troubleshooting
- If tests fail to find slangc, verify the `SLANG_BUILD_DIR` path is correct
- If import fails, check that slangpy was installed: `python -c "import slangpy; print(slangpy.__file__)"`
- For GPU-specific test failures, ensure drivers are up to date

## Interactive Workflow

If `$ARGUMENTS` specifies a build type (`debug` or `release`), use that. Otherwise default to `debug`.

1. Check if slangpy is already cloned at `external/slangpy/`
2. If not, clone it (Step 2)
3. Build and install with the appropriate build type
4. Ask the user what test to run or what issue to investigate
