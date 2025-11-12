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

# Helper function for timeout (works on all platforms without external tools)
run_with_timeout() {
  local timeout_duration=$1
  shift

  # Run command in background
  "$@" &
  local pid=$!

  # Wait for timeout duration
  local count=0
  while [ $count -lt $timeout_duration ]; do
    # Check if process is still running
    if ! kill -0 $pid 2>/dev/null; then
      # Process finished, get exit code
      wait $pid
      return $?
    fi
    sleep 1
    count=$((count + 1))
  done

  # Timeout reached, kill process
  kill $pid 2>/dev/null
  wait $pid 2>/dev/null
  return 124 # Standard timeout exit code
}

# Change to samples directory
cd "$SAMPLES_DIR"

# Load skiplist with platform filtering (using simple string to avoid Bash 4+ requirement)
skip_examples=""
if [ -f "$SKIPLIST_FILE" ]; then
  echo "Loading skiplist from $SKIPLIST_FILE for platform: $PLATFORM"
  while IFS= read -r line || [ -n "$line" ]; do
    # Skip empty lines and comments
    [[ -z "$line" || "$line" =~ ^[[:space:]]*# ]] && continue

    # Strip inline comments (everything after #)
    line="${line%%#*}"

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
        skip_examples="${skip_examples}${example_name}|"
        echo "  Will skip: $example_name (all platforms)"
        continue
      fi

      # Check if current platform is in the list
      if [[ "$platforms" =~ (^|[,[:space:]])$PLATFORM([,[:space:]]|$) ]]; then
        skip_examples="${skip_examples}${example_name}|"
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

  # Check if example is in skiplist (using simple string match)
  if [[ "$skip_examples" == *"|${example_name}|"* ]]; then
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

    if run_with_timeout 60 python "$example_script"; then
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
