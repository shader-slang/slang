#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
cd "$(git -C "$SCRIPT_DIR" rev-parse --show-toplevel)" || exit 1

check_only=0

while [[ "$#" -gt 0 ]]; do
  case $1 in
  -h | --help) help=1 ;;
  --check-only) check_only=1 ;;
  esac
  shift
done

if [ "$help" ]; then
  me=$(basename "$0")
  cat <<EOF
$me: Format or check formatting of files in this repo

Usage: $me [--check-only]

Options:
    --check-only     Check formatting without modifying files
EOF
  exit 0
fi

require_bin() {
  local name="$1"
  local required="$2"
  local version

  if ! command -v "$name" &>/dev/null; then
    echo "This script needs $name, but it isn't in \$PATH"
    missing_bin=1
    return
  fi

  version=$("$name" --version | grep -oP "$name(?:\s+version)?\s+\K\d+\.\d+\.?\d*")
  if ! printf '%s\n%s\n' "$required" "$version" | sort -V -C; then
    echo "$name version $version is too old. Version $required or newer is required."
    missing_bin=1
  fi
}

require_bin "git" "1.8"
require_bin "gersemi" "0.16.2"

if [ "${missing_bin:-}" = "1" ]; then
  exit 1
fi

if [ "$missing_bin" ]; then
  exit 1
fi

exit_code=0

cmake_formatting() {
  readarray -t files < <(git ls-files '*.cmake' 'CMakeLists.txt' '**/CMakeLists.txt')

  common_args=(
    # turn on warning when this is fixed https://github.com/BlankSpruce/gersemi/issues/39
    --no-warn-about-unknown-commands
    --definitions "${files[@]}"
  )

  if [ "$check_only" -eq 1 ]; then
    gersemi "${common_args[@]}" --diff --color "${files[@]}"
    gersemi "${common_args[@]}" --check "${files[@]}" || exit_code=1
  else
    gersemi "${common_args[@]}" --in-place "${files[@]}"
  fi
}

cmake_formatting

exit $exit_code
