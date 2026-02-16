#!/usr/bin/env bash
# Find all tests related to specific diagnostic error codes
# Usage: ./extras/find-diagnostic-tests.sh 1 2 4 5 6 ...
# Outputs test file paths on stdout (suitable for piping to slang-test)
# Outputs informational messages on stderr

if [ $# -eq 0 ]; then
    echo "Usage: $0 <error_code1> <error_code2> ..." >&2
    echo "Example: $0 1 2 4 5 6 12 13 14 16 17 18 19 20 22 28 30 31" >&2
    exit 1
fi

# Build regex pattern for error codes (matches "error N:" or "error N " where N is exact)
codes_pattern=$(printf "|%s" "$@")
codes_pattern="error (${codes_pattern:1})[^0-9]"

echo "=== Searching for error codes: $@ ===" >&2

# Collect all test files
test_files=""

# Find .expected files containing these error codes
expected_files=$(grep -rlE "$codes_pattern" tests/**/*.expected 2>/dev/null | grep -v neural)
if [ -n "$expected_files" ]; then
    echo "=== Found .expected files mentioning these error codes ===" >&2
    for f in $expected_files; do
        # Get the test file (remove .expected suffix)
        test_file="${f%.expected}"
        if [ -f "$test_file" ]; then
            echo "  $test_file" >&2
            test_files="$test_files $test_file"
        else
            echo "  $f (expected file exists but test file missing - skipping)" >&2
        fi
    done
fi

# Find DIAGNOSTIC_TEST files (excluding neural)
echo "=== DIAGNOSTIC_TEST files (excluding neural) ===" >&2
diag_tests=$(grep -rl DIAGNOSTIC_TEST tests 2>/dev/null | grep -v neural | sort -u)
for f in $diag_tests; do
    test_files="$test_files $f"
done

# Output unique test files on stdout (for piping to slang-test)
echo $test_files | tr ' ' '\n' | sort -u | grep -v '^$'
