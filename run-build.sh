#!/usr/bin/env bash
# Build driver for wt-slang-11083. Captures the real exit status of the
# cmake build (not a pipeline stage) into build-exit.txt.
cd /workspace/agent/wt-slang-11083 || exit 99

rm -f /workspace/agent/wt-slang-11083/build-exit.txt

date -u -Iseconds > /workspace/agent/wt-slang-11083/build-started-at

cmake --build --preset debug > /workspace/agent/wt-slang-11083/build.log 2>&1
BUILD_EXIT=$?

echo "BUILD_EXIT=${BUILD_EXIT}" > /workspace/agent/wt-slang-11083/build-exit.txt
date -u -Iseconds >> /workspace/agent/wt-slang-11083/build-exit.txt
exit "${BUILD_EXIT}"
