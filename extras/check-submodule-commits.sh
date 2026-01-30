#!/bin/bash
# Script to verify that any changed submodule commits are in the git history
# of that submodule repository's main branch.
#
# This is used by CI to ensure developers don't accidentally check in commits
# that point to user branches or commits not on any branch.
#
# Usage:
#   ./extras/check-submodule-commits.sh [base-ref]
#
# Arguments:
#   base-ref: Git reference to compare against (default: origin/master)

set -e

# Output helpers
echo_info() {
  echo "INFO: $1"
}

echo_success() {
  echo "SUCCESS: $1"
}

echo_error() {
  echo "ERROR: $1"
}

echo_warning() {
  echo "WARNING: $1"
}

# Get the base reference to compare against
BASE_REF="${1:-origin/master}"

echo_info "Checking submodule commits against their main branches..."
echo_info "Base reference: $BASE_REF"
echo ""

# Check if we're in a git repository
if ! git rev-parse --git-dir > /dev/null 2>&1; then
  echo_error "Not in a git repository"
  exit 1
fi

# Get list of changed submodules using git diff
echo_info "Detecting changed submodules..."

# Use git diff to detect submodule changes
# Format: "Subproject commit <old-hash>..<new-hash>" or "+Subproject commit <hash>"
SUBMODULE_CHANGES=$(git diff "$BASE_REF" HEAD 2>/dev/null | grep -E "^([\+\-]Subproject commit|Subproject commit)" || true)

if [ -z "$SUBMODULE_CHANGES" ]; then
  echo_info "No submodule changes detected"
  echo_success "All submodule commits are valid (no changes detected)"
  exit 0
fi

# Track if we found any issues
ISSUES_FOUND=0

# Get list of all submodule paths from git
while IFS= read -r line; do
  # Parse: " <commit> <path> (<something>)" or "<commit> <path>" (with or without leading space)
  if [[ "$line" =~ ^[[:space:]]*([a-f0-9]+)[[:space:]]+([^[:space:]]+) ]]; then
    commit="${BASH_REMATCH[1]}"
    submodule_path="${BASH_REMATCH[2]}"
    
    # Check if this submodule has changed between BASE_REF and HEAD
    DIFF_OUTPUT=$(git diff "$BASE_REF" HEAD -- "$submodule_path" 2>/dev/null || true)
    
    if [ -z "$DIFF_OUTPUT" ]; then
      # No changes in this submodule
      continue
    fi
    
    # Extract the new commit hash
    NEW_COMMIT=$(echo "$DIFF_OUTPUT" | grep "^+Subproject commit" | awk '{print $3}' || true)
    
    if [ -z "$NEW_COMMIT" ]; then
      # This might be a deletion or no actual commit change
      continue
    fi
    
    echo ""
    echo_info "Checking submodule: $submodule_path"
    echo_info "  New commit: $NEW_COMMIT"
    
    # Initialize the submodule if needed
    if [ ! -d "$submodule_path/.git" ]; then
      echo_info "  Initializing submodule..."
      git submodule update --init "$submodule_path" > /dev/null 2>&1 || {
        echo_warning "  Failed to initialize submodule"
        continue
      }
    fi
    
    # Check the submodule in a subshell to avoid directory issues
    (
      cd "$submodule_path" || exit 1
      
      # Fetch the latest from origin to ensure we have up-to-date refs
      echo_info "  Fetching latest refs from origin..."
      git fetch origin --quiet 2>/dev/null || {
        echo_warning "  Failed to fetch from origin, using cached refs"
      }
      
      # Try to determine the default branch name (main or master)
      DEFAULT_BRANCH=""
      if git show-ref --verify --quiet refs/remotes/origin/main; then
        DEFAULT_BRANCH="origin/main"
      elif git show-ref --verify --quiet refs/remotes/origin/master; then
        DEFAULT_BRANCH="origin/master"
      else
        echo_error "  Could not find origin/main or origin/master branch"
        exit 2
      fi
      
      echo_info "  Default branch: $DEFAULT_BRANCH"
      
      # Check if the commit is in the history of the default branch
      if git merge-base --is-ancestor "$NEW_COMMIT" "$DEFAULT_BRANCH" 2>/dev/null; then
        echo_success "  ✓ Commit $NEW_COMMIT is in the history of $DEFAULT_BRANCH"
        exit 0
      else
        echo_error "  ✗ Commit $NEW_COMMIT is NOT in the history of $DEFAULT_BRANCH"
        echo_error "  This commit may be from a user branch or not on any branch!"
        echo_error "  Please update the submodule to point to a commit on the main branch."
        exit 2
      fi
    )
    
    # Check subshell exit status
    SUBSHELL_EXIT=$?
    if [ $SUBSHELL_EXIT -eq 1 ]; then
      echo_warning "  Failed to enter submodule directory"
      continue
    elif [ $SUBSHELL_EXIT -eq 2 ]; then
      ISSUES_FOUND=1
    fi
    
  fi
done < <(git submodule status)

echo ""
if [ $ISSUES_FOUND -eq 0 ]; then
  echo_success "All submodule commits are on their respective main branches"
  exit 0
else
  echo_error "Some submodule commits are NOT on their respective main branches"
  echo_error "Please update the affected submodules to use commits from the main branch"
  exit 1
fi
