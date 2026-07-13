---
name: repro-remix
description: Reproduce RTX Remix shader compilation issues locally. Clones dxvk-remix, replaces Slang with your local build, and compiles all RTX Remix shaders with SPIRV validation.
argument-hint: "[--clean]"
allowed-tools:
  - Bash
  - Read
  - Grep
  - Glob
---

# RTX Remix Shader Compatibility Testing

The repository includes a script to reproduce RTX Remix shader compilation issues locally.

## Prerequisites

Build Slang in Debug first:
```bash
cmake.exe --preset default
cmake.exe --build --preset debug
```

## Usage

```bash
# Run RTX Remix shader compilation test
./extras/repro-remix.sh

# Clean existing dxvk-remix clone and start fresh
./extras/repro-remix.sh --clean
```

## What it does

- Clones NVIDIA RTX Remix repository to `external/dxvk-remix/`
- Comments out Slang in packman config to prevent overwriting
- Replaces with your Debug build
- Compiles all RTX Remix shaders with SPIRV validation enabled
- Verifies shader compilation output

## When to use

- When RTX Remix nightly CI workflow fails
- To test Slang changes against a large real-world shader codebase
- To reproduce shader compilation issues before fixing them

See `extras/repro-remix.md` for detailed documentation.

## Interactive Workflow

1. Check if Slang is already built in Debug (`build/Debug/bin/slangc` exists)
2. If not, build it first
3. If `$ARGUMENTS` contains `--clean`, pass `--clean` to the script
4. Run `./extras/repro-remix.sh` and monitor output for failures
5. If failures occur, help the user analyze and fix them
