# Slice 147 report: separate compute discovery corpus

## 1. Motivation

The fixed direct-NVVM census had become a useful regression contract, but its progress no longer
answered whether the implemented representations generalized beyond the 448 source files already
represented there. The 452 corpus-v1 workloads and 427 healthy-MVP denominator must remain fixed;
adding new shaders to that table would make later percentages incomparable with Slice 146.

Consider this existing repository test:

```slang
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type

ParameterBlock<Scene> scene;

[numthreads(1, 1, 1)]
void computeMain()
{
    // The checked source supplies deterministic resources and expected output.
}
```

It has no checked-in CUDA lane and therefore is absent from corpus v1, but it already owns the
entry point, input bindings, and expected output needed for a differential CUDA experiment. Adding
permanent direct directives before knowing whether this semantic combination was useful would
turn discovery into fixture promotion. Reconstructing a new source or expected buffer would lose
the repository test's established contract.

Slice 147 establishes a separate rolling discovery corpus. It measures generalization and changes
no compiler, provider, builder ABI, or shader directive.

## 2. Proposed solution

Select 82 repository-local sources before observing their direct result. The checked-in manifest
records one existing active compare-compute directive by source-test ordinal, semantic-combination
tags, and a selection rationale. The runner then:

1. verifies that the frozen Slice 146 table still has exactly 452 workloads, 448 sources, a 427
   healthy-MVP denominator, and 371/375/371 O0/O3/both correctness;
2. rejects any discovery source already represented in that frozen identity set;
3. removes only a bounded set of target/profile/emission arguments from the selected directive and
   appends native CUDA in a disposable hard-linked mirror;
4. preserves source text, `TEST_INPUT` records, compare command, FileCheck buffer, and indexed
   expected-output sidecar;
5. derives native NVRTC O3 and direct NVVM O0/O3 lanes through the established mode builder;
6. classifies and records every lane, then derives exact failure shape, producer, diagnostic, and
   Pareto cluster in separate discovery TSV/JSON artifacts.

E36107 target-capability rejection precedes harness expected/actual text in phase order, so the
discovery classifier records it as infrastructure rather than a runtime mismatch. A native lane
must be correct before its workload enters the healthy discovery denominator.

## 3. Change summary

- `issue-nvvm-backend/discovery-corpus.manifest.tsv`
  - selects 82 non-v1 workloads and documents the semantic combinations they represent;
  - contains no result or expected-failure field.
- `issue-nvvm-backend/run-compute-discovery.py`
  - mechanically audits frozen-v1 metrics and source exclusion;
  - resolves the exact existing directive contract and adapts only target execution arguments;
  - creates one disposable hard-linked test mirror and runs all three modes;
  - records E36107 capability failures before output comparison.
- `issue-nvvm-backend/summarize-compute-discovery.py`
  - joins raw results to selection evidence;
  - preserves the complete quoted preflight type/operation shape rather than only the normalized
    classifier family;
  - requires an audited producer mapping for every observed non-correct signature;
  - emits discovery-only coverage and Pareto evidence alongside a read-only copy of the current
    corpus-v1 cluster counts.
- `issue-nvvm-backend/discovery-census.slice-147.tsv` and
  `discovery-census.slice-147-clusters.json`
  - contain the 82-row matrix, separate denominators, exact diagnostics, producer ownership, and
    both O0/O3 Pareto distributions.
- `issue-nvvm-backend/measure-compute-mvp.py` and
  `discovery-metrics-workloads.slice-147.json`
  - retain the original default representative gates while allowing an explicit workload manifest;
  - reproduce exploratory compile-time, PTX-size, assembly, and end-to-end measurements for three
    larger discovery workloads.
- `docs/design/nvvm-backend.md` and `docs/design/nvvm-backend-capability-ledger.md`
  - record the two-corpus measurement contract, baseline, and cross-corpus priorities.

No source below `source/`, LLVM provider file, builder header, unit-test implementation, or shader
under `tests/` changes in this slice. Builder ABI remains revision 30.

## 4. Concepts and vocabulary

- **Corpus v1:** the frozen 452-workload historical regression census represented by
  `census.slice-146.tsv`; its healthy-MVP denominator is permanently 427.
- **Discovery corpus:** the rolling, explicitly selected non-v1 set used to test whether a change
  generalizes. Its denominator is reported independently and is not corpus v2.
- **Healthy reference:** a discovery workload whose generated native NVRTC O3 lane compiles, runs,
  and matches the existing expected-output contract.
- **Source-test ordinal:** the position of an active `//TEST:` directive in its source. It also
  selects the correct indexed expected-output sidecar.
- **Exact shape:** the complete first rejected canonical operation/type/role from the diagnostic,
  for example `helper function result type: RWStructuredBuffer<int>`, not merely the normalized
  `helper function result type` family.

## 5. Process report

### Selection is independent of direct-NVVM results

Repository enumeration found 1,272 compare-compute sources absent from corpus v1. Whole deferred or
target-specific families were not imported into the ordinary selection. The final 82 sources come
from compute, language-feature, bugs, bindings, IR, and optimization coverage already present in
the repository. The manifest represents overlapping combinations rather than 82 isolated opcodes:

| Selection theme | Workloads carrying the tag |
|---|---:|
| Helper or generic call graph | 39 |
| Aggregate or pointer transport | 37 |
| Mixed resources | 19 |
| Parameter or constant-buffer layout | 19 |
| Control flow | 17 |
| Larger source | 13 |
| Matrix/layout-sensitive storage | 10 |
| Shared memory or barriers | 7 |
| Atomic/wave combination | 3 |

Tags overlap by design. Examples include a large AnyValue aggregate-copy call graph, nested and
generic parameter blocks, resource-valued helper results, groupshared device pointers, coherent
pointer loads, array storage legalization, non-square matrices, and loop/SSA optimization tests.
There are 82 unique sources, 82 unique workloads, and zero source overlap with the 448 sources in
corpus v1. No external project or third-party shader was added.

The manifest has no direct outcome field, and discovery was run only after the set was complete.
This prevents pass-based selection. The runner also stops if an ordinal no longer names an active
compare-compute directive instead of silently selecting a neighboring fixture.

### The generated lane preserves the repository-owned semantic contract

For example, an implicit, CPU, or Vulkan execution directive may contain `-shaderobj`,
`-output-using-type`, compiler `-Xslang` arguments, and a FileCheck buffer. Target adaptation removes
only exact target selectors, target profile/capability, and target-specific SPIR-V/DXBC emission
flags. It retains those semantic and compiler arguments, adds `-cuda`, and then applies the same
O0/O3/direct selector logic used by corpus v1.

The generated source is the original source with execution directives removed and exactly one new
directive prepended. `TEST_INPUT`, imports, source declarations, checks, and the selected ordinal's
expected sidecar are unchanged. A hard-linked copy of the complete test tree preserves relative
module lookup while avoiding three physical copies; only newly generated sibling files are
written. The mirror lives below `build/nvvm-discovery/` and is disposable.

### Native-reference health is an explicit gate

The complete selection produces these raw classifications:

| Route | Correct | Runtime mismatch | Slang NVVM preflight | Provider/libNVVM | Infrastructure/toolchain |
|---|---:|---:|---:|---:|---:|
| Native NVRTC O3 | 72 | 2 | 0 | 0 | 8 |
| Direct NVVM O0 | 45 | 1 | 28 | 1 | 7 |
| Direct NVVM O3 | 45 | 1 | 28 | 1 | 7 |

Seven sources use entry-point features unavailable to the CUDA target and produce E36107 through
all routes. One multisampled-surface source reaches CUDA emission but NVRTC 12.9 rejects the
generated resource declarations. Two native lanes execute but disagree with target-independent
expected output: one non-square column-major result is `11, 1` instead of `11, 22`, and one texture
dimension query lacks values CUDA does not report. None enters the healthy denominator.

The healthy discovery reference denominator is therefore 72. Over that fixed Slice 147 discovery
set, direct O0 is correct for 45, direct O3 is correct for 45, and the same 45 are correct in both
modes (62.5%). The other 27 healthy rows stop at 26 preflight shapes and one provider operation;
there is no healthy-reference runtime mismatch. O0 and O3 identities are identical. These numbers
remain separate from corpus v1 and are not combined into a headline percentage.

Slice 147 unlocks zero workloads because it intentionally changes no backend behavior. The 45
both-mode successes are the discovery baseline against which later slices report newly unlocked
identities.

### Every failure has an exact phase, shape, producer, and diagnostic

`summarize-compute-discovery.py` extracts the full shape inside E52017 rather than losing its type
suffix. It has no generic success-producing fallback: a new preflight or provider shape without a
producer audit stops summary generation. The healthy-reference direct Pareto is:

| Exact root-cause cluster | Healthy rows blocked | All selected rows | Canonical producer and owner |
|---|---:|---:|---|
| Struct-field pointer transport | 7 | 8 | `IRBuilder::emitFieldAddress` -> `IRFieldAddress`; `_validateNVVMFunction` |
| Array-element pointer relation | 2 | 2 | `IRBuilder::emitElementAddress` -> `IRGetElementPtr`; `_validateNVVMFunction` |
| Device load-to-load pointer chain | 2 | 2 | parameter/global pointer lowering -> `IRLoad`; `_validateNVVMPointerValue` |
| Entry-point parameter ABI | 2 | 2 | entry-uniform collection/specialization -> linked `IRFunc` parameter |
| Function identity | 2 | 2 | linkage decorations -> `_getNVVMFunctionName`; `_collectNVVMFunctionNames` |
| Helper aggregate parameter ABI | 2 | 2 | specialized `IRFunc` signature -> `_validateNVVMHelperTarget` |
| Helper pointer parameter ABI | 2 | 2 | specialized pointer signature -> `_validateNVVMHelperTarget` |
| Helper resource result ABI | 2 | 2 | specialized resource result -> `_validateNVVMHelperTarget` |
| Sequential aggregate pointer | 1 | 1 | typed `IRGetElementPtr`; `_getNVVMSequentialElementPointer` |
| Aggregate storage layout | 1 | 1 | buffer/entry lowering -> conventional global layout proof |
| AnyValue UInt64 reconstruction | 1 | 1 | AnyValue marshalling -> `emitMakeUInt64` |
| Fixed-array value construction | 1 | 1 | matrix legalization -> `emitMakeArray` |
| Helper aggregate result ABI | 1 | 1 | specialized aggregate result -> `_validateNVVMHelperTarget` |
| Provider global-user-pointer cast | 1 | 1 | `_convertGlobalNVVMPointerToUserPointer` -> provider address-space cast |

The all-selected Pareto additionally retains seven capability-infrastructure rows, a native-unhealthy
texture-query `IRGenericAsm`, and a native-unhealthy matrix runtime mismatch. The TSV names every
source, exact type string, diagnostic, producer, and raw log. The JSON deduplicates by canonical
producer/type/operation shape and retains examples only as evidence, never as dispatch keys.

### Discovery confirms and sharpens corpus-v1 priorities

Corpus v1 remains frozen at:

| Frozen corpus-v1 metric | Slice 146 value |
|---|---:|
| Workloads | 452 |
| Healthy MVP denominator | 427 |
| Direct O0 correct | 371/427 (86.9%) |
| Direct O3 correct | 375/427 (87.8%) |
| Correct in both modes | 371/427 (86.9%) |
| Selected NVVM regression prefix | 424/424 |

Its leading healthy-MVP clusters are aggregate/pointer/layout, helper ABI/type, and other exact
preflight shapes at eight rows each, followed by function identity at six. Discovery independently
finds 13 healthy first blockers across struct-field, array/sequential, device-pointer, and storage
layout shapes, plus seven helper-ABI blockers and two function-identity blockers. The denominators
are not added, but the ranking agrees: aggregate/pointer representation and helper ABI are the two
largest reusable boundaries in both corpora.

The exact discovery decomposition suggests the next vertical slice should not be another isolated
fixture operation. It should establish one canonical nested aggregate/pointer representation that
covers field addresses, element addresses, and their storage/address-space relation where the
types prove the same invariant. Helper resource/aggregate/pointer ABI is the next cross-corpus
family. Entry-point parameter ABI and function identity remain smaller independent candidates.

Frozen both-mode correctness is 86.9%, below the approximate 90% milestone proposed for considering
a deduplicated corpus v2. This slice therefore does not propose or declare corpus v2.

### Performance evidence remains exploratory

Three larger discovery workloads that are correct in all modes were compiled three times per
configuration. CUDA 12.9 `ptxas` accepted every direct O3 result for SM70, SM80, and SM90:

| Discovery workload | NVRTC O3 median / PTX | Direct O0 SM70 median / PTX | Direct O3 SM70 median / PTX |
|---|---:|---:|---:|
| Generic diamond call graph | 380.8 ms / 8726 B | 262.6 ms / 7432 B | 274.4 ms / 707 B |
| Generic arithmetic + matrix | 427.9 ms / 9168 B | 302.9 ms / 25374 B | 306.9 ms / 1171 B |
| Large loop control flow | 381.6 ms / 8586 B | 257.1 ms / 4302 B | 266.9 ms / 647 B |

NVRTC produced SM75 PTX on this host; direct targets were explicit. The corresponding isolated
census compile/load/run/compare times are approximately 4.8--5.5 seconds per lane and are not
kernel-only runtimes. Runtime comparison remains on the local SM120 GPU. The samples are useful for
spotting gross changes, but cache state, process startup, and target differences are uncontrolled;
they are not production benchmark claims. CUDA 13 tooling and physical SM70/SM80/SM90 runtime
workers remain open productionization requirements.

### Self-review inventory

- The explicit manifest survives. It is the result-independent source of truth for a rolling,
  reviewable selection and contains no pass/fail branch.
- The bounded target adapter survives. Every removed token is an execution target/profile/emission
  option; source, inputs, semantic compiler arguments, and expected output remain owned upstream by
  the existing test. The generated directive probe passes native, direct O0, and direct O3.
- The frozen-v1 audit survives. It rejects row/source/healthy-denominator or historical-numerator
  drift before discovery runs and rejects source overlap.
- E36107 phase ordering survives. The entry point never compiled or executed, so treating the
  following empty expected/actual buffer as a runtime mismatch would be false evidence.
- Every producer map survives. Summary generation fails for an unmapped preflight/provider shape;
  current mappings follow the named canonical IR constructor/pass and direct validation consumer.
- The hard-linked mirror survives. It changes only census performance and writes no canonical test
  source; path validation restricts deletion to `build/nvvm-discovery/mirror`.
- The optional measurement manifest survives. The established MVP gate list remains the default;
  explicit discovery selection only parameterizes an existing measurement workflow.

The diff contains no AST/IR equivalence helper, syntax reconstruction, emitter fallback, fixture
name check, compatibility path, diagnostic weakening, provider callback, or accepted IR shape. No
production self-review item requires a producer-side fix because production code is unchanged.
