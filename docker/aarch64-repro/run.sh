#!/bin/bash
#
# Reproduce aarch64 CI test failures locally on an x86_64 machine.
#
# Prerequisites (one-time):
#   docker run --rm --privileged tonistiigi/binfmt --install arm64
#
# Usage:
#   ./docker/aarch64-repro/run.sh [build|test|shell|all]
#
#   build  - Build Slang for aarch64 inside the container
#   test   - Run the failing unit tests (requires prior build)
#   shell  - Drop into an interactive shell in the container
#   all    - Build and then test (default)
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
IMAGE_NAME="slang-aarch64-repro"
CONTAINER_NAME="slang-aarch64-repro"
BUILD_CONFIG="${BUILD_CONFIG:-release}"

cd "$REPO_ROOT"

build_image() {
    echo "=== Building Docker image (arm64) ==="
    docker build --platform linux/arm64 \
        -t "$IMAGE_NAME" \
        -f docker/aarch64-repro/Dockerfile \
        docker/aarch64-repro/
}

docker_run() {
    docker run --rm \
        --platform linux/arm64 \
        --name "$CONTAINER_NAME" \
        -v "$REPO_ROOT:/workspace:rw" \
        -w /workspace \
        -e CC=gcc \
        -e CXX=g++ \
        -e BUILD_CONFIG="$BUILD_CONFIG" \
        "$IMAGE_NAME" \
        "$@"
}

do_build() {
    echo "=== Building Slang for aarch64 (config=$BUILD_CONFIG) ==="
    echo "    (This will be slow under QEMU emulation -- expect 30-60+ min)"

    local cmake_config
    if [[ "$BUILD_CONFIG" == "release" ]]; then
        cmake_config="Release"
    else
        cmake_config="Debug"
    fi

    docker_run bash -c "
        set -e
        echo 'Architecture: \$(uname -m)'
        echo 'GCC version: \$(gcc -dumpversion)'
        echo 'CMake version: \$(cmake --version | head -1)'

        cmake --preset default --fresh \
            -DSLANG_SLANG_LLVM_FLAVOR=DISABLE \
            -DCMAKE_COMPILE_WARNING_AS_ERROR=false

        cmake --build --preset $BUILD_CONFIG
    "
}

do_test() {
    echo "=== Running failing unit tests on aarch64 ==="

    local cmake_config
    if [[ "$BUILD_CONFIG" == "release" ]]; then
        cmake_config="Release"
    else
        cmake_config="Debug"
    fi

    docker_run bash -c "
        set -e
        echo 'Architecture: \$(uname -m)'

        bin_dir=/workspace/build/$cmake_config/bin
        export LD_LIBRARY_PATH=/workspace/build/$cmake_config/lib:\${LD_LIBRARY_PATH:-}

        echo ''
        echo '=== Running specific failing tests ==='
        failing_tests=(
            'slang-unit-test-tool/CDataLayoutReflectionStride.internal'
            'slang-unit-test-tool/CDataLayoutReflectionStrideWithScalarLayout.internal'
            'slang-unit-test-tool/linkTimeTypeReflection.internal'
            'slang-unit-test-tool/entryPointCompile.internal'
            'slang-unit-test-tool/spirvInterfaceDefaultInitValidation.internal'
            'slang-unit-test-tool/fcpwCompile.internal'
        )

        any_failed=0
        for test in \"\${failing_tests[@]}\"; do
            echo \"\"
            echo \"--- Running: \$test ---\"
            if \"\$bin_dir/slang-test\" \"\$test\" -show-adapter-info -ignore-abort-msg; then
                echo \"PASSED: \$test\"
            else
                echo \"FAILED: \$test\"
                any_failed=1
            fi
        done

        echo ''
        echo '=== Summary ==='
        if [ \$any_failed -eq 1 ]; then
            echo 'Some tests FAILED (as expected -- reproducing CI failure)'
        else
            echo 'All tests PASSED (failure not reproduced)'
        fi
        exit \$any_failed
    "
}

do_smoke() {
    echo "=== Running full smoke test suite on aarch64 ==="

    local cmake_config
    if [[ "$BUILD_CONFIG" == "release" ]]; then
        cmake_config="Release"
    else
        cmake_config="Debug"
    fi

    docker_run bash -c "
        set -e
        bin_dir=/workspace/build/$cmake_config/bin
        export LD_LIBRARY_PATH=/workspace/build/$cmake_config/lib:\${LD_LIBRARY_PATH:-}
        export SLANG_RUN_SPIRV_VALIDATION=1
        export SLANG_USE_SPV_SOURCE_LANGUAGE_UNKNOWN=1

        \"\$bin_dir/slang-test\" \
            -category smoke \
            -expected-failure-list tests/expected-failure-github.txt \
            -expected-failure-list tests/expected-failure-no-gpu.txt \
            -skip-reference-image-generation \
            -show-adapter-info \
            -ignore-abort-msg \
            -enable-debug-layers false
    "
}

do_shell() {
    echo "=== Starting interactive shell in aarch64 container ==="
    docker run --rm -it \
        --platform linux/arm64 \
        --name "$CONTAINER_NAME" \
        -v "$REPO_ROOT:/workspace:rw" \
        -w /workspace \
        -e CC=gcc \
        -e CXX=g++ \
        -e BUILD_CONFIG="$BUILD_CONFIG" \
        "$IMAGE_NAME" \
        bash
}

# --- Main ---

build_image

action="${1:-all}"
case "$action" in
    build)
        do_build
        ;;
    test)
        do_test
        ;;
    smoke)
        do_smoke
        ;;
    shell)
        do_shell
        ;;
    all)
        do_build
        do_test
        ;;
    *)
        echo "Usage: $0 [build|test|smoke|shell|all]"
        echo "  build  - Build Slang for aarch64"
        echo "  test   - Run the specific failing unit tests"
        echo "  smoke  - Run the full smoke test suite (like CI)"
        echo "  shell  - Interactive shell in the aarch64 container"
        echo "  all    - Build + test (default)"
        echo ""
        echo "Environment variables:"
        echo "  BUILD_CONFIG=debug|release (default: release)"
        exit 1
        ;;
esac
