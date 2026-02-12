#!/bin/bash
# Manual test script for SARIF diagnostic format support
# SARIF is supported in GCC 13+ and Clang 15+, but currently disabled by default
# because linkers (ld, lld) don't produce SARIF output, causing link errors to be lost.

set -e

echo "=== SARIF Support Manual Tests ==="
echo ""

# Check compiler version
echo "=== Compiler Version ==="
g++ --version | head -1 || true
clang++ --version | head -1 || true
echo ""

# Test 1: Force SARIF on (requires GCC 13+ or Clang 15+)
echo "=== Test 1: Force SARIF enabled (SLANG_USE_SARIF_DIAGNOSTICS=1) ==="
echo "Expected: Should use SARIF parsing if compiler supports it"
export SLANG_USE_SARIF_DIAGNOSTICS=1
export SLANG_GCC_PARSER_DEBUG=1
./build/Debug/bin/slang-test tests/cpp-compiler/c-compile-error.c 2>&1 | grep -E "SARIF|JSON length|Extracted"
echo ""

# Test 2: Force SARIF off (uses text parsing)
echo "=== Test 2: Force SARIF disabled (SLANG_USE_SARIF_DIAGNOSTICS=0) ==="
echo "Expected: Should use text parsing"
export SLANG_USE_SARIF_DIAGNOSTICS=0
export SLANG_GCC_PARSER_DEBUG=1
./build/Debug/bin/slang-test tests/cpp-compiler/c-compile-error.c 2>&1 | grep -E "SARIF|Text parsing path"
echo ""

# Test 3: Auto-detect (currently disabled by default)
echo "=== Test 3: Auto-detect (unset SLANG_USE_SARIF_DIAGNOSTICS) ==="
echo "Expected: Should use text parsing (SARIF disabled by default due to linker limitations)"
unset SLANG_USE_SARIF_DIAGNOSTICS
export SLANG_GCC_PARSER_DEBUG=1
./build/Debug/bin/slang-test tests/cpp-compiler/c-compile-error.c 2>&1 | grep -E "SARIF|Text parsing path"
echo ""

# Test 4: Link errors with SARIF (shows the limitation)
echo "=== Test 4: Link errors with SARIF enabled ==="
echo "Expected: SARIF parsing succeeds but extracts 0 diagnostics (linker doesn't produce SARIF)"
export SLANG_USE_SARIF_DIAGNOSTICS=1
export SLANG_GCC_PARSER_DEBUG=1
./build/Debug/bin/slang-test tests/cpp-compiler/c-compile-link-error.c 2>&1 | grep -E "SARIF|Extracted.*diagnostics"
echo ""

echo "=== All manual tests complete ==="
echo "Note: SARIF is currently disabled by default because ld/lld don't produce SARIF,"
echo "      causing link errors to be lost. Use SLANG_USE_SARIF_DIAGNOSTICS=1 to force enable."
