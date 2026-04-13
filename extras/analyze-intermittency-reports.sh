#!/usr/bin/env bash
# Analyze intermittency reports from recent CI runs to identify flaky tests
#
# Downloads intermittency-report.json artifacts from recent CI runs and aggregates
# them to show which tests are most frequently intermittent.
#
# Usage:
#   extras/analyze-intermittency-reports.sh [--runs N] [--workflow NAME]
#
# Examples:
#   extras/analyze-intermittency-reports.sh                    # Last 20 CI runs
#   extras/analyze-intermittency-reports.sh --runs 50          # Last 50 runs
#   extras/analyze-intermittency-reports.sh --workflow "CI"     # Specific workflow
#
# Requirements:
#   - gh CLI (authenticated)
#   - jq

set -euo pipefail

REPO="shader-slang/slang"
NUM_RUNS=20
WORKFLOW="CI"

while [[ $# -gt 0 ]]; do
	case "$1" in
	--runs)
		NUM_RUNS="$2"
		shift 2
		;;
	--workflow)
		WORKFLOW="$2"
		shift 2
		;;
	*)
		echo "Usage: extras/analyze-intermittency-reports.sh [--runs N] [--workflow NAME]"
		exit 1
		;;
	esac
done

WORK_DIR=$(mktemp -d)
trap "rm -rf $WORK_DIR" EXIT

echo "=== Intermittency Report Analysis ==="
echo "Repository: $REPO"
echo "Workflow: $WORKFLOW"
echo "Analyzing last $NUM_RUNS runs..."
echo ""

# Step 1: Get recent run IDs
RUN_IDS=$(gh run list --repo "$REPO" --workflow "$WORKFLOW" --limit "$NUM_RUNS" \
	--json databaseId,conclusion,createdAt \
	--jq '.[] | select(.conclusion == "success" or .conclusion == "failure") | .databaseId')

RUN_COUNT=$(echo "$RUN_IDS" | wc -l | tr -d ' ')
echo "Found $RUN_COUNT completed runs"
echo ""

# Step 2: Download intermittency reports
echo "Downloading intermittency reports..."
DOWNLOAD_COUNT=0
REPORT_COUNT=0

for run_id in $RUN_IDS; do
	DOWNLOAD_COUNT=$((DOWNLOAD_COUNT + 1))
	printf "\r  Progress: %d/%d runs checked" "$DOWNLOAD_COUNT" "$RUN_COUNT"

	# Download all intermittency-report artifacts for this run
	gh run download "$run_id" --repo "$REPO" \
		--pattern "intermittency-report-*" \
		--dir "$WORK_DIR/$run_id" 2>/dev/null || continue

	# Get run metadata
	RUN_DATE=$(gh run view "$run_id" --repo "$REPO" --json createdAt --jq '.createdAt' 2>/dev/null | cut -dT -f1)

	# Tag each report with run metadata
	for report_dir in "$WORK_DIR/$run_id"/intermittency-report-*; do
		[[ -d "$report_dir" ]] || continue
		report_file="$report_dir/intermittency-report.json"
		[[ -f "$report_file" ]] || continue

		# Extract platform info from artifact name (intermittency-report-<os>-<config>-slang-test)
		artifact_name=$(basename "$report_dir")
		platform=$(echo "$artifact_name" | sed 's/intermittency-report-//; s/-slang-test$//')

		# Add metadata to a combined file
		jq --arg run_id "$run_id" \
			--arg date "$RUN_DATE" \
			--arg platform "$platform" \
			'{run_id: $run_id, date: $date, platform: $platform} + .' \
			"$report_file" >>"$WORK_DIR/all-reports.jsonl" 2>/dev/null

		REPORT_COUNT=$((REPORT_COUNT + 1))
	done
done

echo ""
echo ""

if [[ "$REPORT_COUNT" -eq 0 ]]; then
	echo "No intermittency reports found in the last $NUM_RUNS runs."
	echo "This means either:"
	echo "  - No tests needed retries (good!)"
	echo "  - Retry reporting is not yet enabled (PR #10813)"
	exit 0
fi

echo "Found $REPORT_COUNT intermittency reports with data"
echo ""

# Step 3: Aggregate results
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Most frequently retried tests (last $NUM_RUNS runs)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Extract all test entries with pass/fail status
jq -r '.retries.tests[] | "\(.name)\t\(.passed_on_retry)"' "$WORK_DIR/all-reports.jsonl" 2>/dev/null |
	sort |
	awk -F'\t' '
    {
        count[$1]++
        if ($2 == "true") passed[$1]++
    }
    END {
        for (test in count) {
            p = (test in passed) ? passed[test] : 0
            printf "%4d  %-70s [%d/%d passed on retry]\n", count[test], test, p, count[test]
        }
    }
' | sort -rn

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "By platform"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

jq -r '"\(.platform)\t\(.retries.total)\t\(.retries.passed_on_retry)"' "$WORK_DIR/all-reports.jsonl" 2>/dev/null |
	awk -F'\t' '
    {
        retries[$1] += $2
        passed[$1] += $3
        runs[$1]++
    }
    END {
        for (platform in retries) {
            printf "  %-25s %3d retries across %2d runs (%d passed on retry)\n",
                platform ":", retries[platform], runs[platform], passed[platform]
        }
    }
' | sort -t: -k2 -rn

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Timeline (most recent first)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

jq -r '"\(.date)\t\(.platform)\t\(.retries.total)\t\(.retries.passed_on_retry)"' "$WORK_DIR/all-reports.jsonl" 2>/dev/null |
	sort -rn |
	awk -F'\t' '
    {
        printf "  %s  %-25s %2d retried, %2d passed on retry\n", $1, $2, $3, $4
    }
'

echo ""

# Step 4: Identify always-fail tests (never pass on retry)
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Always-fail tests (never pass on retry — likely real bugs)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

jq -r '.retries.tests[] | "\(.name)\t\(.passed_on_retry)"' "$WORK_DIR/all-reports.jsonl" 2>/dev/null |
	sort |
	awk -F'\t' '
    {
        count[$1]++
        if ($2 == "true") passed[$1]++
    }
    END {
        found = 0
        for (test in count) {
            if (!(test in passed) && count[test] >= 2) {
                printf "%4d  %s\n", count[test], test
                found++
            }
        }
        if (found == 0) print "  (none — all retried tests passed at least once)"
    }
' | sort -rn

echo ""

# Step 5: GPU crashes and core dumps
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "GPU crashes and core dumps"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

GPU_CRASHES=$(jq -r 'select(.system.gpu_crashed == true) | "\(.date)\t\(.platform)"' "$WORK_DIR/all-reports.jsonl" 2>/dev/null | wc -l | tr -d ' ')
CORE_DUMP_RUNS=$(jq -r 'select(.system.core_dumps > 0) | "\(.date)\t\(.platform)\t\(.system.core_dumps)\t\(.system.core_files)"' "$WORK_DIR/all-reports.jsonl" 2>/dev/null)

echo "  GPU crashes: $GPU_CRASHES"
if [[ "$GPU_CRASHES" -gt 0 ]]; then
	echo "  Runs with GPU crashes:"
	jq -r 'select(.system.gpu_crashed == true) | "    \(.date)  \(.platform)"' "$WORK_DIR/all-reports.jsonl" 2>/dev/null
fi

CORE_COUNT=$(echo "$CORE_DUMP_RUNS" | grep -c . 2>/dev/null || echo "0")
echo "  Runs with core dumps: $CORE_COUNT"
if [[ -n "$CORE_DUMP_RUNS" && "$CORE_COUNT" -gt 0 ]]; then
	echo "$CORE_DUMP_RUNS" | awk -F'\t' '{printf "    %s  %-25s %s core(s): %s\n", $1, $2, $3, $4}'
fi

# Step 6: Scheduling stopped (too many consecutive failures)
STOPPED_COUNT=$(jq -r 'select(.scheduling_stopped == true) | "\(.date)\t\(.platform)"' "$WORK_DIR/all-reports.jsonl" 2>/dev/null | wc -l | tr -d ' ')
echo ""
echo "  Test scheduling stopped (cascade failure): $STOPPED_COUNT"
if [[ "$STOPPED_COUNT" -gt 0 ]]; then
	jq -r 'select(.scheduling_stopped == true) | "    \(.date)  \(.platform)"' "$WORK_DIR/all-reports.jsonl" 2>/dev/null
fi

echo ""
echo "=== Analysis complete ==="
