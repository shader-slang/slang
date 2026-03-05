#!/bin/bash

echo "[$(date)] Version checking ..."
for requiredUtil in cmake ninja node tsc
do
	# When the version of Node is too old, tsc may fail.
	# We want to stop when it is the case.
	if ! $requiredUtil --version
	then
		echo "Error: a required tool is not found: $requiredUtil"
		exit 1
	fi
done

# Setup pinned emsdk version using shared script
source ./build_scripts/setup-emsdk.sh

pushd emsdk
	sed -i 's/\r$//' emsdk emsdk_env.sh
	/bin/sh ./emsdk install latest
	/bin/sh ./emsdk activate latest
	source ./emsdk_env.sh
popd

echo "[$(date)] Sync slang repo ..."
if [ ! -d slang-repo ]
then
	git clone https://github.com/shader-slang/slang.git slang-repo
fi

pushd slang-repo
git submodule update --init --recursive

# The `,` syntax in sed specifies a range from the line that starts
# with `target_link_options(` to the line that has just a `)` (possibly indented).
#
# /start/,/end/c\ ... replaces the block.
#
sed -i '/^[[:space:]]*target_link_options(/,/^[[:space:]]*)/c\
    target_link_options(slang-wasm PUBLIC\
        \"--bind\"\
        --emit-tsd \"$<TARGET_FILE_DIR:slang-wasm>/slang-wasm.d.ts\"\
        -sMODULARIZE=1\
        -sEXPORT_ES6=0\
    	-sSINGLE_FILE=1\
  		-sENVIRONMENT=worker\
        -sEXPORTED_RUNTIME_METHODS=['FS']\
    )' "source/slang-wasm/CMakeLists.txt"

if ! grep -q slang-wasm.d.ts "source/slang-wasm/CMakeLists.txt"
then
	echo "Error: failed to override CMake option in source/slang-wasm/CMakeLists.txt"
	exit 1
fi

echo "[$(date)] Setup generator configuration ..."
if ! cmake --workflow --preset generators --fresh
then
	echo "Error: CMake configuration failed."
	exit 1
fi

echo "[$(date)] Build generators ..."
mkdir generators
if ! cmake --install build --prefix generators --component generators
then
	echo "Error: CMake build for generators failed."
	exit 1
fi

echo "[$(date)] Setup slang-wasm configuration ..."
if ! emcmake cmake -DSLANG_GENERATORS_PATH=generators/bin --preset emscripten -DSLANG_ENABLE_RELEASE_LTO=OFF
then
	echo "Error: emcmake failed."
	exit 1
fi

cp build/CMakeCache.txt ../../CMakeCache.txt
cp build.em/CMakeCache.txt ../../CMakeCache.em.txt

# --- Build for Web Worker (default) ---
echo "[$(date)] Build slang as wasm (web worker) ..."
if ! cmake --build --preset emscripten --target slang-wasm
then
    echo "Build with CMake failed (web worker)."
    exit 1
fi

popd

# Copy and rename output for web worker
cp slang-repo/build.em/Release/bin/slang-wasm.js slang-wasm.worker.js
cp slang-repo/build.em/Release/bin/slang-wasm.d.ts slang-wasm.worker.d.ts
# Also keep the default (web worker) as slang-wasm.js/d.ts for backward compatibility
cp slang-wasm.worker.js slang-wasm.js
cp slang-wasm.worker.d.ts slang-wasm.d.ts

# --- Build for Node.js ---
pushd slang-repo
sed -i '/^[[:space:]]*target_link_options(/,/^[[:space:]]*)/c\
    target_link_options(slang-wasm PUBLIC\
        "--bind"\
        --emit-tsd "$<TARGET_FILE_DIR:slang-wasm>/slang-wasm.d.ts"\
        -sMODULARIZE=1\
        -sEXPORT_ES6=0\
        -sSINGLE_FILE=1\
        -sENVIRONMENT=node\
        -sEXPORTED_RUNTIME_METHODS=['FS']\
    )' "source/slang-wasm/CMakeLists.txt"

if ! emcmake cmake -DSLANG_GENERATORS_PATH=generators/bin --preset emscripten -DSLANG_ENABLE_RELEASE_LTO=OFF
then
    echo "Error: emcmake failed (node)."
    exit 1
fi

if ! cmake --build --preset emscripten --target slang-wasm
then
    echo "Build with CMake failed (node)."
    exit 1
fi
popd

# Copy and rename output for node
cp slang-repo/build.em/Release/bin/slang-wasm.js slang-wasm.node.js
cp slang-repo/build.em/Release/bin/slang-wasm.d.ts slang-wasm.node.d.ts

git -C ./slang-repo rev-parse HEAD > key.txt
echo "key: $(cat key.txt)"

