#!/usr/bin/env bash
# This script generates a release note.
# It prints information about breaking-changes first and the rest.
# The content is mostly based on `git log --oneline --since 202X-YY-ZZ`.

# Usage: the script takes command-line arguments to specify the range of commits to include.
# You can use either:
# 1. Date-based range with --since: docs/scripts/release-note.sh --since 2025-08-06
# 2. Hash-based range with --previous-hash: docs/scripts/release-note.sh --previous-hash abc123
# 3. Legacy positional argument (deprecated): docs/scripts/release-note.sh 2024-07-01

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
    shift 2
    ;;
  --previous-hash)
    previous_hash="$2"
    use_hash=true
    shift 2
    ;;
  *)
    # Legacy positional argument support
    if [ "$since" = "" ] && [ "$previous_hash" = "" ]; then
      since="$1"
      use_hash=false
    else
      echo "Too many arguments or mixed argument styles"
      exit 1
    fi
    shift
    ;;
  esac
done

# Validate arguments
if [ "$since" = "" ] && [ "$previous_hash" = "" ]; then
  echo "This script requires either --since or --previous-hash option."
  echo "Usage: $0 [--since DATE | --previous-hash HASH]"
  echo "  --since DATE          Generate notes since the given date (e.g., 2025-08-06)"
  echo "  --previous-hash HASH  Generate notes since the given commit hash"
  echo ""
  echo "Legacy usage (deprecated): $0 DATE"
  exit 1
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
  pr="$(echo "$line" | grep '#[1-9][0-9][0-9][0-9][0-9]*' | sed 's|.* (\#\([1-9][0-9][0-9][0-9][0-9]*\))|\1|')"
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
  pr="$(echo "$line" | grep '#[1-9][0-9][0-9][0-9][0-9]*' | sed 's|.* (\#\([1-9][0-9][0-9][0-9][0-9]*\))|\1|')"
  if [ "$pr" != "" ]; then
    # Mark breaking changes with "[BREAKING]"
    if "$gh" issue view "$pr" --json labels | grep -q 'pr: breaking change'; then
      result="[BREAKING] $line"
    fi
  fi
  echo "$result"
done
