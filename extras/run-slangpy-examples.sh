#!/usr/bin/env bash
#
# Run slangpy-samples examples with platform-specific skiplist support
#
# Usage: run-slangpy-examples.sh <platform> <slangpy-samples-dir> <site-packages>
#   platform: Current platform (windows, macos, linux)
#   slangpy-samples-dir: Path to slangpy-samples directory
#   site-packages: Path to Python site-packages directory
#
# Exit codes:
#   0 - All examples passed
#   1 - One or more examples failed

set -euo pipefail

# Check Bash version (associative arrays require Bash 4+)
if [ "${BASH_VERSINFO[0]}" -lt 4 ]; then
  echo "Error: Bash 4 or newer is required. Current version: $BASH_VERSION" >&2
  if [[ "$(uname)" == "Darwin" ]]; then
    echo "Please install a newer version of Bash using Homebrew:" >&2
    echo "  brew install bash" >&2
    echo "Then use /usr/local/bin/bash or /opt/homebrew/bin/bash" >&2
  fi
  exit 1
fi

# Parse arguments
if [ $# -ne 3 ]; then
  echo "Usage: $0 <platform> <slangpy-samples-dir> <site-packages>"
  exit 1
fi

PLATFORM="$1"
SAMPLES_DIR="$2"
SITE_PACKAGES="$3"

# Get script directory for finding skiplist
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SKIPLIST_FILE="$SCRIPT_DIR/expected-slangpy-example-failure.txt"

echo "Running slangpy-samples examples..."
echo "Platform: $PLATFORM"
echo "Samples directory: $SAMPLES_DIR"
echo "Site packages: $SITE_PACKAGES"

# Set PYTHONPATH
export PYTHONPATH="$SITE_PACKAGES"

# Determine timeout command (gtimeout on macOS, timeout elsewhere)
if [[ "$PLATFORM" == "macos" ]]; then
  TIMEOUT_CMD="gtimeout"
else
  TIMEOUT_CMD="timeout"
fi

# Change to samples directory
cd "$SAMPLES_DIR"

# Load skiplist with platform filtering
declare -A skip_examples
if [ -f "$SKIPLIST_FILE" ]; then
  echo "Loading skiplist from $SKIPLIST_FILE for platform: $PLATFORM"
  while IFS= read -r line || [ -n "$line" ]; do
    # Skip empty lines and comments
    [[ -z "$line" || "$line" =~ ^[[:space:]]*# ]] && continue

    # Extract example name and platform list
    # Format: example_name (platform1, platform2, ...)
    if [[ "$line" =~ ^([^(]+)(\(([^)]+)\))?[[:space:]]*$ ]]; then
      example_name=$(echo "${BASH_REMATCH[1]}" | xargs)
      platforms="${BASH_REMATCH[3]}"

      # Skip if example name is empty
      if [[ -z "$example_name" ]]; then
        continue
      fi

      # If no platforms specified, skip on all platforms
      if [[ -z "$platforms" ]]; then
        skip_examples["$example_name"]=1
        echo "  Will skip: $example_name (all platforms)"
        continue
      fi

      # Check if current platform is in the list
      if [[ "$platforms" =~ (^|[,[:space:]])$PLATFORM([,[:space:]]|$) ]]; then
        skip_examples["$example_name"]=1
        echo "  Will skip: $example_name ($platforms)"
      fi
    fi
  done <"$SKIPLIST_FILE"
else
  echo "No skiplist found at $SKIPLIST_FILE"
fi

# List available examples
echo ""
echo "Available examples:"
ls -la examples/

# Find all Python example scripts (excluding __pycache__ and other non-example files)
# Run each example script
failed_examples=()
successful_examples=()
skipped_examples=()

for example_dir in examples/*/; do
  # Skip if not a directory
  [ -d "$example_dir" ] || continue

  example_name=$(basename "$example_dir")

  # Skip cmake and other non-example directories
  if [[ "$example_name" == "CMakeLists.txt" ]]; then
    continue
  fi

  # Check if example is in skiplist
  # Use ${skip_examples[$example_name]:-} to safely handle missing keys
  if [[ -n "${skip_examples[$example_name]:-}" ]]; then
    echo ""
    echo "[SKIP] $example_name (in skiplist)"
    skipped_examples+=("$example_name")
    continue
  fi

  # Look for Python script with same name as directory, or main.py
  example_script="$example_dir${example_name}.py"
  if [ ! -f "$example_script" ]; then
    example_script="$example_dir/main.py"
  fi

  if [ -f "$example_script" ]; then
    echo ""
    echo "=========================================="
    echo "Running example: $example_name"
    echo "=========================================="

    if $TIMEOUT_CMD 60 python "$example_script"; then
      echo "[PASS] $example_name succeeded"
      successful_examples+=("$example_name")
    else
      echo "[FAIL] $example_name failed"
      failed_examples+=("$example_name")
    fi
  else
    echo "[WARN] No script found at $example_dir${example_name}.py or $example_dir/main.py, skipping"
  fi
done

echo ""
echo "=========================================="
echo "Summary"
echo "=========================================="
echo "Successful examples (${#successful_examples[@]}):"
for example in "${successful_examples[@]}"; do
  echo "  [PASS] $example"
done

if [ ${#skipped_examples[@]} -gt 0 ]; then
  echo ""
  echo "Skipped examples (${#skipped_examples[@]}):"
  for example in "${skipped_examples[@]}"; do
    echo "  [SKIP] $example"
  done
fi

if [ ${#failed_examples[@]} -gt 0 ]; then
  echo ""
  echo "Failed examples (${#failed_examples[@]}):"
  for example in "${failed_examples[@]}"; do
    echo "  [FAIL] $example"
  done
  exit 1
else
  echo ""
  echo "All examples passed!"
  exit 0
fi
