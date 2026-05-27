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
require_reference tests/spirv/spirv-opt-merge-return-groupshared-threadlocal.slang

python3 - <<'PY'
import json
import re
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

with open("source/slang-glslang/slang-glslang.cpp", encoding="utf-8") as file:
    source = file.read()

guarded_pair_pattern = re.compile(
    r"#if\s+SLANG_ENABLE_SPIRV_OPT_MERGE_RETURN\s*\n"
    r"\s*optimizer\.RegisterPass\(spvtools::CreateMergeReturnPass\(\)\);\s*\n"
    r"\s*optimizer\.RegisterPass\(spvtools::CreateInlineExhaustivePass\(\)\);\s*\n"
    r"\s*#endif",
    re.MULTILINE,
)
guarded_pairs = guarded_pair_pattern.findall(source)
merge_passes = source.count("spvtools::CreateMergeReturnPass()")
inline_passes = source.count("spvtools::CreateInlineExhaustivePass()")
if len(guarded_pairs) != 4 or merge_passes != 4 or inline_passes != 4:
    print(
        f"{option} must guard exactly four MergeReturnPass + InlineExhaustivePass pairs; "
        f"got {len(guarded_pairs)} guarded pairs, {merge_passes} merge passes, "
        f"{inline_passes} inline passes"
    )
    sys.exit(1)

test_path = "tests/spirv/spirv-opt-merge-return-groupshared-threadlocal.slang"
with open(test_path, encoding="utf-8") as file:
    test_source = file.read()
for flag in ("-emit-spirv-via-glsl", "-emit-spirv-directly"):
    if flag not in test_source:
        print(f"Missing {flag} compile-smoke coverage in {test_path}")
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
