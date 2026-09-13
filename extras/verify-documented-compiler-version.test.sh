#!/usr/bin/env bash

# Regression test for verify-documented-compiler-version.sh (see #13041).
# Invariant: the script is best-effort — under `set -euo pipefail` its detection
# probes must warn and exit 0, never abort, even when a probe exits non-zero.
# GPU-free (stub compilers). Run: bash extras/verify-documented-compiler-version.test.sh

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
SCRIPT="$SCRIPT_DIR/verify-documented-compiler-version.sh"

failures=0

# Runs the script in a throwaway sandbox (fake checkout: build/CMakeCache.txt -> $stub,
# docs/building.md = $doc_line) and asserts exit code + an output substring. Setup runs
# under `set -e` so a broken fixture fails loudly instead of faking a pass.
# Args: name stub-basename stub-body doc-line expected-rc expected-substring
run_case() {
  local name="$1" stub_name="$2" stub_body="$3" doc_line="$4" want_rc="$5" want_out="$6"

  local sandbox
  sandbox=$(mktemp -d)
  mkdir -p "$sandbox/build" "$sandbox/docs" "$sandbox/bin"

  printf '%s\n' "$stub_body" >"$sandbox/bin/$stub_name"
  chmod +x "$sandbox/bin/$stub_name"

  echo "CMAKE_CXX_COMPILER:FILEPATH=$sandbox/bin/$stub_name" >"$sandbox/build/CMakeCache.txt"
  printf '%s\n' "$doc_line" >"$sandbox/docs/building.md"

  local out rc=0
  out=$(cd "$sandbox" && bash "$SCRIPT" 2>&1) || rc=$?

  rm -rf "$sandbox"

  if [[ "$rc" -ne "$want_rc" ]]; then
    echo "FAIL [$name]: expected exit $want_rc, got $rc"
    echo "  output: $out"
    failures=$((failures + 1))
    return
  fi
  if [[ "$out" != *"$want_out"* ]]; then
    echo "FAIL [$name]: output did not contain '$want_out'"
    echo "  output: $out"
    failures=$((failures + 1))
    return
  fi
  echo "PASS [$name]"
}

# The #13041 field case: cl.exe prints a parseable banner yet exits non-zero (4)
# because it was given no source file, so `|| true` must keep the version (19).
run_case "msvc-nonzero-exit-still-matches" \
  "cl.exe" \
  $'#!/usr/bin/env bash\necho "Microsoft (R) C/C++ Optimizing Compiler Version 19.51.36231 for ARM64" 1>&2\nexit 4' \
  "_MSVC_ 19 is tested in CI and is the recommended minimum version." \
  0 "Compiler version matches"

run_case "gcc-unparseable-version-warns" \
  "gcc" \
  $'#!/usr/bin/env bash\necho "gcc (some vendor build with no version number)"' \
  "_GCC_ 11.4 and 13.3 are tested in CI and is the recommended minimum version." \
  0 "::warning::Could not determine version"

run_case "doc-marker-missing-warns" \
  "gcc" \
  $'#!/usr/bin/env bash\necho "gcc (Ubuntu) 11.4.0"' \
  "_GCC_ 11.4 is used for continuous integration." \
  0 "::warning::Could not find expected version"

run_case "gcc-version-mismatch-warns" \
  "gcc" \
  $'#!/usr/bin/env bash\necho "gcc (Ubuntu) 99.9.0"' \
  "_GCC_ 11.4 and 13.3 are tested in CI and is the recommended minimum version." \
  0 "::warning::Compiler version mismatch"

if [[ "$failures" -ne 0 ]]; then
  echo "$failures test case(s) failed"
  exit 1
fi
echo "All test cases passed"
