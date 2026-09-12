#!/usr/bin/env bash
# Fail if any first-party CMake file references ${CMAKE_BINARY_DIR}.
#
# Slang's own build outputs must use the project-scoped slang_BINARY_DIR, not
# CMAKE_BINARY_DIR: under add_subdirectory the latter is the superproject's build
# tree, so an output path built from it escapes Slang's own build directory.
#
# The search is over tracked files (git grep), which covers first-party CMake
# including external/CMakeLists.txt but does not descend into submodule working
# trees, whose vendored CMake legitimately uses CMAKE_BINARY_DIR.

set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

# git grep exit status: 0 = matches found, 1 = no matches, >1 = real error.
# Capture it so a grep failure fails the guard loudly instead of reading as clean.
# -w matches CMAKE_BINARY_DIR as a whole word, not as part of a larger identifier.
status=0
matches=$(git grep -wn 'CMAKE_BINARY_DIR' -- '*.cmake' '*.cmake.in' '*CMakeLists.txt') || status=$?

if [ "$status" -gt 1 ]; then
  echo "ERROR: 'git grep' failed with status $status." >&2
  exit "$status"
fi

if [ -n "$matches" ]; then
  echo "ERROR: first-party CMake files must not reference \${CMAKE_BINARY_DIR}." >&2
  echo "" >&2
  echo "CMAKE_BINARY_DIR is the top-level build dir; under add_subdirectory that is" >&2
  echo "the superproject's tree, so Slang's outputs escape its own build directory." >&2
  echo "Use the project-scoped slang_BINARY_DIR (== PROJECT_BINARY_DIR in Slang) instead." >&2
  echo "" >&2
  echo "Offending lines:" >&2
  echo "$matches" >&2
  exit 1
fi

echo "OK: no first-party CMake file references \${CMAKE_BINARY_DIR}."
