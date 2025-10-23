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

    <table>
        <tr><th>Date</th><th>Commit</th><th>Report</th></tr>
EOF

# List all reports in reverse chronological order
for dir in $(ls -r "${HISTORY_DIR}/" | grep -E '^[0-9]{4}-[0-9]{2}-[0-9]{2}-[a-f0-9]+$'); do
  date_part=$(echo $dir | cut -d'-' -f1-3)
  commit_part=$(echo $dir | cut -d'-' -f4)
  echo "        <tr><td>$date_part</td><td><a href=\"https://github.com/shader-slang/slang/commit/$commit_part\">$commit_part</a></td><td><a href=\"$dir/index.html\">View Report</a></td></tr>" >>"${HISTORY_DIR}/index.html"
done

# Generate HTML footer
cat >>"${HISTORY_DIR}/index.html" <<'EOF'
    </table>
</body>
</html>
EOF

echo "Generated historical index with $(ls -1 "${HISTORY_DIR}/" | grep -cE '^[0-9]{4}-[0-9]{2}-[0-9]{2}-[a-f0-9]+$') reports"
