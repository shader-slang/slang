# Materialize canonical undefined CUDA sampler placeholders

This ExecPlan follows `.agent/PLANS.md`. The NVVM workflow explicitly requires this completed plan
and its report to be committed with the slice; the integrating agent owns acceptance and commit.

## Purpose and Observable Result

Compile and execute a real texture SampleLevel call with a local uninitialized SamplerState,
including transport through noinline helpers, at NVVM O0/O3 and NVRTC O3. CUDA textures own sampling
state, so this source placeholder cannot affect independently expected sampled results.

## Progress

- [x] 2026-09-24: Read workflow, optimized checkpoint, slice 202, and local slang-build skill;
      verified clean base `60e2277f1522fd64a062960b43471d8a4ef33423` and healthy L4.
- [x] 2026-09-24: Audit canonical SSA producer and existing sampler classification; select bounded
      targeted regression domain before implementation.
- [x] 2026-09-24: Final focused source passes NVRTC O3 and four negative checks; direct O0/O3
      reject LoadFromUninitializedMemory. IR retains sampler undefined values, branch phi and helper call.
- [x] 2026-09-24: Extend existing resolver/emitter only and rebuild optimized compiler.
- [x] 2026-09-24: Final focused 17/17, runtime 4/4, units 473/473 (+one skip), toolkit
      18/18; exact 72 selected runtime cells and six complex cells completed.
- [x] 2026-09-24: Exact preservation comparison, final hashes, self-review, report and draft
      STATUS completed; parent owns final acceptance and commit.

## Surprises and Discoveries

`readVar`/`readVarRec` create canonical LoadFromUninitializedMemory. Its documented contract is a
consistent arbitrary value, akin to freeze(undefined). Existing lowering chooses concrete numeric
zeros but excludes SamplerState. `asNVVMSupportedSamplerValueType` already admits exactly ordinary
SamplerState (not comparison samplers). CUDA prelude documents the unused placeholder;
`_resolveNVVMTextureGenericAsm` validates its helper parameter then omits it from provider sampling.
The provider's existing sampler representation is i64. Resource-containing undefined aggregates
are a separate contract and will not be admitted by widening the copyable-value classification.

## Decision Log

- 2026-09-24, worker: Select sampler undefined support over independent wave transport because the
  complex material exhibits this exact valid producer and real bound textures provide an oracle.
- 2026-09-24, worker: Reuse ordinary sampler classification and generic integer constants; leave
  shared type lowering, provider ABI 35 and numeric copyable algebra unchanged. Revisit if the
  focused IR cannot preserve the exact shape or backend requires broader shared changes.

## Outcomes and Retrospective

The independently expected gradient/zero sampler fixture now passes all three runtime modes.
The actual revert drill on the identical fixture restores both direct failures. All 69 replayed
old cells retain outcomes and diagnostics; three added cells pass; 1,536 old cells remain inherited.
Both complex direct entries/modes now reach integer Texture2D.GetDimensions GenericAsm rejection.
No resource-aggregate, shared type lowering, provider or ABI change was needed. Scope is complete;
the integrating agent owns acceptance/commit. Implementation slices since full checkpoint: one
upon acceptance. The remaining material runtime contract is still unavailable.

## Context and Current Pipeline

The material's render.TextureHandle.sample reconstructs Texture2D<T> from a descriptor, declares
`SamplerState sampler;`, then passes it to render.ExplicitLodSampler.sample. SSA construction emits
LoadFromUninitializedMemory : SamplerState. `_resolveNVVMEphemeralValue` rejects it today. The
existing `_emitNVVMChosenUndefinedValue` owns one concrete choice cached in the SSA value map.
Texture SampleLevel resolution ignores sampler payload because CUDA texture objects own state.

## Scope and Non-Goals

Ordinary scalar SamplerState undefined values and real texture sampling only. No arbitrary resource
handles, comparison sampling, resource-containing undefined aggregates, provider/ABI changes,
source reconstruction, material runtime claims, wave transport or next independent blocker fixes.

## Architecture and Invariants

A canonical undefined instruction has one consistent selected value per instruction. A sampler
placeholder uses existing i64 lowering and does not create or initialize a CUDA texture. Resource
aggregates remain excluded from the numeric copyable algebra. All consumers share the existing
resolved ephemeral record. No new representation, helper, fallback or producer repair is expected.

## Interfaces and Dependencies

Native Linux RelWithDebInfo tools, CUDA 12.9.2, LLVM14 provider ABI35, L4 SM89 targeting SM80.
Use local build skill; env.sh defaults to Debug, so explicitly override all paths. Build with
`CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server`.
One suite at a time, at most four CPU workers; no performance experiments.

## Milestones

1. Add runnable fixture and negative probes, preserve fail-before logs under slice-203-before.
2. Extend existing resolver and emitter in source/slang/slang-emit-nvvm.cpp and rebuild.
3. Validate exact same focused inputs; add eligible source to discovery manifest without overlap.
4. Finish selected gates, retain per-cell outcomes/hashes and reassess both material entries.

## Validation and Acceptance

Inherited baseline: runtime-validation.optimized-checkpoint.json and its two outcome TSVs: 452
frozen plus 83 discovery identities, 1605 cells (1544 correct, 61 failures). This is historical
preservation evidence, not fresh execution on slice 203.

Targeted domain fixed before implementation: every frozen texture/sampler source; both
bugs/legalize-defuse-no-zero-init-non-var*.slang numeric undefined scalar/aggregate fixtures;
every discovery texture/sampler source; discovery compute/struct-make.slang and
compute/ssa-reduce-bug.slang neighboring aggregate SSA transport; both language-feature/types/opaque/*-opaque-type-in-struct
resource transport fixtures. Resolve exact identities into checked-in slice-203 subset TSVs and
manifest before execution. Run each at NVRTC O3, NVVM O0 and NVVM O3. Register new fixture separately.

Focused fixture uses real textures and independent expected output, initialized failure sentinels,
ordinary local samplers transported across helpers and branch/join boundaries. Negative coverage
must retain rejection of undefined real resources/resource aggregates and comparison samplers at
an appropriate compiler boundary. Do not dispatch undefined resource reads.

After final code change run: matching slang-test focused fixtures; validate-nvvm-runtime.py (4
fixtures); relevant NVVM/routing/reporter units; validate-nvvm-toolkit.py CUDA12.9 SM80 (18 cells);
selected run-compute-census.py and run-compute-discovery.py with --jobs 4; run-complex-corpus.py
--warmup 0 --samples 1 (all six cells). Bound suites with timeout and retain exit codes. Compare
exact keys, duplicates, omissions, execution counts and classifications to optimized baseline;
separate additions. Full checkpoint mandatory if shared lowering/ABI/provider changes or impact
cannot be bounded. No full replay claimed for this compiler-local admission change.

## Failure and Recovery

Stop GPU work on loss; no driver changes/reboot. Unexpected lost pass blocks acceptance. Retain
failed probe evidence and either fix the responsible boundary or revert. Revert drill consists of
fail-before against original compiler then pass-after on the identical focused source; broaden only
if a boundary is inseparable. Never weaken an oracle or resource rejection to move a diagnostic.

## Artifacts and Hand-Off

Raw logs, IR/PTX and binaries remain ignored under build/nvvm-loop/slice-203-{before,after}.
Durable plan, five-part report, runtime manifest, selected per-cell TSVs, design note and STATUS
record final source/binary hashes, fresh/inherited counts, full-checkpoint cadence and next blocker.
The worker does not commit; parent performs acceptance review and local commit.

## Implementation Notes

The exact targeted selection resolves to 16 frozen identities and seven old discovery identities,
plus one new discovery identity. The independent gradient oracle samples four corner centers and
expects `(xCorner, yCorner, 0, 1)`, plus a separately bound zero texture. A prototype noinline
sampler-return helper hit the independent unsupported helper-result classification; it was removed
rather than widening scope. Final fixture transports sampler arguments through existing helpers.
Pragma warnings 41016/41035 are disabled only for intentionally undefined placeholder reads.
Both resource negatives reach LoadFromUninitializedMemory rejection before dispatch.

The first post-change focused run passes 15/15, including real gradient/zero sampling in all three
modes. Explicit comparison-sampler helper input remains rejected as
`helper function parameter: SamplerComparisonState`; its two compile-only lanes are added separately.
For a literal revert drill, rebuild HEAD's original emitter, rerun the final formatted fixture and
all negatives, capture hashes, then restore the narrow change and rebuild before final acceptance.
No shared type lowering change is needed, so targeted scope remains sufficient.

Discovery execution adjustment: its loader requires 50--100 manifest contracts even for a targeted
run. The initial eight-row manifest invocation exited before execution. Keep the exact subset TSV
as the selection ledger; execute the unchanged full manifest with sequential `--match` batches:
`texture`, `sampler`, `opaque-type-in-struct`, `struct-make`, and `ssa-reduce-bug`. Merge their results
only after checking disjoint keys and exact equality to the eight predeclared identities. This uses
existing runner interfaces without altering selection/execution code or expanding the domain.

Parent integration review accepted the bounded compiler-layer change, targeted outcome comparisons,
final source/binary hashes, and explicit inherited obligations. Slice 203 advances the cadence to one.
