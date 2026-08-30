# Slice 132: Recursive copyable-value and helper transport

## Result

Slice 132 replaces the overlapping helper scalar/struct/array rules with one compiler-owned,
recursive copyable-value contract. Selected scalar and vector leaves, positive-size fixed arrays,
and nonempty structs may now compose freely across local storage, block parameters, direct helper
parameters/results, calls, returns, aggregate construction/extraction, and keyed field access. The
isolated LLVM 14 provider remains at ABI revision 24; every new shape uses its existing generic
type, pointer, aggregate, function, call, load, and store operations.

The full denominator increased by the new focused fixture from 451 to 452 workloads. Direct O0
correctness rose from 196 to 218, and direct O3 correctness rose from 192 to 214. This comprises 21
previously failing corpus workloads plus the new fixture at both optimization levels. An identity-
level comparison found no previously correct workload that became incorrect in either lane.

| Mode | Correct | Compiles and runs, mismatches | Slang preflight | NVVM/provider failure | Infrastructure |
| --- | ---: | ---: | ---: | ---: | ---: |
| NVRTC O3 | 449 | 0 | 0 | 0 | 3 |
| Direct NVVM O0 | 218 | 7 | 222 | 5 | 0 |
| Direct NVVM O3 | 214 | 15 | 222 | 1 | 0 |

There are 430 MVP workloads and 22 measured extension workloads. Among the 427 MVP workloads with
a healthy native reference, 217 (50.8%) compare correctly at O0, 212 (49.6%) compare correctly at
O3, and 209 (48.9%) compare correctly at both. All 22 advanced wave/device-clock extension rows
remain preflight failures. The selected unit-test prefix is a separate regression score and passes
400/400.

## Canonical representation and code trace

Consider the focused source shape:

```slang
struct Payload
{
    double values[2];
    int bias;
}

[noinline]
double adjust(Payload payload, out double shifted) { ... }
```

After specialization and linking, the emitter sees a concrete `IRFunc` whose signature contains a
finite struct value, a fixed array of `Double`, and `OutParam<Double>`. The ordinary front-end and
linking pipeline, not the NVVM emitter, produce those exact types. `_collectNVVMFunctions` reaches
the helper, `_validateNVVMHelperTarget` validates its final signature, and
`_validateNVVMFunction` validates local variables, calls, returns, and aggregate consumers.
`NVVMTypeLoweringContext::lowerType` then lowers the same admitted algebra through generic builder
types, and `_emitNVVMModule` uses generic local pointers, loads/stores, aggregate operations, calls,
and returns.

`isNVVMSupportedCopyableValueType` is the shared, cycle-safe source of truth. It admits only finite
compositions of already selected leaves; unbounded arrays, empty structs, cycles, resources not
already modeled as copyable values, and deferred scalar families remain rejected. Internal helper
and local values use one consistent LLVM representation and therefore do not acquire an unrelated
CUDA external-storage layout requirement. CUDA layout comparison remains mandatory at actual
resource, parameter-group, and other external storage boundaries.

Canonical pointer ownership is deliberately split:

- `asNVVMSupportedLocalCopyableValuePointerType` recognizes only plain local `Ptr<T>`, exact
  `OutParam<T>`, and exact mutable `BorrowInOutParam<T>` producers.
- `asNVVMSupportedDerivedCopyableValuePointerType` recognizes typed derived `Ptr<T>` spellings used
  in exact call relations, but does not claim that their storage is mutable or local.
- field and element pointer consumers still validate their producer chain. In particular, an
  immutable field address cannot become writable merely because its final type is `Ptr<T>`.

This distinction is necessary, not a compatibility fallback. An early widening treated derived
field pointers as local mutable roots and regressed eight matrix/resource workloads. Restoring
producer-owned mutability recovered all eight; a separate three-workload internal-layout audit
recovered the remaining regressions. The final census proves all eleven prior correct rows are
restored. The caller-side exact-pointee relation remains the only legal conversion from local
`Ptr<T>` to helper `OutParam<T>`/`BorrowInOutParam<T>`.

The other newly admitted canonical operation is `MakeArrayFromElement`. Its producer creates a
fixed array splat after specialization; the emitter builds that value by repeating the exact typed
operand through the existing aggregate-construction operation. Nested fixed-array element and
keyed field access recurse over the same copyable algebra. No source syntax, fixture name, custom
struct equivalence, or provider callback participates.

## Measured helper-cluster transition

The pre-change typed inventory contains all 51 MVP helper-ABI rows: 27 first fail on a final helper
parameter and 24 on a final helper result. Exact types divide into finite copyable values; local
`OutParam`/mutable-borrow pointers; device/shared pointers; `RefParam`/read-only borrow forms;
resource-view types; specialized aggregate wrappers with deferred leaves; and BFloat16/FP8. This
slice admits only the first two families when their pointee/value is in the recursive algebra.

At both O0 and O3, the original 51 rows finish as follows:

| Outcome | Workloads |
| --- | ---: |
| Correct | 10 |
| Still helper ABI type contract | 28 |
| Reached ordinary `GenericAsm` semantics | 4 |
| Reached ordinary numeric/bit operation | 4 |
| Reached another exact preflight shape | 3 |
| Reached aggregate/pointer/layout transport | 2 |

Thus 23/51 rows move beyond the helper-signature gate, but only the ten correct rows count as
coverage gained from that original cluster. Eleven additional workloads from the neighboring
aggregate/pointer/layout cluster become correct because the same recursive value and producer-
owned pointer invariant governs their downstream transport. One further aggregate workload,
`generic-interface-dynamic-param`, now reaches the provider and deterministically fails its
`by-value aggregate field pointer` operation; it is recorded separately rather than counted as a
success or hidden inside the unoptimized-half provider cluster.

The 21 promoted existing workloads are:

- `compute/dynamic-dispatch-18`, `compute/nested-assoc-types`, and
  `compute/pack-any-value-16bit`;
- `hlsl-intrinsic/size-of/align-of-3` and `hlsl-intrinsic/size-of/size-of-3`;
- dynamic-dispatch `buffer-interface-array-field`, `layout-16bit-vectors`, `layout-array-2d`,
  `layout-array-field`, `layout-array-of-vectors`, `layout-conditional-field`,
  `layout-conditional-zero-size`, `layout-matrix-field`, `layout-optional-field`,
  `layout-tuple-field`, `layout-vector-field`, `layout-vector-matrix-mixed`,
  `nested-existential-dynamic-dispatch`, and `tagged-union-lowering-runtime`;
- `language-feature/if-let/if-let-less-than` and
  `language-feature/scalar-ternary-op-short-and-non-short-circuit`.

Each now has native CUDA plus direct O0/O3 runtime directives. The new
`tests/cuda/nvvm-helper-copyable-values.slang` fixture adds a compact direct representation gate
for nested Double arrays, a struct transported by value, and a scalar Double output parameter.

## Post-slice Pareto and priorities

The leading remaining MVP O0 root causes are:

| Root-cause cluster | Workloads | Cumulative failures covered |
| --- | ---: | ---: |
| Ordinary intrinsic `GenericAsm` semantics | 66 | 66 |
| Common wave/reconvergence `GenericAsm` semantics | 31 | 97 |
| Helper ABI type contract | 28 | 125 |
| Aggregate/pointer/layout transport | 23 | 148 |
| Ordinary numeric/bit operation | 16 | 164 |

O3 has the same preflight clusters; its additional eight narrow-integer runtime mismatches remain
a correctness cluster rather than an admission feature. The next implementation slice should
inventory the 66 ordinary `GenericAsm` rows by exact semantic operation and select a reusable typed
lowering group. Wave/reconvergence remains a separate convergence-contract slice. Remaining helper
types are now dominated by device/shared pointers, `RefParam`/read-only borrow semantics,
resource-view parameters, and deferred BFloat16/FP8 rather than finite aggregate transport.

## Representative gates and measurements

`dynamic-dispatch-bindless-texture`, `parameter-block`, and
`groupshared-multi-barrier-functional` remain correct through NVRTC O3 and direct NVVM O0/O3 in the
full census. Direct O3 outputs for each assemble with CUDA 12.9.86 `ptxas` for SM70, SM80, and
SM90. Runtime execution is on the local SM120 RTX 5090; physical SM70/80/90 execution and CUDA 13
remain infrastructure gaps.

| Workload | Route | Compile median | PTX bytes | Census compile/load/run/compare |
| --- | --- | ---: | ---: | ---: |
| Resource aggregate/helper | NVRTC O3 | 381.7 ms | 8,889 | 3,945 ms |
|  | Direct O0 | 266.6 ms | 6,102 | 3,713 ms |
|  | Direct O3 | 271.1 ms | 919 | 3,904 ms |
| Parameter-block layout | NVRTC O3 | 361.6 ms | 8,839 | 3,907 ms |
|  | Direct O0 | 242.1 ms | 917 | 3,762 ms |
|  | Direct O3 | 245.0 ms | 793 | 3,964 ms |
| Shared control/barriers | NVRTC O3 | 371.0 ms | 9,190 | 3,901 ms |
|  | Direct O0 | 247.6 ms | 1,940 | 3,632 ms |
|  | Direct O3 | 250.8 ms | 1,404 | 3,998 ms |

These startup-inclusive compiler and end-to-end harness numbers are exploratory, not kernel-only
performance measurements. Runtime behavior is differentially correct for all three gates, but a
kernel-only benchmark remains part of productionization.

## Validation

- Release `slang-unit-test` host target built successfully outside the sandbox.
- The isolated Release `slang-llvm-nvvm` provider rebuilt successfully; ABI remains revision 24.
- Focused existing, recursive, stale-negative, and unsupported-IR unit tests pass.
- The focused real fixture passes 3/3 native/direct O0/direct O3 lanes.
- The complete selected NVVM prefix passes 400/400.
- The full 452-workload NVRTC/direct O0/direct O3 census completed with zero old-correct
  regressions.
- All promoted direct CUDA lanes pass. A broader dynamic-layout prefix also exercised unrelated
  WGPU lanes and retains 15 existing Dawn/WGPU layout failures on this host; no NVVM lane failed.
- All three workload gates compile, assemble, and compare correctly; direct O3 assembly succeeds
  for SM70, SM80, and SM90.
- Pinned clang-format 17 and `git diff --check` run before commit.

The committed census TSV and cluster JSON are the authoritative post-slice evidence. Generated
mirrors, raw logs, PTX, cubins, the 51-row typed inventory, and measurement samples remain under
ignored `build/nvvm-census/`.
