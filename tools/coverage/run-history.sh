#!/usr/bin/env bash
# Run coverage measurements for bootstrap + master test suite states
# against the current coverage build. Does NOT rebuild Slang.
# Restores branch HEAD after each run.
# Results land in docs/generated/tests/_meta/coverage-snapshot/suite-{bootstrap,master}-*
set -euo pipefail

REPO_ROOT="$(git rev-parse --show-toplevel)"
cd "$REPO_ROOT"

BRANCH_HEAD="$(git rev-parse HEAD)"
RUNNER="tools/coverage/run-generated-coverage.sh"
EXPECTED_FAILURES="docs/generated/tests/_meta/expected-failures.txt"

run_for_commit() {
  local commit="$1"
  local label="$2"
  echo "=== [$label] checking out test suite at $commit ==="
  git checkout "$commit" -- docs/generated/tests/

  # Bootstrap predates expected-failures.txt — create empty placeholder
  if [[ ! -f "$EXPECTED_FAILURES" ]]; then
    echo "(no expected-failures.txt at $commit — creating empty placeholder)"
    touch "$EXPECTED_FAILURES"
  fi

  echo "=== [$label] starting coverage run ==="
  bash "$RUNNER" --label "$label"
  echo "=== [$label] done ==="

  echo "=== [$label] restoring branch HEAD ==="
  git checkout "$BRANCH_HEAD" -- docs/generated/tests/
}

run_for_commit "5377f3e02" "suite-bootstrap"
run_for_commit "eb0dba821" "suite-master"

echo "=== ALL DONE ==="
echo "Results in docs/generated/tests/_meta/coverage-snapshot/suite-{bootstrap,master}-*"
echo "Branch (suite-branch) = step3 already measured (52.27% / 131922 lines)"
