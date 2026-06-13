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
# Prerequisites:
#   Slang must already be built with the gfx target ENABLED (the default), e.g.
#     cmake --preset default
#     cmake --build --preset release   # or --preset debug
#   Falcor imports both `slang` and `slang-gfx`, so a Slang built with
#   -DSLANG_ENABLE_GFX=0 will not satisfy Falcor's CMake imports.
#   The host also needs whatever Falcor itself requires (CMake, a C++ toolchain,
#   and a GPU/driver for the image tests). Falcor supports Linux and Windows.
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

# Host detection picks a sensible default Falcor preset and the platform-specific
# Slang shared-library names that Falcor imports.
case "$(uname -s)" in
MINGW* | MSYS* | CYGWIN*)
  HOST_OS=windows
  DEFAULT_PRESET=windows-vs2022
  ;;
Linux*)
  HOST_OS=linux
  DEFAULT_PRESET=linux-clang
  ;;
*)
  # Falcor only ships Windows and Linux presets, so for anything else (e.g.
  # macOS) require the user to pass --preset explicitly.
  HOST_OS=other
  DEFAULT_PRESET=""
  ;;
esac

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
  --slang-config <Debug|Release>   Slang build config to use (default: Release)
  --falcor-config <Debug|Release>  Falcor build config to build/test (default: Release)
  --preset <name>                  Falcor CMake configure preset
                                   (default: $DEFAULT_PRESET)
  --slang-dir <path>               Slang source dir for FALCOR_LOCAL_SLANG_DIR
                                   (default: this repository's root)
  --falcor-dir <path>              Where to clone/use Falcor
                                   (default: external/falcor)
  --ref <git-ref>                  Falcor branch/tag/commit to clone
                                   (default: Falcor's default branch)
  --config-string <str>            Override the test runner --config value
                                   (default: <preset>-<falcor-config>)
  --image-tests                    Also run Falcor's image tests (needs a GPU)
  --clean                          For clone: remove any existing Falcor clone first
  --help                           Show this help message
EOF
}

# Defaults (may be overridden by options below).
COMMAND=""
SLANG_CONFIG="Release"
FALCOR_CONFIG="Release"
FALCOR_PRESET="$DEFAULT_PRESET"
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

# Run Falcor's dependency-fetch step (git submodules + packman) for the host.
run_falcor_setup() {
  if [ "$HOST_OS" = windows ]; then
    cmd.exe /C "setup.bat"
  else
    ./setup.sh
  fi
}

# Fail early (with build instructions) when the requested local Slang build is
# missing, and warn when slang-gfx is absent since Falcor imports it.
verify_slang_build() {
  if [ ! -f "$SLANG_DIR/include/slang.h" ]; then
    log_error "Slang headers not found at $SLANG_DIR/include/slang.h"
    log_error "Pass --slang-dir to point at the Slang source tree."
    exit 1
  fi

  local base="$SLANG_DIR/build/$SLANG_CONFIG"
  local slang_lib gfx_lib
  if [ "$HOST_OS" = windows ]; then
    slang_lib="$base/bin/slang.dll"
    gfx_lib="$base/bin/gfx.dll"
  else
    slang_lib="$base/lib/libslang.so"
    gfx_lib="$base/lib/libgfx.so"
  fi

  if [ ! -f "$slang_lib" ]; then
    log_error "Local Slang $SLANG_CONFIG build not found: $slang_lib"
    log_error "Build Slang first, for example:"
    log_error "  cmake --preset default"
    log_error "  cmake --build --preset $(echo "$SLANG_CONFIG" | tr '[:upper:]' '[:lower:]')"
    exit 1
  fi

  if [ ! -f "$gfx_lib" ]; then
    log_warning "slang-gfx library not found: $gfx_lib"
    log_warning "Falcor imports slang-gfx; build Slang with SLANG_ENABLE_GFX=ON (the default)."
  fi

  log_success "Found local Slang $SLANG_CONFIG build at $base"
}

ensure_falcor_present() {
  if [ ! -d "$FALCOR_DIR" ]; then
    log_error "Falcor not found at $FALCOR_DIR. Run: $0 clone"
    exit 1
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
    git clone https://github.com/NVIDIAGameWorks/Falcor.git "$FALCOR_DIR"
    if [ -n "$FALCOR_REF" ]; then
      log_info "Checking out Falcor ref: $FALCOR_REF"
      (cd "$FALCOR_DIR" && git checkout "$FALCOR_REF")
    fi
  fi

  log_info "Fetching Falcor dependencies (this runs Falcor's setup script)..."
  (cd "$FALCOR_DIR" && run_falcor_setup)
  log_success "Falcor ready at $FALCOR_DIR ($(cd "$FALCOR_DIR" && git rev-parse --short HEAD))"
}

cmd_build() {
  require_preset
  ensure_falcor_present
  verify_slang_build

  log_info "Configuring Falcor (preset $FALCOR_PRESET) with local Slang ($SLANG_CONFIG)..."
  # FALCOR_LOCAL_SLANG_BUILD_DIR is resolved relative to FALCOR_LOCAL_SLANG_DIR
  # by Falcor (SLANG_DIR = DIR/BUILD_DIR), so it must name the per-config build
  # subdirectory (e.g. build/Release), not an absolute path.
  (
    cd "$FALCOR_DIR" &&
      cmake --preset "$FALCOR_PRESET" \
        -DFALCOR_LOCAL_SLANG=ON \
        -DFALCOR_LOCAL_SLANG_DIR="$SLANG_DIR" \
        -DFALCOR_LOCAL_SLANG_BUILD_DIR="build/$SLANG_CONFIG"
  )

  log_info "Building Falcor ($FALCOR_CONFIG)..."
  cmake --build "$(falcor_build_dir)" --config "$FALCOR_CONFIG"
  log_success "Falcor built; local Slang deployed via deploy_dependencies."
}

cmd_install() {
  require_preset
  ensure_falcor_present
  verify_slang_build

  log_info "Re-deploying local Slang into Falcor (deploy_dependencies)..."
  if cmake --build "$(falcor_build_dir)" --config "$FALCOR_CONFIG" --target deploy_dependencies; then
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
  # by extension also skips any subdirectories under lib/.
  if [ "$HOST_OS" = windows ]; then
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
  local ext="sh"
  [ "$HOST_OS" = windows ] && ext="bat"
  local unit_runner="$FALCOR_DIR/tests/run_unit_tests.$ext"
  local image_runner="$FALCOR_DIR/tests/run_image_tests.$ext"

  if [ ! -f "$unit_runner" ]; then
    log_error "Falcor test runner not found: $unit_runner"
    log_error "Build Falcor first: $0 build"
    exit 1
  fi

  log_info "Running Falcor unit tests (--config $config_string)..."
  "$unit_runner" --config "$config_string"

  if [ "$IMAGE_TESTS" = true ]; then
    if [ ! -f "$image_runner" ]; then
      log_error "Falcor image-test runner not found: $image_runner"
      exit 1
    fi
    log_info "Running Falcor image tests (--config $config_string)..."
    "$image_runner" --config "$config_string" --run-only
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
