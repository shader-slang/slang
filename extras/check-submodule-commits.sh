#!/usr/bin/env bash
# Verify that every submodule pin in this superproject points at a commit that
# is reachable from the upstream submodule's tracked branch.
#
# Motivated by issue #9335: a PR landed with external/slang-rhi pointing at a
# developer branch instead of main. Reviewers see only the pointer diff, so
# this check enforces the invariant in CI.
#
# Opt-out: setting `submodule.<name>.slang-skip-pin-check = true` in
# .gitmodules disables the branch-reachability check for that submodule. The
# script still verifies that the pinned SHA is fetchable from the URL (so
# typos and rewritten history are still caught) — it just doesn't insist on
# branch membership. Use sparingly: this is intended for vendored/forked
# submodules whose pinned commit deliberately isn't on the upstream's
# branches (e.g. external/imgui, which carries a slang-local patch).
#
# See issue #9336.

set -euo pipefail

usage() {
  cat <<EOF
Usage: $0 [--diff-base <ref>] [--help]

Checks that every submodule's pinned commit (as recorded in HEAD) is reachable
from the upstream branch tracked in .gitmodules (the 'branch =' override if
set, otherwise the remote's default branch).

Options:
  --diff-base <ref>   Only check submodules whose pinned SHA differs between
                      <ref> and HEAD. Useful for limiting CI work to the set
                      of submodules a PR actually touched. When unset, every
                      submodule is checked (the default for ad-hoc local use).
  --help              Show this message.
EOF
}

DIFF_BASE=""
while [[ $# -gt 0 ]]; do
  case "$1" in
  --diff-base)
    if [[ $# -lt 2 ]]; then
      echo "ERROR: --diff-base requires an argument" >&2
      exit 2
    fi
    DIFF_BASE="$2"
    shift 2
    ;;
  --help | -h)
    usage
    exit 0
    ;;
  *)
    echo "ERROR: unknown argument: $1" >&2
    usage >&2
    exit 2
    ;;
  esac
done

if [[ ! -f .gitmodules ]]; then
  echo "INFO: no .gitmodules at $(pwd); nothing to check."
  exit 0
fi

WORK_DIR="$(mktemp -d)"
# Drop the temp dir on any exit. The bare repos inside can be hundreds of MB
# after a full unshallow, so leaving them around would accumulate fast.
trap 'rm -rf "$WORK_DIR"' EXIT

resolve_default_branch() {
  # Parse the symref line emitted by `git ls-remote --symref <url> HEAD`:
  #   ref: refs/heads/main	HEAD
  # We want just "main". Use awk for the field split + sub to keep this
  # portable across BSD/GNU `sed` differences.
  local url="$1"
  git ls-remote --symref "$url" HEAD 2>/dev/null |
    awk '$1 == "ref:" { sub("refs/heads/", "", $2); print $2; exit }'
}

ensure_bare_repo() {
  # Each submodule gets its own bare repo keyed by URL hash so re-fetches
  # against the same URL share the object store across iterations of the
  # depth-escalation loop.
  local url="$1"
  local hash
  hash="$(printf '%s' "$url" | git hash-object --stdin)"
  local repo="$WORK_DIR/$hash"
  if [[ ! -d "$repo" ]]; then
    git init --bare --quiet "$repo"
  fi
  printf '%s\n' "$repo"
}

is_ancestor() {
  local repo="$1"
  local sha="$2"
  local ref="$3"
  git -C "$repo" merge-base --is-ancestor "$sha" "$ref" 2>/dev/null
}

# Fetch the pinned SHA directly (no branch context) to confirm it exists at
# the URL. Used for opt-out submodules where we don't require branch
# membership but still want to catch typos and rewritten history.
verify_sha_exists() {
  local repo="$1"
  local url="$2"
  local sha="$3"

  if git -C "$repo" cat-file -e "$sha" 2>/dev/null; then
    return 0
  fi

  # Not all servers allow fetching by SHA (uploadpack.allowReachableSHA1InWant
  # / allowAnySHA1InWant). GitHub does for public repos, which covers our
  # current submodule set.
  if git -C "$repo" fetch --quiet --filter=blob:none --depth=1 \
    "$url" "$sha" 2>/dev/null; then
    return 0
  fi

  return 1
}

# Try fetching with progressively deeper history until the pinned commit is
# reachable, or until we've done a full unshallow and confirmed it isn't.
# Returns 0 on success, 1 on definitive failure.
verify_reachable() {
  local repo="$1"
  local url="$2"
  local branch="$3"
  local sha="$4"
  local refspec="refs/heads/$branch:refs/remotes/origin/$branch"

  local depth
  for depth in 50 500; do
    if git -C "$repo" fetch --quiet --filter=blob:none --depth="$depth" \
      "$url" "$refspec" 2>/dev/null; then
      if is_ancestor "$repo" "$sha" "refs/remotes/origin/$branch"; then
        return 0
      fi
    fi
  done

  # Final attempt: full history. --unshallow only works on an existing shallow
  # repo, so on the first try we just fetch without --depth.
  if git -C "$repo" fetch --quiet --filter=blob:none --unshallow \
    "$url" "$refspec" 2>/dev/null ||
    git -C "$repo" fetch --quiet --filter=blob:none \
      "$url" "$refspec" 2>/dev/null; then
    if is_ancestor "$repo" "$sha" "refs/remotes/origin/$branch"; then
      return 0
    fi
  fi

  return 1
}

# Build the list of submodule names from .gitmodules. The output of
# get-regexp is "submodule.<name>.path <value>", one per line.
mapfile -t SUBMODULE_NAMES < <(
  git config -f .gitmodules --get-regexp '^submodule\..*\.path$' |
    awk '{print $1}' |
    sed -E 's/^submodule\.(.*)\.path$/\1/'
)

if [[ ${#SUBMODULE_NAMES[@]} -eq 0 ]]; then
  echo "INFO: .gitmodules has no submodule entries; nothing to check."
  exit 0
fi

declare -a FAILURES=()
declare -i CHECKED=0
declare -i SKIPPED=0

for name in "${SUBMODULE_NAMES[@]}"; do
  path="$(git config -f .gitmodules "submodule.${name}.path")"
  url="$(git config -f .gitmodules "submodule.${name}.url")"
  branch_override="$(git config -f .gitmodules --default '' "submodule.${name}.branch")"
  skip_pin_check="$(git config -f .gitmodules --default '' "submodule.${name}.slang-skip-pin-check")"

  if [[ -z "$path" || -z "$url" ]]; then
    echo "WARNING: submodule '$name' is missing path or url in .gitmodules; skipping." >&2
    SKIPPED+=1
    continue
  fi

  # Resolve the pinned SHA from HEAD's tree. ls-tree avoids needing the
  # submodule contents checked out and works even on a fresh clone.
  ls_tree_line="$(git ls-tree HEAD -- "$path" || true)"
  if [[ -z "$ls_tree_line" ]]; then
    echo "WARNING: submodule '$name' (path=$path) has no entry at HEAD; skipping." >&2
    SKIPPED+=1
    continue
  fi
  mode="$(printf '%s\n' "$ls_tree_line" | awk '{print $1}')"
  pinned_sha="$(printf '%s\n' "$ls_tree_line" | awk '{print $3}')"
  if [[ "$mode" != "160000" ]]; then
    echo "WARNING: '$path' is not a gitlink (mode=$mode); skipping." >&2
    SKIPPED+=1
    continue
  fi

  if [[ -n "$DIFF_BASE" ]]; then
    base_line="$(git ls-tree "$DIFF_BASE" -- "$path" 2>/dev/null || true)"
    base_sha="$(printf '%s\n' "$base_line" | awk '{print $3}')"
    if [[ -n "$base_sha" && "$base_sha" == "$pinned_sha" ]]; then
      SKIPPED+=1
      continue
    fi
  fi

  repo="$(ensure_bare_repo "$url")"

  if [[ "$skip_pin_check" == "true" ]]; then
    echo "INFO: '$name' (path=$path) skipping branch check (opted out via submodule.${name}.slang-skip-pin-check); verifying SHA $pinned_sha is fetchable."
    if verify_sha_exists "$repo" "$url" "$pinned_sha"; then
      echo "  PASS: $pinned_sha is fetchable from $url."
    else
      FAILURES+=("$name|$path|$url|<opted out>|$pinned_sha|pinned commit not fetchable from URL (typo or rewritten history?)")
    fi
    CHECKED+=1
    continue
  fi

  if [[ -n "$branch_override" ]]; then
    branch="$branch_override"
    branch_source="branch override in .gitmodules"
  else
    branch="$(resolve_default_branch "$url" || true)"
    branch_source="remote default branch"
    if [[ -z "$branch" ]]; then
      FAILURES+=("$name|$path|$url|<unknown>|$pinned_sha|could not resolve remote default branch")
      CHECKED+=1
      continue
    fi
  fi

  echo "INFO: checking '$name' (path=$path) pinned $pinned_sha against $branch ($branch_source)"

  if verify_reachable "$repo" "$url" "$branch" "$pinned_sha"; then
    echo "  PASS: $pinned_sha is reachable from $branch."
  else
    FAILURES+=("$name|$path|$url|$branch|$pinned_sha|pinned commit not reachable from branch")
  fi
  CHECKED+=1
done

echo
echo "Submodules checked: $CHECKED  skipped: $SKIPPED  failed: ${#FAILURES[@]}"

if [[ ${#FAILURES[@]} -gt 0 ]]; then
  echo
  echo "ERROR: one or more submodule pins are not reachable from their tracked branch."
  echo
  for entry in "${FAILURES[@]}"; do
    IFS='|' read -r name path url branch sha reason <<<"$entry"
    echo "  Submodule:  $name"
    echo "    path:     $path"
    echo "    url:      $url"
    echo "    branch:   $branch"
    echo "    pinned:   $sha"
    echo "    reason:   $reason"
    echo "    fix:      the pinned commit is not reachable from $branch; either land"
    echo "              the commit on $branch or re-point the submodule to a commit"
    echo "              that is on $branch. If you intended to pin a tag, note that"
    echo "              tags are not branches: this check verifies branch reachability,"
    echo "              so the tagged commit must also exist on $branch."
    echo
  done
  exit 1
fi

echo "All submodule pins are reachable from their tracked branches."
exit 0
