#!/usr/bin/env bash

set -euo pipefail

# CI wrapper for the slang-ir-version-check tool. It materializes the base
# (target-branch) revisions of the IR stable-names table and slang-ir.h next to
# the working-tree (PR) versions, then runs the tool to enforce the module
# version-bump policy (see docs/design/ir-instruction-definition.md and
# tools/slang-ir-version-check).
#
# Runs only for pull_request events (it needs a base ref to diff against); on any
# other event it is a no-op success. The advisory comment produced by
# check-inst-version-changes.sh / check-ir-version.yml is unaffected and remains
# the fallback for changes this tool cannot decide from instruction names alone.
#
# Environment:
#   GITHUB_EVENT_NAME  - the triggering event; only "pull_request" does work.
#   GITHUB_BASE_REF    - the PR's target branch (e.g. "master").
#   IR_VERSION_CHECK_BIN (optional) - path to the built tool; if unset the script
#                        locates it under build/generators/*/bin.

if [[ "${GITHUB_EVENT_NAME:-}" != "pull_request" ]]; then
  echo "Not a pull_request event; skipping IR version-bump check."
  exit 0
fi

base_ref="${GITHUB_BASE_REF:?GITHUB_BASE_REF must be set for a pull_request}"

# The base ref may not be present in a shallow checkout; fetch it so the
# git-show below can resolve it. Failing to fetch is fatal (fail closed).
git fetch --no-tags --depth=1 origin "$base_ref"

stable_names="source/slang/slang-ir-insts-stable-names.lua"
ir_header="source/slang/slang-ir.h"

# Locate the built tool. It is produced by the all-generators target and placed
# under the generator output directory.
bin="${IR_VERSION_CHECK_BIN:-}"
if [[ -z "$bin" ]]; then
  bin=$(find build/generators -name 'slang-ir-version-check' -type f 2>/dev/null | head -n 1 || true)
fi
if [[ -z "$bin" || ! -x "$bin" ]]; then
  echo "::error::slang-ir-version-check binary not found; build the all-generators target first."
  exit 1
fi

workdir=$(mktemp -d)
trap 'rm -rf "$workdir"' EXIT

# Materialize the base revision of each file. A file absent at the base (first
# introduction) leaves no base copy; the tool treats a missing base stable-names
# file as an empty instruction set.
base_stable="$workdir/base-stable-names.lua"
base_header="$workdir/base-slang-ir.h"
git show "origin/$base_ref:$stable_names" >"$base_stable" 2>/dev/null || rm -f "$base_stable"
if ! git show "origin/$base_ref:$ir_header" >"$base_header" 2>/dev/null; then
  echo "::error::Could not read $ir_header at origin/$base_ref."
  exit 1
fi

"$bin" \
  --base-stable-names "$base_stable" \
  --new-stable-names "$stable_names" \
  --base-ir-h "$base_header" \
  --new-ir-h "$ir_header"
