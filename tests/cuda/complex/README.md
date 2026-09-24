# Complex CUDA / NVVM compile corpus

These application-sized shaders are compile-and-assemble workloads. Their identities and source
hashes live in [the complex-corpus manifest](../../../issue-nvvm-backend/complex-corpus.manifest.json).
They are deliberately separate from the frozen and discovery **runtime** corpora: a successful
compile does not establish correct GPU output, and no historical denominator changes here.

## Tiled brass material

`tiled_brass_material.slang` is the supplied isolated Falcor2/MaterialX tiled-brass material, with
NVIDIA's Apache-2.0 license header retained. Its two independent compute workloads are:

- `tiled-brass-material/eval_buffer`: evaluate the BSDF and PDF for supplied directions.
- `tiled-brass-material/sample_buffer`: sample an outgoing direction, PDF, weight, and flags.

The corpus copy only normalizes CRLF to LF and adds the user-approved `__TARGET_CUDA__=1` definition
and explanatory comment. The original attachment hash and adapted source hash are in the manifest.
Keep this application workload intact; put reduced compiler regressions beside related focused tests.

The initial [assessment](../../../issue-nvvm-backend/report.tiled-brass-assessment.md) finds both
entries compile and assemble through NVRTC, while direct NVVM O0/O3 reject
`CastUInt64ToDescriptorHandle`. This is a tracked support gap, not an expected-error language test.

## Reproduce

From the repository root, using the configured native Linux development environment:

```bash
source build/nvvm-setup/env.sh
python3 issue-nvvm-backend/run-complex-corpus.py \
    --slangc build/Debug/bin/slangc --build-label Debug \
    --provider build/Debug/bin/libslang-llvm-nvvm.so \
    --cuda-root "$CUDA_PATH" --output build/nvvm-complex
```

The runner compiles both entries separately at SM80 through NVRTC O3 and NVVM O0/O3, checks fresh
PTX entry/target declarations, and assembles successful outputs with `ptxas -v`. It records one
warmup and three timed successful compilations, phase timers, exact commands, compiler/provider/
toolkit hashes, PTX/cubin sizes, and assembler resource reports. A rejected shader stops after its
first failed attempt; failure latency is not reported as a successful compilation metric.

Exit 0 means all cells compile and assemble. Exit 1 retains incomplete shader support or per-cell
failures in `results.json`; **the initial material assessment is expected to exit 1**. Exit 2 reports
invalid inputs or missing prerequisites. Successful compiler/assembler output is not GPU execution.
The source SHA check requires an intentional manifest update when a workload changes.

Use an optimized compiler build and controlled repeated measurements before comparing compilation
speed. PTX size, register counts, stack usage and spills are useful observations, but executable
correctness and timed kernels on representative inputs are required to judge generated code quality.

## Runtime contract still needed

Both entries require a generated material data record and input/output buffers. The selected material
also uses bindless texture handles and lookup-table buffers in `lut_globals`. Establish the host
mapping for the packed texture indices and CUDA texture objects, real or explicitly scoped texture/LUT
fixtures, direction/UV/seed cases, and expected BSDF/PDF/sample behavior before claiming runtime
coverage. Neither zero dispatches nor missing/default resources can stand in for this contract.
