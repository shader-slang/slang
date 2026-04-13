#!/usr/bin/env bash
# Analyze retry reports from recent CI runs to identify flaky tests
#
# Downloads retry-report.json artifacts from recent CI runs and aggregates
# them to show which tests are most frequently intermittent.
#
# Usage:
#   extras/analyze-retry-reports.sh [--runs N] [--workflow NAME]
#
# Examples:
#   extras/analyze-retry-reports.sh                    # Last 20 CI runs
#   extras/analyze-retry-reports.sh --runs 50          # Last 50 runs
#   extras/analyze-retry-reports.sh --workflow "CI"     # Specific workflow
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
		echo "Usage: extras/analyze-retry-reports.sh [--runs N] [--workflow NAME]"
		exit 1
		;;
	esac
done

WORK_DIR=$(mktemp -d)
trap "rm -rf $WORK_DIR" EXIT

echo "=== Retry Report Analysis ==="
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

# Step 2: Download retry reports
echo "Downloading retry reports..."
DOWNLOAD_COUNT=0
REPORT_COUNT=0

for run_id in $RUN_IDS; do
	DOWNLOAD_COUNT=$((DOWNLOAD_COUNT + 1))
	printf "\r  Progress: %d/%d runs checked" "$DOWNLOAD_COUNT" "$RUN_COUNT"

	# Download all retry-report artifacts for this run
	gh run download "$run_id" --repo "$REPO" \
		--pattern "retry-report-*" \
		--dir "$WORK_DIR/$run_id" 2>/dev/null || continue

	# Get run metadata
	RUN_DATE=$(gh run view "$run_id" --repo "$REPO" --json createdAt --jq '.createdAt' 2>/dev/null | cut -dT -f1)

	# Tag each report with run metadata
	for report_dir in "$WORK_DIR/$run_id"/retry-report-*; do
		[[ -d "$report_dir" ]] || continue
		report_file="$report_dir/retry-report.json"
		[[ -f "$report_file" ]] || continue

		# Extract platform info from artifact name (retry-report-<os>-<config>-slang-test)
		artifact_name=$(basename "$report_dir")
		platform=$(echo "$artifact_name" | sed 's/retry-report-//; s/-slang-test$//')

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
	echo "No retry reports found in the last $NUM_RUNS runs."
	echo "This means either:"
	echo "  - No tests needed retries (good!)"
	echo "  - Retry reporting is not yet enabled (PR #10813)"
	exit 0
fi

echo "Found $REPORT_COUNT retry reports with data"
echo ""

# Step 3: Aggregate results
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Most frequently retried tests (last $NUM_RUNS runs)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Extract all test entries with pass/fail status
jq -r '.tests[] | "\(.name)\t\(.passed_on_retry)"' "$WORK_DIR/all-reports.jsonl" 2>/dev/null |
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

jq -r '"\(.platform)\t\(.total_retried)\t\(.passed_on_retry)"' "$WORK_DIR/all-reports.jsonl" 2>/dev/null |
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

jq -r '"\(.date)\t\(.platform)\t\(.total_retried)\t\(.passed_on_retry)"' "$WORK_DIR/all-reports.jsonl" 2>/dev/null |
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

jq -r '.tests[] | "\(.name)\t\(.passed_on_retry)"' "$WORK_DIR/all-reports.jsonl" 2>/dev/null |
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
echo "=== Analysis complete ==="
