#!/usr/bin/env bash
#
# falcor.sh - Test Slang against the public Falcor on a local machine
#
# Clones the public Falcor (https://github.com/NVIDIAGameWorks/Falcor), builds
# it against a *local* Slang build, and runs Falcor's tests. This lets a
# developer confirm that a local Slang change still builds and runs Falcor,
# off-loading the bottlenecked CI Falcor runner.
#
# Falcor is wired to the local Slang via Falcor's own CMake hook
# (FALCOR_LOCAL_SLANG / FALCOR_LOCAL_SLANG_DIR / FALCOR_LOCAL_SLANG_BUILD_DIR);
# Falcor's `deploy_dependencies` target then copies the local Slang binaries
# into Falcor's output directory during the build, so a separate manual install
# is normally unnecessary - the `install` command exists only to refresh those
# binaries after a Slang-only rebuild.
#
# Platforms: Falcor builds on Windows and Linux. The build target is chosen by
# host (and overridable with --target-os):
#   - native Windows (git-bash/MSYS)  -> windows
#   - WSL                             -> windows (WSL runs on a Windows host, and
#                                        many Slang devs build Slang for Windows;
#                                        pass --target-os linux for an in-WSL
#                                        Linux build instead)
#   - native Linux                    -> linux
# Under WSL with a Windows target, Windows tools (cmake.exe, cmd.exe) are used
# and paths are translated with wslpath.
#
# Prerequisites:
#   Slang must already be built for the target OS with the gfx target ENABLED
#   (the default), e.g.
#     cmake --preset default
#     cmake --build --preset release   # or --preset debug
#   Falcor imports both `slang` and `slang-gfx`, so a Slang built with
#   -DSLANG_ENABLE_GFX=0 will not satisfy Falcor's CMake imports.
#   The host also needs whatever Falcor itself requires (CMake, a C++ toolchain,
#   and a GPU/driver for the image tests).
#
# Usage:
#   ./extras/falcor.sh <command> [options]
#
# This script must be run from a checkout of the Slang repository (it locates
# the repository root from its own path).
#
set -e # Exit on error
set -u # Exit on undefined variable

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[OK]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[FAIL]${NC} $1" >&2; }

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SLANG_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$SLANG_ROOT" || exit 1

if [ ! -f "CMakeLists.txt" ]; then
  log_error "This script must be run from a Slang repository checkout"
  exit 1
fi

# Detect the host OS and whether we are running under WSL (Linux-by-uname, but
# on a Windows machine). These pick the default build target below.
case "$(uname -s)" in
MINGW* | MSYS* | CYGWIN*) RAW_HOST=windows ;;
Linux*) RAW_HOST=linux ;;
*) RAW_HOST=other ;;
esac

IS_WSL=false
if grep -qiE "microsoft|wsl" /proc/version 2>/dev/null || [ -n "${WSL_DISTRO_NAME:-}" ]; then
  IS_WSL=true
fi

# Default build target: native Windows and WSL build for Windows; native Linux
# builds for Linux. Unsupported hosts (e.g. macOS) fall through to linux naming
# but get no default preset, so build/install will require --preset.
if [ "$RAW_HOST" = windows ] || [ "$IS_WSL" = true ]; then
  DEFAULT_TARGET_OS=windows
else
  DEFAULT_TARGET_OS=linux
fi

usage() {
  cat <<EOF
falcor.sh - Test Slang against the public Falcor on a local machine

Usage:
  ./extras/falcor.sh <command> [options]

Commands:
  clone      Clone Falcor into the Falcor dir and fetch its dependencies
  build      Configure and build Falcor against the local Slang build
  install    Re-deploy the local Slang binaries into an already-built Falcor
             (only needed after rebuilding Slang without rebuilding Falcor)
  test       Run Falcor's unit tests (add --image-tests to also run image tests)
  all        Run clone, build and test in sequence

Options:
  --target-os <windows|linux>      OS to build Falcor/Slang for
                                   (default: $DEFAULT_TARGET_OS on this host;
                                   WSL defaults to windows - see header)
  --slang-config <Debug|Release>   Slang build config to use (default: Release)
  --falcor-config <Debug|Release>  Falcor build config to build/test (default: Release)
  --preset <name>                  Falcor CMake configure preset
                                   (default: windows-vs2022 for a windows target,
                                   linux-clang for a linux target)
  --slang-dir <path>               Slang source dir for FALCOR_LOCAL_SLANG_DIR
                                   (default: this repository's root)
  --falcor-dir <path>              Where to clone/use Falcor
                                   (default: external/falcor)
  --ref <git-ref>                  Falcor branch/tag/commit to clone
                                   (default: Falcor's default branch; fresh clones only)
  --config-string <str>            Override the test runner --config value
                                   (default: <preset>-<falcor-config>)
  --image-tests                    Also run Falcor's image tests (needs a GPU)
  --clean                          For clone: remove any existing Falcor clone first
  --help                           Show this help message
EOF
}

# Defaults (may be overridden by options below). TARGET_OS and FALCOR_PRESET are
# left empty here and derived after parsing so --target-os can influence the
# default preset.
COMMAND=""
TARGET_OS=""
SLANG_CONFIG="Release"
FALCOR_CONFIG="Release"
FALCOR_PRESET=""
SLANG_DIR="$SLANG_ROOT"
FALCOR_DIR="$SLANG_ROOT/external/falcor"
FALCOR_REF=""
CONFIG_STRING=""
IMAGE_TESTS=false
CLEAN=false

# Abort with a clear message when an option that expects a value has none.
# $1 is the option name, $2 is the remaining argument count ($#); checking the
# count (rather than $2 directly) keeps this safe under `set -u`.
require_value() {
  if [ "$2" -lt 2 ]; then
    log_error "Option $1 requires a value"
    exit 1
  fi
}

while [[ $# -gt 0 ]]; do
  case $1 in
  --target-os)
    require_value "$1" "$#"
    TARGET_OS="$2"
    shift 2
    ;;
  --slang-config)
    require_value "$1" "$#"
    SLANG_CONFIG="$2"
    shift 2
    ;;
  --falcor-config)
    require_value "$1" "$#"
    FALCOR_CONFIG="$2"
    shift 2
    ;;
  --preset)
    require_value "$1" "$#"
    FALCOR_PRESET="$2"
    shift 2
    ;;
  --slang-dir)
    require_value "$1" "$#"
    SLANG_DIR="$(cd "$2" && pwd)"
    shift 2
    ;;
  --falcor-dir)
    require_value "$1" "$#"
    FALCOR_DIR="$2"
    shift 2
    ;;
  --ref)
    require_value "$1" "$#"
    FALCOR_REF="$2"
    shift 2
    ;;
  --config-string)
    require_value "$1" "$#"
    CONFIG_STRING="$2"
    shift 2
    ;;
  --image-tests)
    IMAGE_TESTS=true
    shift
    ;;
  --clean)
    CLEAN=true
    shift
    ;;
  -h | --help)
    usage
    exit 0
    ;;
  -*)
    log_error "Unknown option: $1"
    usage
    exit 1
    ;;
  *)
    if [ -z "$COMMAND" ]; then
      COMMAND="$1"
      shift
    else
      log_error "Unexpected argument: $1"
      usage
      exit 1
    fi
    ;;
  esac
done

if [ -z "$COMMAND" ]; then
  usage
  exit 1
fi

# Resolve the build target and its default preset now that options are parsed.
[ -z "$TARGET_OS" ] && TARGET_OS="$DEFAULT_TARGET_OS"
case "$TARGET_OS" in
windows | linux) ;;
*)
  log_error "Invalid --target-os '$TARGET_OS' (expected: windows or linux)"
  exit 1
  ;;
esac

if [ -z "$FALCOR_PRESET" ]; then
  if [ "$RAW_HOST" = other ]; then
    # e.g. macOS: Falcor ships no preset we can pick, so require --preset.
    FALCOR_PRESET=""
  elif [ "$TARGET_OS" = windows ]; then
    FALCOR_PRESET="windows-vs2022"
  else
    FALCOR_PRESET="linux-clang"
  fi
fi

# Under WSL with a Windows target we must drive the Windows toolchain (cmake.exe,
# cmd.exe) and translate WSL paths to Windows paths for those tools.
USE_EXE=false
if [ "$IS_WSL" = true ] && [ "$TARGET_OS" = windows ]; then
  USE_EXE=true
fi
CMAKE=cmake
[ "$USE_EXE" = true ] && CMAKE=cmake.exe

# Translate a WSL path to a Windows path when invoking Windows tools; otherwise
# echo it unchanged. Used for absolute paths handed to cmake.exe under WSL.
to_native() {
  if [ "$USE_EXE" = true ] && command -v wslpath >/dev/null 2>&1; then
    wslpath -w "$1"
  else
    printf '%s\n' "$1"
  fi
}

# A Falcor configure preset is only needed by commands that configure/build
# Falcor (build, install) or derive the default test config string; clone does
# not need one. Hosts without an auto-detected preset (e.g. macOS) must pass
# --preset for those commands.
require_preset() {
  if [ -z "$FALCOR_PRESET" ]; then
    log_error "No Falcor preset for this host ($(uname -s)); pass --preset <name>"
    exit 1
  fi
}

# Falcor writes runtime binaries to build/<preset>/bin/<CONFIG> (the
# FALCOR_OUTPUT_DIRECTORY for the Ninja Multi-Config presets).
falcor_build_dir() { printf '%s\n' "$FALCOR_DIR/build/$FALCOR_PRESET"; }
falcor_output_bin() { printf '%s\n' "$FALCOR_DIR/build/$FALCOR_PRESET/bin/$FALCOR_CONFIG"; }

# Run Falcor's dependency-fetch step (git submodules + packman) for the target.
run_falcor_setup() {
  if [ "$TARGET_OS" = windows ]; then
    cmd.exe /C "setup.bat"
  else
    # Invoke via `sh` so that a CRLF (^M) shebang on a Windows-checked-out
    # setup.sh doesn't make the kernel look for the interpreter `/bin/sh<CR>`.
    sh ./setup.sh
  fi
}

# Invoke a Falcor test wrapper. Under WSL with a Windows target the wrapper is a
# .bat that must be launched through cmd.exe; otherwise it runs directly.
run_test_runner() {
  local runner="$1"
  shift
  if [ "$USE_EXE" = true ]; then
    (cd "$(dirname "$runner")" && cmd.exe /C "$(basename "$runner")" "$@")
  else
    "$runner" "$@"
  fi
}

# Fail early, with clear guidance, when the CMake that the chosen target needs
# isn't reachable, or (for a WSL->Windows build) when Falcor lives on a
# WSL-native path that Windows tools cannot use as a working directory.
preflight_target_tools() {
  if ! command -v "$CMAKE" >/dev/null 2>&1; then
    log_error "'$CMAKE' not found on PATH for target '$TARGET_OS'."
    if [ "$USE_EXE" = true ]; then
      log_error "A Windows CMake (cmake.exe) must be reachable from WSL, or use --target-os linux."
    fi
    exit 1
  fi
  if [ "$USE_EXE" = true ]; then
    case "$(to_native "$FALCOR_DIR")" in
    '\\'*)
      log_error "Falcor is on a WSL-native path; Windows tools need a Windows-drive path."
      log_error "Clone under /mnt/<drive>/... for --target-os windows, or use --target-os linux."
      exit 1
      ;;
    esac
  fi
}

# Fail early (with build instructions) when the requested local Slang build is
# missing, and warn when slang-gfx is absent since Falcor imports it. The
# artifact names follow the build target, not the host: a Windows target needs
# slang.dll, a Linux target needs libslang.so.
verify_slang_build() {
  if [ ! -f "$SLANG_DIR/include/slang.h" ]; then
    log_error "Slang headers not found at $SLANG_DIR/include/slang.h"
    log_error "Pass --slang-dir to point at the Slang source tree."
    exit 1
  fi

  local base="$SLANG_DIR/build/$SLANG_CONFIG"
  local slang_lib gfx_lib
  if [ "$TARGET_OS" = windows ]; then
    slang_lib="$base/bin/slang.dll"
    gfx_lib="$base/bin/gfx.dll"
  else
    slang_lib="$base/lib/libslang.so"
    gfx_lib="$base/lib/libgfx.so"
  fi

  if [ ! -f "$slang_lib" ]; then
    log_error "Local Slang $SLANG_CONFIG build for target '$TARGET_OS' not found: $slang_lib"
    if [ "$IS_WSL" = true ]; then
      log_error "WSL detected with --target-os $TARGET_OS. If you built Slang for the"
      log_error "other OS, switch with --target-os windows|linux; otherwise build Slang:"
    else
      log_error "Build Slang first, for example:"
    fi
    log_error "  cmake --preset default"
    log_error "  cmake --build --preset $(echo "$SLANG_CONFIG" | tr '[:upper:]' '[:lower:]')"
    exit 1
  fi

  if [ ! -f "$gfx_lib" ]; then
    log_warning "slang-gfx library not found: $gfx_lib"
    log_warning "Falcor imports slang-gfx; build Slang with SLANG_ENABLE_GFX=ON (the default)."
  fi

  log_success "Found local Slang $SLANG_CONFIG build (target $TARGET_OS) at $base"
}

ensure_falcor_present() {
  if [ ! -d "$FALCOR_DIR" ]; then
    log_error "Falcor not found at $FALCOR_DIR. Run: $0 clone"
    exit 1
  fi
}

# Warn when a previously-cloned Falcor has CRLF line endings in its shell
# scripts (a Windows git checkout with core.autocrlf=true), which breaks
# setup.sh under a Unix shell. Fresh clones avoid this (we clone with
# core.autocrlf=false), so the fix is to re-clone with --clean.
warn_if_crlf_setup() {
  if [ "$TARGET_OS" != windows ] && [ -f "$FALCOR_DIR/setup.sh" ] &&
    grep -q $'\r' "$FALCOR_DIR/setup.sh" 2>/dev/null; then
    log_warning "Falcor's setup.sh has CRLF (Windows) line endings."
    log_warning "If setup fails with '/bin/sh^M: bad interpreter' or 'packman: not found',"
    log_warning "re-clone with: $0 clone --clean (fresh clones use core.autocrlf=false)."
  fi
}

cmd_clone() {
  if [ "$CLEAN" = true ] && [ -d "$FALCOR_DIR" ]; then
    # Resolve to a canonical absolute path before deleting so that aliases like
    # `.`, `..`, or a trailing slash cannot bypass the string-equality guards
    # below and wipe an unintended directory.
    local canon slang_canon
    canon="$(cd "$FALCOR_DIR" && pwd -P)"
    slang_canon="$(cd "$SLANG_ROOT" && pwd -P)"
    # Refuse the filesystem root, $HOME, the Slang checkout, any ancestor of it,
    # or anything that is not itself a git clone.
    if [ "$canon" = "/" ] || [ "$canon" = "$HOME" ] || [ "$canon" = "$slang_canon" ] || [ ! -d "$canon/.git" ]; then
      log_error "Refusing to --clean '$canon' (root/\$HOME, the Slang checkout, or not a git clone)"
      exit 1
    fi
    case "$slang_canon/" in
    "$canon"/*)
      log_error "Refusing to --clean '$canon' (it contains the Slang checkout)"
      exit 1
      ;;
    esac
    log_warning "Removing existing Falcor clone at $canon..."
    rm -rf "$canon"
    # If the configured path was a symlink to that clone, drop the now-dangling
    # link so the re-clone below creates a fresh real directory.
    [ -L "$FALCOR_DIR" ] && rm -f "$FALCOR_DIR"
  fi

  if [ -d "$FALCOR_DIR/.git" ]; then
    log_info "Falcor already cloned at $FALCOR_DIR"
  else
    log_info "Cloning Falcor into $FALCOR_DIR..."
    mkdir -p "$(dirname "$FALCOR_DIR")"
    # Force LF line endings regardless of the user's global git config so that
    # Falcor's shell scripts (setup.sh, tools/packman/packman) stay runnable
    # under a Unix shell (e.g. WSL on a Windows drive).
    git clone -c core.autocrlf=false -c core.eol=lf \
      https://github.com/NVIDIAGameWorks/Falcor.git "$FALCOR_DIR"
    if [ -n "$FALCOR_REF" ]; then
      log_info "Checking out Falcor ref: $FALCOR_REF"
      (cd "$FALCOR_DIR" && git checkout "$FALCOR_REF")
    fi
  fi

  warn_if_crlf_setup
  log_info "Fetching Falcor dependencies (this runs Falcor's setup script)..."
  (cd "$FALCOR_DIR" && run_falcor_setup)
  log_success "Falcor ready at $FALCOR_DIR ($(cd "$FALCOR_DIR" && git rev-parse --short HEAD))"
}

cmd_build() {
  require_preset
  ensure_falcor_present
  preflight_target_tools
  verify_slang_build

  [ "$IS_WSL" = true ] && log_info "WSL detected; building for target '$TARGET_OS' (override with --target-os)."

  log_info "Configuring Falcor (preset $FALCOR_PRESET) with local Slang ($SLANG_CONFIG)..."
  # FALCOR_LOCAL_SLANG_BUILD_DIR is resolved relative to FALCOR_LOCAL_SLANG_DIR
  # by Falcor (SLANG_DIR = DIR/BUILD_DIR), so it names the per-config build
  # subdirectory (e.g. build/Release), not an absolute path.
  # -DCMAKE_POLICY_VERSION_MINIMUM=3.5 lets Falcor's pinned pybind11 (which
  # declares cmake_minimum_required < 3.5) configure under CMake >= 4; it is
  # scoped to Falcor's configure here and never applied to the Slang build.
  (
    cd "$FALCOR_DIR" &&
      "$CMAKE" --preset "$FALCOR_PRESET" \
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
        -DFALCOR_LOCAL_SLANG=ON \
        -DFALCOR_LOCAL_SLANG_DIR="$(to_native "$SLANG_DIR")" \
        -DFALCOR_LOCAL_SLANG_BUILD_DIR="build/$SLANG_CONFIG"
  )

  log_info "Building Falcor ($FALCOR_CONFIG)..."
  "$CMAKE" --build "$(to_native "$(falcor_build_dir)")" --config "$FALCOR_CONFIG"
  log_success "Falcor built; local Slang deployed via deploy_dependencies."
}

cmd_install() {
  require_preset
  ensure_falcor_present
  preflight_target_tools
  verify_slang_build

  log_info "Re-deploying local Slang into Falcor (deploy_dependencies)..."
  if "$CMAKE" --build "$(to_native "$(falcor_build_dir)")" --config "$FALCOR_CONFIG" --target deploy_dependencies; then
    log_success "Re-deployed local Slang via Falcor's deploy_dependencies target."
    return 0
  fi

  log_warning "deploy_dependencies unavailable; falling back to a direct copy."
  local out base
  out="$(falcor_output_bin)"
  base="$SLANG_DIR/build/$SLANG_CONFIG"
  mkdir -p "$out"
  # Copy just the shared runtime libraries. verify_slang_build already confirmed
  # the primary one is present, so each glob matches at least that file; matching
  # by extension also skips any subdirectories.
  if [ "$TARGET_OS" = windows ]; then
    cp -f "$base/bin/"*.dll "$out/"
  else
    cp -f "$base/lib/"*.so* "$out/"
  fi
  log_success "Copied local Slang runtime libraries into $out"
}

cmd_test() {
  ensure_falcor_present

  if [ -z "$CONFIG_STRING" ] && [ -z "$FALCOR_PRESET" ]; then
    log_error "No test config; pass --preset (to derive <preset>-<config>) or --config-string"
    exit 1
  fi
  local config_string="${CONFIG_STRING:-$FALCOR_PRESET-$FALCOR_CONFIG}"
  local ext=sh
  [ "$TARGET_OS" = windows ] && ext=bat
  local unit_runner="$FALCOR_DIR/tests/run_unit_tests.$ext"
  local image_runner="$FALCOR_DIR/tests/run_image_tests.$ext"

  if [ ! -f "$unit_runner" ]; then
    log_error "Falcor test runner not found: $unit_runner"
    log_error "Build Falcor first: $0 build"
    exit 1
  fi

  log_info "Running Falcor unit tests (--config $config_string)..."
  run_test_runner "$unit_runner" --config "$config_string"

  if [ "$IMAGE_TESTS" = true ]; then
    if [ ! -f "$image_runner" ]; then
      log_error "Falcor image-test runner not found: $image_runner"
      exit 1
    fi
    log_info "Running Falcor image tests (--config $config_string)..."
    run_test_runner "$image_runner" --config "$config_string" --run-only
  fi
  log_success "Falcor tests finished."
}

case "$COMMAND" in
clone) cmd_clone ;;
build) cmd_build ;;
install) cmd_install ;;
test) cmd_test ;;
all)
  cmd_clone
  cmd_build
  cmd_test
  ;;
*)
  log_error "Unknown command: $COMMAND"
  usage
  exit 1
  ;;
esac
