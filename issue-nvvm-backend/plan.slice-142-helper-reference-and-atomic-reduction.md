# Generalize helper references and lower common atomic reductions

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires this experimental branch's slice plan to be committed with its implementation, which is
an exception to the repository's default active-plan lifetime policy.

## Purpose and Observable Result

After this slice, one canonical helper-reference contract transports selected pointer pointees
through direct helper calls without erasing their source address space accidentally. The first
vertical consumer is the CUDA atomic-reduction family produced by `core.meta.slang`: common
integer and floating additions plus integer min/max/bitwise/inc/dec reductions lower through one
typed atomic descriptor instead of remaining `GenericAsm`.

The bounded target population is the seven healthy-MVP helper rows whose canonical parameters are
`RefParam<T>`, `BorrowInParam<T>`, or an explicit groupshared `Ptr<T>`: five atomic-reduction
workloads, `pointer/const-ref.slang`, and `pointer/ptr-to-groupshared-1.slang`. Every workload that
becomes correct at direct O0 and O3 is promoted. Later independent blockers remain measured rather
than broadening this slice.

## Progress

- [x] (2026-08-30) Committed Slice 141 as `a3367ba7e` with eight both-mode gains and refreshed the
  fixed 452-workload Pareto.
- [x] (2026-08-30) Decomposed the leading 16-row healthy helper-ABI cluster. Seven rows share
  pointer-bearing helper parameters: five `RefParam`, one immutable `BorrowInParam`, and one
  explicit groupshared `Ptr`.
- [x] (2026-08-30) Traced final linked integer-reduction IR. `RWStructuredBufferGetElementPtr`
  produces `Ptr<T, Generic, ScalarLayout>` and calls an exact `RefParam<T, Generic,
  DefaultLayout>` helper whose body is one bounded `__slang_atomic_reduce_*` `IRGenericAsm`.
- [x] (2026-08-30) Established one role-aware helper-reference classifier, call relation, address-space
  conversion, type lowering, validation, and focused positive/negative coverage.
- [x] (2026-08-30) Lowered the exact common atomic-reduction assembly/full-signature family through the existing
  generic atomic provider operation, widening its established semantic catalog only as proven.
- [x] (2026-08-30) Probed all seven workloads, promoted three exact successes, regenerated the fixed census/Pareto and
  representative metrics, self-review, document, validate, and commit Slice 142.

## Surprises and Discoveries

- A source `__ref T` is not malformed final IR. Specialization preserves it as a four-operand
  `RefParam(T, ReadWrite, Generic, DefaultLayout)`, while a structured-buffer subscript is a
  four-operand `Ptr(T, ReadWrite, Generic, ScalarLayout)`. They differ semantically in pointer role
  and layout metadata but both lower to an LLVM pointer to the same physical `T`.
- The existing call relation already distinguishes local roots from derived resource pointers and
  validates their producer chains. The missing piece is a canonical helper-reference parameter
  classifier plus an explicit physical address-space conversion when a global/shared producer is
  passed to a generic helper parameter.
- Atomic reduction helpers are final target-specialized one-block `IRGenericAsm` definitions, not
  ordinary `IRAtomic*` instructions. Exact assembly and complete signature must choose their
  semantic descriptor, as with other bounded CUDA helper families.
- The builder ABI already exposes add, subtract, bitwise, min, max, and exchange operation IDs in
  one typed atomic callback. Revision 29 need not change unless real provider validation proves a
  concrete reduction form cannot be represented by that callback.
- LLVM 14 represents Float32/Float64 atomic add as typed `atomicrmw fadd`, but CUDA's LLVM-7-era
  NVVM reader rejects that operation. The provider must preserve the typed LLVM operation while
  translating only its exact global scalar serialization to the corresponding legacy NVVM
  intrinsic at the existing dialect boundary.
- The bounded seven-row probe yields three both-mode successes. Half reduction remains an exact
  unsupported scalar-Half reduction helper; the two method fixtures next stop at ordinary
  `atomicLoad`; immutable struct reference transport next stops at canonical struct field
  addressing. Those are independent clusters, not reasons to weaken this slice's contracts.
- The first complete selected-prefix run caught stale builder-test expectations for signed/wide
  and floating atomic descriptors. Updating that contract test to assert the new positive family
  and adjacent negative cases brought the final prefix to 421/421.

## Decision Log

- Decision: Take helper references and atomic reductions as one vertical slice.
  Rationale: Merely admitting `RefParam` would move five important tests to a later `GenericAsm`
  diagnostic without making them usable. The canonical reference ABI plus its largest measured
  consumer establishes a reusable representation and end-to-end runtime result.
  Date/author: 2026-08-30, Codex.
- Decision: Keep ordinary atomic load/store/compare-exchange outside this slice.
  Rationale: The current provider callback represents typed read-modify-write operations. Atomic
  load, store, and compare-exchange demonstrate a separate concrete interface question and must
  not be approximated by stronger or semantically different RMW operations merely to unlock a
  fixture.
  Date/author: 2026-08-30, Codex.
- Decision: Classify the complete final signature and producer role, never helper or fixture name.
  Rationale: `RefParam`, `BorrowInParam`, structured-buffer element pointers, local roots, and
  groupshared pointers carry distinct mutability/provenance even when their LLVM pointer width is
  identical. Those canonical types and producers are the source of truth.
  Date/author: 2026-08-30, Codex.
- Decision: Reuse the existing generic atomic callback before revising ABI 29.
  Rationale: The operation enum, type descriptor, address space, memory order, pointer, value, and
  result already express the reduction primitive. Reduction simply discards the old value.
  Date/author: 2026-08-30, Codex.
- Decision: Keep floating atomic semantics typed through the provider and translate only the
  final legacy NVVM serialization.
  Rationale: LLVM 14 can represent the canonical operation correctly, while libNVVM's LLVM-7-era
  parser requires the exact `llvm.nvvm.atomic.load.add` intrinsic spelling. This is the established
  provider dialect boundary, not compiler-side text reconstruction or a compatibility fallback.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

Complete. Exact generic read/write and borrowed helper references now lower to generic provider
pointers; producer-proven global arguments are widened only at the call boundary and recovered as
global inside an admitted atomic-reduction helper. Exact groupshared numeric helper pointers retain
address space three. The established atomic callback now covers relaxed global scalar 32/64-bit
integer add/and/or/xor/min/max and Float32/Float64 add. Inc/dec and subtraction are compiler-owned
typed recipes over that family. Provider ABI revision 29 remains unchanged.

Three of seven bounded workloads become correct at O0 and O3: `atomic-reduce-float`,
`atomic-reduce-intrinsics`, and `ptr-to-groupshared-1`. The other four report their independent
first blockers: Half reduction GenericAsm, two ordinary `atomicLoad` operations, and struct field
addressing. The fixed 452-row census reaches 341 O0 and 346 O3 successes, exact +3/+3 with zero
old-correct loss. Against 427 healthy MVP references, O0/O3/both correctness is 339/343/339
(79.4%/80.3%/79.4%). The selected prefix passes 421/421, all 12 active lanes in the three promoted
fixture files pass, and representative O3 PTX assembles with CUDA 12.9 for SM70, SM80, and SM90.

The leading healthy-MVP blockers are aggregate/pointer/layout transport (12), preflight other
(11), atomic/wave operations (10), residual target markers (10), helper ABI/type contracts (9),
and wave/reconvergence GenericAsm (8). The next slice should decompose the aggregate/pointer/layout
cluster by canonical producer and choose the largest reusable representation, while keeping
ordinary atomic load/store/compare-exchange as a separate concrete interface decision.

## Context and Current Pipeline

Consider:

```slang
__atomic_reduce_add(target[0], 1u);
```

`hlsl.meta.slang`/`core.meta.slang` declare `__atomic_reduce_add` with `__ref T`. Specialization and
linking produce a helper with signature:

```text
Func<Void, RefParam<UInt, Generic, ReadWrite, DefaultLayout>, UInt, Int>
```

Its body is exactly:

```text
GenericAsm("__slang_atomic_reduce_add($0, $1, (int)$2)")
```

The caller loads the structured-buffer descriptor,
`RWStructuredBufferGetElementPtr` produces a canonical writable element pointer with scalar
resource layout, and the direct call supplies that pointer, a typed value, and a relaxed
`MemoryOrder` literal. `_validateNVVMHelperTarget` currently rejects the `RefParam` before the
consumer can classify the exact assembly.

When admitted, helper parameters are lowered by role in `NVVMTypeLoweringContext`. Calls use
`_isSupportedNVVMHelperArgumentType` and `_getLoweredNVVMHelperValue`; pointer validation separately
proves whether the argument comes from local, resource, global, or shared storage. The new
reference contract must remain consistent through all four stages.

Atomic reduction does not return the old value, but its memory effect is the same typed relaxed
atomic RMW already modeled by `SlangNVVMAtomicOperationDesc`. The emitter can invoke that operation
and discard its returned original value. Inc/dec are add with typed `+1`/`-1`; source reduction
subtraction is already specialized as add of a negated value. No text reaches LLVM.

## Scope and Non-Goals

In scope:

- exact generic `RefParam<T>` and `BorrowInParam<T>` helper parameters over selected finite
  pointees, preserving read-write versus read-only access;
- exact explicit groupshared helper pointer transport for selected finite pointees;
- exact call argument/pointee relations and provider address-space conversion at the helper ABI
  boundary when required by canonical producer provenance;
- exact final atomic-reduction assembly/full-signature classification;
- relaxed add for proven scalar floating/integer forms and measured half/vector forms only if the
  existing callback can represent them correctly;
- relaxed integer min/max/and/or/xor and inc/dec through typed operations;
- focused fake/real/negative/ptxas tests, seven-row probe, promotions, fixed census and metrics.

Out of scope:

- atomic load/store/compare-exchange or compatibility emulation through unrelated RMW operations;
- unrestricted pointer kinds, arbitrary address-space erasure, mismatched pointees/layouts, or
  reconstructing source `ref` syntax;
- BFloat16, FP8, extension-tier vector atomic reductions unless an MVP workload proves ownership;
- dynamic parallelism, RDC/LTO, fixture checks, source-name checks, downstream malformed-IR
  patches, or provider ABI revision 30 without a demonstrated interface gap.

## Architecture and Invariants

- The final linked pointer type and producer role are canonical. A helper reference is admitted
  only for one exact pointer op/mutability/address-space/layout contract and a selected pointee.
- Call compatibility compares canonical pointees and mutability. It does not use a custom
  structural equivalence relation or ignore address-space provenance.
- Global/shared-to-generic conversion occurs only at the helper call boundary after the argument
  producer has proven its physical address space. Ordinary resource/global/shared loads, stores,
  and atomics retain their physical address-space types.
- Atomic reduction recognition requires a canonical final one-block value-less helper, an exact
  assembly spelling, exact `Void(reference<T>, T, Int)` or inc/dec signature, and a relaxed
  executable memory-order literal. Adjacent signatures remain deterministic failures.
- The operation descriptor records exact type, operation, physical/generic address space, and
  order. Preflight and emission consume the same descriptor.
- Provider capability is queried before module construction. An unsupported type/operation pair
  remains preflight, never an opportunistic fallback.

## Interfaces and Dependencies

Expected compiler changes are in `source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` and
`source/slang/slang-emit-nvvm.cpp`. Shared atomic legality lives in
`source/compiler-core/slang-nvvm-semantic-catalog.h`; the isolated LLVM provider changes only if
the existing callback needs implementation widening for an admitted descriptor. ABI revision 29
remains the default expectation.

Focused sources and fake graph assertions live in `tools/slang-unit-test/unit-test-nvvm-support.h`
and `unit-test-nvvm-emitter.cpp`; real differential and `ptxas` coverage live in
`unit-test-nvvm-integration.cpp`. Promoted corpus lanes remain beside native CUDA directives.

## Milestones

1. Inventory the seven final helper signatures, call argument pointer shapes, and immediate helper
   bodies. Group exact admitted and adjacent rejected forms by pointer role and atomic semantic.
2. Add a single helper-reference classifier and route helper admission, type lowering, call
   compatibility, pointer validation, and call emission/address-space conversion through it.
3. Add a complete atomic-reduction descriptor/resolver, operation requirements, and emitter using
   the existing atomic callback. Prove exact memory order, signature, type, and spelling.
4. Build host/provider and run focused fake/real/negative tests. Probe all seven rows at O0/O3;
   record later blockers and narrow any form the provider cannot compile correctly.
5. Promote exact successes, regenerate fixed census/Pareto and representative metrics, self-review
   every widening/special case, format, validate the selected prefix, document, and commit.

## Validation and Acceptance

All builds and tests run outside the sandbox. Acceptance requires focused positive and adjacent
negative helper-reference tests; fake-provider proof of exact call conversion and atomic
descriptors; O0/O3 differential runtime and real `ptxas`; all newly promoted CUDA lanes; an exact
seven-row probe; the fixed 452-workload census with zero old-correct regression; representative
metrics and direct O3 SM70/80/90 assembly; Release host/provider builds; the complete selected
prefix; pinned formatting; and `git.exe diff --check`.

## Failure and Recovery

If a pointer signature passes but its body reaches an unrelated operation, record that later
producer and leave it for its owning cluster. If global/shared-to-generic transport fails provider
verification, inspect the actual LLVM address-space boundary and fix the compiler-owned call
representation rather than weakening provider type checking. If one atomic descriptor cannot be
expressed by the current callback, retain a minimal real-provider proof before considering ABI 30.

All IR dumps, focused probes, PTX, cubins, and metrics remain under ignored
`build/nvvm-census/slice142-*` and are safe to regenerate.

## Self-Review

Inventory every new classifier, relation branch, conversion, recipe row, and provider case. For
each, record the exact canonical input, producer, why it is valid, and which test fails without it.
Reject syntax reconstruction, fixture names, pointer-layout erasure, silent defaults, and any
operation admitted only because it moves a diagnostic. Perform a revert drill on the shared
reference classifier and the atomic-reduction resolver.

## Artifacts and Hand-Off

Commit the completed plan with implementation, promoted fixtures, fixed Slice 142 census TSV and
cluster JSON, five-part report, and durable design/capability updates. Keep raw evidence below
`build/nvvm-census/`. The hand-off must re-rank helper ABI, aggregate/layout, residual marker,
ordinary atomic, and remaining wave clusters by healthy-MVP impact.
