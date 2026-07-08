# Shader Coverage Tutorial Files

These are the companion files for the user-guide chapter
[Shader Execution Coverage](../../docs/user-guide/a1-06-shader-coverage.md), which walks
through them command by command. Unlike the other coverage examples, this one is driven
entirely from the command line — there is no CMake target to build.

| File | Role |
| --- | --- |
| `hello-coverage.slang` | The compute shader the chapter instruments. |
| `hello-coverage-host.cpp` | A minimal host program that loads the `slangc`-precompiled CPU kernel, binds the coverage buffer where the sidecar manifest says, dispatches, and writes the raw counters. It uses no Slang headers or library. |

## Quick run

With `slangc` on your `PATH` (any Slang release) and Python 3:

```bash
# 1. Precompile the shader to a directly callable CPU kernel, with coverage.
slangc hello-coverage.slang -target shader-sharedlib -stage compute -entry computeMain \
    -trace-coverage -o hello-coverage-kernel.so     # use .dll on Windows

# 2. Build and run the host program (an ordinary C++ compile, no SDK paths).
c++ -std=c++17 hello-coverage-host.cpp -o hello-coverage-host
./hello-coverage-host

# 3. Turn the counters into an LCOV report.
python3 <slang-repo>/tools/shader-coverage/slang-coverage-to-lcov.py \
    --manifest hello-coverage-kernel.so.coverage-manifest.json \
    --counters hello-coverage.counters.bin --output hello-coverage.lcov
genhtml hello-coverage.lcov --output-directory coverage-html
```

Expected host-program output:

```
counter[0] = 4
counter[1] = 4
counter[2] = 0
counter[3] = 4
counter[4] = 4
counter[5] = 4
counter[6] = 0
counter[7] = 4
```

The two zero slots are the lines the inputs never exercise — `value = 0.0` (line 19) and
the `return value` fallthrough in `applyGain` (line 9) — which the LCOV report shows in red.

For GPU dispatch and the in-process (C++ API) workflow, see
[`examples/shader-coverage-image-pipeline`](../shader-coverage-image-pipeline) and
[`examples/shader-coverage-bvh-traversal`](../shader-coverage-bvh-traversal).
