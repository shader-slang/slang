# Slice 151 report: canonical resource helper results

## 1. Motivation

Consider a helper that returns a structured-buffer view:

```slang
RWStructuredBuffer<int> inputBuffer;

RWStructuredBuffer<int> getInputBuffer()
{
    return inputBuffer;
}

int test(int index)
{
    return getInputBuffer()[index];
}
```

After specialization and linking, `getInputBuffer` remains an ordinary `IRFunc` whose exact result
type is `RWStructuredBuffer<int>`. Direct NVVM already accepted that type as an entry/global value,
a helper parameter, and a call operand. It lowered the view to the canonical LLVM struct containing
the global element pointer and 64-bit count. The helper-result role alone rejected it before any
provider mutation.

The discovery corpus contained this raw-buffer case and a `Texture2D` result case. Frozen corpus v1
also contained a helper returning `RWStructuredBuffer<vector<half,2>>`. All three first stopped at
`_validateNVVMHelperTarget` with the same role diagnostic even though the existing generic builder
could already declare, call, and return their physical types.

## 2. Proposed solution

Admit only the two resource-value families proved by the measured canonical signatures:

1. Reuse `getNVVMSupportedRawBufferType` for structured and byte-address buffer helper results.
2. Reuse `getNVVMSupportedReadOnlyTextureType` for sampled-texture helper results.
3. Express the same set in `_isSupportedNVVMHelperResultType` and the
   `NVVMTypeUse::HelperResult` legality branch.
4. Keep the existing physical lowering and generic function/call/return path unchanged.

The fake provider now recognizes its already-existing resource-view type as a legal generic
function result and checks call and return values against that exact handle. This is test harness
coverage, not a provider callback or ABI revision.

## 3. Change summary

- `source/slang/slang-emit-nvvm.cpp`
  - admits exact raw-buffer and read-only-texture helper results in linked-IR preflight;
  - prints fixed-array element/count information in canonical type diagnostics;
  - reports the exact rejected type for unsupported loads.
- `source/slang/slang-emit-nvvm-type-lowering.cpp`
  - admits the same two families for the role-sensitive helper-result representation.
- `tools/slang-unit-test/unit-test-nvvm-support.h` and
  `tools/slang-unit-test/unit-test-nvvm-emitter.cpp`
  - model resource-view function results in the strict fake provider;
  - add named no-inline helpers returning `RWStructuredBuffer<float>` and `Texture2D`;
  - prove exact declaration, call, extraction, return, and downstream texture/store use.
- `tests/optimization/func-resource-result/func-resource-result-simple.slang`
  - adds stable direct-NVVM O0 and O3 differential lanes.
- `issue-nvvm-backend/summarize-compute-discovery.py`
  - owns the exact `IRLoad<Array<Texture2D, 2>>` cascade at its conventional-global producer.
- Slice 151 frozen/discovery TSV, cluster JSON, measurement manifest, report, plan, design, and
  capability ledger retain the separate coverage evidence.

## 4. Concepts and vocabulary

- **Resource view:** the first-class executable value for a raw buffer. Its provider representation
  is a struct containing a typed global data pointer and an i64 element/byte count.
- **Role-sensitive lowering:** one canonical Slang type can be legal in some producer/consumer
  roles and illegal in others. Type lowering checks the role before consulting cached handles.
- **Advanced row:** a workload whose former first blocker is removed but which reaches a different
  unsupported canonical shape. It is not counted as correct or promoted.

## 5. Process report

### The helper-result rejection duplicated an already-proved representation

`_validateNVVMHelperTarget` sees the final linked helper signature and calls
`_isSupportedNVVMHelperResultType`. That function previously accepted selected helper values and
pointers but omitted resources. Later, `NVVMTypeLoweringContext::lowerType` independently rejected
the same source type for `HelperResult`, even though its ordinary-value branches already call
`_lowerRawBufferType` or produce an i64 sampled-texture handle.

The input shapes are canonical and intentionally allowed. Resource result types are produced by
ordinary post-specialization `IRFunc` signatures, not by a source-syntax reconstruction. The two
existing classifiers prove the precise raw-buffer element/layout/access contract or read-only
texture shape. Both gates now reuse those classifiers. Removing either widening returns
`func-resource-result-simple.slang` to the exact helper-result diagnostic.

No new provider representation exists. Function declaration passes the existing type handle to
`NVVMIRBuilder::getFunctionType`. `_emitNVVMFunctionValueReturn` returns the existing SSA handle,
and callers use generic `emitCall`; a raw-buffer call result is then consumed by the established
aggregate-element extraction and pointer-offset path. Provider ABI revision 30 remains unchanged.

### Focused coverage protects the exact boundary

The first fixture version used trivial identity helpers. Normal specialization erased them, so
aggregate call counts did not prove helper-result transport. The retained fixture marks the two
helpers `[noinline]`, locates them by their real mangled names, and checks their exact function type,
no-inline flag, call result kind, raw-view extraction, texture operation, return, and final store.

The fake provider's `ResourceView` result kind survives because it preserves strict type checking
for the production generic path. `getFunctionType` accepts only an existing fake resource-view
handle. Calls record the exact result handle, returns compare the value to that handle, and raw-view
extraction recovers the element type from the call result. This is not a production fallback and
does not make different resource types interchangeable.

### Two candidates expose independent later blockers

The simple discovery workload becomes correct at O0 and O3. The other two candidates advance:

- `func-resource-result-complex.slang` now reaches
  `IRLoad<Array<Texture2D, 2>>`. `collectEntryPointUniforms` places the texture array in the
  synthesized `GlobalParams` struct; the helper loads the complete array and applies `getElement`
  before returning a texture. First-class resource-array loading is independent of helper result
  transport and remains unsupported.
- `compute/reinterpret-structured-buffer.slang` now reaches a field address whose exact result is
  `Ptr<DescriptorHandle, ..., ScalarLayout>`. The handle wrapper and aggregate pointer/layout
  representation own that failure, not the helper ABI.

Neither shape is admitted in this slice. The generic load diagnostic originally emitted only
`load result type`, which was insufficient for the required producer/type/operation
deduplication. `_diagnoseUnsupportedIRType` now prints the exact array element and count, yielding
`load result type: Array<Texture2D, 2>`. The strict discovery summarizer maps that exact family to
`collectEntryPointUniforms -> synthesized GlobalParams resource-array field -> IRLoad` and rejects
unknown preflight shapes.

### Coverage remains split across frozen v1 and discovery

Frozen corpus v1 remains exactly 452 workloads with 427 healthy MVP references:

| Frozen corpus-v1 metric | Slice 150 | Slice 151 |
|---|---:|---:|
| Direct O0 correct | 372/427 | 372/427 (87.1%) |
| Direct O3 correct | 376/427 | 376/427 (88.1%) |
| Correct in both modes | 372/427 | 372/427 (87.1%) |
| Newly correct in both modes | - | 0 |
| Old-correct regressions | - | 0 |
| Selected NVVM regression prefix | 426/426 | 427/427 |

Across all frozen rows, native NVRTC O3 remains 449 correct and three infrastructure results.
Direct O0 remains 385 correct, 52 preflight, eight runtime mismatches, and seven provider failures;
direct O3 remains 390 correct, 52 preflight, eight runtime mismatches, and two provider failures.
The advanced reinterpret row moves from `helper-abi-type-contract` to
`aggregate-pointer-layout-transport`, changing those O0/O3 cluster counts from 10/6 to 9/7 without
changing correctness.

The separate discovery corpus remains 82 workloads with 72 healthy references:

| Discovery metric | Slice 150 | Slice 151 |
|---|---:|---:|
| Direct O0 correct | 51/72 | 52/72 (72.2%) |
| Direct O3 correct | 51/72 | 52/72 (72.2%) |
| Correct in both modes | 51/72 | 52/72 (72.2%) |
| Newly correct in both modes | - | 1 |
| Old-correct regressions | - | 0 |

The newly correct identity is
`optimization/func-resource-result/func-resource-result-simple.slang#discovery-1`. Across all 82
rows, native NVRTC O3 remains 72 correct, two runtime mismatches, and eight infrastructure results.
Each direct mode now has 52 correct, 21 preflight, one provider, seven infrastructure, and one
runtime-mismatch result. The previous two-row helper-resource-result cluster leaves one correct row
and one separately owned resource-array load row. The corpus denominators are not combined, and no
corpus v2 is proposed.

The promoted O0/O3 lanes pass 1/1 each. Running the complete shader prefix also observes an
unrelated existing WGPU synthetic-lane bind-group failure; native CUDA and both direct lanes pass.

### Exploratory performance and architecture evidence

Three standalone compilations of the new raw-buffer-result workload give:

| Configuration | Median compile | PTX size | Cubin size |
|---|---:|---:|---:|
| NVRTC O3 native | 357.8 ms | 8705 B | 13664 B |
| Direct NVVM O0 SM70 | 238.9 ms | 2541 B | 3688 B |
| Direct NVVM O3 SM70 | 243.5 ms | 732 B | 2792 B |
| Direct NVVM O3 SM80 | 243.5 ms | 732 B | 2920 B |
| Direct NVVM O3 SM90 | 248.3 ms | 732 B | 3360 B |

CUDA 12.9 `ptxas` accepts direct O3 PTX for this kernel and all nine established discovery gates at
SM70, SM80, and SM90. Census end-to-end times include compilation, loading, execution, and
comparison; these remain uncontrolled exploratory measurements. CUDA 13 and physical SM70/SM80/
SM90 runtime workers remain open productionization requirements.

### Self-review inventory

- The two helper-result classifier calls survive. They reuse canonical type classification and are
  required by the newly correct raw-buffer workload and the focused raw/texture fixture.
- The matching `HelperResult` legality terms survive. Role checking occurs before type-cache reuse,
  preventing a legal body value from accidentally widening a forbidden signature.
- The fake `ResourceView` result kind and call-value recognition survive. They are limited to the
  test provider, preserve exact handle checks, and fail the focused test if removed.
- Fixed-array diagnostic spelling survives. It records canonical element/count information for the
  newly exposed load and does not affect accepted IR.
- The discovery cluster case survives. It names the exact producer chain for
  `Array<Texture2D, 2>` and the summarizer continues to reject unknown preflight shapes.
- The promotion survives. It adds a new semantic helper-result combination, is deterministic, and
  passes real O0/O3 differential execution.

The diff adds no fixture-name check, syntax reconstruction, custom semantic equivalence,
compatibility fallback, provider callback, or downstream repair of malformed IR.
The installed `clang-format` 21.1.8 is outside the repository's accepted 17/18 range, so formatting
was reviewed manually and `git diff --check` is clean.
