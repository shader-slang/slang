#!/usr/bin/env bash
#
# Localize the bundled SPIRV-Tools/glslang strong global symbols in the partially
# linked slang-glslang object so they cannot collide with — or be overridden by — a
# consumer application that links its own copy of SPIRV-Tools/glslang. This is the
# symbol-localization step of the SLANG_LIB_TYPE=STATIC slang-glslang build; see
# source/slang-glslang/CMakeLists.txt.
#
# Only *strong* global symbols are localized: those are the ODR-collision risk (two
# strong definitions at final link = a "multiple definition" error). Weak/COMDAT
# symbols (C++ template instantiations, inline methods, std:: helpers) are left
# untouched so the final linker's normal COMDAT de-duplication keeps working;
# localizing them would leave dangling references into discarded COMDAT sections
# ("referenced in section ...: defined in discarded section ..."). The named entry
# points are kept global so slang can bind them in-process.
#
# Usage: localize-static-symbols.sh <nm> <objcopy> <in.o> <out.o> <keep-global>...
set -euo pipefail

if [ "$#" -lt 5 ]; then
  echo "usage: $0 <nm> <objcopy> <in.o> <out.o> <keep-global>..." >&2
  exit 2
fi

nm_tool=$1
objcopy_tool=$2
in_obj=$3
out_obj=$4
shift 4

keep_file=$(mktemp)
localize_file=$(mktemp)
trap 'rm -f "$keep_file" "$localize_file"' EXIT

printf '%s\n' "$@" >"$keep_file"

# `nm -g --defined-only` lists external (global + weak) defined symbols as
# "<value> <type> <name>". Strong globals have an uppercase, non-weak type
# (weak symbol types are W/V, and lowercase w/v). Keep the entry points global and
# localize the remaining strong globals.
"$nm_tool" -g --defined-only "$in_obj" |
  awk '$2 != "W" && $2 != "V" && $2 != "w" && $2 != "v" { print $3 }' |
  grep -vxF -f "$keep_file" >"$localize_file" || true

if [ -s "$localize_file" ]; then
  "$objcopy_tool" --localize-symbols="$localize_file" "$in_obj" "$out_obj"
else
  cp "$in_obj" "$out_obj"
fi
