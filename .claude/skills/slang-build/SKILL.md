---
name: slang-build
description: Platform-aware build instructions for the Slang compiler. Detects OS, selects CMake presets, handles submodules, and knows platform limitations. Referenced by other skills.
---

# Slang Build

**For**: Building the Slang compiler on any supported platform.

**Usage**: Referenced by other skills. Can also be invoked directly: `/slang-build [debug|release]`

---

## Step 1: Detect Platform

Detect the current platform and set variables accordingly:

```bash
# Detect OS
case "$(uname -s)" in
  Darwin)  PLATFORM="macos" ;;
  Linux)   PLATFORM="linux" ;;
  MINGW*|MSYS*|CYGWIN*) PLATFORM="windows" ;;
  *)       PLATFORM="unknown" ;;
esac

# Detect WSL
if grep -qi microsoft /proc/version 2>/dev/null; then
  PLATFORM="wsl"
fi
```

### Platform Capabilities

| Capability | macOS | Linux | Windows | WSL |
|-----------|-------|-------|---------|-----|
| CPU tests | yes | yes | yes | yes |
| Vulkan | limited | yes | yes | yes |
| CUDA | no | yes | yes | yes |
| D3D12 | no | no | yes | yes |
| Metal | yes | no | no | no |
| SPIRV validation | yes | yes | yes | yes |

**Important**: On macOS, CUDA and D3D tests will be **skipped**, not failed. A test run
showing all passes with many skips may hide real failures. Always check skip counts.

---

## Step 2: Initialize Submodules

Required before first build:

```bash
git submodule update --init --recursive
```

---

## Step 3: Configure

```bash
cmake --preset default
```

On Windows (non-WSL), prefer the Visual Studio preset:

```bash
cmake.exe --preset vs2022 -DSLANG_IGNORE_ABORT_MSG=ON
```

The `-DSLANG_IGNORE_ABORT_MSG=ON` flag suppresses modal abort dialogs during unattended builds.

### Optional: sccache

For faster rebuilds, pass `-DSLANG_USE_SCCACHE=ON` at configure time. Requires `sccache` in PATH.

**Debugging pitfall**: When debugging with printf/debug output, sccache may return cached
objects that don't include your changes. If edits seem to have no effect, either:
- Force recache: `SCCACHE_RECACHE=1 cmake --build --preset <preset> --target slangc`
- Or temporarily disable: reconfigure with `-DSLANG_USE_SCCACHE=OFF`

---

## Step 4: Build

### Preset Selection

| Use Case | Preset | Binary Path | When |
|----------|--------|-------------|------|
| **Default** | `releaseWithDebugInfo` | `build/RelWithDebInfo/bin/` | General development, most work |
| Bug investigation | `debug` | `build/Debug/bin/` | Need assertions, full debugging symbols |
| Performance testing | `release` | `build/Release/bin/` | Benchmarking, CI-like validation |

**Default**: Use `releaseWithDebugInfo` for general development — it balances debug info with
reasonable build times. Use `debug` when you need assertions to catch invariant violations.

### Build Commands

```bash
# Build specific targets (preferred — faster than building everything)
cmake --build --preset <preset> --target slangc slang-test \
  >/dev/null 2>&1 || cmake --build --preset <preset> --target slangc slang-test
```

The redirect-and-retry pattern avoids wasting LLM tokens on successful build output.
On failure, the second invocation shows the actual errors.

### WSL Notes

On WSL, append `.exe` to host tools:
- `cmake.exe` instead of `cmake`
- `python.exe` instead of `python`
- `gh.exe` instead of `gh`

---

## Step 5: Verify

After building, verify the binaries exist:

```bash
# Adjust path based on preset
ls build/<Debug|RelWithDebInfo|Release>/bin/slangc
ls build/<Debug|RelWithDebInfo|Release>/bin/slang-test
```

---

## Quick Reference

### One-liner: configure + build (first time)

```bash
git submodule update --init --recursive && \
cmake --preset default && \
cmake --build --preset releaseWithDebugInfo --target slangc slang-test \
  >/dev/null 2>&1 || cmake --build --preset releaseWithDebugInfo --target slangc slang-test
```

### One-liner: rebuild (already configured)

```bash
cmake --build --preset releaseWithDebugInfo --target slangc slang-test \
  >/dev/null 2>&1 || cmake --build --preset releaseWithDebugInfo --target slangc slang-test
```
