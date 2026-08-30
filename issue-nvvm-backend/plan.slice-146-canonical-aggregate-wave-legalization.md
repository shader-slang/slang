# Legalize canonical CUDA-prelude aggregate wave operations

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires this experimental branch's slice plan to be committed with its implementation, which is
an exception to the repository's default active-plan lifetime policy.

## Purpose and Observable Result

After this slice, the direct NVVM path consumes the canonical aggregate wave helpers emitted by
CUDA specialization as compiler-owned scalar recipes. A single recursive representation walks
homogeneous 32-bit numeric vectors and fixed arrays, applies the already-supported scalar shuffle,
reduction, or prefix operation to every leaf, and reconstructs or stores the exact aggregate.
Canonical scalar and `uint4` active-mask helpers use the existing ballot operation. The provider
ABI remains revision 30 unless a measured canonical operation cannot be expressed through its
generic value, control-flow, aggregate, pointer, and store operations.

The bounded population is the 22 Slice 145 census rows with one of these shared producers:

- four healthy-MVP aggregate shuffle rows and two extension matrix/reduction rows using an
  out-parameter aggregate ABI;
- four healthy/extension value-return aggregate reductions and the remaining observed aggregate
  reduction and prefix spellings from the same CUDA-prelude family;
- two healthy-MVP active/converged-mask rows.

The exact population will be recorded from the fixed census before implementation. Prefix-count
and wave-rotate helpers remain out of scope because their scalar semantics are different; they
stay independently diagnosed. Every bounded row that becomes correct at O0 and O3 is promoted.

## Progress

- [x] (2026-08-30) Committed Slice 145 as `7792585b6`, with eight both-mode gains, zero losses,
  423/423 selected tests, and 363/427 healthy-MVP both-mode correctness.
- [x] (2026-08-30) Compared the four tied leading healthy-MVP clusters by exact shape and selected
  the aggregate CUDA-prelude wave family because one representation covers 22 observed corpus
  rows across ordinary shuffle, reduction, prefix, and active-mask behavior.
- [x] (2026-08-30) Froze the 22-row bounded population and audited every exact assembly/signature spelling,
  canonical producer, scalar leaf type, and value-return versus out-parameter ABI.
- [x] (2026-08-30) Implemented one recursive aggregate wave recipe shared by preflight and emission, plus the
  exact active-mask recipes, using revision-30 generic operations.
- [x] (2026-08-30) Added focused positive and negative contract tests, probed all bounded rows at
  O0 and O3, and promoted every both-mode differential success.
- [x] (2026-08-30) Formatted, built, ran selected and promoted regressions, regenerated the complete
  fixed census and representative metrics, self-reviewed the full diff, and updated durable docs.

## Surprises and Discoveries

- Slice 145's `generic-asm-wave-reconvergence` label contains 25 correct-reference failures: eight
  MVP and seventeen extension rows. Exact final assembly decomposes it into 20 aggregate
  shuffle/reduction/prefix rows, two active-mask rows, one prefix-count row, and two rotate rows.
- Matrix helpers use a canonical `Void(T, ..., OutParam<T>)` ABI after matrix lowering, where `T`
  is `array<vector<leaf, columns>, rows>`. Vector helpers normally use a value-return `T(T, ...)`
  ABI. Both carry the same leaf-wise prelude semantics and differ only in result transport.
- The existing scalar masked-wave implementation already expresses reduction and prefix behavior
  through revision-30 value operations and compiler-generated loops. The missing abstraction is
  applying one validated scalar recipe recursively to a homogeneous aggregate, not a new LLVM
  intrinsic.
- Direct active-mask tests already model `__activemask()` portably as
  `ballot_sync(0xffffffff, true)`, so both scalar and `uint4` results can be constructed without an
  ABI revision.
- Matrix wave values reach the emitter as nested fixed-array/vector types. The homogeneous-leaf
  classifier must recurse through a semantic vector element instead of treating its lane count as
  a scalar leaf; matrix reductions and shuffles prove the distinction.
- A semantic-type query placed inside `SLANG_ASSERT` disappears in Release builds. The first
  matrix-shuffle probe exposed this immediately; the final resolver performs the query in ordinary
  checked control flow.
- Float32 aggregate prefix min/max compiled but produced invalid PTX until the scalar combine
  family's libdevice demand was propagated through aggregate preflight. The focused test now
  observes exactly one lazy libdevice module.
- Nineteen bounded workloads become correct. Three multi-operation workloads advance to their
  separate Float64 scalar masked-wave diagnostic, so they are measured later blockers rather than
  aggregate regressions.

## Decision Log

- Decision: Choose canonical aggregate wave legalization ahead of the tied aggregate-layout,
  helper-ABI, and miscellaneous-preflight buckets.
  Rationale: Exact decomposition reveals 22 rows behind one established CUDA-prelude convention,
  including all eight healthy-MVP rows in the headline cluster. It also exercises combinations of
  aggregate transport, helper ABI, control flow, and wave operations that isolated scalar tests
  do not cover.
  Date/author: 2026-08-30, Codex.
- Decision: Treat value-return and out-parameter aggregate helpers as two transports of one
  recursively validated semantic recipe.
  Rationale: Matrix lowering intentionally changes result transport while retaining an exact
  homogeneous aggregate type. The helper signature is the canonical source of truth; no source
  matrix syntax needs reconstruction.
  Date/author: 2026-08-30, Codex.
- Decision: Exclude prefix-count and rotate helpers from Slice 146.
  Rationale: They require different scalar recipes rather than the aggregate adapter. Keeping
  their existing exact diagnostics prevents this slice from becoming a grab bag of wave syntax.
  Date/author: 2026-08-30, Codex.
- Decision: Keep provider ABI revision 30 and express aggregate traversal entirely in the compiler.
  Rationale: The existing interface already supplies typed wave operations, extraction,
  construction, CFG, phi, store, and return. No concrete canonical LLVM operation is missing.
  Date/author: 2026-08-30, Codex.
- Decision: Stop the bounded workload at its newly exposed Float64 scalar-wave blocker.
  Rationale: Float64 scalar shuffling is a distinct representation problem. Accepting it here would
  broaden the selected homogeneous-32-bit aggregate invariant and hide the exact next Pareto item.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

The compiler now legalizes exact homogeneous selected-32-bit aggregate shuffle, reduction, prefix,
and converged-mask helpers with no provider change. Nineteen workloads gain correct O0 and O3
execution and receive 38 retained direct lanes; the remaining three bounded workloads expose exact
Float64 scalar-wave diagnostics. The fixed census reaches 384/452 O0 and 389/452 O3 successes with
zero old-correct identity loss. Among 427 healthy MVP references, O0/O3/both correctness reaches
371/375/371 (86.9%/87.8%/86.9%), removing every healthy-MVP wave/reconvergence failure.

The selected prefix passes 424/424 and all promoted files pass 57/57 CUDA lanes. Representative
direct O3 PTX remains accepted with CUDA 12.9 for SM70, SM80, and SM90. The detailed producer audit,
failure boundary, metrics, and self-review are retained in
`report.slice-146-canonical-aggregate-wave-legalization.md`.

## Context and Current Pipeline

Consider:

```slang
matrix<int, 2, 2> value = ...;
matrix<int, 2, 2> shuffled = WaveMaskReadLaneAt(mask, value, lane);
```

CUDA specialization selects the standard-module intrinsic assembly
`_waveShuffleMultiple($0, $1, $2)`. Matrix lowering represents the value as
`array<vector<int,2>,2>` and uses a void helper with an `OutParam` result. This is an intentional
canonical ABI, not a malformed substitute for a matrix. The established CUDA prelude implements
the operation by applying `__shfl_sync` to every homogeneous element.

Likewise:

```slang
int2 result = WaveMaskSum(mask, int2(lane, lane + 1));
```

specializes to `_waveSumMultiple($1.x, $0)` with a value-return `int2(int2, uint4)` helper. Direct
NVVM already validates and emits the exact scalar `_waveSum` recipe. It currently rejects the
aggregate helper only because `_resolveNVVMMaskedWaveScalarOperation` requires a scalar result and
the compound resolver only handles selected vector shuffle/all-equal forms.

`WaveGetConvergedMask` and `WaveGetConvergedMulti` produce exact `__activemask()` and
`make_uint4(__activemask(), 0, 0, 0)` terminal helpers. Existing direct wave-mask runtime coverage
proves the equivalent active mask from a full-mask ballot of `true`.

## Scope and Non-Goals

In scope:

- the exact aggregate shuffle, reduction, and prefix assembly/signature spellings present in the
  Slice 145 fixed census;
- homogeneous recursive vector/fixed-array values with selected 32-bit signed, unsigned, or
  floating scalar leaves;
- exact value-return and `OutParam<T>` result transport;
- exact scalar and `uint4` active-mask terminal helpers;
- one shared resolver for preflight requirements and emission;
- focused fake-provider tests, O0/O3 differential probes, promotions, full fixed census/Pareto,
  representative metrics, and durable design/capability updates.

Out of scope:

- wave rotate, clustered rotate, prefix count-bits, arbitrary inline assembly, or parsing generic
  assembly placeholders;
- unsupported scalar widths, heterogeneous structs, resources, pointers, or dynamically sized
  aggregate leaves;
- changing standard-module IR to suit this emitter or reconstructing source matrices;
- fixing an unrelated later blocker exposed by a bounded workload;
- fixture-name checks, compatibility fallbacks, or downstream repair for malformed upstream IR.

## Architecture and Invariants

1. The final `IRGenericAsm` text and complete specialized helper signature jointly identify a
   CUDA-prelude semantic. Text alone never widens an operation.
2. Aggregate legality follows canonical IR type structure. Every accepted leaf has one selected
   scalar semantic type, and every reconstructed value has the exact original lowered type.
3. One aggregate adapter owns traversal and transport. Scalar recipes remain the source of truth
   for identities, combine operations, lane selection, and prefix inclusion.
4. An out-parameter result is stored only through its validated `OutParam<T>` handle with the
   established executable alignment; it is not converted into a second value ABI.
5. Active-mask helpers use the already-supported ballot semantic and construct `uint4` through
   existing vector operations.
6. Preflight operation requirements and emission use the same resolved recipe. Any unknown
   spelling, signature, type relation, or transport fails before provider discovery.
7. Provider ABI revision 30 changes only if this exact canonical algebra exposes a concrete
   operation it cannot express correctly.

## Interfaces and Dependencies

Compiler work belongs primarily in `source/slang/slang-emit-nvvm.cpp`. Existing type lowering in
`source/slang/slang-emit-nvvm-type-lowering.*` should be widened only if an already-accepted helper
aggregate cannot be lowered through the value role. The existing `NVVMIRBuilder` operations for
value operations, element extraction, vector/aggregate construction, stores, branches, phis, and
returns are expected to suffice.

Focused sources and assertions belong in `tools/slang-unit-test/unit-test-nvvm-support.h` and
`unit-test-nvvm-emitter.cpp`. Successful real rows receive explicit O0/O3 direct lanes in their
existing files.

## Milestones

1. Materialize a bounded manifest from Slice 145, group exact spellings and signatures, and record
   the standard-module/IR producer chain.
2. Extract a scalar masked-wave value emitter from the current function-return implementation,
   preserving its exact recipe and insertion-block contract.
3. Add a recursive homogeneous aggregate resolver/emitter and exact result transport. Add active
   mask resolution through the established ballot operation.
4. Prove rejection of malformed spellings/signatures and positive value/out transports with the
   fake provider, then run all bounded real workloads at O0 and O3.
5. Promote every both-mode success, format and rebuild, run selected and promoted tests, regenerate
   the fixed corpus census/Pareto and metrics, perform the special-case inventory, and document the
   outcome.

## Validation and Acceptance

All builds and tests run outside the sandbox, as required by `AGENTS.md`.

- Provider build only if the provider changes:
  `cmake.exe --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release`
- Host build:
  `cmake.exe --build build --config Release --target slang-unit-test`
- Selected regression prefix with `SLANG_NVVM_BUILDER_PATH` set:
  `.\build\Release\bin\slang-test.exe slang-unit-test-tool/nvvm`
- Bounded O0/O3 census probe using `run-compute-census.py` and the frozen workload expression.
- Complete fixed census and cluster summary against Slice 145 identity.
- Representative workload metrics using `measure-compute-mvp.py`, including CUDA 12.9 PTX
  assembly for SM70, SM80, and SM90 on this machine.
- Formatter, `git diff --check`, focused negative tests, and zero loss among every Slice 145
  correct workload identity.

Acceptance requires exact resolver/emitter agreement, correct differential execution in both modes
for every promoted row, deterministic diagnostics for excluded wave shapes, no previous correct
identity loss, updated denominators and root-cause clusters, and a documented producer/type/ABI
audit for every retained widening.

## Failure and Recovery

Refactor the scalar emitter without changing its behavior before adding aggregate traversal. If a
bounded signature contains a nonhomogeneous or unsupported leaf, retain its exact diagnostic and
record the measured boundary instead of guessing a physical representation. If a provider gap is
found, stop and describe the missing canonical operation before revising revision 30. Generated
census and metric directories under `build/nvvm-census/` are disposable and rerunnable.

## Artifacts and Hand-Off

Retain the post-slice fixed census TSV and cluster JSON in `issue-nvvm-backend/`. Distill the
aggregate-wave contract, updated coverage denominator, and remaining wave gaps into
`docs/design/nvvm-backend.md` and `docs/design/nvvm-backend-capability-ledger.md`. Complete this
plan and a five-part Slice 146 report with the exact producer/signature audit, rejected alternatives,
validation evidence, gains, losses, remaining clusters, and productionization gaps.
