#!/usr/bin/env bash
# Generate main landing page for coverage reports

set -e

REPORTS_DIR="$1"
OUTPUT_FILE="$2"

if [[ -z "$REPORTS_DIR" ]]; then
  echo "Usage: $0 <reports-directory> [output-file]"
  echo "  If output-file not specified, generates to <reports-directory>/index.html"
  exit 1
fi

# Determine if we're generating for root or reports/ subdirectory
if [[ -z "$OUTPUT_FILE" ]]; then
  OUTPUT_FILE="${REPORTS_DIR}/index.html"
  LINK_PREFIX=""
else
  # Generating for root, so add reports/ prefix to links
  LINK_PREFIX="reports/"
fi

LATEST_SUMMARY="${REPORTS_DIR}/latest/coverage-summary.json"

# Function to extract JSON value (get first match only)
get_json_value() {
  local key=$1
  local file=$2
  grep -o "\"$key\"[[:space:]]*:[[:space:]]*\"[^\"]*\"" "$file" | head -1 | cut -d'"' -f4 || echo ""
}

get_json_number() {
  local key=$1
  local file=$2
  grep -o "\"$key\"[[:space:]]*:[[:space:]]*[0-9]*" "$file" | head -1 | grep -o '[0-9]*$' || echo "0"
}

# Function to get coverage badge color
get_badge_color() {
  local pct=$1
  pct=${pct%\%}

  if [[ "$pct" =~ ^[0-9]+\.?[0-9]*$ ]]; then
    if (($(echo "$pct >= 80" | bc -l))); then
      echo "#27ae60"
    elif (($(echo "$pct >= 60" | bc -l))); then
      echo "#f39c12"
    else
      echo "#e74c3c"
    fi
  else
    echo "#95a5a6"
  fi
}

# Function to format diff
format_diff() {
  local diff=$1

  if [[ -z "$diff" ]]; then
    echo ""
  elif [[ "$diff" =~ ^-?0\.00$ ]]; then
    echo ""
  else
    # Check if positive or negative
    if [[ "$diff" =~ ^- ]]; then
      echo "<span style=\"color: #e74c3c; font-size: 0.85em;\"> (${diff}pp)</span>"
    else
      echo "<span style=\"color: #27ae60; font-size: 0.85em;\"> (+${diff}pp)</span>"
    fi
  fi
}

# Check if we have combined multi-platform summary or single platform
COMBINED_SUMMARY="${REPORTS_DIR}/latest/combined-summary.json"

if [[ -f "$COMBINED_SUMMARY" ]]; then
  # New multi-platform format
  IS_MULTIPLATFORM=true
  DATE=$(get_json_value "date" "$COMBINED_SUMMARY")
  COMMIT=$(get_json_value "commit" "$COMBINED_SUMMARY")

  # Check which platforms are available
  LINUX_SUMMARY="${REPORTS_DIR}/latest/linux/coverage-summary.json"
  MACOS_SUMMARY="${REPORTS_DIR}/latest/macos/coverage-summary.json"

  HAS_LINUX=false
  HAS_MACOS=false

  if [[ -f "$LINUX_SUMMARY" ]]; then
    HAS_LINUX=true
    LINUX_LINE_COV=$(get_json_value "line_coverage" "$LINUX_SUMMARY")
    LINUX_LINES_HIT=$(get_json_number "lines_hit" "$LINUX_SUMMARY")
    LINUX_LINES_FOUND=$(get_json_number "lines_found" "$LINUX_SUMMARY")
    LINUX_REGION_COV=$(get_json_value "region_coverage" "$LINUX_SUMMARY")
    LINUX_REGIONS_HIT=$(get_json_number "regions_hit" "$LINUX_SUMMARY")
    LINUX_REGIONS_FOUND=$(get_json_number "regions_found" "$LINUX_SUMMARY")
    LINUX_FUNCTION_COV=$(get_json_value "function_coverage" "$LINUX_SUMMARY")
    LINUX_FUNCTIONS_HIT=$(get_json_number "functions_hit" "$LINUX_SUMMARY")
    LINUX_FUNCTIONS_FOUND=$(get_json_number "functions_found" "$LINUX_SUMMARY")
    LINUX_BRANCH_COV=$(get_json_value "branch_coverage" "$LINUX_SUMMARY")
    LINUX_BRANCHES_HIT=$(get_json_number "branches_hit" "$LINUX_SUMMARY")
    LINUX_BRANCHES_FOUND=$(get_json_number "branches_found" "$LINUX_SUMMARY")
  fi

  if [[ -f "$MACOS_SUMMARY" ]]; then
    HAS_MACOS=true
    MACOS_LINE_COV=$(get_json_value "line_coverage" "$MACOS_SUMMARY")
    MACOS_LINES_HIT=$(get_json_number "lines_hit" "$MACOS_SUMMARY")
    MACOS_LINES_FOUND=$(get_json_number "lines_found" "$MACOS_SUMMARY")
    MACOS_REGION_COV=$(get_json_value "region_coverage" "$MACOS_SUMMARY")
    MACOS_REGIONS_HIT=$(get_json_number "regions_hit" "$MACOS_SUMMARY")
    MACOS_REGIONS_FOUND=$(get_json_number "regions_found" "$MACOS_SUMMARY")
    MACOS_FUNCTION_COV=$(get_json_value "function_coverage" "$MACOS_SUMMARY")
    MACOS_FUNCTIONS_HIT=$(get_json_number "functions_hit" "$MACOS_SUMMARY")
    MACOS_FUNCTIONS_FOUND=$(get_json_number "functions_found" "$MACOS_SUMMARY")
    MACOS_BRANCH_COV=$(get_json_value "branch_coverage" "$MACOS_SUMMARY")
    MACOS_BRANCHES_HIT=$(get_json_number "branches_hit" "$MACOS_SUMMARY")
    MACOS_BRANCHES_FOUND=$(get_json_number "branches_found" "$MACOS_SUMMARY")
  fi

  # Use Linux coverage for the main badge (most comprehensive usually)
  if [[ "$HAS_LINUX" == "true" ]]; then
    LINE_COV="$LINUX_LINE_COV"
    LINE_COLOR=$(get_badge_color "$LINE_COV")
  elif [[ "$HAS_MACOS" == "true" ]]; then
    LINE_COV="$MACOS_LINE_COV"
    LINE_COLOR=$(get_badge_color "$LINE_COV")
  fi

  # Find previous report for diff calculation
  HISTORY_DIR="${REPORTS_DIR}/history"
  if [[ -d "$HISTORY_DIR" ]]; then
    # Get the most recent historical report (excluding current date/commit)
    # Use find to safely handle any number of directories
    PREV_REPORT=$(find "${HISTORY_DIR}" -maxdepth 1 -type d -name '[0-9]*-[0-9]*-[0-9]*-*' ! -name "${DATE}-${COMMIT}" -exec basename {} \; | sort -r | head -1)

    if [[ -n "$PREV_REPORT" ]]; then
      PREV_LINUX_SUMMARY="${HISTORY_DIR}/${PREV_REPORT}/linux/coverage-summary.json"
      PREV_MACOS_SUMMARY="${HISTORY_DIR}/${PREV_REPORT}/macos/coverage-summary.json"

      # Calculate diffs for Linux
      if [[ -f "$PREV_LINUX_SUMMARY" && "$HAS_LINUX" == "true" ]]; then
        prev_line=$(get_json_value "line_coverage" "$PREV_LINUX_SUMMARY")
        prev_region=$(get_json_value "region_coverage" "$PREV_LINUX_SUMMARY")
        prev_function=$(get_json_value "function_coverage" "$PREV_LINUX_SUMMARY")
        prev_branch=$(get_json_value "branch_coverage" "$PREV_LINUX_SUMMARY")

        LINUX_LINE_DIFF=$(awk -v curr="$LINUX_LINE_COV" -v prev="$prev_line" 'BEGIN {gsub(/%/, "", curr); gsub(/%/, "", prev); printf "%.2f", curr - prev}')
        LINUX_REGION_DIFF=$(awk -v curr="$LINUX_REGION_COV" -v prev="$prev_region" 'BEGIN {gsub(/%/, "", curr); gsub(/%/, "", prev); printf "%.2f", curr - prev}')
        LINUX_FUNCTION_DIFF=$(awk -v curr="$LINUX_FUNCTION_COV" -v prev="$prev_function" 'BEGIN {gsub(/%/, "", curr); gsub(/%/, "", prev); printf "%.2f", curr - prev}')
        LINUX_BRANCH_DIFF=$(awk -v curr="$LINUX_BRANCH_COV" -v prev="$prev_branch" 'BEGIN {gsub(/%/, "", curr); gsub(/%/, "", prev); printf "%.2f", curr - prev}')
      fi

      # Calculate diffs for macOS
      if [[ -f "$PREV_MACOS_SUMMARY" && "$HAS_MACOS" == "true" ]]; then
        prev_line=$(get_json_value "line_coverage" "$PREV_MACOS_SUMMARY")
        prev_region=$(get_json_value "region_coverage" "$PREV_MACOS_SUMMARY")
        prev_function=$(get_json_value "function_coverage" "$PREV_MACOS_SUMMARY")
        prev_branch=$(get_json_value "branch_coverage" "$PREV_MACOS_SUMMARY")

        MACOS_LINE_DIFF=$(awk -v curr="$MACOS_LINE_COV" -v prev="$prev_line" 'BEGIN {gsub(/%/, "", curr); gsub(/%/, "", prev); printf "%.2f", curr - prev}')
        MACOS_REGION_DIFF=$(awk -v curr="$MACOS_REGION_COV" -v prev="$prev_region" 'BEGIN {gsub(/%/, "", curr); gsub(/%/, "", prev); printf "%.2f", curr - prev}')
        MACOS_FUNCTION_DIFF=$(awk -v curr="$MACOS_FUNCTION_COV" -v prev="$prev_function" 'BEGIN {gsub(/%/, "", curr); gsub(/%/, "", prev); printf "%.2f", curr - prev}')
        MACOS_BRANCH_DIFF=$(awk -v curr="$MACOS_BRANCH_COV" -v prev="$prev_branch" 'BEGIN {gsub(/%/, "", curr); gsub(/%/, "", prev); printf "%.2f", curr - prev}')
      fi
    fi
  fi

  HAS_DATA=true
elif [[ -f "$LATEST_SUMMARY" ]]; then
  # Old single-platform format (backwards compatibility)
  IS_MULTIPLATFORM=false
  DATE=$(get_json_value "date" "$LATEST_SUMMARY")
  COMMIT=$(get_json_value "commit" "$LATEST_SUMMARY")

  LINE_COV=$(get_json_value "line_coverage" "$LATEST_SUMMARY")
  LINES_HIT=$(get_json_number "lines_hit" "$LATEST_SUMMARY")
  LINES_FOUND=$(get_json_number "lines_found" "$LATEST_SUMMARY")

  REGION_COV=$(get_json_value "region_coverage" "$LATEST_SUMMARY")
  REGIONS_HIT=$(get_json_number "regions_hit" "$LATEST_SUMMARY")
  REGIONS_FOUND=$(get_json_number "regions_found" "$LATEST_SUMMARY")

  FUNCTION_COV=$(get_json_value "function_coverage" "$LATEST_SUMMARY")
  FUNCTIONS_HIT=$(get_json_number "functions_hit" "$LATEST_SUMMARY")
  FUNCTIONS_FOUND=$(get_json_number "functions_found" "$LATEST_SUMMARY")

  BRANCH_COV=$(get_json_value "branch_coverage" "$LATEST_SUMMARY")
  BRANCHES_HIT=$(get_json_number "branches_hit" "$LATEST_SUMMARY")
  BRANCHES_FOUND=$(get_json_number "branches_found" "$LATEST_SUMMARY")

  LINE_COLOR=$(get_badge_color "$LINE_COV")
  HAS_DATA=true
else
  HAS_DATA=false
  IS_MULTIPLATFORM=false
fi

# Generate HTML
cat >"${OUTPUT_FILE}" <<'EOF'
<!DOCTYPE html>
<html>
<head>
    <title>Slang Code Coverage Reports</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .header {
            text-align: center;
            color: white;
            margin-bottom: 40px;
            padding: 20px;
        }
        .header h1 {
            font-size: 3em;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .header p {
            font-size: 1.2em;
            opacity: 0.9;
        }
        .card {
            background: white;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            padding: 40px;
            margin-bottom: 30px;
        }
        .coverage-badge {
            display: inline-flex;
            align-items: center;
            justify-content: center;
            width: 200px;
            height: 200px;
            border-radius: 50%;
            background: BADGE_COLOR;
            color: white;
            font-size: 3.5em;
            font-weight: bold;
            margin: 30px auto;
            box-shadow: 0 8px 20px rgba(0,0,0,0.2);
            text-shadow: 2px 2px 4px rgba(0,0,0,0.2);
        }
        .metrics {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin: 30px 0;
        }
        .metric {
            background: #f8f9fa;
            padding: 25px;
            border-radius: 10px;
            border-left: 4px solid #667eea;
        }
        .metric-label {
            color: #6c757d;
            font-size: 0.9em;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            margin-bottom: 8px;
        }
        .metric-value {
            font-size: 2em;
            font-weight: bold;
            color: #2c3e50;
            margin-bottom: 5px;
        }
        .metric-detail {
            color: #6c757d;
            font-size: 0.95em;
        }
        .info-section {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
        }
        .info-row {
            display: flex;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid #dee2e6;
        }
        .info-row:last-child {
            border-bottom: none;
        }
        .info-label {
            font-weight: 600;
            color: #495057;
        }
        .info-value {
            color: #6c757d;
        }
        .buttons {
            display: flex;
            gap: 15px;
            margin-top: 30px;
            flex-wrap: wrap;
        }
        .btn {
            display: inline-block;
            padding: 15px 30px;
            border-radius: 8px;
            text-decoration: none;
            font-weight: 600;
            transition: all 0.3s ease;
            flex: 1;
            text-align: center;
            min-width: 200px;
        }
        .btn-primary {
            background: #667eea;
            color: white;
        }
        .btn-primary:hover {
            background: #5568d3;
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }
        .btn-secondary {
            background: #6c757d;
            color: white;
        }
        .btn-secondary:hover {
            background: #5a6268;
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(108, 117, 125, 0.4);
        }
        .no-data {
            text-align: center;
            padding: 40px;
            color: #6c757d;
        }
        @media (max-width: 768px) {
            .header h1 {
                font-size: 2em;
            }
            .coverage-badge {
                width: 150px;
                height: 150px;
                font-size: 2.5em;
            }
            .metrics {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>📊 Slang Code Coverage</h1>
            <p>Nightly Coverage Reports for shader-slang/slang</p>
        </div>
EOF

if [[ "$HAS_DATA" == "true" ]]; then
  # Replace BADGE_COLOR with actual color
  # macOS requires empty string for -i, Linux doesn't
  if [[ "$OSTYPE" == "darwin"* ]]; then
    sed -i '' "s|BADGE_COLOR|${LINE_COLOR}|g" "${OUTPUT_FILE}"
  else
    sed -i "s|BADGE_COLOR|${LINE_COLOR}|g" "${OUTPUT_FILE}"
  fi

  if [[ "$IS_MULTIPLATFORM" == "true" ]]; then
    # Multi-platform display
    cat >>"${OUTPUT_FILE}" <<EOF
        <div class="card">
            <h2 style="color: #2c3e50; margin-bottom: 20px; text-align: center;">Coverage by Platform</h2>

            <style>
                .cov-good { color: #27ae60; font-weight: 600; }
                .cov-medium { color: #f39c12; font-weight: 600; }
                .cov-low { color: #e74c3c; font-weight: 600; }
            </style>

            <table style="width: 100%; border-collapse: collapse; margin: 20px 0;">
                <thead>
                    <tr style="background: #34495e; color: white;">
                        <th style="padding: 15px; text-align: left;">Platform</th>
                        <th style="padding: 15px; text-align: center;">Lines</th>
                        <th style="padding: 15px; text-align: center;">Regions</th>
                        <th style="padding: 15px; text-align: center;">Functions</th>
                        <th style="padding: 15px; text-align: center;">Branches</th>
                        <th style="padding: 15px; text-align: center;">Report</th>
                    </tr>
                </thead>
                <tbody>
EOF

    if [[ "$HAS_LINUX" == "true" ]]; then
      # Get color classes for each metric
      linux_line_class=$(get_badge_color "$LINUX_LINE_COV" | sed 's/#27ae60/cov-good/; s/#f39c12/cov-medium/; s/#e74c3c/cov-low/')
      linux_region_class=$(get_badge_color "$LINUX_REGION_COV" | sed 's/#27ae60/cov-good/; s/#f39c12/cov-medium/; s/#e74c3c/cov-low/')
      linux_function_class=$(get_badge_color "$LINUX_FUNCTION_COV" | sed 's/#27ae60/cov-good/; s/#f39c12/cov-medium/; s/#e74c3c/cov-low/')
      linux_branch_class=$(get_badge_color "$LINUX_BRANCH_COV" | sed 's/#27ae60/cov-good/; s/#f39c12/cov-medium/; s/#e74c3c/cov-low/')

      # Format diffs
      linux_line_diff_html=$(format_diff "$LINUX_LINE_DIFF")
      linux_region_diff_html=$(format_diff "$LINUX_REGION_DIFF")
      linux_function_diff_html=$(format_diff "$LINUX_FUNCTION_DIFF")
      linux_branch_diff_html=$(format_diff "$LINUX_BRANCH_DIFF")

      cat >>"${OUTPUT_FILE}" <<EOF
                    <tr style="border-bottom: 1px solid #ddd;">
                        <td style="padding: 15px;"><strong>🐧 Linux (x86_64)</strong></td>
                        <td style="padding: 15px; text-align: center;"><span class="${linux_line_class}">${LINUX_LINE_COV}</span>${linux_line_diff_html}<br><small>${LINUX_LINES_HIT}/${LINUX_LINES_FOUND}</small></td>
                        <td style="padding: 15px; text-align: center;"><span class="${linux_region_class}">${LINUX_REGION_COV}</span>${linux_region_diff_html}<br><small>${LINUX_REGIONS_HIT}/${LINUX_REGIONS_FOUND}</small></td>
                        <td style="padding: 15px; text-align: center;"><span class="${linux_function_class}">${LINUX_FUNCTION_COV}</span>${linux_function_diff_html}<br><small>${LINUX_FUNCTIONS_HIT}/${LINUX_FUNCTIONS_FOUND}</small></td>
                        <td style="padding: 15px; text-align: center;"><span class="${linux_branch_class}">${LINUX_BRANCH_COV}</span>${linux_branch_diff_html}<br><small>${LINUX_BRANCHES_HIT}/${LINUX_BRANCHES_FOUND}</small></td>
                        <td style="padding: 15px; text-align: center;"><a href="${LINK_PREFIX}latest/linux/index.html" style="color: #667eea;">View</a></td>
                    </tr>
EOF
    fi

    if [[ "$HAS_MACOS" == "true" ]]; then
      # Get color classes for each metric
      macos_line_class=$(get_badge_color "$MACOS_LINE_COV" | sed 's/#27ae60/cov-good/; s/#f39c12/cov-medium/; s/#e74c3c/cov-low/')
      macos_region_class=$(get_badge_color "$MACOS_REGION_COV" | sed 's/#27ae60/cov-good/; s/#f39c12/cov-medium/; s/#e74c3c/cov-low/')
      macos_function_class=$(get_badge_color "$MACOS_FUNCTION_COV" | sed 's/#27ae60/cov-good/; s/#f39c12/cov-medium/; s/#e74c3c/cov-low/')
      macos_branch_class=$(get_badge_color "$MACOS_BRANCH_COV" | sed 's/#27ae60/cov-good/; s/#f39c12/cov-medium/; s/#e74c3c/cov-low/')

      # Format diffs
      macos_line_diff_html=$(format_diff "$MACOS_LINE_DIFF")
      macos_region_diff_html=$(format_diff "$MACOS_REGION_DIFF")
      macos_function_diff_html=$(format_diff "$MACOS_FUNCTION_DIFF")
      macos_branch_diff_html=$(format_diff "$MACOS_BRANCH_DIFF")

      cat >>"${OUTPUT_FILE}" <<EOF
                    <tr style="border-bottom: 1px solid #ddd;">
                        <td style="padding: 15px;"><strong>🍎 macOS (aarch64)</strong></td>
                        <td style="padding: 15px; text-align: center;"><span class="${macos_line_class}">${MACOS_LINE_COV}</span>${macos_line_diff_html}<br><small>${MACOS_LINES_HIT}/${MACOS_LINES_FOUND}</small></td>
                        <td style="padding: 15px; text-align: center;"><span class="${macos_region_class}">${MACOS_REGION_COV}</span>${macos_region_diff_html}<br><small>${MACOS_REGIONS_HIT}/${MACOS_REGIONS_FOUND}</small></td>
                        <td style="padding: 15px; text-align: center;"><span class="${macos_function_class}">${MACOS_FUNCTION_COV}</span>${macos_function_diff_html}<br><small>${MACOS_FUNCTIONS_HIT}/${MACOS_FUNCTIONS_FOUND}</small></td>
                        <td style="padding: 15px; text-align: center;"><span class="${macos_branch_class}">${MACOS_BRANCH_COV}</span>${macos_branch_diff_html}<br><small>${MACOS_BRANCHES_HIT}/${MACOS_BRANCHES_FOUND}</small></td>
                        <td style="padding: 15px; text-align: center;"><a href="${LINK_PREFIX}latest/macos/index.html" style="color: #667eea;">View</a></td>
                    </tr>
EOF
    fi

    cat >>"${OUTPUT_FILE}" <<EOF
                </tbody>
            </table>

            <div class="info-section">
                <div class="info-row">
                    <span class="info-label">Report Date:</span>
                    <span class="info-value">${DATE}</span>
                </div>
                <div class="info-row">
                    <span class="info-label">Commit:</span>
                    <span class="info-value">
                        <a href="https://github.com/shader-slang/slang/commit/${COMMIT}"
                           style="color: #667eea; text-decoration: none;">${COMMIT}</a>
                    </span>
                </div>
            </div>

            <div class="buttons">
                <a href="${LINK_PREFIX}history/index.html" class="btn btn-secondary">📚 Historical Reports</a>
            </div>
        </div>
EOF
  else
    # Single-platform display (backwards compatibility)
    cat >>"${OUTPUT_FILE}" <<EOF
        <div class="card">
            <center>
                <div class="coverage-badge">${LINE_COV}</div>
                <h2 style="color: #2c3e50; margin-bottom: 10px;">Latest Line Coverage</h2>
            </center>

            <div class="metrics">
                <div class="metric">
                    <div class="metric-label">Line Coverage</div>
                    <div class="metric-value">${LINE_COV}</div>
                    <div class="metric-detail">${LINES_HIT} / ${LINES_FOUND} lines</div>
                </div>
                <div class="metric">
                    <div class="metric-label">Region Coverage</div>
                    <div class="metric-value">${REGION_COV}</div>
                    <div class="metric-detail">${REGIONS_HIT} / ${REGIONS_FOUND} regions</div>
                </div>
                <div class="metric">
                    <div class="metric-label">Function Coverage</div>
                    <div class="metric-value">${FUNCTION_COV}</div>
                    <div class="metric-detail">${FUNCTIONS_HIT} / ${FUNCTIONS_FOUND} functions</div>
                </div>
                <div class="metric">
                    <div class="metric-label">Branch Coverage</div>
                    <div class="metric-value">${BRANCH_COV}</div>
                    <div class="metric-detail">${BRANCHES_HIT} / ${BRANCHES_FOUND} branches</div>
                </div>
            </div>

            <div class="info-section">
                <div class="info-row">
                    <span class="info-label">Report Date:</span>
                    <span class="info-value">${DATE}</span>
                </div>
                <div class="info-row">
                    <span class="info-label">Commit:</span>
                    <span class="info-value">
                        <a href="https://github.com/shader-slang/slang/commit/${COMMIT}"
                           style="color: #667eea; text-decoration: none;">${COMMIT}</a>
                    </span>
                </div>
            </div>

            <div class="buttons">
                <a href="${LINK_PREFIX}latest/index.html" class="btn btn-primary">📄 View Detailed Report</a>
                <a href="${LINK_PREFIX}history/index.html" class="btn btn-secondary">📚 Historical Reports</a>
            </div>
        </div>
EOF
  fi
else
  cat >>"${OUTPUT_FILE}" <<'EOF'
        <div class="card">
            <div class="no-data">
                <h2>No Coverage Data Available</h2>
                <p style="margin-top: 10px;">Coverage reports will appear here after the first nightly run.</p>
            </div>

            <div class="buttons">
                <a href="${LINK_PREFIX}history/index.html" class="btn btn-secondary">📚 Historical Reports</a>
            </div>
        </div>
EOF
fi

cat >>"${OUTPUT_FILE}" <<EOF
        <div style="text-align: center; color: white; opacity: 0.8; margin-top: 30px;">
            <p>
                Download data: <a href="${LINK_PREFIX}coverage-data.csv" download style="color: white; text-decoration: underline;">CSV</a> |
                <a href="${LINK_PREFIX}coverage-data.json" download style="color: white; text-decoration: underline;">JSON</a>
            </p>
            <p style="margin-top: 10px;">Generated by <a href="https://github.com/shader-slang/slang"
               style="color: white; text-decoration: underline;">shader-slang/slang</a> nightly coverage workflow</p>
        </div>
    </div>
</body>
</html>
EOF

echo "Generated landing page at ${OUTPUT_FILE}"
