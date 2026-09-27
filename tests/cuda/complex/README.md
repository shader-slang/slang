# Complex CUDA / NVVM compile corpus

These application-sized shaders are compile-and-assemble workloads. Their identities and source
hashes live in the [material manifest](../../../issue-nvvm-backend/complex-corpus.manifest.json).
They are separate from the frozen and discovery runtime corpora. Successful compilation and assembly
do not establish correct GPU output.

## Tiled brass material

`tiled_brass_material.slang` is the supplied isolated Falcor2/MaterialX tiled-brass material, with
NVIDIA's Apache-2.0 license header retained. Its two independent compute workloads are:

- `tiled-brass-material/eval_buffer`: evaluate the BSDF and PDF for supplied directions.
- `tiled-brass-material/sample_buffer`: sample an outgoing direction, PDF, weight, and flags.

The corpus copy normalizes CRLF to LF and adds the approved `__TARGET_CUDA__=1` definition and
explanatory comment. Original and adapted hashes remain in the manifest. Keep the workload intact;
put reduced compiler regressions beside related focused tests.

Both entries pass NVRTC O3 and NVVM O0/O3 compilation and SM80 assembly in the
[current accepted baseline](../../../issue-nvvm-backend/accepted-baseline.json). Earlier descriptor-
handle rejection is historical, not a current expected failure. Compiler identity and limitations
are recorded in [STATUS](../../../issue-nvvm-backend/STATUS.md).

## Reproduce and interpret

Follow the environment, correctness and material measurement commands in
[RESULTS](../../../issue-nvvm-backend/RESULTS.md). The maintained
`issue-nvvm-backend/run-complex-corpus.py` compiles each entry separately, checks PTX entry/target
contracts and assembles with `ptxas -v`. Its bounded shared-session protocol is distinct from the
fresh-process material benchmark. Preserve failed attempts, input hashes and exact tool identities;
failure latency is not successful compile time.

Exit0 requires all selected cells to compile and assemble. Exit1 preserves incomplete support or
per-cell failures; exit2 reports invalid inputs or prerequisites. Output inspection and hash checks
remain necessary. PTX/cubin size, registers, stack and spills are scoped observations, not GPU-speed
or numerical-correctness evidence.

## Runtime contract still needed

Both entries require a generated material data record and input/output buffers. The material also
uses bindless texture handles and lookup-table buffers in `lut_globals`. Establish the mapping for
packed texture indices and CUDA texture objects, scoped texture/LUT fixtures, direction/UV/seed
cases, and expected BSDF/PDF/sample behavior before claiming runtime coverage. Zero dispatches and
missing/default resources cannot stand in for that contract.
