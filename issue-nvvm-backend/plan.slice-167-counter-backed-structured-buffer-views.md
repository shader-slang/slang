# Carry canonical counter-backed structured-buffer views through direct NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, direct NVVM carries the canonical two-field resource aggregate produced for
`AppendStructuredBuffer<T>` and `ConsumeStructuredBuffer<T>` through helper calls and executes its
element and atomic-counter operations at O0 and O3. Success is observable when the two healthy
frozen-v1 workloads compare correctly in both modes through generic type, aggregate, raw-buffer,
atomic, and semantic-operation interfaces, with no fixture-name checks or provider callback.

## Progress

- [x] (2026-09-01) Completed and committed Slice 166 as `2c4de39b1`; frozen v1 advances to
  400/400/400 and discovery remains 66/66/66 O0/O3/both.
- [x] (2026-09-01) Re-ranked both corpora and selected the two healthy MVP counter-backed buffer
  workloads from the heterogeneous helper-ABI census cluster.
- [x] (2026-09-01) Traced the first failure to the exact two-operand structured-buffer field types
  intentionally produced by `lowerStructuredBufferType`.
- [x] (2026-09-01) Proved the producer/type invariant, retained adjacent-negative coverage, and
  widened the exact structured-buffer classifier to the canonical two- or three-operand contract.
- [x] (2026-09-01) Carried both workloads through structured-buffer dimensions and the exact CTA
  memory fence using the established raw view and one appended typed operation ID.
- [x] (2026-09-01) Promoted four real lanes, regenerated both corpora and a 32-gate measurement
  matrix, completed documentation and self-review, and prepared Slice 167 for commit.
- [x] (2026-09-01) Rebuilt the Release host and isolated provider, passed the 433/433 selected
  prefix and final 2/2 focused plus 4/4 promoted checks, verified corpus identities and JSON,
  attempted repository formatting, and removed unrelated fallback-formatter churn.

## Surprises and Discoveries

- The frozen `helper-abi-type-contract` cluster contains unrelated families: excluded FP8/BFloat
  values, extension-only kernel contexts, and the two counter-backed resource aggregates. The
  cluster count is therefore not itself a valid implementation boundary.
- `lowerStructuredBufferType` replaces each Append/Consume type with a named struct containing an
  element `RWStructuredBuffer<T>` and counter `RWStructuredBuffer<Atomic<int>>`. It deliberately
  constructs both field types from the element and data-layout operands only. The third operand
  present on many post-legalization structured-buffer types is not part of this producer's form.
- The generated helper bodies already use ordinary `get_field`, typed structured-buffer element
  pointers, relaxed `atomicInc`/`atomicDec`, loads/stores, control flow, and direct calls. The
  current first diagnostic occurs before any of those established operations are examined.
- Once the type contract was admitted, the next exact operations were
  `IRStructuredBufferGetDimensions` and canonical GenericAsm `__threadfence_block`. Dimensions
  are the runtime count from the existing `{data, count}` view plus the compile-time CUDA storage
  stride. `__threadfence_block` is neither synchronizing `barrier0` nor device-scope `membar.gl`.
- A synthetic Append/Consume fake-provider fixture reached the fake's intentionally incomplete
  nested resource-aggregate parameter model. It was removed rather than widening a fake ABI that
  cannot prove real execution; provider serialization, adjacent rejection, and four real
  differential lanes cover the owning layers.
- The repository formatting script could not run because `gersemi`, `clang-format`, `prettier`, and
  `shfmt` were absent from its bash PATH. The installed Windows `clang-format.exe` was applied, its
  unrelated whole-file churn was removed, and the retained Slice 167 C/C++ hunks remain formatted.

## Decision Log

- Decision: split the heterogeneous helper-ABI census label by canonical type family and address
  counter-backed resource aggregates as one bounded slice.
  Rationale: Append/Consume are common compute resources inside the MVP, share one producer and
  representation, and do not justify pulling excluded substandard floating-point types or advanced
  wave kernel contexts into the same change.
  Date/author: 2026-09-01, Codex.
- Decision: retain `lowerStructuredBufferType`'s two-operand buffer types and generalize the direct
  raw-buffer classifier only if focused evidence proves that form is a valid IR contract.
  Rationale: the producer constructs the semantic element and explicit data layout directly. A
  direct-emitter patch must not synthesize or rediscover a conformance operand that the canonical
  representation does not require.
  Date/author: 2026-09-01, Codex.
- Decision: lower `IRStructuredBufferGetDimensions` from the established raw view and selected
  storage layout.
  Rationale: count and stride already have authoritative runtime and compile-time sources; no new
  representation, callback, or syntax recovery is needed.
  Date/author: 2026-09-01, Codex.
- Decision: append a workgroup-memory-fence operation ID and advance the forward-only ABI to
  revision 32.
  Rationale: canonical `__threadfence_block` requires `llvm.nvvm.membar.cta`, which cannot be
  represented correctly as either the existing synchronizing barrier or device-scope fence. The
  generic catalog remains sufficient, so no callback is added.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Append and Consume now compare correctly at O0 and O3. Frozen corpus v1 remains exactly 452
workloads/427 healthy references and advances from 400/400/400 to 402/402/402 O0/O3/both, with
exactly two gains and no old-correct loss. Discovery remains exactly 82/72 at 66/66/66 with no
gain or loss. The selected prefix passes 433/433, and the promoted fixture lanes pass 4/4.

All 32 representative gates produced five measurement rows and assembled cubins for SM70, SM80,
and SM90, for 160/160 total. The slice establishes one reusable counter-backed resource path while
keeping malformed raw-buffer shapes rejected and avoiding fixture recognition, fallback, text
rewriting, or provider callbacks.

## Context and Current Pipeline

The direct backend already supports structured and byte-address views, resource-containing structs
by value, generic aggregate field extraction, helper parameters/results, global element pointers,
relaxed atomic operations, barriers, and ordinary control flow. `getNVVMSupportedRawBufferType`
currently accepts only structured-buffer types with exactly three operands even though
`IRHLSLStructuredBufferTypeBase` defines the element plus optional data-layout contract and the
Append/Consume lowering intentionally constructs two operands.

For `AppendStructuredBuffer<int>`, the producer creates:

```text
struct AppendStructuredBuffer<int> {
    elements : RWStructuredBuffer(Int, DefaultLayout)
    counter  : RWStructuredBuffer(Atomic(Int), DefaultLayout)
}
```

Its generated Append helper extracts both views, performs relaxed `atomicInc` on counter element
zero, and stores the value through the resulting element index. Consume uses the same representation
with `atomicDec`, subtraction, conditional control flow, and a selected element load.

## Scope and Non-Goals

In scope are exactly canonical HLSL structured-buffer view operand contracts, resource-value
classification, helper transport for the producer-generated two-field aggregate, generated
Append/Consume helper bodies, adjacent invalid-layout/element rejection, both real workloads, both
fixed corpora, and representative measurement evidence.

Out of scope are source-name or struct-name recognition, reconstructing a missing layout or witness,
new atomic semantics, GenericAsm atomics, byte-address Half operations, substandard floating-point
types, advanced wave kernel contexts, descriptor-layout mismatches, discovery helper pointers,
provider callbacks, compatibility fallbacks, and unrelated census failures.

## Architecture and Invariants

- `lowerStructuredBufferType` remains the canonical producer and its element/data-layout operands
  remain the semantic source of truth.
- The classifier may accept a two-operand structured-buffer type only by validating the same exact
  supported opcode, element family, and explicit supported data layout as the established form.
- Optional extra operands cannot be ignored blindly. Any accepted three-operand form retains its
  existing exact contract, and malformed counts or layouts remain rejected before provider mutation.
- The generated aggregate is lowered structurally. No Append/Consume name, fixture, decoration, or
  source syntax may influence emission.
- Existing generic builder operations must express the complete result. An ABI revision is allowed
  only for a concrete canonical operation that the current typed operation set cannot represent.

## Interfaces and Dependencies

The primary classification boundary is
`source/slang/slang-emit-nvvm-type-lowering.cpp`. Producer evidence lives in
`source/slang/slang-ir-lower-append-consume-structured-buffer.cpp`. Focused compiler tests belong in
the split NVVM emitter/support unit files, while real runtime coverage belongs in the two existing
HLSL fixtures. The fixed census and measurement scripts remain unchanged unless classification of
an already-recorded diagnostic needs a precise producer/type label.

## Milestones

1. Audit the IR type definition, producer, existing consumers, and negative tests to state the
   exact two- versus three-operand invariant.
2. Change the shared classifier at the canonical type boundary, add focused positive and negative
   tests, rebuild outside the sandbox, and run the selected compiler prefix.
3. Follow each next real-workload failure to its producer. Reuse existing generic operations or
   stop if the representation is malformed or requires a materially broader feature.
4. Promote the stable real lanes, run both exact corpora and representative measurements, complete
   design/ledger/report records and the self-review, format, audit, and commit.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools. Acceptance requires
focused classifier and compiler evidence; correct O0/O3 differential execution for Append and
Consume; selected NVVM prefix success; zero old-correct regression; frozen identity 452/427;
discovery identity 82/72; separate census/Pareto artifacts; representative PTX assembly for SM70,
SM80, and SM90; formatting attempt; `git diff --check`; artifact integrity; and an exact staged-file
audit excluding `external/slang-binaries/`.

## Failure and Recovery

All changes are additive and independently testable. If accepting the canonical type exposes a
malformed producer or a feature outside this bounded representation, revert the classifier change,
record the exact next shape, and either narrow the result to diagnostic evidence or stop for design
discussion. Do not recognize names, infer missing operands, weaken layouts, or patch serialized IR.

## Artifacts and Hand-Off

Keep raw IR, PTX, and logs under ignored `build/nvvm-census` paths. Retain a completed plan only if
the slice yields a committed result under the user's workflow exception. Distill the accepted type
contract into `docs/design/nvvm-backend.md`, exact coverage into the capability ledger and separate
census artifacts, and the full producer/input-shape audit into the five-part Slice 167 report.
