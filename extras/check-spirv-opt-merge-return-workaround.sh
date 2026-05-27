#!/usr/bin/env bash
set -euo pipefail

option="SLANG_ENABLE_SPIRV_OPT_MERGE_RETURN"

require_reference() {
  local path="$1"
  if ! grep -q "$option" "$path"; then
    echo "Missing $option reference in $path"
    exit 1
  fi
}

require_reference CMakeLists.txt
require_reference source/slang-glslang/CMakeLists.txt
require_reference source/slang-glslang/slang-glslang.cpp
require_reference docs/building.md
require_reference .github/cmake-options-matrix.json

python3 - <<'PY'
import json
import sys

option = "SLANG_ENABLE_SPIRV_OPT_MERGE_RETURN"
with open(".github/cmake-options-matrix.json", encoding="utf-8") as file:
    matrix = json.load(file)

values = {
    entry.get("value")
    for entry in matrix.get("include", [])
    if entry.get("option") == option
}
if values != {"ON", "OFF"}:
    print(f"{option} must have explicit ON and OFF cmake-options matrix entries; got {sorted(values)}")
    sys.exit(1)
PY

override_files=$(
  grep -rl -- "-D${option}=OFF" .github/workflows |
    sort ||
    true
)

if [ -z "$override_files" ]; then
  echo "No PR CI workflow overrides found for $option"
  exit 1
fi

while IFS= read -r path; do
  if ! grep -q "TODO(#11146)" "$path"; then
    echo "Missing TODO(#11146) near $option override in $path"
    exit 1
  fi
done <<<"$override_files"

echo "$option workaround references are consistent."
