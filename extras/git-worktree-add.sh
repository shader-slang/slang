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
TMUX_EXE="${TMUX_EXE:-tmux}"

show_usage() {
  me=$(basename "$0")
  cat <<EOF
Usage:
  $me [options] <branch-name>
  $me --review <pull-request-url> [options] [branch-name]

Create a sibling git worktree for a new branch or GitHub pull request review.

Options:
  --base <ref>       Base branch or commit for the new worktree.
                     Defaults to the current branch, which must be master,
                     main, or release/*.
  --review <url>     Fetch a GitHub pull request head and create a review
                     branch/worktree. Defaults to review-pr-<number>.
  --dir <path>       Destination directory. Defaults to ../<branch-name>.
                     If the branch name contains slashes, they are replaced
                     by dashes. In review mode, defaults to
                     ../review-pr-<number>.
  --tmux             Start a tmux session named after the branch or review in
                     the new worktree after setup completes.
  --no-submodules    Skip submodule initialization.
  -h, --help         Show this help.

Examples:
  $me git-worktree-add
  $me --tmux git-worktree-add
  $me --base release/2026.1 --dir ../descriptor-fix descriptor-heap-access
  $me --review https://github.com/shader-slang/slang/pull/11267
  $me --review https://github.com/shader-slang/slang/pull/11267 review-11267

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

git_run_noninteractive() {
  GIT_TERMINAL_PROMPT=0 "$GIT_EXE" "$@" </dev/null
}

is_git_repository() {
  git_run -C "$1" rev-parse --git-dir >/dev/null 2>&1
}

start_tmux_session() {
  local sessionName="$1"
  local sessionDir="$2"

  if [[ -n "${TMUX:-}" ]]; then
    "$TMUX_EXE" new-session -d -s "$sessionName" -c "$sessionDir"
    "$TMUX_EXE" switch-client -t "=$sessionName"
  else
    "$TMUX_EXE" new-session -s "$sessionName" -c "$sessionDir"
  fi
}

parse_review_pr_url() {
  local input="$1"
  local clean="$input"

  clean="${clean%%#*}"
  clean="${clean%%\?*}"
  clean="${clean%/}"

  if [[ "$clean" =~ ^https?://github\.com/([^/]+)/([^/]+)/pull/([0-9]+)(/.*)?$ ]]; then
    reviewOwner="${BASH_REMATCH[1]}"
    reviewRepo="${BASH_REMATCH[2]%.git}"
    reviewNumber="${BASH_REMATCH[3]}"
    reviewRepoUrl="https://github.com/$reviewOwner/$reviewRepo.git"
    return
  fi

  die "Unsupported review URL: $input. Expected https://github.com/<owner>/<repo>/pull/<number>."
}

branchInput=""
baseRef=""
dstDirInput=""
initSubmodules=1
startTmux=0
reviewInput=""
reviewOwner=""
reviewRepo=""
reviewNumber=""
reviewRepoUrl=""

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
  --review)
    if [[ $# -lt 2 || -z "${2:-}" ]]; then
      die "--review requires a pull request URL."
    fi
    reviewInput="$2"
    shift
    ;;
  --dir | -d)
    if [[ $# -lt 2 || -z "${2:-}" ]]; then
      die "--dir requires a value."
    fi
    dstDirInput="$2"
    shift
    ;;
  --tmux)
    startTmux=1
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

if [[ -n "$reviewInput" && -n "$baseRef" ]]; then
  die "Cannot use --base with --review."
fi

if [[ -z "$reviewInput" && -z "$branchInput" ]]; then
  show_usage >&2
  exit 1
fi

require_command "$GIT_EXE"
log_tool "Git" "$GIT_EXE"
if [[ $startTmux -eq 1 ]]; then
  require_command "$TMUX_EXE"
  log_tool "tmux" "$TMUX_EXE"
fi

startDirShell="$(pwd -P)"

if ! repoRootGit="$(git_run rev-parse --show-toplevel 2>/dev/null)"; then
  die "This script must be run from inside a git worktree."
fi
repoRootShell="$(to_shell_path "$repoRootGit")"
cd "$repoRootShell"

branchName=""

if [[ -n "$reviewInput" ]]; then
  parse_review_pr_url "$reviewInput"
  baseRef="refs/pr/$reviewNumber/head"
  branchName="$branchInput"
  if [[ -z "$branchName" ]]; then
    branchName="review-pr-$reviewNumber"
  fi
  worktreeName="$branchName"
  worktreeName="${worktreeName//\//-}"
  sessionName="$worktreeName"

  log "Fetching PR #$reviewNumber from $reviewRepoUrl..."
  git_run fetch -q "$reviewRepoUrl" "+pull/$reviewNumber/head:$baseRef"

  if ! git_run check-ref-format --branch "$branchName" >/dev/null 2>&1; then
    die "Invalid branch name: $branchName"
  fi

  if git_run show-ref --verify --quiet "refs/heads/$branchName"; then
    die "Branch already exists: $branchName"
  fi
else
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

  worktreeName="$branchName"
  worktreeName="${worktreeName//\//-}"
  sessionName="$branchName"
fi

if ! git_run rev-parse --verify --quiet "$baseRef^{commit}" >/dev/null; then
  die "Base ref does not resolve to a commit: $baseRef"
fi

if [[ $startTmux -eq 1 ]] && "$TMUX_EXE" has-session -t "=$sessionName" 2>/dev/null; then
  die "tmux session already exists: $sessionName"
fi

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
if [[ -n "$reviewInput" ]]; then
  log "Review: $reviewInput"
fi
log "Base ref: $baseRef"
log "Branch: $branchName"
log "Worktree: $dstDirShell"

log "Pruning stale worktree records..."
git_run worktree prune

log "Adding worktree and creating branch..."
git_run worktree add -q -b "$branchName" "$dstDirGit" "$baseRef"
cd "$dstDirShell"

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
  submoduleJobCount="${#submodules[@]}"
  if [[ $submoduleJobCount -eq 0 ]]; then
    submoduleJobCount=1
  fi

  if [[ ${#submodules[@]} -gt 0 ]]; then
    git_run submodule -q init -- "${submodules[@]}"
  fi

  log "Updating top-level submodules concurrently..."
  submoduleTempDir="$(mktemp -d "${TMPDIR:-/tmp}/git-worktree-add-submodules.XXXXXX")"
  trap 'rm -rf "$submoduleTempDir"' EXIT
  submoduleErrorFile="$submoduleTempDir/errors"
  submoduleFailureFile="$submoduleTempDir/failed"
  for submodulePath in "${submodules[@]}"; do
    log "Updating: $submodulePath"

    (
      moduleReferenceGit="$repoRootGit/$submodulePath"
      submoduleUpdateArgs=(submodule -q update)
      if is_git_repository "$moduleReferenceGit"; then
        submoduleUpdateArgs+=(--reference "$moduleReferenceGit")
      fi
      submoduleUpdateArgs+=(-- "$submodulePath")

      if ! git_run_noninteractive "${submoduleUpdateArgs[@]}" >/dev/null 2>>"$submoduleErrorFile"; then
        : >"$submoduleFailureFile"
      fi
    ) &
    sleep 1 # Stagger background submodule updates to reduce Git lock contention.
  done

  wait

  if [[ -e "$submoduleFailureFile" ]]; then
    if [[ -s "$submoduleErrorFile" ]]; then
      sed 's/^/  /' "$submoduleErrorFile" >&2
    fi
    echo "Submodule update failed. You may want to manually run:" >&2
    echo "  \"$GIT_EXE\" submodule update --init --recursive --jobs $submoduleJobCount" >&2
    exit 2
  fi

  log "Updating submodules recursively..."
  if ! git_run_noninteractive submodule -q update --init --recursive --jobs "$submoduleJobCount"; then
    echo "Submodule update failed. You may want to manually run:" >&2
    echo "  \"$GIT_EXE\" submodule update --init --recursive --jobs $submoduleJobCount" >&2
    exit 2
  fi

  log "Updating submodules completed."
fi

if [[ $startTmux -eq 1 ]]; then
  log "Starting tmux session: $sessionName"
  start_tmux_session "$sessionName" "$dstDirShell"
fi
