#!/usr/bin/env bash
# run-tutorial.sh — execute every step of the Shader Execution Coverage
# tutorial (docs/user-guide/a1-06-shader-coverage.md) in one go, from
# this directory. Each step below is named after the chapter section it
# comes from, so you can follow along in the text.
#
# Prerequisites: slangc (any Slang release, or a repo build), a C++
# compiler, and Python 3. genhtml (from the lcov package) is optional —
# the HTML step is skipped with a note when it is missing.
#
# Usage:
#   ./run-tutorial.sh
#   SLANGC=/path/to/slangc ./run-tutorial.sh   # explicit compiler

set -euo pipefail
cd "$(dirname "$0")"

# --- Step 0: find the tools -------------------------------------------------
# slangc is taken from $SLANGC, then PATH, then a sibling repo build
# (convenient when running from a shader-slang/slang checkout). An old
# slangc without coverage support is skipped: releases that predate the
# feature reject -trace-coverage.
supports_coverage() {
  local help_text
  help_text=$("$1" -h 2>&1) || true
  [[ $help_text == *trace-coverage* ]]
}

if [[ -n "${SLANGC:-}" ]]; then
  supports_coverage "$SLANGC" || {
    echo "error: $SLANGC does not support -trace-coverage; use a newer Slang release" >&2
    exit 1
  }
else
  for candidate in slangc ../../build/Release/bin/slangc ../../build/Debug/bin/slangc; do
    command -v "$candidate" >/dev/null || continue
    supports_coverage "$candidate" && SLANGC=$candidate && break
  done
fi
[[ -n "${SLANGC:-}" ]] || {
  echo "error: no slangc with -trace-coverage support found on PATH or in ../../build;" >&2
  echo "install a recent Slang release or set SLANGC" >&2
  exit 1
}
echo "using slangc: $SLANGC"

# --- Step 1: "Compiling with coverage" ------------------------------------
# One flag, -trace-coverage, turns on line coverage. Two files appear:
# the compiled shader and the .coverage-manifest.json sidecar that maps
# counter slots back to source locations.
"$SLANGC" hello-coverage.slang -target spirv -stage compute -entry computeMain \
  -trace-coverage -o hello-coverage.spv
echo "wrote hello-coverage.spv and hello-coverage.spv.coverage-manifest.json"

# --- Step 2: "Reading the manifest" -----------------------------------------
# Pretty-print the sidecar. Note the buffer block: on SPIR-V the hidden
# counter buffer binds at a descriptor (set, binding).
python3 -m json.tool hello-coverage.spv.coverage-manifest.json | sed -n '1,14p'

# --- Step 3: "Dispatching the precompiled kernel" ---------
# Compile the same shader once more, to a directly callable CPU shared
# library. slangc drives the system C++ compiler; the new sidecar
# reports uniform_offset instead of a descriptor location.
"$SLANGC" hello-coverage.slang -target shader-sharedlib -stage compute -entry computeMain \
  -trace-coverage -o hello-coverage-kernel.so
echo "wrote hello-coverage-kernel.so and its sidecar manifest"

# Build the host program — an ordinary C++ compile with no Slang SDK
# paths — then dispatch. It loads the precompiled kernel, binds the
# coverage buffer at the manifest-reported uniform_offset, runs one
# thread group, prints the raw counter slots, and writes
# hello-coverage.counters.bin.
c++ -std=c++17 hello-coverage-host.cpp -o hello-coverage-host -ldl
./hello-coverage-host

# --- Step 4: "Generating a report" ------------------------------------
# The LCOV converter joins the raw counters with the manifest's source
# attribution. Expect two zero-count lines: the negative-input clamp
# and the applyGain fallthrough, which these inputs never reach.
python3 ../../tools/shader-coverage/slang-coverage-to-lcov.py \
  --manifest hello-coverage-kernel.so.coverage-manifest.json \
  --counters hello-coverage.counters.bin --output hello-coverage.lcov
cat hello-coverage.lcov

# Render HTML when genhtml (lcov package) is installed.
if command -v genhtml >/dev/null; then
  genhtml hello-coverage.lcov --output-directory coverage-html >/dev/null
  echo "open coverage-html/index.html to see the annotated source"
else
  echo "genhtml not found - skipping HTML report (install the lcov package, or open hello-coverage.lcov in an LCOV viewer such as VS Code Coverage Gutters)"
fi
