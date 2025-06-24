#!/usr/bin/env bash

set -euo pipefail

# File paths
DOCS_FILE="docs/building.md"
CMAKE_CACHE="build/CMakeCache.txt"

# Check if required files exist
if [[ ! -f "$DOCS_FILE" ]]; then
  echo "::warning::Documentation file $DOCS_FILE not found"
  exit 0
fi

if [[ ! -f "$CMAKE_CACHE" ]]; then
  echo "::warning::CMakeCache.txt not found at $CMAKE_CACHE"
  exit 0
fi

# Extract compiler path from CMakeCache.txt
COMPILER_PATH=$(grep "^CMAKE_CXX_COMPILER:FILEPATH=" "$CMAKE_CACHE" | cut -d'=' -f2)

if [[ -z "$COMPILER_PATH" ]]; then
  echo "::warning::Could not find CMAKE_CXX_COMPILER in CMakeCache.txt"
  exit 0
fi

# Determine compiler type and get version
COMPILER_NAME=$(basename "$COMPILER_PATH")
COMPILER_VERSION=""
COMPILER_TYPE=""

if [[ "$COMPILER_NAME" =~ ^g\+\+|^gcc ]]; then
  COMPILER_TYPE="GCC"
  # Get GCC version (e.g., "gcc (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0")
  COMPILER_VERSION=$("$COMPILER_PATH" --version | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)
elif [[ "$COMPILER_NAME" =~ ^clang\+\+|^clang ]]; then
  COMPILER_TYPE="Clang"
  # Get Clang version (e.g., "Ubuntu clang version 15.0.7")
  COMPILER_VERSION=$("$COMPILER_PATH" --version | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)
elif [[ "$COMPILER_NAME" =~ ^cl\.exe|^cl$ ]]; then
  COMPILER_TYPE="MSVC"
  # MSVC version is more complex, extract major version
  # MSVC outputs version like "19.29.30133" where 19 is the major version
  COMPILER_VERSION=$("$COMPILER_PATH" 2>&1 | grep -oE 'Version [0-9]+\.[0-9]+' | grep -oE '[0-9]+\.[0-9]+' | head -1)
else
  echo "::warning::Unknown compiler type: $COMPILER_NAME"
  exit 0
fi

if [[ -z "$COMPILER_VERSION" ]]; then
  echo "::warning::Could not determine version for compiler: $COMPILER_PATH"
  exit 0
fi

# Extract major.minor version only
COMPILER_MAJOR_MINOR=$(echo "$COMPILER_VERSION" | cut -d'.' -f1,2)

# Parse the documentation for the expected version
# Look for pattern: _COMPILER_ xx.yy is tested in CI
EXPECTED_VERSION=""

case "$COMPILER_TYPE" in
"GCC")
  EXPECTED_VERSION=$(grep -E "^_GCC_ [0-9]+\.[0-9]+ is tested in CI" "$DOCS_FILE" | grep -oE '[0-9]+\.[0-9]+' | head -1)
  ;;
"Clang")
  EXPECTED_VERSION=$(grep -E "^_Clang_ [0-9]+\.[0-9]+ is tested in CI" "$DOCS_FILE" | grep -oE '[0-9]+\.[0-9]+' | head -1)
  ;;
"MSVC")
  # For MSVC, we only compare major version
  EXPECTED_VERSION=$(grep -E "^_MSVC_ [0-9]+ is tested in CI" "$DOCS_FILE" | grep -oE '[0-9]+' | head -1)
  COMPILER_MAJOR_MINOR=$(echo "$COMPILER_VERSION" | cut -d'.' -f1)
  ;;
esac

if [[ -z "$EXPECTED_VERSION" ]]; then
  echo "::warning::Could not find expected version for $COMPILER_TYPE in $DOCS_FILE"
  exit 0
fi

# Compare versions
if [[ "$COMPILER_MAJOR_MINOR" != "$EXPECTED_VERSION" ]]; then
  echo "::warning::Compiler version mismatch for $COMPILER_TYPE: $DOCS_FILE says $EXPECTED_VERSION but found $COMPILER_MAJOR_MINOR (full version: $COMPILER_VERSION)"
else
  echo "✓ Compiler version matches: $COMPILER_TYPE $COMPILER_MAJOR_MINOR"
fi

exit 0
