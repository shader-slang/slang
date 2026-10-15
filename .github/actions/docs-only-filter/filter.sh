#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "::error::docs-only-filter requires exactly one changed-files path argument"
  exit 1
fi

filesPath=$1

if [ ! -f "$filesPath" ]; then
  echo "::error::Changed-files list does not exist: $filesPath"
  exit 1
fi

shouldRun=true

if [ ! -s "$filesPath" ]; then
  echo "No changed files listed, running checks"
elif grep -Eqx 'docs/(command-line-slangc-reference.md|user-guide/a4-02-reference-capability-atoms.md)' "$filesPath"; then
  echo "Generated reference document changed, running checks"
elif grep -qvE '^(docs/|LICENSES/|LICENSE$|\.claude/|.*\.md$|\.coderabbit\.yaml$)' "$filesPath"; then
  shouldRun=true
else
  echo "Only documentation files changed, skipping"
  shouldRun=false
fi

echo "should-run=$shouldRun" >>"$GITHUB_OUTPUT"
