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
- A platform Falcor supports (Windows or Linux) — see [Platforms and WSL](#platforms-and-wsl) for how the build target is chosen and overridden.

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

| Option                             | Default                                                         | Meaning                                                                                 |
| ---------------------------------- | --------------------------------------------------------------- | --------------------------------------------------------------------------------------- |
| `--target-os <windows\|linux>`     | by host (Windows/WSL → `windows`, Linux → `linux`)              | OS to build Falcor/Slang for (see Platforms and WSL).                                   |
| `--slang-config <Debug\|Release>`  | `Release`                                                       | Which local Slang build (`build/<config>`) to point Falcor at.                          |
| `--falcor-config <Debug\|Release>` | `Release`                                                       | Which Falcor configuration to build and test.                                           |
| `--preset <name>`                  | `windows-vs2022` (windows target), `linux-clang` (linux target) | Falcor CMake configure preset.                                                          |
| `--slang-dir <path>`               | this repository's root                                          | `FALCOR_LOCAL_SLANG_DIR` (Slang source tree).                                           |
| `--falcor-dir <path>`              | `external/falcor`                                               | Where Falcor is cloned/used.                                                            |
| `--ref <git-ref>`                  | Falcor's default branch                                         | Falcor branch/tag/commit to check out (fresh clones only).                              |
| `--config-string <str>`            | `<preset>-<falcor-config>`                                      | Override the value passed to Falcor's test runner `--config`.                           |
| `--image-tests`                    | off                                                             | Also run Falcor's image tests (requires a GPU).                                         |
| `--werror` (`--strict-warnings`)   | off (warnings relaxed)                                          | Keep Falcor's strict warnings-as-errors; by default the build relaxes them (see below). |
| `--clean`                          | off                                                             | For `clone`: remove an existing Falcor clone first.                                     |

### Mixing build variants

The Slang configuration (`--slang-config`) and the Falcor configuration (`--falcor-config`) are chosen independently, because neither side redirects the other's output:

```bash
# Debug Slang, Release Falcor
./extras/falcor.sh build --slang-config Debug --falcor-config Release

# Release Slang, Debug Falcor
./extras/falcor.sh build --slang-config Release --falcor-config Debug
```

## Slang build options

Falcor consumes Slang as a CMake-imported library, so the Slang **defaults work** — you do not need a special configuration. The only options that matter for Falcor compatibility:

| Slang CMake option                                                      | Needed value       | Why                                                                                                                                        |
| ----------------------------------------------------------------------- | ------------------ | ------------------------------------------------------------------------------------------------------------------------------------------ |
| `SLANG_ENABLE_GFX`                                                      | `ON` (default)     | Falcor imports both `slang` and `slang-gfx`; with `-DSLANG_ENABLE_GFX=0` there is no `gfx` library to import, so Falcor's configure fails. |
| `SLANG_LIB_TYPE`                                                        | `SHARED` (default) | Falcor imports the shared `slang`/`slang-gfx` libraries (`libslang.so` / `slang.dll`).                                                     |
| `SLANG_ENABLE_TESTS`, `SLANG_ENABLE_EXAMPLES`, `SLANG_ENABLE_SLANG_RHI` | any                | Not consumed by Falcor; leave at defaults or disable to speed the Slang build.                                                             |

`cmake --preset default` already sets the needed values. The CI Falcor job (`.github/workflows/falcor-test.yml`) uses a trimmed, known-good Slang configuration, but that set is for CI's copy-into-prebuilt-Falcor flow; for this script the defaults are sufficient. **Do not disable `SLANG_ENABLE_GFX`.**

## Platforms and WSL

Falcor builds on Windows and Linux. The script picks a build target by host, overridable with `--target-os`:

| Host                           | Default target | Notes                                                                |
| ------------------------------ | -------------- | -------------------------------------------------------------------- |
| native Windows (git-bash/MSYS) | `windows`      | Uses `windows-vs2022` preset, `setup.bat`, `slang.dll`.              |
| WSL                            | `windows`      | WSL runs on a Windows host; many Slang devs build Slang for Windows. |
| native Linux                   | `linux`        | Uses `linux-clang` preset, `setup.sh`, `libslang.so`.                |

**WSL.** WSL reports as Linux (`uname -s` = `Linux`) but runs on a Windows machine, so the script detects it (`/proc/version` / `$WSL_DISTRO_NAME`) and defaults to a **Windows** target — matching a Slang build done on the Windows side (`build/<config>/bin/slang.dll`). Under WSL with a Windows target the script drives the Windows toolchain (`cmake.exe`, `cmd.exe`) and translates paths with `wslpath`. If you instead built Slang for Linux inside WSL and want an in-WSL Linux build, pass `--target-os linux`:

```bash
# In-WSL Linux build/test (needs a Linux Slang build: libslang.so)
./extras/falcor.sh build --target-os linux
```

Notes for WSL/Windows-drive checkouts:

- **Line endings.** The `clone` command clones Falcor with `core.autocrlf=false` so its shell scripts keep LF endings; a clone made earlier with `autocrlf=true` can fail Falcor's `setup.sh` with `/bin/sh^M: bad interpreter` or `packman: not found` — re-clone with `clone --clean`.
- **`/mnt/<drive>` paths.** Building under a Windows drive mounted in WSL (e.g. `/mnt/d/...`) is slower and occasionally trips Falcor's packman dependency fetch; a checkout on a native filesystem for the chosen toolchain is more reliable.

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
- **CMake ≥ 4.** Falcor's pinned `pybind11` declares `cmake_minimum_required(VERSION < 3.5)`, which CMake 4 rejects. The `build` command passes `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` to **Falcor's** configure (only there, never to the Slang build) to allow it to configure; the mitigation is the one CMake itself suggests in the error.
- **Warnings as errors.** Falcor builds with warnings-as-errors (`/WX` on MSVC) and has no CMake toggle for it, so a newer Slang's deprecation warning (e.g. a `[[deprecated]]` reflection method) would fail Falcor's build. By default `build` relaxes this — it backs up and patches `Source/Falcor/CMakeLists.txt` (in the clone only, reversibly) to drop the warnings-as-error flags, so the build proceeds to any real errors. The **warnings still print**, so Slang version-skew stays visible; this is not suppression of the signal. Pass `--werror` to keep Falcor strict, or re-run `clone --clean` to restore the original.

## Related documentation

- [Falcor CI workflow](../.github/workflows/falcor-test.yml)
- [Building Slang](../docs/building.md)
- [Falcor repository](https://github.com/NVIDIAGameWorks/Falcor)
