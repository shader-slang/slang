#!/usr/bin/env bash

# Install git hooks from extras/git-hooks/ to .git/hooks/
# This script should be run from the repository root

set -e

REPO_ROOT=$(git rev-parse --show-toplevel 2>/dev/null) || {
  echo "Error: Not in a git repository"
  exit 1
}

HOOKS_SOURCE="$REPO_ROOT/extras/git-hooks"
HOOKS_TARGET="$REPO_ROOT/.git/hooks"

if [ ! -d "$HOOKS_SOURCE" ]; then
  echo "Error: Source hooks directory not found: $HOOKS_SOURCE"
  exit 1
fi

if [ ! -d "$HOOKS_TARGET" ]; then
  echo "Error: Target hooks directory not found: $HOOKS_TARGET"
  exit 1
fi

echo "Installing git hooks from $HOOKS_SOURCE to $HOOKS_TARGET..."

installed_count=0
skipped_count=0

# Install each hook from the source directory
for hook in "$HOOKS_SOURCE"/*; do
  if [ ! -f "$hook" ]; then
    continue
  fi

  hook_name=$(basename "$hook")
  target_hook="$HOOKS_TARGET/$hook_name"

  # Check if hook already exists
  if [ -f "$target_hook" ] || [ -L "$target_hook" ]; then
    echo "  $hook_name: already exists, skipping"
    ((skipped_count++)) || :
    continue
  fi

  # Copy the hook and make it executable
  cp "$hook" "$target_hook"
  chmod +x "$target_hook"
  echo "  $hook_name: installed"
  ((installed_count++)) || :
done

echo ""
echo "Installation complete:"
echo "  - Installed: $installed_count hook(s)"
echo "  - Skipped: $skipped_count hook(s)"

if [ $installed_count -gt 0 ]; then
  echo ""
  echo "Git hooks are now active for this repository."
  echo "To uninstall, simply remove the files from .git/hooks/"
elif [ $skipped_count -gt 0 ]; then
  echo ""
  echo "All hooks are already installed."
fi

exit 0
