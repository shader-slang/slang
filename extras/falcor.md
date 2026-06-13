# Testing Slang Against the Public Falcor

`falcor.sh` builds and tests the public [Falcor](https://github.com/NVIDIAGameWorks/Falcor) against a **local** Slang build. It lets a developer confirm that a Slang change still builds and runs Falcor on their own machine, instead of waiting on the bottlenecked CI Falcor runner.

## Purpose

Falcor is a real-world consumer of Slang. CI runs a Falcor job (`.github/workflows/falcor-test.yml`), but that runner is a shared bottleneck. This script reproduces the same kind of validation locally: it clones Falcor, points it at your local Slang build, builds it, and runs Falcor's unit (and optionally image) tests.

## How the local Slang is wired in

Falcor exposes CMake cache variables to consume a local Slang build instead of the version it normally fetches via packman (pinned at `2024.1.34`):

- `FALCOR_LOCAL_SLANG` (`ON`/`OFF`) — enable the local-Slang path.
- `FALCOR_LOCAL_SLANG_DIR` — the Slang **source** directory. Falcor reads public headers from `<dir>/include`.
- `FALCOR_LOCAL_SLANG_BUILD_DIR` — the Slang **build** directory, resolved **relative to** `FALCOR_LOCAL_SLANG_DIR`. Falcor computes `SLANG_DIR = FALCOR_LOCAL_SLANG_DIR/FALCOR_LOCAL_SLANG_BUILD_DIR` and then imports `slang` and `slang-gfx` from `SLANG_DIR/lib` (and `SLANG_DIR/bin` on Windows).

Because Slang's default (Ninja Multi-Config) build writes each configuration to `build/<config>/{bin,lib}`, `FALCOR_LOCAL_SLANG_BUILD_DIR` must be the **per-config** subdirectory, e.g. `build/Release` — not a bare `build`. The script sets these variables for you from `--slang-dir` and `--slang-config`.

During the Falcor build, Falcor's `deploy_dependencies` target copies the imported Slang binaries into Falcor's output directory, so no separate install step is normally needed. The `install` command re-runs that deploy step to refresh the binaries after you rebuild only Slang. If `deploy_dependencies` is unavailable, `install` falls back to copying only the local Slang binaries into Falcor's output directory — a Slang-only refresh, not a full Falcor dependency deploy.

## Prerequisites

- A Slang build at `build/<config>/` with the **gfx target enabled** (the default). Falcor imports both `slang` and `slang-gfx`, so a Slang configured with `-DSLANG_ENABLE_GFX=0` will not satisfy Falcor's CMake imports. Build Slang with:
  ```bash
  cmake --preset default
  cmake --build --preset release   # or: --preset debug
  ```
- Whatever Falcor itself needs to build and run: CMake, a C++ toolchain, and — for the image tests — a GPU with a working driver.
- **Platforms**: Falcor ships Windows and Linux presets only. Linux is the primary, best-supported path for this script; Windows is handled on a best-effort basis. macOS is not supported by Falcor — pass `--preset` if you try anyway.

## Usage

Run from a Slang repository checkout:

```bash
# One-shot: clone Falcor, build it against the local Release Slang, run unit tests
./extras/falcor.sh all

# Or step by step
./extras/falcor.sh clone
./extras/falcor.sh build
./extras/falcor.sh test
```

### Commands

| Command   | What it does                                                                              |
| --------- | ----------------------------------------------------------------------------------------- |
| `clone`   | Clone Falcor into `external/falcor` and fetch its dependencies (Falcor's `setup` script). |
| `build`   | Configure + build Falcor against the local Slang build via `FALCOR_LOCAL_SLANG`.          |
| `install` | Re-deploy the local Slang binaries into an already-built Falcor (after a Slang rebuild).  |
| `test`    | Run Falcor's unit tests; add `--image-tests` to also run the image tests.                 |
| `all`     | Run `clone`, `build`, and `test` in sequence.                                             |

### Options

| Option                             | Default                                           | Meaning                                                        |
| ---------------------------------- | ------------------------------------------------- | -------------------------------------------------------------- |
| `--slang-config <Debug\|Release>`  | `Release`                                         | Which local Slang build (`build/<config>`) to point Falcor at. |
| `--falcor-config <Debug\|Release>` | `Release`                                         | Which Falcor configuration to build and test.                  |
| `--preset <name>`                  | `linux-clang` (Linux), `windows-vs2022` (Windows) | Falcor CMake configure preset.                                 |
| `--slang-dir <path>`               | this repository's root                            | `FALCOR_LOCAL_SLANG_DIR` (Slang source tree).                  |
| `--falcor-dir <path>`              | `external/falcor`                                 | Where Falcor is cloned/used.                                   |
| `--ref <git-ref>`                  | Falcor's default branch                           | Falcor branch/tag/commit to check out (fresh clones only).     |
| `--config-string <str>`            | `<preset>-<falcor-config>`                        | Override the value passed to Falcor's test runner `--config`.  |
| `--image-tests`                    | off                                               | Also run Falcor's image tests (requires a GPU).                |
| `--clean`                          | off                                               | For `clone`: remove an existing Falcor clone first.            |

### Mixing build variants

The Slang configuration (`--slang-config`) and the Falcor configuration (`--falcor-config`) are chosen independently, because neither side redirects the other's output:

```bash
# Debug Slang, Release Falcor
./extras/falcor.sh build --slang-config Debug --falcor-config Release

# Release Slang, Debug Falcor
./extras/falcor.sh build --slang-config Release --falcor-config Debug
```

## Typical workflow

1. Build Slang locally (with gfx enabled — the default):
   ```bash
   cmake --preset default
   cmake --build --preset release
   ```
2. Clone and build Falcor against it, then run the unit tests:
   ```bash
   ./extras/falcor.sh all
   ```
3. After changing Slang and rebuilding it, refresh Falcor's copy without a full Falcor rebuild, then re-test:
   ```bash
   cmake --build --preset release
   ./extras/falcor.sh install
   ./extras/falcor.sh test
   ```

## Known limitations

- **Version skew.** Public Falcor pins Slang `2024.1.34`. Building it against a top-of-tree Slang can surface C++ API/ABI or command-line drift — catching such breakage is part of the point, but expect that a green run is not guaranteed on the first try.
- **GPU required for image tests.** The unit tests are the default; image tests (`--image-tests`) need a GPU and reference images and will not pass on a headless/GPU-less machine.
- **Falcor build cost.** A full Falcor build is large; the first `clone`+`build` can take a long time and significant disk space under `external/falcor`.

## Related documentation

- [Falcor CI workflow](../.github/workflows/falcor-test.yml)
- [Building Slang](../docs/building.md)
- [Falcor repository](https://github.com/NVIDIAGameWorks/Falcor)
