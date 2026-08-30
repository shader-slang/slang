# Direct libNVVM compute-corpus census

Slice 131 establishes a broad, reproducible baseline for direct-libNVVM compute usability. It is a
measurement slice: it changes no compiler behavior, provider operation, builder ABI, or checked-in
test fixture. The selected `slang-unit-test-tool/nvvm` result remains a regression score; it is not
used as the coverage denominator in this report.

## Scope and method

The census discovers every active CUDA `COMPARE_COMPUTE` or `COMPARE_COMPUTE_EX` directive under
`tests/`. It preserves each source, entry point, specialization arguments, `TEST_INPUT` metadata,
and indexed expected-output sidecar in an ignored mirror. For each eligible workload it runs:

1. the existing NVRTC lane at O3 as the differential reference;
2. direct libNVVM at O0; and
3. direct libNVVM at O3.

Each workload runs independently so that one compiler or driver failure cannot hide later results.
The committed [census.slice-131.tsv](census.slice-131.tsv) contains one row per workload and records
the phase, first canonical IR shape, producer/owning consumer, and diagnostic for every failure.
Raw generated sources, logs, timing records, final-IR probes, PTX, and cubins remain under ignored
`build/nvvm-census/`.

The candidate universe contains 683 sources. The initial compute census excludes whole feature
families that are outside the bounded MVP:

| Excluded family | Sources | Reason |
| --- | ---: | --- |
| Autodiff and differentiable standard library | 182 | Outside the initial compute MVP |
| Cooperative matrix | 37 | Advanced matrix/tensor execution |
| Neural operations | 15 | Advanced neural execution |
| Ray tracing | 1 | OptiX/ray-tracing family |
| FP8 outside cooperative matrix | 1 | Explicitly deferred scalar family |
| **Total excluded** | **236** | |

The remaining 447 sources contain 451 eligible CUDA workload lanes. Of those, 429 are MVP lanes.
The other 22 remain classified as visible extension evidence: advanced wave prefix/matrix/multi,
rotate/quad operations, and the device-clock workload. They are not silently discarded and do not
control initial MVP readiness.

## Environment

- Host and runtime GPU: NVIDIA GeForce RTX 5090, compute capability 12.0, driver 610.62.
- Active toolkit: CUDA 12.9.86 (`nvcc` and `ptxas`).
- Installed toolkit directories: CUDA 12.8 and 12.9; the CUDA 13.0 directory contains no tools.
- Direct provider: isolated LLVM 14 `slang-llvm-nvvm`, builder ABI revision 24.
- Host compiler and test binaries: Release configuration.

The physical runtime result therefore covers only SM120 on this machine. Direct PTX was separately
compiled and assembled for SM70, SM80, and SM90. CUDA 13 is an infrastructure gap, not a backend
failure or a claimed validation result.

## Result categories

Every lane has exactly one of the requested classifications:

1. **Correct**: compilation, execution, and expected-output comparison all pass.
2. **Runtime mismatch**: compilation and execution complete, but output differs.
3. **Slang NVVM preflight**: deterministic compiler-owned E52017 rejection before provider mutation.
4. **Provider/libNVVM**: LLVM verification, provider emission, or libNVVM compilation fails.
5. **Infrastructure/toolchain**: the native reference, generated test contract, provider discovery,
   CUDA toolkit, or hardware setup prevents a meaningful backend result.

The final matrix contains no unknown or unclassified results.

## Coverage denominators and results

### Complete eligible corpus

| Route | Correct | Runtime mismatch | Preflight | Provider/libNVVM | Infrastructure | Compiles and runs |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| NVRTC O3 reference | 448 | 0 | 0 | 0 | 3 | 448/451 (99.3%) |
| Direct NVVM O0 | 196 | 7 | 244 | 4 | 0 | 203/451 (45.0%) |
| Direct NVVM O3 | 192 | 15 | 244 | 0 | 0 | 207/451 (45.9%) |

There are 448 healthy native references. Differential correctness is 195/448 (43.5%) at O0 and
190/448 (42.4%) at O3. The direct route is correct at both optimization levels for 187 healthy
native workloads.

### Bounded MVP tier

| Route | Correct | Runtime mismatch | Preflight | Provider/libNVVM | Infrastructure | Compiles and runs |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| NVRTC O3 reference | 426 | 0 | 0 | 0 | 3 | 426/429 (99.3%) |
| Direct NVVM O0 | 196 | 7 | 222 | 4 | 0 | 203/429 (47.3%) |
| Direct NVVM O3 | 192 | 15 | 222 | 0 | 0 | 207/429 (48.3%) |

Over the 426 healthy MVP references, differential correctness is 195/426 (45.8%) at O0 and
190/426 (44.6%) at O3. These are the initial broad-coverage baselines. The separate selected NVVM
regression is 399/399 after Slice 130.

## Pareto analysis

The O3 MVP has 237 non-correct results. Clusters are keyed by canonical operation/type boundary and
producer, not by individual fixture. Path families are used only to subdivide an already-identified
`GenericAsm` semantic family or to label runtime-result families in this planning report.

| Rank | O3 MVP root-cause cluster | Blocked workloads | Cumulative | First canonical shape and owner |
| ---: | --- | ---: | ---: | --- |
| 1 | Ordinary intrinsic `GenericAsm` semantics | 62 | 62 (26.2%) | `IRGenericAsm`; `StmtLoweringVisitor::visitIntrinsicAsmStmt` to `_validateNVVMFunction` |
| 2 | Helper ABI type contract | 51 | 113 (47.7%) | Helper parameter/result type; specialized linked `IRFunc` to `_validateNVVMHelperTarget` |
| 3 | Aggregate, pointer, and layout transport | 39 | 152 (64.1%) | Canonical aggregate/pointer IR to `_validateNVVMFunction` |
| 4 | Common wave/reconvergence `GenericAsm` | 31 | 183 (77.2%) | `IRGenericAsm`; intrinsic-asm lowering to semantic resolution |
| 5 | Ordinary numeric and bit operations | 11 | 194 (81.9%) | Typed numeric IR operation to `_resolveNVVMValueOperation` |
| 6 | Atomic and wave operations | 8 | 202 (85.2%) | Canonical atomic/wave legalization operation to preflight |
| 7 | O3 narrow-integer conversion mismatch | 8 | 210 (88.6%) | Optimized PTX executed by compare-compute harness |
| 8 | Residual target markers or undefined values | 7 | 217 (91.6%) | Named upstream IR producer to the preflight default rejection |
| 9 | Function identity | 6 | 223 (94.1%) | Specialized linked `IRFunc` to `_collectNVVMFunctionNames` |
| 10 | Raw-buffer view/access | 4 | 227 (95.8%) | Raw-buffer legalization to the direct raw-buffer resolver |
| 11 | Descriptor-handle runtime layout | 3 | 230 (97.0%) | Compiled PTX executed by compare-compute harness |
| 12 | Atomic `GenericAsm` semantics | 2 | 232 (97.9%) | `IRGenericAsm`; intrinsic-asm lowering to semantic resolution |
| 13 | Texture `GenericAsm` semantics | 1 | 233 (98.3%) | `IRGenericAsm`; intrinsic-asm lowering to semantic resolution |
| 14 | Constant-buffer runtime layout | 1 | 234 (98.7%) | Compiled PTX executed by compare-compute harness |
| 15 | Bounds-check zero-index runtime result | 1 | 235 (99.2%) | Compiled PTX executed by compare-compute harness |
| 16 | SM90 mixed-width atomic runtime/toolchain case | 1 | 236 (99.6%) | Compiled PTX executed by compare-compute harness |
| 17 | `AnyValue` runtime layout | 1 | 237 (100%) | Compiled PTX executed by compare-compute harness |

The first three clusters block 152/237, or 64.1%, of current MVP failures. They are the next
priorities because each points to a reusable representation or semantic boundary and together
cover substantially more than any collection of isolated operation fixtures.

Across the full 451-workload corpus, O3 adds 14 extension wave/reconvergence `GenericAsm` failures,
five extension helper-ABI failures, one numeric/bit failure, one residual-marker failure, and the
device-clock `GenericAsm` failure. The full leading counts are ordinary `GenericAsm` 62, helper ABI
56, wave/reconvergence `GenericAsm` 45, aggregate transport 39, numeric/bit operations 12, and the
remaining clusters shown in [census.slice-131-clusters.json](census.slice-131-clusters.json).

## Canonical producer and diagnostic evidence

| Cluster | Representative first shapes | Exact producer/owner | Diagnostic contract |
| --- | --- | --- | --- |
| Ordinary/wave/atomic/texture/clock intrinsic semantics | `GenericAsm` | `StmtLoweringVisitor::visitIntrinsicAsmStmt` creates `IRGenericAsm`; `_findNVVMGenericAsmSemantic` is called from `_validateNVVMFunction` | E52017: direct NVVM lowering does not support Slang IR instruction or shape `GenericAsm` |
| Helper ABI type contract | Helper function parameter; helper function result type | Post-specialization linked `IRFunc` signature; `_validateNVVMHelperTarget` | E52017 naming the unsupported helper parameter or result type |
| Aggregate/pointer/layout transport | Struct field address; local resource-struct layout; sequential element pointer; block parameter; `makeStruct`; structured-buffer aggregate layout; entry parameter; call argument type; `global_param` | Canonical linked aggregate/pointer IR; `_validateNVVMFunction` | E52017 naming the first unsupported canonical aggregate or pointer shape |
| Ordinary numeric/bit operation | `intCast`; `shr`; `bitfieldInsert`; `bitfieldExtract`; `castIntToFloat`; `bitCast` | Typed expression/intrinsic IR op; `_resolveNVVMValueOperation` | E52017 naming the unsupported typed value operation |
| Atomic/wave operation | `waveMaskMatch`; compare-exchange; exchange; and; store; selected atomic operation | Atomic/wave legalization IR op; `_validateNVVMFunction` | E52017 naming the canonical atomic or wave operation |
| Raw-buffer view/access | Equivalent structured-buffer view; core byte-address access | Raw-buffer legalization IR op; direct raw-buffer resolver | E52017 naming the rejected view/access shape |
| Function identity | Function name | Post-specialization linked `IRFunc`; `_collectNVVMFunctionNames` | E52017 naming the unsupported function identity |
| Residual markers/undefined values | Load from uninitialized memory; debug no-scope; derivative/prelude/string-hash/reconvergence requirements | Named upstream IR producer; `_validateNVVMFunction` default rejection | E52017 naming the residual instruction |
| Unoptimized Half provider operation | Supported preflight shape reaches provider at O0 | `NVVMTypeLoweringContext`/`emitNVVMIRFromLinkedIR` to libNVVM | libNVVM compilation failed: `Error: unsupported operation` |
| Runtime-result families | Emitted PTX | CUDA compare-compute execution and expected/actual comparison | `slang-test` expected/actual runtime result mismatch |

An optimized final-IR dump for `bugs/frexp.slang` confirms repeated
`GenericAsm("$P_frexp($0, $1)")` instructions before E52017. The matrix keeps the exact diagnostic
for each row; raw logs preserve full context. None of these observations introduces a fixture-name
check, syntax reconstruction, or downstream patch in the compiler.

## Optimization-level differences

The O0-to-O3 transition table is:

| O0 result | O3 result | Workloads | Interpretation |
| --- | --- | ---: | --- |
| Preflight | Preflight | 244 | Same unsupported canonical boundary |
| Correct | Correct | 188 | 187 also have a healthy native reference |
| Correct | Runtime mismatch | 8 | Narrow signed/unsigned 8/16/32/64 conversion family |
| Runtime mismatch | Runtime mismatch | 7 | Layout/bounds/atomic families remain wrong |
| Provider/libNVVM | Correct | 4 | Unoptimized Half operations rejected by libNVVM only at O0 |

The four O0 provider failures are `half-rw-texture-simple`, `half-vector-calc`,
`half-vector-compare`, and `cuda/nvvm-half-values`. They all report libNVVM's unsupported-operation
diagnostic and pass direct O3. The eight O3-only runtime mismatches cover the narrow integer
conversion fixtures for signed and unsigned 8-, 16-, 32-, and 64-bit destinations.

Other O3 runtime mismatches are three descriptor-handle dynamic-dispatch layouts, one aligned
`float3` constant-buffer layout, one `AnyValue` layout, one bounds-check zero-index case, and one
SM90 mixed-width atomic case. The last case also lacks a healthy native reference on this host.

The three native infrastructure failures are independently auditable: the SM90 mixed-width atomic
lane emits a profile-upgrade warning that violates an empty diagnostic expectation; CUDA 12.9
NVRTC rejects `__half4.xyz` in `half-vector-calc`; and CUDA 12.9 NVRTC rejects the surface-object
subscript used by `texture-subscript`.

## Representative workload gates

The initial release gates deliberately combine features that isolated unit fixtures do not:

| Gate | Source | Composition | Native/O0/O3 |
| --- | --- | --- | --- |
| Resource aggregate/helper | `tests/compute/dynamic-dispatch-bindless-texture.slang` | Resource aggregates, descriptor transport, texture access, helper calls | Correct/correct/correct |
| Parameter-block layout | `tests/compute/parameter-block.slang` | Entry ABI, parameter-block layout, buffer access | Correct/correct/correct |
| Shared control/barriers | `tests/language-feature/execution-model/groupshared-multi-barrier-functional.slang` | Shared storage, control flow, loops, barriers | Correct/correct/correct |

All three direct O3 modules compile and assemble for SM70, SM80, and SM90. They execute only on
the local SM120 GPU in this census.

## Exploratory compile and artifact measurements

The following are medians of three standalone `slangc` invocations. They include process and
compiler startup, so they are useful as a repeatable baseline rather than a kernel-only compiler
benchmark. NVRTC chose SM75 PTX on this host even though the input capability was SM70; direct PTX
retained the requested architecture. Cubins are assembled for the PTX module's actual target.

| Gate | Route | Compile median | PTX bytes | Cubin bytes |
| --- | --- | ---: | ---: | ---: |
| Resource aggregate/helper | NVRTC O3, SM75 emitted | 382.2 ms | 8,889 | 13,664 |
| Resource aggregate/helper | Direct O0, SM70 | 284.4 ms | 6,102 | 4,200 |
| Resource aggregate/helper | Direct O3, SM70 | 268.2 ms | 919 | 2,792 |
| Parameter-block layout | NVRTC O3, SM75 emitted | 372.8 ms | 8,839 | 13,664 |
| Parameter-block layout | Direct O0, SM70 | 254.3 ms | 917 | 2,920 |
| Parameter-block layout | Direct O3, SM70 | 256.7 ms | 793 | 2,920 |
| Shared control/barriers | NVRTC O3, SM75 emitted | 375.7 ms | 9,190 | 13,984 |
| Shared control/barriers | Direct O0, SM70 | 246.9 ms | 1,940 | 3,680 |
| Shared control/barriers | Direct O3, SM70 | 250.7 ms | 1,404 | 3,168 |

For direct O3, SM80 cubins are 2,920, 3,048, and 3,296 bytes respectively; SM90 cubins are 3,488,
3,360, and 3,872 bytes. The PTX text size is unchanged across those direct targets for these gates.

Across all census lanes, the startup-inclusive end-to-end median/p90/mean times are
3,710/3,931/3,754 ms for NVRTC O3, 3,566/3,789/3,602 ms for direct O0, and
3,617/3,893/3,653 ms for direct O3. These include compilation, module loading, execution, and
comparison. The census establishes differential runtime correctness, but it does not yet isolate
kernel execution time; a production runtime-performance comparison remains open.

## Bounded usable-compute MVP

The initial MVP covers:

- conventional compute entry points and launch/global ABI;
- buffers, parameter blocks, constant buffers, textures, and samplers;
- ordinary scalar, vector, and matrix operations;
- direct helpers, control flow, loops, barriers, and mutable local/shared/global storage;
- commonly used atomics and wave operations;
- deterministic diagnostics for unsupported features; and
- CUDA 12 and 13 validation across representative SM70, SM80, and SM90 configurations.

OptiX, RDC/device LTO, dynamic parallelism, device syscalls, FP8, advanced wave operations, and
source-level debugging are outside the initial MVP unless a selected real workload requires them.
Autodiff, cooperative-matrix, and neural families are likewise excluded from this first denominator.

The measurable completion gate is at least 80% differential correctness at both O0 and O3 over
healthy native MVP references, no unexplained provider or runtime mismatch in the claimed supported
subset, all three representative workload gates, deterministic diagnostics, and the packaging plus
toolkit/architecture matrix. This threshold can be revised only with representative-workload
evidence.

## Productionization baseline

`slang-llvm-nvvm` is an optional compiler-matched provider built against isolated LLVM 14 and the
CUDA libNVVM contract. ABI revision 24 is exact: provider updates move with the Slang commit and
build recipe, and no backward compatibility is promised across revisions. The intended deployment
is next to the Slang binaries; `SLANG_NVVM_BUILDER_PATH` remains an explicit development or
deployment override. A session caches one provider-load result. Explicit direct selection must
fail deterministically with E52016 when discovery or ABI matching fails and must never silently
fall back to NVRTC.

Packaging automation, install/deployment validation, provider update policy enforcement, cache and
failure tests, CUDA 13 tooling, physical SM70/SM80/SM90 runtime workers, and kernel-only performance
instrumentation remain open productionization work.

## Prioritized follow-up

The next reusable vertical slices should address:

1. the helper ABI type contract (51 MVP workloads), by inventorying valid specialized canonical
   parameter/result families and generalizing the compiler-owned type contract through existing
   generic builder operations;
2. aggregate, pointer, and layout transport (39 MVP workloads), coordinated with the same canonical
   type representation rather than patched per addressing instruction; and
3. ordinary intrinsic `GenericAsm` semantics (62 MVP workloads), by classifying the intrinsic
   semantic at its canonical producer/consumer boundary and emitting it through existing builder
   operations wherever possible.

These three clusters are selected by workloads unlocked and architectural reuse. Common
wave/reconvergence semantics, at 31 MVP workloads plus 14 extension workloads, follow them. A
provider callback or ABI revision is justified only if a concrete canonical operation cannot be
expressed through builder ABI 24.

## Reproduction

From a Release build with the provider environment configured:

```powershell
python.exe issue-nvvm-backend\run-compute-census.py --discover-only
python.exe issue-nvvm-backend\run-compute-census.py
python.exe issue-nvvm-backend\summarize-compute-census.py `
    --table issue-nvvm-backend\census.slice-131.tsv `
    --clusters issue-nvvm-backend\census.slice-131-clusters.json
python.exe issue-nvvm-backend\measure-compute-mvp.py
```

The full census is intentionally not a normal presubmit: it creates 1,353 independent runtime
lanes. Its scripts, denominators, and committed evidence make the baseline repeatable without
pretending that a selected positive regression set measures overall backend coverage.
