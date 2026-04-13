#!/usr/bin/env bash
# Analyze core dumps from CI test failures
#
# Downloads crash artifacts and build artifacts from a CI run,
# then runs gdb in the matching container to extract stack traces.
#
# Usage:
#   extras/analyze-ci-crash.sh <run-id-or-url> [--config debug|release]
#
# Examples:
#   extras/analyze-ci-crash.sh 24341817865
#   extras/analyze-ci-crash.sh https://github.com/shader-slang/slang/actions/runs/24341817865
#   extras/analyze-ci-crash.sh 24341817865 --config debug
#
# Requirements:
#   - gh CLI (authenticated)
#   - docker

set -euo pipefail

REPO="shader-slang/slang"
CONFIG="release"
CONTAINER_IMAGE="ghcr.io/shader-slang/slang-linux-gpu-ci:v1.5.1"

# Parse arguments
RUN_ID=""
while [[ $# -gt 0 ]]; do
	case "$1" in
	--config)
		CONFIG="$2"
		shift 2
		;;
	http*)
		# Extract run ID from URL
		RUN_ID=$(echo "$1" | grep -oE '[0-9]{10,}')
		shift
		;;
	*)
		RUN_ID="$1"
		shift
		;;
	esac
done

if [[ -z "$RUN_ID" ]]; then
	echo "Usage: extras/analyze-ci-crash.sh <run-id-or-url> [--config debug|release]"
	exit 1
fi

echo "=== CI Crash Analyzer ==="
echo "Run ID: $RUN_ID"
echo "Config: $CONFIG"
echo ""

WORK_DIR=$(mktemp -d)
echo "Working directory: $WORK_DIR"

cleanup() {
	echo ""
	echo "Crash data preserved at: $WORK_DIR"
	echo "Clean up manually: rm -rf $WORK_DIR"
}
trap cleanup EXIT

# Step 1: Check what artifacts are available
echo ""
echo "=== Step 1: Checking available artifacts ==="
gh run view "$RUN_ID" --repo "$REPO" --json jobs --jq '.jobs[] | select(.conclusion == "failure") | .name' 2>&1 || true
echo ""

CRASH_ARTIFACT="crash-logs-${CONFIG}-slang-test"
BUILD_ARTIFACT="slang-tests-linux-x86_64-gcc-${CONFIG}"

echo "Looking for artifacts:"
echo "  Crash: $CRASH_ARTIFACT"
echo "  Build: $BUILD_ARTIFACT"
echo ""

# Step 2: Download crash artifacts
echo "=== Step 2: Downloading crash artifacts ==="
if ! gh run download "$RUN_ID" --repo "$REPO" --name "$CRASH_ARTIFACT" --dir "$WORK_DIR/crash" 2>&1; then
	echo "No crash artifact found ($CRASH_ARTIFACT)"
	echo ""
	echo "Available artifacts:"
	gh run download "$RUN_ID" --repo "$REPO" --pattern "*crash*" --dir "$WORK_DIR/crash" 2>&1 || true
	gh run download "$RUN_ID" --repo "$REPO" --pattern "*retry*" --dir "$WORK_DIR/retry" 2>&1 || true

	# Show intermittency report if available
	if [[ -f "$WORK_DIR/retry/intermittency-report.json" ]]; then
		echo ""
		echo "=== Intermittency Report ==="
		cat "$WORK_DIR/retry/intermittency-report.json"
	fi

	echo ""
	echo "No core dumps found. The test server may have exited without crashing."
	echo "Check the CI log for JSON RPC failure details:"
	echo "  gh run view $RUN_ID --repo $REPO --log | grep 'JSON RPC failure\\|FAILED test'"
	exit 0
fi

# Count core files
CORE_COUNT=$(find "$WORK_DIR/crash" -name "core.*" 2>/dev/null | wc -l)
echo "Found $CORE_COUNT core dump(s)"

if [[ "$CORE_COUNT" -eq 0 ]]; then
	echo "No core dumps in the artifact. Listing contents:"
	find "$WORK_DIR/crash" -type f
	exit 0
fi

# Step 3: Download build artifacts (for debug symbols)
echo ""
echo "=== Step 3: Downloading build artifacts ==="
if ! gh run download "$RUN_ID" --repo "$REPO" --name "$BUILD_ARTIFACT" --dir "$WORK_DIR/build" 2>&1; then
	echo "Warning: Could not download build artifact ($BUILD_ARTIFACT)"
	echo "Stack traces will lack symbol names."
fi

# Step 4: Analyze each core dump
echo ""
echo "=== Step 4: Analyzing core dumps ==="

# Find the binary path in the build artifact
if [[ "$CONFIG" == "release" ]]; then
	CMAKE_CONFIG="Release"
elif [[ "$CONFIG" == "debug" ]]; then
	CMAKE_CONFIG="Debug"
else
	CMAKE_CONFIG="RelWithDebInfo"
fi

for core_file in "$WORK_DIR"/crash/core.*; do
	[[ -f "$core_file" ]] || continue

	# Extract executable name from core file name (core.PID.EXECUTABLE)
	core_basename=$(basename "$core_file")
	exe_name=$(echo "$core_basename" | cut -d. -f3)
	if [[ -z "$exe_name" ]]; then
		exe_name="test-server"
	fi

	echo ""
	echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	echo "Core dump: $core_basename"
	echo "Executable: $exe_name"
	echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

	# Find the matching binary
	BINARY=$(find "$WORK_DIR/build" -name "$exe_name" -type f 2>/dev/null | head -1)
	if [[ -z "$BINARY" ]]; then
		BINARY=$(find "$WORK_DIR/build" -name "$exe_name" -o -name "${exe_name}.exe" 2>/dev/null | head -1)
	fi

	if [[ -z "$BINARY" ]]; then
		echo "Warning: Could not find binary '$exe_name' in build artifacts"
		echo "Attempting analysis without symbols..."
		BINARY=""
	else
		echo "Binary: $BINARY"
	fi

	# Run gdb in the container for accurate library resolution
	echo ""
	echo "--- Backtrace ---"

	GDB_COMMANDS=$(
		cat <<'GDB_EOF'
set pagination off
set print thread-events off
echo \n=== Signal Info ===\n
info signal
echo \n=== Backtrace (crashing thread) ===\n
bt full 30
echo \n=== All Thread Backtraces ===\n
thread apply all bt 15
echo \n=== Registers ===\n
info registers
quit
GDB_EOF
	)

	if [[ -n "$BINARY" ]]; then
		docker run --rm \
			-v "$WORK_DIR:/analysis:ro" \
			"$CONTAINER_IMAGE" \
			bash -c "
                apt-get update -qq && apt-get install -y -qq gdb >/dev/null 2>&1
                gdb -batch \
                    -ex 'set pagination off' \
                    -ex 'bt full 30' \
                    -ex 'thread apply all bt 15' \
                    -ex 'info registers' \
                    '/analysis/build/$CMAKE_CONFIG/bin/$exe_name' \
                    '/analysis/crash/$core_basename' \
                    2>&1
            " 2>&1 || echo "(gdb analysis failed — container may not match the CI environment)"
	else
		echo "(No binary available for gdb analysis)"
		echo "Core file preserved at: $core_file"
		echo "To analyze manually:"
		echo "  docker run --rm -it -v $WORK_DIR:/analysis $CONTAINER_IMAGE bash"
		echo "  apt-get update && apt-get install -y gdb"
		echo "  gdb <binary> /analysis/crash/$core_basename"
	fi
done

# Step 5: Also show intermittency report if available
echo ""
echo "=== Step 5: Checking intermittency report ==="
gh run download "$RUN_ID" --repo "$REPO" --pattern "*retry*${CONFIG}*" --dir "$WORK_DIR/retry" 2>&1 || true

RETRY_FILE=$(find "$WORK_DIR/retry" -name "intermittency-report.json" 2>/dev/null | head -1)
if [[ -n "$RETRY_FILE" ]]; then
	echo "Retry report:"
	cat "$RETRY_FILE"
else
	echo "No intermittency report found (no tests were retried, or retries not yet enabled)"
fi

echo ""
echo "=== Analysis complete ==="
