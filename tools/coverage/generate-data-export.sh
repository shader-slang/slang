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
date,commit,platform,line_coverage,lines_hit,lines_found,region_coverage,regions_hit,regions_found,function_coverage,functions_hit,functions_found,branch_coverage,branches_hit,branches_found,slangc_line_coverage,slangc_lines_hit,slangc_lines_found,slangc_region_coverage,slangc_regions_hit,slangc_regions_found,slangc_function_coverage,slangc_functions_hit,slangc_functions_found,slangc_branch_coverage,slangc_branches_hit,slangc_branches_found
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

        # slangc compiler-only metrics (may not exist in older reports)
        sl_line_cov=$(get_json_value "slangc_line_coverage" "$platform_summary")
        sl_lines_hit=$(get_json_number "slangc_lines_hit" "$platform_summary")
        sl_lines_found=$(get_json_number "slangc_lines_found" "$platform_summary")
        sl_region_cov=$(get_json_value "slangc_region_coverage" "$platform_summary")
        sl_regions_hit=$(get_json_number "slangc_regions_hit" "$platform_summary")
        sl_regions_found=$(get_json_number "slangc_regions_found" "$platform_summary")
        sl_function_cov=$(get_json_value "slangc_function_coverage" "$platform_summary")
        sl_functions_hit=$(get_json_number "slangc_functions_hit" "$platform_summary")
        sl_functions_found=$(get_json_number "slangc_functions_found" "$platform_summary")
        sl_branch_cov=$(get_json_value "slangc_branch_coverage" "$platform_summary")
        sl_branches_hit=$(get_json_number "slangc_branches_hit" "$platform_summary")
        sl_branches_found=$(get_json_number "slangc_branches_found" "$platform_summary")

        # Add to CSV
        echo "${date_part},${commit_part},${platform},${line_cov},${lines_hit},${lines_found},${region_cov},${regions_hit},${regions_found},${function_cov},${functions_hit},${functions_found},${branch_cov},${branches_hit},${branches_found},${sl_line_cov},${sl_lines_hit},${sl_lines_found},${sl_region_cov},${sl_regions_hit},${sl_regions_found},${sl_function_cov},${sl_functions_hit},${sl_functions_found},${sl_branch_cov},${sl_branches_hit},${sl_branches_found}" >>"$CSV_FILE"

        # Add to JSON (include slangc fields if present)
        if [[ "$first_entry" == "false" ]]; then
          echo "," >>"$JSON_FILE"
        fi
        first_entry=false

        SLANGC_JSON_FIELDS=""
        if [[ -n "$sl_line_cov" ]]; then
          SLANGC_JSON_FIELDS=$(cat <<SLANGC_EOF
    "slangc_line_coverage": ${sl_line_cov},
    "slangc_lines_hit": ${sl_lines_hit},
    "slangc_lines_found": ${sl_lines_found},
    "slangc_region_coverage": ${sl_region_cov},
    "slangc_regions_hit": ${sl_regions_hit},
    "slangc_regions_found": ${sl_regions_found},
    "slangc_function_coverage": ${sl_function_cov},
    "slangc_functions_hit": ${sl_functions_hit},
    "slangc_functions_found": ${sl_functions_found},
    "slangc_branch_coverage": ${sl_branch_cov},
    "slangc_branches_hit": ${sl_branches_hit},
    "slangc_branches_found": ${sl_branches_found},
SLANGC_EOF
          )
        fi

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
    "branches_found": ${branches_found},
    ${SLANGC_JSON_FIELDS}
    "_version": 2
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

    # Add to CSV (assume linux for old reports, empty slangc fields)
    echo "${date_part},${commit_part},linux,${line_cov},${lines_hit},${lines_found},${region_cov},${regions_hit},${regions_found},${function_cov},${functions_hit},${functions_found},${branch_cov},${branches_hit},${branches_found},,,,,,,,,,,," >>"$CSV_FILE"

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
