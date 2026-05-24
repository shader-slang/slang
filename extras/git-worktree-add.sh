#!/usr/bin/env bash

set -euo pipefail

is_wsl() {
  grep -qi microsoft /proc/version 2>/dev/null
}

default_tool() {
  local baseName="$1"
  local exeName="$baseName.exe"

  case "$(uname -s)" in
  MINGW* | MSYS* | CYGWIN*)
    printf '%s\n' "$exeName"
    return
    ;;
  esac

  if is_wsl; then
    printf '%s\n' "$exeName"
  else
    printf '%s\n' "$baseName"
  fi
}

GIT_EXE="${GIT_EXE:-$(default_tool git)}"
GH_EXE="${GH_EXE:-$(default_tool gh)}"

show_usage() {
  me=$(basename "$0")
  cat <<EOF
Usage: $me [options] <branch-name>

Create a sibling git worktree for a new branch.

Options:
  --base <ref>       Base branch or commit for the new worktree.
                     Defaults to the current branch, which must be master,
                     main, or release/*.
  --dir <path>       Destination directory. Defaults to ../<branch-name>.
                     If the branch name contains slashes, they are replaced
                     by dashes.
  --issue <issue>    Create and checkout the branch through GitHub CLI so it
                     is linked to the issue.
  --no-submodules    Skip submodule initialization.
  -h, --help         Show this help.

Examples:
  $me git-worktree-add
  $me descriptor-heap-access --issue 1234
  $me --base release/2026.1 --dir ../descriptor-fix descriptor-heap-access

Prefer branch names without slashes for predictable worktree directory names.
Use --dir when creating a worktree for an existing branch name that contains slashes.
EOF
}

log() {
  echo "[$(date '+%H:%M:%S')] $*"
}

die() {
  echo "Error: $*" >&2
  exit 1
}

require_command() {
  local cmd="$1"
  if ! command -v "$cmd" >/dev/null 2>&1; then
    die "'$cmd' not found in PATH."
  fi
}

log_tool() {
  local label="$1"
  local cmd="$2"
  local cmdPath

  cmdPath="$(command -v "$cmd")"
  log "$label: $cmd ($cmdPath)"
}

to_shell_path() {
  local path="$1"
  case "$path" in
  [A-Za-z]:* | \\\\*)
    if command -v wslpath >/dev/null 2>&1; then
      wslpath -u "$path"
    else
      printf '%s\n' "$path"
    fi
    ;;
  *)
    printf '%s\n' "$path"
    ;;
  esac
}

to_git_path() {
  local path="$1"
  case "$path" in
  /*)
    if is_wsl && git_uses_windows_paths && command -v wslpath >/dev/null 2>&1; then
      wslpath -w "$path"
    else
      printf '%s\n' "$path"
    fi
    ;;
  *)
    printf '%s\n' "$path"
    ;;
  esac
}

git_uses_windows_paths() {
  case "${GIT_EXE##*/}" in
  *.exe | *.EXE) return 0 ;;
  *) return 1 ;;
  esac
}

make_abs_shell_path() {
  local path="$1"
  case "$path" in
  [A-Za-z]:* | \\\\*)
    to_shell_path "$path"
    ;;
  /*)
    printf '%s\n' "$path"
    ;;
  *)
    printf '%s/%s\n' "$startDirShell" "$path"
    ;;
  esac
}

git_run() {
  "$GIT_EXE" "$@"
}

resolve_issue_base() {
  local ref="$1"

  case "$ref" in
  refs/heads/*)
    ref="${ref#refs/heads/}"
    ;;
  refs/remotes/*/*)
    ref="${ref#refs/remotes/}"
    ref="${ref#*/}"
    ;;
  esac

  if git_run show-ref --verify --quiet "refs/heads/$ref" ||
    git_run show-ref --verify --quiet "refs/remotes/origin/$ref"; then
    printf '%s\n' "$ref"
    return 0
  fi

  if git_run show-ref --verify --quiet "refs/remotes/$ref"; then
    printf '%s\n' "${ref#*/}"
    return 0
  fi

  return 1
}

branchInput=""
baseRef=""
dstDirInput=""
githubIssue=""
initSubmodules=1

while [[ $# -gt 0 ]]; do
  case "$1" in
  -h | --help)
    show_usage
    exit 0
    ;;
  --base | -base)
    if [[ $# -lt 2 || -z "${2:-}" ]]; then
      die "--base requires a value."
    fi
    baseRef="$2"
    shift
    ;;
  --dir | -d)
    if [[ $# -lt 2 || -z "${2:-}" ]]; then
      die "--dir requires a value."
    fi
    dstDirInput="$2"
    shift
    ;;
  --issue | -issue)
    if [[ $# -lt 2 || -z "${2:-}" ]]; then
      die "--issue requires a value."
    fi
    githubIssue="$2"
    shift
    ;;
  --no-submodules)
    initSubmodules=0
    ;;
  --)
    shift
    break
    ;;
  -*)
    die "Unknown option: $1"
    ;;
  *)
    if [[ -n "$branchInput" ]]; then
      die "Cannot use more than one branch name: $branchInput $1"
    fi
    branchInput="$1"
    ;;
  esac
  shift
done

while [[ $# -gt 0 ]]; do
  if [[ -n "$branchInput" ]]; then
    die "Cannot use more than one branch name: $branchInput $1"
  fi
  branchInput="$1"
  shift
done

if [[ -z "$branchInput" ]]; then
  show_usage >&2
  exit 1
fi

require_command "$GIT_EXE"
log_tool "Git" "$GIT_EXE"
if [[ -n "$githubIssue" ]]; then
  require_command "$GH_EXE"
  log_tool "GitHub CLI" "$GH_EXE"
fi

startDirShell="$(pwd -P)"

if ! repoRootGit="$(git_run rev-parse --show-toplevel 2>/dev/null)"; then
  die "This script must be run from inside a git worktree."
fi
repoRootShell="$(to_shell_path "$repoRootGit")"
cd "$repoRootShell"

branchName="$branchInput"

if ! git_run check-ref-format --branch "$branchName" >/dev/null 2>&1; then
  die "Invalid branch name: $branchName"
fi

if git_run show-ref --verify --quiet "refs/heads/$branchName"; then
  die "Branch already exists: $branchName"
fi

if [[ -z "$baseRef" ]]; then
  baseRef="$(git_run branch --show-current)"
  if [[ -z "$baseRef" ]]; then
    die "Cannot infer a base branch from detached HEAD. Use --base <ref>."
  fi
  case "$baseRef" in
  master | main | release | release/*) ;;
  *)
    die "Current branch must be master, main, or release/*. Current branch: $baseRef"
    ;;
  esac
fi

if ! git_run rev-parse --verify --quiet "$baseRef^{commit}" >/dev/null; then
  die "Base ref does not resolve to a commit: $baseRef"
fi

issueBase=""
if [[ -n "$githubIssue" ]] && ! issueBase="$(resolve_issue_base "$baseRef")"; then
  die "--issue requires --base to be a branch name, because gh issue develop does not accept arbitrary commits: $baseRef"
fi

worktreeName="$branchName"
worktreeName="${worktreeName//\//-}"

maxWorktreeNameLength=50
if [[ -n "$dstDirInput" ]]; then
  dstDirShell="$(make_abs_shell_path "$dstDirInput")"
else
  if [[ ${#worktreeName} -gt $maxWorktreeNameLength ]]; then
    die "Worktree directory name is too long (${#worktreeName} characters). Maximum is $maxWorktreeNameLength: $worktreeName"
  fi
  dstDirShell="$(dirname "$repoRootShell")/$worktreeName"
fi
dstDirGit="$(to_git_path "$dstDirShell")"

if [[ -e "$dstDirShell" ]]; then
  die "Destination already exists: $dstDirShell"
fi

dstParent="$(dirname "$dstDirShell")"
if [[ ! -d "$dstParent" ]]; then
  die "Destination parent directory does not exist: $dstParent"
fi

log "Repository: $repoRootShell"
log "Base ref: $baseRef"
log "Branch: $branchName"
log "Worktree: $dstDirShell"

log "Pruning stale worktree records..."
git_run worktree prune

if [[ -n "$githubIssue" ]]; then
  log "Adding detached worktree..."
  git_run worktree add -q --detach "$dstDirGit" "$baseRef"

  cd "$dstDirShell"
  log "Creating GitHub issue branch for issue $githubIssue..."
  "$GH_EXE" issue develop "$githubIssue" --name "$branchName" --base "$issueBase" --checkout
else
  log "Adding worktree and creating branch..."
  git_run worktree add -q -b "$branchName" "$dstDirGit" "$baseRef"
  cd "$dstDirShell"
fi

if [[ $initSubmodules -eq 0 ]]; then
  log "Skipping submodule initialization."
elif [[ ! -f .gitmodules ]]; then
  log "Skipping submodule initialization because .gitmodules is not found."
else
  log "Initializing submodules..."
  submodules=()
  while IFS= read -r line; do
    submodulePath="${line#* }"
    if [[ -n "$submodulePath" ]]; then
      submodules+=("$submodulePath")
    fi
  done < <(git_run config --file .gitmodules --get-regexp '^submodule\..*\.path$' 2>/dev/null || true)

  for submodulePath in "${submodules[@]}"; do
    moduleReferenceGit="$repoRootGit/$submodulePath"
    moduleReferenceShell="$(to_shell_path "$moduleReferenceGit")"
    log "Initializing: $submodulePath"
    if [[ -d "$moduleReferenceShell" ]]; then
      git_run submodule -q update --init --reference "$moduleReferenceGit" -- "$submodulePath"
    else
      git_run submodule -q update --init -- "$submodulePath"
    fi
  done

  log "Updating submodules recursively..."
  if ! git_run submodule -q update --init --recursive; then
    echo "Submodule update failed. You may want to manually run:" >&2
    echo "  git.exe submodule update --init --recursive" >&2
    exit 2
  fi
fi

log "Done."
