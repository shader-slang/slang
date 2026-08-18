#!/usr/bin/env bash
# This script generates a release note.
# It prints information about breaking-changes first and the rest.
# The content is mostly based on `git log --oneline --since 202X-YY-ZZ`.

# Usage: the script takes command-line arguments to specify the range of commits to include.
# You can use either:
# 1. Default (no arguments): auto-detects the previous release tag via git describe
# 2. Date-based range with --since: docs/scripts/release-note.sh --since 2025-08-06
# 3. Hash-based range with --previous-hash: docs/scripts/release-note.sh --previous-hash abc123
# 4. Help: docs/scripts/release-note.sh --help

# This script is supposed to work on all Windows based shell systems including WSL and git-bash.
# If you make any modifications, please test them, because CI doesn't test this script.

verbose=true
$verbose && echo "Reminder: PLEASE make sure your local repo is up-to-date before running the script." >&2

gh=""
for candidate in \
  "$(which gh)" \
  "$(which gh.exe)" \
  "/mnt/c/Program Files/GitHub CLI/gh.exe" \
  "/c/Program Files/GitHub CLI/gh.exe" \
  "/cygdrive/c/Program Files/GitHub CLI/gh.exe"; do
  if [ -x "$candidate" ]; then
    gh="$candidate"
    break
  fi
done
if [ "$gh" = "" ] || ! [ -x "$gh" ]; then
  echo "File not found: gh or gh.exe"
  echo "GitHub CLI can be downloaded from https://cli.github.com"
  exit 1
fi
$verbose && echo "gh is found from: $gh" >&2

# Parse command-line arguments
use_hash=false
since=""
previous_hash=""

while [[ $# -gt 0 ]]; do
  case $1 in
  --since)
    since="$2"
    use_hash=false
    shift
    ;;
  --previous-hash)
    previous_hash="$2"
    use_hash=true
    shift
    ;;
  --help | -h)
    echo "Usage: $0 [--since DATE | --previous-hash HASH | --help]"
    echo "  (no arguments)        Auto-detect the previous release tag via 'git describe'"
    echo "  --since DATE          Generate notes since the given date (e.g., 2025-08-06)"
    echo "  --previous-hash HASH  Generate notes since the given commit hash"
    echo "  --help, -h            Show this help message"
    exit 0
    ;;
  *)
    echo "Not recognized command-line argument: $1"
    ;;
  esac
  shift
done

if [ "$since" = "" ] && [ "$previous_hash" = "" ]; then
  $verbose && echo "Using auto-detect mode; use --help or -h for more information" >&2
  previous_tag="$(git describe --tags --match 'v20[0-9][0-9].[0-9]*' --abbrev=0 2>/dev/null)"
  if [ "$previous_tag" = "" ]; then
    echo "Error: Could not auto-detect a previous release tag matching 'v20[YY].[N]*'." >&2
    echo "Please specify --since DATE or --previous-hash HASH, or run with --help for usage." >&2
    exit 1
  fi
  $verbose && echo "Auto-detected previous release tag: $previous_tag" >&2
  $verbose && echo "Generating release notes since $previous_tag..." >&2
  previous_hash="$previous_tag"
  use_hash=true
fi

# Get commits based on the specified range
if [ "$use_hash" = true ]; then
  commits="$(git log --oneline "$previous_hash"..HEAD)"
else
  commits="$(git log --oneline --since "$since")"
fi
commitsCount="$(echo "$commits" | wc -l)"

echo "=== Breaking changes ==="
breakingChanges=""
for i in $(seq "$commitsCount"); do
  line="$(echo "$commits" | head -n "$i" | tail -1)"

  # Get PR number from the git commit title
  pr="$(echo "$line" | grep '#[1-9][0-9][0-9][0-9][0-9]*' | sed 's|.*(\#\([1-9][0-9][0-9][0-9][0-9]*\))$|\1|')"
  [ "$pr" = "" ] && continue

  # Check if the PR is marked as a breaking change
  if "$gh" issue view "$pr" --json labels | grep -q 'pr: breaking change'; then
    breakingChanges+="$line"$'\n'
  fi
done
if [ "$breakingChanges" = "" ]; then
  echo "No breaking changes"
else
  echo "$breakingChanges"
fi

echo "=== All changes for this release ==="
for i in $(seq "$commitsCount"); do
  line="$(echo "$commits" | head -n "$i" | tail -1)"

  result="$line"
  # Get PR number from the git commit title
  pr="$(echo "$line" | grep '#[1-9][0-9][0-9][0-9][0-9]*' | sed 's|.*(\#\([1-9][0-9][0-9][0-9][0-9]*\))$|\1|')"
  if [ "$pr" != "" ]; then
    # Mark breaking changes with "[BREAKING]"
    if "$gh" issue view "$pr" --json labels | grep -q 'pr: breaking change'; then
      result="[BREAKING] $line"
    fi
  fi
  echo "$result"
done
