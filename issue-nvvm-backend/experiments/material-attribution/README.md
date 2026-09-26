# Reproduce material attribution and aggregate probes

Research264 retains a measurement-only patch and two general shaders. No optimization is installed.
The patch was measured on f9532f06e, with provider ABI42 and the environment in
[evidence264](../../timing-evidence.slice-264.json). The material source and numeric options are fixed.

For a new attribution run, first follow WORKFLOW/RESULTS to establish the accepted compiler and
preserve its complete bin/lib/cache layout. In a clean authorized experimental checkout, run
`git apply --check issue-nvvm-backend/experiments/material-attribution/instrumentation.patch`, then
apply it and build using the slang-build skill. On later revisions inspect/rebase the patch's scope
boundaries; do not force application or infer that identical timer names mean identical intervals.
Qualify material and quality output against the matching uninstrumented baseline before measuring.
Use the maintained `nvvm-results.py material` command with a unique output root and normal protocol.
The patch does not add a provider DSO profiler or change ABI. Reverse it and restore the accepted
layout when the experiment ends; preserve compiler, modules and caches together, without overlaying
an old directory onto new generated files. Refresh CMake version metadata on the next real build.

Scopes are inclusive. `nvvmEmitIR` contains serialization as well as capability preflight,
construction and teardown; subtracting serialization does not yield pure LLVM construction time.
Vendor compile excludes verification/load/destruction. The size-query and write serializer calls
each verify and materialize the module. Their difference is not an observed optimization saving.
Use the report command's stage-attribution option documented in RESULTS after the reporting refresh.

The two shader files differ only in the local representation of a graph: one groups payload and
dynamically indexed weights in a single object; the other separates their storage. They are research
fixtures, outside the registered frozen/discovery inventories. From the repository root:

```bash
export CUDA_PATH=/usr/local/cuda-12.9
export LD_LIBRARY_PATH="$CUDA_PATH/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
build/RelWithDebInfo/bin/slang-test \
  issue-nvvm-backend/experiments/material-attribution/aggregate-constant-probe.slang \
  issue-nvvm-backend/experiments/material-attribution/aggregate-separated-probe.slang \
  -use-test-server -server-count 1 -disable-retries
```

Compile each file with `slangc -target ptx -stage compute -entry constantMain -O3
-capability cuda_sm_8_0 -o <unique.ptx>`, once normally and once with `-emit-cuda-via-nvvm`.
Repeat with `controlMain`; assemble each with `ptxas -arch=sm_80 -v`. Add `-dump-intermediates
-dump-intermediate-prefix <unique-prefix>` for untimed LLVM/CUDA inspection.
The runtime oracle covers both array endpoints and constant-zero/false versus runtime-nonzero/true
payloads. Compiler inputs are unknown at compile time; expected outputs are29,29,33,33.

Both backends miss constant elimination in the grouped probe. Both eliminate it in the separated
probe. Thus the probe identifies a general aggregate limitation but does not reproduce the material's
NVRTC/NVVM difference. Do not headline its source-level register/stack improvement as a compiler
improvement. Future promotion needs a differential reproducer, a principled transformation, alias/
mutation/escape/control-flow boundary tests, full relevant correctness and paired timings.
