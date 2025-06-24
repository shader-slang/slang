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

# Function to extract versions from a line
extract_versions() {
  local line="$1"
  local is_msvc="$2"

  # Extract the part between compiler name and "are/is tested in CI"
  local version_part
  version_part=$(echo "$line" | sed -E 's/^_[^_]+_ //; s/ (are|is) tested in CI.*//')

  # Split by "and" and extract version numbers
  local versions=()
  if [[ "$is_msvc" == "true" ]]; then
    # For MSVC, just extract numbers
    while IFS= read -r version; do
      version=$(echo "$version" | grep -oE '[0-9]+' | head -1)
      [[ -n "$version" ]] && versions+=("$version")
    done < <(echo "$version_part" | tr ',' '\n' | sed 's/ and /\n/g')
  else
    # For GCC/Clang, extract major.minor versions
    while IFS= read -r version; do
      version=$(echo "$version" | grep -oE '[0-9]+\.[0-9]+' | head -1)
      [[ -n "$version" ]] && versions+=("$version")
    done < <(echo "$version_part" | tr ',' '\n' | sed 's/ and /\n/g')
  fi

  echo "${versions[@]}"
}

# Parse the documentation for the expected versions
EXPECTED_VERSIONS=()
VERSION_FOUND=false

case "$COMPILER_TYPE" in
"GCC")
  DOC_LINE=$(grep -E "^_GCC_ .+ (are|is) tested in CI" "$DOCS_FILE" | head -1)
  if [[ -n "$DOC_LINE" ]]; then
    # shellcheck disable=SC2207
    EXPECTED_VERSIONS=($(extract_versions "$DOC_LINE" "false"))
  fi
  ;;
"Clang")
  DOC_LINE=$(grep -E "^_Clang_ .+ (are|is) tested in CI" "$DOCS_FILE" | head -1)
  if [[ -n "$DOC_LINE" ]]; then
    # shellcheck disable=SC2207
    EXPECTED_VERSIONS=($(extract_versions "$DOC_LINE" "false"))
  fi
  ;;
"MSVC")
  # For MSVC, we only compare major version
  DOC_LINE=$(grep -E "^_MSVC_ .+ (are|is) tested in CI" "$DOCS_FILE" | head -1)
  if [[ -n "$DOC_LINE" ]]; then
    # shellcheck disable=SC2207
    EXPECTED_VERSIONS=($(extract_versions "$DOC_LINE" "true"))
  fi
  COMPILER_MAJOR_MINOR=$(echo "$COMPILER_VERSION" | cut -d'.' -f1)
  ;;
esac

if [[ ${#EXPECTED_VERSIONS[@]} -eq 0 ]]; then
  echo "::warning::Could not find expected version for $COMPILER_TYPE in $DOCS_FILE"
  exit 0
fi

# Check if current version matches any expected version
for expected in "${EXPECTED_VERSIONS[@]}"; do
  if [[ "$COMPILER_MAJOR_MINOR" == "$expected" ]]; then
    VERSION_FOUND=true
    break
  fi
done

# Report results
if [[ "$VERSION_FOUND" == "false" ]]; then
  EXPECTED_LIST=$(
    IFS=', '
    echo "${EXPECTED_VERSIONS[*]}"
  )
  echo "::warning::Compiler version mismatch for $COMPILER_TYPE: Documentation says $EXPECTED_LIST but found $COMPILER_MAJOR_MINOR (full version: $COMPILER_VERSION)"
else
  echo "✓ Compiler version matches: $COMPILER_TYPE $COMPILER_MAJOR_MINOR"
fi

exit 0
