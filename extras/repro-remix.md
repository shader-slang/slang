# RTX Remix Shader Compilation Reproduction Script

This script (`repro-remix.sh`) helps reproduce issues from the [RTX Remix nightly workflow](../.github/workflows/compile-rtx-remix-shaders-nightly.yml) locally.

## Purpose

The Slang project has a nightly CI job that tests compatibility with NVIDIA RTX Remix shaders. When this workflow fails, this script allows developers to reproduce the issue locally for debugging.

## What It Does

The script replicates the CI workflow steps:

1. **Verifies Slang Debug build** exists at `build/Debug/bin/`
2. **Clones dxvk-remix** repository to `external/dxvk-remix`
3. **Comments out Slang in packman-external.xml** to prevent packman from managing Slang
4. **Downloads dependencies** via packman (excluding Slang)
5. **Creates custom Slang directory** at `external/dxvk-remix/external/slang` with your Debug build
6. **Compiles RTX Remix shaders** with SPIRV validation enabled
7. **Verifies shader output** (checks for generated `.h` and `.spv` files)

## Prerequisites

- **Windows** (same as CI environment)
- **Git Bash** or similar bash shell
- **Python 3.11+** (for Meson build system)
- **Ninja** (usually pre-installed with VS)
- **Slang Debug build** already available at `build/Debug/bin/slangc.exe`
  - Build first with: `cmake.exe --preset default && cmake.exe --build --preset debug`

## Usage

### Build Slang First

Before running the script, ensure Slang is built in Debug configuration:

```bash
cmake.exe --preset default
cmake.exe --build --preset debug
```

### Basic Usage

From the Slang repository root:

```bash
./extras/repro-remix.sh
```

This runs all steps: verifying Slang build, cloning dxvk-remix, and compiling shaders.

### Clean Start

To remove existing dxvk-remix clone and start fresh:

```bash
./extras/repro-remix.sh --clean
```

## Directory Structure

After running, you'll have:

```
slang/
├── build/Debug/bin/           # Your built Slang Debug binaries (required)
├── external/
│   └── dxvk-remix/            # Cloned RTX Remix repository
│       ├── packman-external.xml         # Packman config file
│       ├── packman-external.xml.backup  # Backup before modification
│       ├── external/slang/              # Your custom Debug Slang build
│       └── _BuildShadersOnly/           # Shader compilation output
│           ├── meson-logs/              # Build logs (useful for debugging)
│           └── src/dxvk/rtx_shaders/    # Generated .h and .spv files
└── extras/
    └── repro-remix.sh          # This script
```

## Approach: Comment Out Slang in Packman

This script uses a clean approach to avoid binary overwrite issues:

**What it does:**
- Comments out the Slang dependency in `packman-external.xml`
- Creates a backup file (`packman-external.xml.backup`)
- Packman skips downloading Slang entirely
- Creates `external/slang` directory manually with your Debug build

**Benefits:**
- ✅ **No overwrite risk** - Packman never downloads or manages Slang
- ✅ **Clean separation** - Your custom build is independent from packman
- ✅ **Easy to verify** - Check `packman-external.xml` to see commented section
- ✅ **Idempotent** - Can run multiple times safely (detects if already commented)
- ✅ **Reversible** - Backup file available for restoration

## Debugging Failed Runs

### Verify Packman XML Was Modified

Check that the Slang section was commented out:

```bash
cat external/dxvk-remix/packman-external.xml | grep -A 2 "slang"
```

You should see:
```xml
    <!-- Slang section commented out by repro-remix.sh
    <dependency name="slang" linkPath="external/slang">
        <package name="rtx-remix-slang" version="..." />
    </dependency>
    -->
```

### Verify Custom Slang Directory

```bash
ls external/dxvk-remix/external/slang/
external/dxvk-remix/external/slang/slangc.exe -version
```

### Check Build Logs

If shader compilation fails:

```bash
cat external/dxvk-remix/_BuildShadersOnly/meson-logs/meson-log.txt
```

### Inspect Generated Shaders

```bash
cd external/dxvk-remix/_BuildShadersOnly/src/dxvk/rtx_shaders/
ls *.h *.spv
```

## Environment Variables

The script sets these environment variables during shader compilation (matching CI):

- `SLANG_RUN_SPIRV_VALIDATION=1` - Enables SPIRV validation
- `SLANG_USE_SPV_SOURCE_LANGUAGE_UNKNOWN=1` - SPIRV source language setting

You can modify these in the script if needed for testing.

## Typical Workflow

When the nightly CI fails:

1. **Pull latest Slang changes**:
   ```bash
   git pull origin master
   ```

2. **Build Slang in Debug**:
   ```bash
   cmake.exe --preset default
   cmake.exe --build --preset debug
   ```

3. **Run reproduction script**:
   ```bash
   ./extras/repro-remix.sh
   ```

4. **If it fails locally**, investigate:
   - Check meson logs
   - Look at SPIRV validation errors
   - Compare IR dumps with working version
   - Use `-dump-ir` with problematic shaders

5. **Iterate and test fixes**:
   ```bash
   # Make changes to Slang source
   cmake.exe --build --preset debug
   ./extras/repro-remix.sh
   ```

## Differences from CI

The script closely matches the CI workflow, with these minor differences:

- **Cache**: CI caches build artifacts and packman packages (not replicated locally)
- **Timing**: CI has strict timeout limits (45 minutes total)
- **Notifications**: CI sends Slack notifications (not included in script)
- **Artifacts**: CI uploads logs on failure (script just leaves them on disk)

## Tips

- First run takes ~15-20 minutes (packman download + shader compilation)
- Subsequent runs are faster if packman cache exists (~10-15 minutes)
- The `external/dxvk-remix` clone uses `--depth 1` (shallow) to save space
- Packman downloads ~340MB to `C:\packman-repo` (persisted across runs)
- Debug builds include more symbols for debugging but run slower than Release

## Troubleshooting

### "slangc.exe not found"

Make sure you built Slang in Debug configuration first:
```bash
cmake.exe --preset default
cmake.exe --build --preset debug
```

### "Packman directory not found"

Packman may have failed. Check network connection and try again with `--clean`.

### "Unexpectedly low shader count"

If you see this error:
```
ERROR: Unexpectedly low shader count: 54 headers, 46 .spv files
ERROR: Expected at least 100 shaders - this indicates a partial build failure
```

This indicates a partial compilation failure. Common causes:
- Some shaders failed to compile (check meson logs)
- Build script stopped early
- Missing dependencies or tools

**To debug:**
```bash
# Verify the actual shader count
find external/dxvk-remix/_BuildShadersOnly/src/dxvk/rtx_shaders -name '*.spv' | wc -l
find external/dxvk-remix/_BuildShadersOnly/src/dxvk/rtx_shaders -name '*.h' | wc -l

# Check build logs for errors
cat external/dxvk-remix/_BuildShadersOnly/meson-logs/meson-log.txt | grep -i error

# Look for compilation failures
cat external/dxvk-remix/_BuildShadersOnly/meson-logs/meson-log.txt | grep -i "failed"
```

### "packman-external.xml not found"

If you see this error, the dxvk-remix repository structure may have changed. Check:
```bash
ls external/dxvk-remix/
```

### XML commenting failed

If the awk command fails to comment out Slang, you can manually edit:
```bash
# Open in editor
notepad external/dxvk-remix/packman-external.xml

# Find the slang section and wrap it in <!-- ... -->
```

### PowerShell execution errors

If `build_shaders_only.ps1` fails, you might need to adjust execution policy:
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

## Related Documentation

- [Main workflow file](../.github/workflows/compile-rtx-remix-shaders-nightly.yml)
- [Building Slang](../docs/building.md)
- [SPIRV debugging guide](../CLAUDE.md#slangc-with--target-spirv-asm)
- [dxvk-remix repository](https://github.com/NVIDIAGameWorks/dxvk-remix)
