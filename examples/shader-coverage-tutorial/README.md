# Shader Coverage Tutorial Files

These are the companion files for the user-guide chapter
[Shader Execution Coverage](../../docs/user-guide/a1-06-shader-coverage.md), which walks
through them command by command. Unlike the other coverage examples, this one is driven
entirely from the command line — there is no CMake target to build.

| File | Role |
| --- | --- |
| `hello-coverage.slang` | The compute shader the chapter instruments. |
| `hello-coverage-host.cpp` | A minimal host program that loads the `slangc`-precompiled CPU kernel, binds the coverage buffer where the sidecar manifest says, dispatches, prints the outputs, and writes the raw counters. It uses no Slang headers or library. Pass `--no-coverage` to run it as a plain CPU shared-library dispatch (its `coverageEnabled` blocks are exactly what coverage adds to a host). |
| `run-tutorial.sh` / `run-tutorial.ps1` | Commented scripts that execute every tutorial step in order — each step is labeled with the chapter section it comes from. |

## Quick run

Run all the steps at once. The scripts pick the first `slangc` with coverage support
from `PATH` or a sibling repo build (`../../build`), or take one explicitly:

```bash
./run-tutorial.sh                            # Linux / macOS
SLANGC=/path/to/slangc ./run-tutorial.sh     # explicit compiler

./run-tutorial.ps1                           # Windows (PowerShell)
./run-tutorial.ps1 -Slangc C:/path/to/slangc.exe
```

Or step by step, with `slangc` on your `PATH` (any Slang release) and Python 3:

```bash
# 1. Precompile the shader to a directly callable CPU kernel, with coverage.
slangc hello-coverage.slang -target shader-sharedlib -stage compute -entry computeMain \
    -trace-coverage -o hello-coverage-kernel.so     # use .dll on Windows

# 2. Build and run the host program (an ordinary C++ compile, no SDK paths).
#    -ldl: dlopen lives in libdl on glibc older than 2.34; harmless elsewhere.
c++ -std=c++17 hello-coverage-host.cpp -o hello-coverage-host -ldl
./hello-coverage-host

# 3. Turn the counters into an LCOV report.
python3 <slang-repo>/tools/shader-coverage/slang-coverage-to-lcov.py \
    --manifest hello-coverage-kernel.so.coverage-manifest.json \
    --counters hello-coverage.counters.bin --output hello-coverage.lcov
genhtml hello-coverage.lcov --output-directory coverage-html
```

`genhtml` (lcov package) has no common Windows distribution. Where it is unavailable, the
repository's own renderer produces an equivalent report anywhere Python runs:
`python3 <slang-repo>/tools/coverage-html/slang-coverage-html.py hello-coverage.lcov --output-dir coverage-html`.
LCOV viewers such as the VS Code Coverage Gutters extension read `hello-coverage.lcov`
directly.

Expected host-program output:

```
output[0] = 2
output[1] = 4
output[2] = 6
output[3] = 8
counter[0] = 4
counter[1] = 4
counter[2] = 0
counter[3] = 4
counter[4] = 4
counter[5] = 4
counter[6] = 0
counter[7] = 4
```

The two zero slots are the lines the inputs never exercise — slot 2 is the `return value`
fallthrough in `applyGain` (line 9), slot 6 is `value = 0.0` (line 19) — which the LCOV
report shows in red.

For GPU dispatch and the in-process (C++ API) workflow, see
[`examples/shader-coverage-image-pipeline`](../shader-coverage-image-pipeline) and
[`examples/shader-coverage-bvh-traversal`](../shader-coverage-bvh-traversal).
