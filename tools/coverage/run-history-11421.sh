#!/usr/bin/env bash
# Add PR #11421 (conformance/ + design/ restructure) as a coverage data point.
# Waits for run-history.sh to finish first (shared test dir).
set -euo pipefail

REPO_ROOT="$(git rev-parse --show-toplevel)"
cd "$REPO_ROOT"

BRANCH_HEAD="$(git rev-parse HEAD)"
RUNNER="tools/coverage/run-generated-coverage.sh"

echo "=== Waiting for run-history.sh to finish (if still running) ==="
while pgrep -f run-history.sh > /dev/null 2>&1; do
  sleep 30
done

echo "=== [suite-post-11421] checking out test suite at 0486484fb ==="
git checkout 0486484fb -- docs/generated/tests/

echo "=== [suite-post-11421] starting coverage run ==="
bash "$RUNNER" --label suite-post-11421

echo "=== [suite-post-11421] done — restoring branch HEAD ==="
git checkout "$BRANCH_HEAD" -- docs/generated/tests/

echo "=== ALL DONE ==="
grep "^TOTAL" docs/generated/tests/_meta/coverage-snapshot/suite-post-11421-slangc-report.txt
