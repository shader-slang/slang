#!/usr/bin/env bash
# run-backends.sh — build (if needed) and run the shader-coverage-backends
# example on one or more backends, from this directory.
#
# Usage:
#   ./run-backends.sh                  # run every backend: cpu cuda vulkan metal
#   ./run-backends.sh cpu metal        # run a subset
#
# A backend that this machine or build cannot run (no CUDA toolkit, no
# Vulkan loader, not an Apple platform) fails its own run with a clear
# message from the example; the wrapper keeps going and reports a
# per-backend summary at the end.
set -uo pipefail
cd "$(dirname "$0")"

# Git Bash / MSYS / Cygwin: use the PowerShell runner, same delegation
# as the tutorial's run script.
case "$(uname -s)" in
MINGW* | MSYS* | CYGWIN*)
  exec powershell.exe -NoProfile -ExecutionPolicy Bypass -File run-backends.ps1 "$@"
  ;;
esac

usage() {
  echo "usage: $0 [cpu|cuda|vulkan|metal ...]   (default: all four)"
}

backends=()
for arg in "$@"; do
  case "$arg" in
  cpu | cuda | vulkan | metal) backends+=("$arg") ;;
  -h | --help)
    usage
    exit 0
    ;;
  *)
    echo "error: unknown backend '$arg'" >&2
    usage >&2
    exit 2
    ;;
  esac
done
[[ ${#backends[@]} -eq 0 ]] && backends=(cpu cuda vulkan metal)

# Find an existing binary in any CMake configuration, or build the
# release one. Rebuild manually after source changes:
#   cmake --build --preset release --target shader-coverage-backends
root=../..
bin=""
for config in Release RelWithDebInfo Debug; do
  candidate="$root/build/examples/shader-coverage-backends/$config/shader-coverage-backends"
  [[ -x $candidate ]] && bin=$candidate && break
done
if [[ -z $bin ]]; then
  echo "binary not found; building shader-coverage-backends (release)..."
  (cd "$root" && cmake --build --preset release --target shader-coverage-backends) || {
    echo "error: build failed; configure first with: cmake --preset default" >&2
    exit 1
  }
  bin=$root/build/examples/shader-coverage-backends/Release/shader-coverage-backends
fi
echo "using binary: $bin"

# Run each backend, collecting exit codes rather than stopping at the
# first failure. Note for macOS + MoltenVK: the vulkan run can crash in
# MoltenVK's own teardown after printing a complete, correct hit table
# — judge that backend by its output, not only the summary line.
summary=()
for b in "${backends[@]}"; do
  echo
  echo "=== --backend=$b ==="
  if "$bin" --backend="$b"; then
    summary+=("$b: ok")
  else
    summary+=("$b: exit $?")
  fi
done

echo
echo "summary:"
printf '  %s\n' "${summary[@]}"
