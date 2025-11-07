#!/usr/bin/env bash
# Generate historical coverage index HTML

set -e

HISTORY_DIR="$1"

if [[ -z "$HISTORY_DIR" ]]; then
  echo "Usage: $0 <history-directory>"
  exit 1
fi

# Generate HTML header
cat >"${HISTORY_DIR}/index.html" <<'EOF'
<!DOCTYPE html>
<html>
<head>
    <title>Historical Coverage Reports - Slang</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            max-width: 1000px;
            margin: 50px auto;
            padding: 20px;
            line-height: 1.6;
            color: #333;
        }
        h1 {
            color: #2c3e50;
            border-bottom: 3px solid #3498db;
            padding-bottom: 10px;
        }
        .nav {
            margin: 20px 0;
        }
        .nav a {
            color: #3498db;
            text-decoration: none;
            font-weight: 500;
        }
        .nav a:hover {
            text-decoration: underline;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 30px 0;
            background: white;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        th, td {
            padding: 15px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        th {
            background-color: #34495e;
            color: white;
            font-weight: 600;
        }
        tr:hover {
            background-color: #f5f5f5;
        }
        a {
            color: #3498db;
            text-decoration: none;
        }
        a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <h1>📊 Historical Coverage Reports</h1>

    <div class="nav">
        <a href="../../index.html">← Back to Home</a> |
        <a href="../latest/index.html">View Latest Report</a>
    </div>

    <style>
        .coverage-cell {
            text-align: center;
            font-family: 'Monaco', 'Courier New', monospace;
        }
        .coverage-good { color: #27ae60; font-weight: 600; }
        .coverage-medium { color: #f39c12; font-weight: 600; }
        .coverage-low { color: #e74c3c; font-weight: 600; }
        .coverage-na { color: #95a5a6; }
    </style>
EOF

# Add the table header
cat >>"${HISTORY_DIR}/index.html" <<'EOF'
    <table>
        <tr>
            <th>Date</th>
            <th>Commit</th>
            <th>🐧 Linux</th>
            <th>🍎 macOS</th>
        </tr>
EOF

# Function to get coverage class based on percentage
get_coverage_class() {
  local pct=$1
  # Remove % sign if present
  pct=${pct%\%}

  if [[ "$pct" =~ ^[0-9]+\.?[0-9]*$ ]]; then
    if (($(echo "$pct >= 80" | bc -l))); then
      echo "coverage-good"
    elif (($(echo "$pct >= 60" | bc -l))); then
      echo "coverage-medium"
    else
      echo "coverage-low"
    fi
  else
    echo "coverage-na"
  fi
}

# Function to format coverage cell
format_coverage() {
  local pct=$1
  local hit=$2
  local total=$3

  if [[ -n "$pct" && "$pct" != "null" ]]; then
    local class
    class=$(get_coverage_class "$pct")
    echo "<span class=\"$class\">${pct}</span><br><small>${hit} / ${total}</small>"
  else
    echo "<span class=\"coverage-na\">N/A</span>"
  fi
}

# Function to get platform coverage with link
get_platform_coverage() {
  local report_dir=$1
  local platform=$2
  local dir_name=$3
  local platform_summary="${report_dir}/${platform}/coverage-summary.json"

  if [[ -f "$platform_summary" ]]; then
    local line_cov
    local lines_hit
    local lines_found
    line_cov=$(grep -o '"line_coverage"[[:space:]]*:[[:space:]]*"[^"]*"' "$platform_summary" | cut -d'"' -f4)
    lines_hit=$(grep -o '"lines_hit"[[:space:]]*:[[:space:]]*[0-9]*' "$platform_summary" | grep -o '[0-9]*$')
    lines_found=$(grep -o '"lines_found"[[:space:]]*:[[:space:]]*[0-9]*' "$platform_summary" | grep -o '[0-9]*$')

    local coverage_html
    coverage_html=$(format_coverage "$line_cov" "$lines_hit" "$lines_found")
    echo "${coverage_html}<br><a href=\"${dir_name}/${platform}/index.html\" style=\"font-size: 0.9em;\">View →</a>"
  else
    echo "<span class=\"coverage-na\">N/A</span>"
  fi
}

# List all reports in reverse chronological order
# Use find to safely handle any number of directories
report_count=0
while IFS= read -r report_path; do
  dir=$(basename "$report_path")
  date_part=$(echo "$dir" | cut -d'-' -f1-3)
  commit_part=$(echo "$dir" | cut -d'-' -f4)

  report_dir="${HISTORY_DIR}/${dir}"
  combined_summary="${report_dir}/combined-summary.json"
  old_summary="${report_dir}/coverage-summary.json"
  legacy_html="${report_dir}/index.html"

  # Detect report format: new multi-platform, old single-platform, or legacy llvm-cov
  if [[ -f "$combined_summary" ]]; then
    # New multi-platform report - link directly to platform reports
    linux_cell=$(get_platform_coverage "$report_dir" "linux" "$dir")
    macos_cell=$(get_platform_coverage "$report_dir" "macos" "$dir")
  elif [[ -f "$old_summary" ]]; then
    # Old single-platform report with JSON (post-summary, pre-multiplatform)
    line_cov=$(grep -o '"line_coverage"[[:space:]]*:[[:space:]]*"[^"]*"' "$old_summary" | cut -d'"' -f4)
    lines_hit=$(grep -o '"lines_hit"[[:space:]]*:[[:space:]]*[0-9]*' "$old_summary" | grep -o '[0-9]*$')
    lines_found=$(grep -o '"lines_found"[[:space:]]*:[[:space:]]*[0-9]*' "$old_summary" | grep -o '[0-9]*$')

    coverage_html=$(format_coverage "$line_cov" "$lines_hit" "$lines_found")
    linux_cell="${coverage_html}<br><a href=\"$dir/index.html\" style=\"font-size: 0.9em;\">View →</a>"
    macos_cell="<span class=\"coverage-na\">N/A</span>"
  elif [[ -f "$legacy_html" ]]; then
    # Legacy llvm-cov HTML report (pre-JSON summary)
    linux_cell="<span class=\"coverage-na\" style=\"font-size: 0.85em; font-style: italic;\">Legacy</span><br><a href=\"$dir/index.html\" style=\"font-size: 0.9em;\">View →</a>"
    macos_cell="<span class=\"coverage-na\">N/A</span>"
  else
    # No data found at all
    linux_cell="<span class=\"coverage-na\">N/A</span>"
    macos_cell="<span class=\"coverage-na\">N/A</span>"
  fi

  {
    echo "        <tr>"
    echo "            <td>$date_part</td>"
    echo "            <td><a href=\"https://github.com/shader-slang/slang/commit/$commit_part\">$commit_part</a></td>"
    echo "            <td class=\"coverage-cell\">$linux_cell</td>"
    echo "            <td class=\"coverage-cell\">$macos_cell</td>"
    echo "        </tr>"
  } >>"${HISTORY_DIR}/index.html"
  report_count=$((report_count + 1))
done < <(find "${HISTORY_DIR}" -maxdepth 1 -type d -name '[0-9]*-[0-9]*-[0-9]*-*' | sort -r)

# Generate HTML footer
cat >>"${HISTORY_DIR}/index.html" <<'EOF'
    </table>

    <div style="text-align: center; margin-top: 30px; color: #6c757d; font-size: 0.9em;">
        <p>
            Download data: <a href="../coverage-data.csv" download style="color: #3498db;">CSV</a> |
            <a href="../coverage-data.json" download style="color: #3498db;">JSON</a>
        </p>
    </div>
</body>
</html>
EOF

echo "Generated historical index with ${report_count} reports"
