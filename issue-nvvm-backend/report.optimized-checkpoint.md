# Validate the optimized NVVM configuration

## Motivation

The accepted slice-202 results were produced by a Debug host compiler. The development workflow
now selects RelWithDebInfo, so those results alone cannot establish that the optimized compiler,
its matching provider and test libraries preserve working runtime contracts. For example, the new
texture-descriptor fixture must still load real textures and preserve UInt64 payload bits in all
three shader modes after changing the host build configuration.

## Proposed solution

Build the unchanged compiler and provider from the accepted source, then replay the complete
frozen and discovery selections at NVRTC O3 and NVVM O0/O3. Compare every identity/mode pair with
the immutable slice-202 outcomes. Run the small actual-execution gate first, then relevant units,
CUDA toolkit assembly coverage and both complex entry points in every registered mode.

## Change summary

This checkpoint changes documentation and durable evidence only. The completed ExecPlan records
execution decisions; the runtime manifest records source/binary/toolkit identity, gate results,
exact preservation comparisons and the unresolved-failure ledger. Two TSV files retain every
runtime cell outcome for future comparisons. STATUS names the accepted configuration checkpoint
separately from feature slice 202 and preserves the implementation-slice cadence.

## Concepts and vocabulary

A runtime cell is one workload identity in one shader backend/optimization mode. RelWithDebInfo
is the host compiler build configuration; it does not replace the required shader O0/O3 modes.
Frozen and discovery selections preserve their existing workload contracts and historical healthy
denominators. Complex cells compile and assemble application entry points without executing the
material: their missing bindings and output oracles still prevent material runtime claims.

## Process report

The starting tree was clean at `ecfacff50002bd9b60f5250b56a2023a399581e5`. The native Linux
Ninja Multi-Config build already selected the isolated, pinned LLVM14 provider dependency and
kept the core module binary external. Building preset `releaseWithDebugInfo` selects matching
compiler, provider, render tool, unit library and test server artifacts. The ignored environment
helper selects Debug, so the checkpoint commands explicitly use optimized paths and the CUDA
12.9 installation.

The first build used four outer jobs, but the provider ExternalProject inherited the same four-job
limit. Its two compilation objects could briefly overlap three parent jobs; the actual overlap
was not measured. Future incremental builds must use inherited level 1 with an explicit four-job
outer limit. Validation suites run sequentially with at most four workers. No build-speed claim
is drawn from the initial build.

The plan establishes 1,605 runtime cells as obligations: 452 frozen identities and 83 discovery
identities, each at NVRTC O3 and NVVM O0/O3. The accepted Debug record contains 1,544 correct cells
and 61 failures. Prior failures keep their historical evidence references; fresh evidence records
the actual diagnostics, execution counts and log hashes. A lost pass cannot become an optimized
baseline. Missing or duplicate cells also block acceptance.

All 1,605 runtime cells were freshly replayed. Exact keys matched with no duplicates, missing or
additional cells. All 1,544 previous correct cells remained correct; all 61 failures retained their
classification, return code and execution counts. Frozen correct counts are 449/438/438 and
discovery counts are 73/73/73 for NVRTC O3/NVVM O0/NVVM O3. The four runtime fixtures, 473 selected
unit tests (plus one Windows-only skip), and 18 toolkit cells passed. Both complex NVRTC entries
assembled; all four direct complex cells retained `LoadFromUninitializedMemory` rejection. These
complex results establish support status only. The L4 remained healthy after validation.

One known failure produced different diagnostics: `compute/texture-subscript-multisample.slang`
at discovery NVRTC O3. Consider its existing writable multisample texture declarations:

```slang
RWTexture2DMS<int4> outputTexture2DMS;
RWTexture2DMSArray<int4> outputTexture2DMSArray;
```

`CPPSourceEmitter::_getTypeName` in `source/slang/slang-emit-cpp.cpp:117` begins with a null string
handle and only fills it when `calcTypeName` succeeds. Debug stops at its line-133 assertion that
the handle is non-null. `SLANG_ASSERT` is conditional on `_DEBUG` in
`source/core/slang-common.h:363`; RelWithDebInfo proceeds far enough to emit declarations missing
their type names. NVRTC reports `this declaration has no storage class or type specifier` for
`outputTexture2DMS_0;` and `outputTexture2DMSArray_0;`, followed by invalid subscript operations.
The raw evidence is
`build/nvvm-loop/optimized-checkpoint-20260924/discovery/logs/nvrtc-o3/be15e8b3d4c153109f34d6110f40510a53614204.log`.
The runner's parsed diagnostic is empty for these NVRTC errors, so the manifest also retains raw
error excerpts. This is the same pre-existing unsupported case exposed differently by the host
configuration. It remains an infrastructure failure with return code 1 and zero passing executions;
no baseline, test oracle or pass requirement was weakened.

After correctness validation, a fixed two-fixture compile experiment ran without concurrent suites:
one warmup and three measured samples per configuration, alternating the order. At shader NVVM O3,
`nvvm-core-execution.slang` medians were 0.369 seconds (Debug) and 0.300 seconds (RelWithDebInfo),
a 1.228 ratio; `nvvm-texture-descriptor-conversion.slang` medians were 0.416 and 0.328 seconds,
a 1.270 ratio. PTX was identical across configurations for each fixture. These small measurements
characterize host compilation for those fixtures only, not full-build speed or kernel performance.

No helper, fallback, compiler special case, AST/IR representation, provider ABI or workload oracle
changed, so no producer-side input-shape audit is implicated. The next feature can be ranked from
the existing sampler-placeholder/undefined-read material blocker and remaining wave transport
coverage after the integrating agent reviews this checkpoint. No next feature was started.
