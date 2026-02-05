#!/usr/bin/env bash

# Rebuilds SPIRV-Tools and SPIRV-Headers.
#
# This script does the following steps in `docs/update_spirv.md`:
# - Build spirv-tools
# - Update SPIRV-Headers
# - Copy the generated files from spirv-tools/build/ to spirv-tools-generated/

set -euo pipefail

cd "$(dirname $0)"/..

echo ">>>"
echo ">>> Building SPIRV-Tools"
echo ">>>"

cd external/spirv-tools
utils/git-sync-deps
rm -rf build
cmake . -B build
cmake --build build --config Release -j
cd ../..

SPIRV_HEADERS_COMMIT="$(git -C external/spirv-tools/external/spirv-headers rev-parse HEAD)"

echo ">>>"
echo ">>> Updating SPIRV-Headers to commit ${SPIRV_HEADERS_COMMIT}"
echo ">>>"

git -C external/spirv-headers fetch
git -C external/spirv-headers checkout "$SPIRV_HEADERS_COMMIT"

echo ">>>"
echo ">>> Copying the generated .h/.inc files from spirv-tools/build/ to spirv-tools-generated/"
echo ">>>"

rm -f external/spirv-tools-generated/*.h
rm -f external/spirv-tools-generated/*.inc
cp external/spirv-tools/build/*.h external/spirv-tools-generated/
cp external/spirv-tools/build/*.inc external/spirv-tools-generated/

echo ">>>"
echo ">>> Done"
echo ">>>"

