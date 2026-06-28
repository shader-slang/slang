#!/usr/bin/env bash
set -euo pipefail

git submodule sync --recursive
git submodule update --init --recursive

submodule_status="$(git submodule status --recursive)"
if [ -n "$submodule_status" ]; then
  printf '%s\n' "$submodule_status"
fi

out_of_sync="$(printf '%s\n' "$submodule_status" | grep -E '^[+-U]' || true)"
if [ -n "$out_of_sync" ]; then
  echo "::error::submodules are not synchronized with the superproject"
  printf '%s\n' "$out_of_sync"
  exit 1
fi

status="$(git status --short --untracked-files=all -- .)"
if [ -n "$status" ]; then
  echo "::error::checkout is not clean after submodule synchronization"
  printf '%s\n' "$status"
  exit 1
fi
