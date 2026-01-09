#!/bin/bash
# generate-examples.sh
# Generates test files from all example templates in this directory

set -e

# Get the script directory and workspace root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

# Path to slang-fiddle (try Release first, then Debug)
SLANG_FIDDLE="$WORKSPACE_ROOT/build/generators/Release/bin/slang-fiddle"
if [ ! -f "$SLANG_FIDDLE" ]; then
  SLANG_FIDDLE="$WORKSPACE_ROOT/build/generators/Debug/bin/slang-fiddle"
fi

if [ ! -f "$SLANG_FIDDLE" ]; then
  echo "Error: slang-fiddle not found. Please build the project first."
  echo "Tried:"
  echo "  $WORKSPACE_ROOT/build/generators/Release/bin/slang-fiddle"
  echo "  $WORKSPACE_ROOT/build/generators/Debug/bin/slang-fiddle"
  exit 1
fi

# Output directory
OUTPUT_DIR="$WORKSPACE_ROOT/tests/generated-tests"

echo "========================================="
echo "Test Generation from Templates"
echo "========================================="
echo "Workspace: $WORKSPACE_ROOT"
echo "Templates: $SCRIPT_DIR"
echo "Output:    $OUTPUT_DIR"
echo "Fiddle:    $SLANG_FIDDLE"
echo ""

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Counter for statistics
total_templates=0
total_tests=0

# Process each .slang file in the examples directory
for template in "$SCRIPT_DIR"/*.slang; do
  if [ -f "$template" ]; then
    template_name=$(basename "$template")
    echo "Processing: $template_name"

    # Run slang-fiddle in test-gen mode
    output=$("$SLANG_FIDDLE" --mode test-gen --input "$template" --output-dir "$OUTPUT_DIR" 2>&1)

    # Count generated files
    count=$(echo "$output" | grep -c "^Generated:" || true)

    echo "$output"
    echo "  → Generated $count test file(s)"
    echo ""

    total_templates=$((total_templates + 1))
    total_tests=$((total_tests + count))
  fi
done

echo "========================================="
echo "Summary"
echo "========================================="
echo "Templates processed: $total_templates"
echo "Tests generated:     $total_tests"
echo ""
echo "Generated tests are in: $OUTPUT_DIR"
echo ""
echo "To run the generated tests:"
echo "  cd $WORKSPACE_ROOT"
echo "  ./build/Release/bin/slang-test $OUTPUT_DIR"
