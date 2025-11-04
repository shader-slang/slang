#!/usr/bin/env bash
# Generate CSV and JSON exports of historical coverage data

set -e

HISTORY_DIR="$1"

if [[ -z "$HISTORY_DIR" ]]; then
  echo "Usage: $0 <history-directory>"
  exit 1
fi

OUTPUT_DIR="$(dirname "$HISTORY_DIR")"
CSV_FILE="${OUTPUT_DIR}/coverage-data.csv"
JSON_FILE="${OUTPUT_DIR}/coverage-data.json"

# Initialize CSV with header
cat >"$CSV_FILE" <<'EOF'
date,commit,platform,line_coverage,lines_hit,lines_found,region_coverage,regions_hit,regions_found,function_coverage,functions_hit,functions_found,branch_coverage,branches_hit,branches_found
EOF

# Initialize JSON array
echo "[" >"$JSON_FILE"
first_entry=true

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

# Process each historical report (iterate in reverse chronological order)
# Use find to safely handle any number of directories
while IFS= read -r report_path; do
  dir=$(basename "$report_path")
  date_part=$(echo "$dir" | cut -d'-' -f1-3)
  commit_part=$(echo "$dir" | cut -d'-' -f4)

  report_dir="${HISTORY_DIR}/${dir}"
  combined_summary="${report_dir}/combined-summary.json"
  old_summary="${report_dir}/coverage-summary.json"

  # Check if this is a multi-platform report or old single-platform
  if [[ -f "$combined_summary" ]]; then
    # Multi-platform report - process each platform
    for platform in linux macos windows; do
      platform_summary="${report_dir}/${platform}/coverage-summary.json"

      if [[ -f "$platform_summary" ]]; then
        # Extract all metrics
        line_cov=$(get_json_value "line_coverage" "$platform_summary")
        lines_hit=$(get_json_number "lines_hit" "$platform_summary")
        lines_found=$(get_json_number "lines_found" "$platform_summary")

        region_cov=$(get_json_value "region_coverage" "$platform_summary")
        regions_hit=$(get_json_number "regions_hit" "$platform_summary")
        regions_found=$(get_json_number "regions_found" "$platform_summary")

        function_cov=$(get_json_value "function_coverage" "$platform_summary")
        functions_hit=$(get_json_number "functions_hit" "$platform_summary")
        functions_found=$(get_json_number "functions_found" "$platform_summary")

        branch_cov=$(get_json_value "branch_coverage" "$platform_summary")
        branches_hit=$(get_json_number "branches_hit" "$platform_summary")
        branches_found=$(get_json_number "branches_found" "$platform_summary")

        # Add to CSV
        echo "${date_part},${commit_part},${platform},${line_cov},${lines_hit},${lines_found},${region_cov},${regions_hit},${regions_found},${function_cov},${functions_hit},${functions_found},${branch_cov},${branches_hit},${branches_found}" >>"$CSV_FILE"

        # Add to JSON
        if [[ "$first_entry" == "false" ]]; then
          echo "," >>"$JSON_FILE"
        fi
        first_entry=false

        cat >>"$JSON_FILE" <<EOF
  {
    "date": "${date_part}",
    "commit": "${commit_part}",
    "platform": "${platform}",
    "line_coverage": ${line_cov},
    "lines_hit": ${lines_hit},
    "lines_found": ${lines_found},
    "region_coverage": ${region_cov},
    "regions_hit": ${regions_hit},
    "regions_found": ${regions_found},
    "function_coverage": ${function_cov},
    "functions_hit": ${functions_hit},
    "functions_found": ${functions_found},
    "branch_coverage": ${branch_cov},
    "branches_hit": ${branches_hit},
    "branches_found": ${branches_found}
  }
EOF
      fi
    done
  elif [[ -f "$old_summary" ]]; then
    # Old single-platform report (backwards compatibility)
    line_cov=$(get_json_value "line_coverage" "$old_summary")
    lines_hit=$(get_json_number "lines_hit" "$old_summary")
    lines_found=$(get_json_number "lines_found" "$old_summary")

    region_cov=$(get_json_value "region_coverage" "$old_summary")
    regions_hit=$(get_json_number "regions_hit" "$old_summary")
    regions_found=$(get_json_number "regions_found" "$old_summary")

    function_cov=$(get_json_value "function_coverage" "$old_summary")
    functions_hit=$(get_json_number "functions_hit" "$old_summary")
    functions_found=$(get_json_number "functions_found" "$old_summary")

    branch_cov=$(get_json_value "branch_coverage" "$old_summary")
    branches_hit=$(get_json_number "branches_hit" "$old_summary")
    branches_found=$(get_json_number "branches_found" "$old_summary")

    # Add to CSV (assume linux for old reports)
    echo "${date_part},${commit_part},linux,${line_cov},${lines_hit},${lines_found},${region_cov},${regions_hit},${regions_found},${function_cov},${functions_hit},${functions_found},${branch_cov},${branches_hit},${branches_found}" >>"$CSV_FILE"

    # Add to JSON
    if [[ "$first_entry" == "false" ]]; then
      echo "," >>"$JSON_FILE"
    fi
    first_entry=false

    cat >>"$JSON_FILE" <<EOF
  {
    "date": "${date_part}",
    "commit": "${commit_part}",
    "platform": "linux",
    "line_coverage": ${line_cov},
    "lines_hit": ${lines_hit},
    "lines_found": ${lines_found},
    "region_coverage": ${region_cov},
    "regions_hit": ${regions_hit},
    "regions_found": ${regions_found},
    "function_coverage": ${function_cov},
    "functions_hit": ${functions_hit},
    "functions_found": ${functions_found},
    "branch_coverage": ${branch_cov},
    "branches_hit": ${branches_hit},
    "branches_found": ${branches_found}
  }
EOF
  fi
done < <(find "${HISTORY_DIR}" -maxdepth 1 -type d -name '[0-9]*-[0-9]*-[0-9]*-*' | sort -r)

# Close JSON array
echo "" >>"$JSON_FILE"
echo "]" >>"$JSON_FILE"

echo "Generated data exports:"
echo "  CSV:  $CSV_FILE"
echo "  JSON: $JSON_FILE"
echo ""
echo "Total data points: $(tail -n +2 "$CSV_FILE" | wc -l | tr -d ' ')"
