#!/usr/bin/env bash

set -e

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
source_dir="$(dirname "$script_dir")"

check_only=0
no_version_check=0

while [[ "$#" -gt 0 ]]; do
  case $1 in
  -h | --help) help=1 ;;
  --check-only) check_only=1 ;;
  --no-version-check) no_version_check=1 ;;
  --source)
    source_dir="$2"
    shift
    ;;
  esac
  shift
done

if [ "$help" ]; then
  me=$(basename "$0")
  cat <<EOF
$me: Format or check formatting of files in this repo

Usage: $me [--check-only] [--no-version-check] [--source <path>]

Options:
    --check-only       Check formatting without modifying files
    --no-version-check Skip version compatibility checks
    --source           Path to source directory to format (defaults to parent of script directory)
EOF
  exit 0
fi

cd "$source_dir" || exit 1

require_bin() {
  local name="$1"
  local required="$2"
  local version

  if ! command -v "$name" &>/dev/null; then
    echo "This script needs $name, but it isn't in \$PATH"
    missing_bin=1
    return
  fi

  if [ "$no_version_check" -eq 0 ]; then
    version=$("$name" --version | grep -oP "$name(?:\s+version)?\s+\K\d+\.\d+\.?\d*")
    if ! printf '%s\n%s\n' "$required" "$version" | sort -V -C; then
      echo "$name version $version is too old. Version $required or newer is required."
      missing_bin=1
    fi
  fi
}

require_bin "git" "1.8"
require_bin "gersemi" "0.17"
require_bin "clang-format" "18"

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

cpp_formatting() {
  readarray -t files < <(git ls-files '*.cpp' '*.hpp' '*.c' '*.h')

  if [ "$check_only" -eq 1 ]; then
    local tmpdir
    tmpdir=$(mktemp -d)
    trap 'rm -rf "$tmpdir"' EXIT

    printf '%s\n' "${files[@]}" | xargs -P "$(nproc)" -I{} bash -c "
      mkdir -p \"\$(dirname \"$tmpdir/{}\")\"
      diff -u --color=always --label \"{}\" --label \"{}\" \"{}\" <(clang-format \"{}\") > \"$tmpdir/{}\" 
      :
    "

    for file in "${files[@]}"; do
      if [ -s "$tmpdir/$file" ]; then
        cat "$tmpdir/$file"
        exit_code=1
      fi
    done
  else
    printf '%s\n' "${files[@]}" | xargs -n1 -P "$(nproc)" clang-format -i
  fi
}

cmake_formatting
cpp_formatting

exit $exit_code
