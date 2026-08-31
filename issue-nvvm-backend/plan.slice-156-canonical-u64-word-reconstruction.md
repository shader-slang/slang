# Lower canonical UInt64 word reconstruction

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM executes the canonical `makeUInt64(low, high)` instruction emitted by
AnyValue unmarshalling. Two exact `UInt32` words become one `UInt64` value by zero extension,
shifting the high word by 32, and combining the disjoint bit ranges.

The bounded target is the four healthy first blockers shared across both corpora: frozen
`layout-64bit-scalar`, `layout-64bit-vector`, and `layout-mixed-bitwidths`, plus discovery
`anyvalue-bulk-copy`. Each workload will be run at direct O0 and O3; later failures remain separate.

## Progress

- [x] (2026-08-31) Reconciled Slice 155 Pareto rows and selected the exact four-row
  `makeUInt64` producer/type/operation shape over broader fixture labels.
- [x] (2026-08-31) Dumped final linked IR for `layout-64bit-scalar` and confirmed
  `makeUInt64(UInt, UInt) -> UInt64` is produced while reconstructing Double, Int64, and UInt64
  fields from two AnyValue words.
- [x] (2026-08-31) Defined one exact canonical word-reconstruction recipe using the existing
  UInt32-to-UInt64 conversion, UInt64 shift, and UInt64 bitwise-or typed operations.
- [x] (2026-08-31) Built and ran all four motivating workloads at O0/O3; all four are correct and
  receive eight permanent direct differential lanes.
- [x] (2026-08-31) Ran the 427/427 selected prefix, both complete corpora, thirteen representative
  measurement gates, integrity checks, documentation, and self-review for Slice 156.

## Surprises and Discoveries

- The ordinary LLVM emitter already lowers `IRMakeUInt64` as two zero extensions to i64, a 32-bit
  left shift of the high word, and bitwise-or. The direct NVVM provider already exposes all three
  operations generically; the missing contract is compiler-side instruction classification.
- The existing CUDA-prelude `DoubleFromWords` scalar recipe independently uses the same three typed
  provider descriptors before its final UInt64-to-Double reinterpretation. This is evidence that
  the provider operation set and libNVVM accept the intended representation without an ABI change.
- Frozen both-mode correctness reaches 384/427 (89.9%). This satisfies the previously defined
  approximately-90% checkpoint for proposing a deduplicated corpus v2, but this slice does not
  freeze a new denominator or alter either current corpus.
- The repository formatting check runs but this machine still lacks `gersemi`, `clang-format`,
  `prettier`, and `shfmt`. The C++ and test-directive changes were manually reviewed, and
  `git diff --check` is clean.

## Decision Log

- Decision: support only exact `makeUInt64(UInt32, UInt32) -> UInt64` in this slice.
  Rationale: `IRBuilder::emitMakeUInt64` and AnyValue unmarshalling own that canonical contract, and
  every current cross-corpus blocker has exactly that shape. No evidence requires signed words,
  vector words, another result type, or inferred conversions.
  Date/author: 2026-08-31, Codex.
- Decision: express reconstruction as a compiler-owned finite recipe of generic typed operations.
  Rationale: integer conversion, shift-left, constants, and bitwise-or are already queried and
  emitted through revision 30. A provider callback would duplicate expressible semantics.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Direct NVVM now resolves exact `makeUInt64(UInt, UInt) -> UInt64`, collects its complete typed
operation closure before provider discovery, validates both word operands through ordinary SSA
availability, and emits two zero extensions followed by the high-word shift and bitwise-or. The
provider ABI remains revision 30.

The three frozen AnyValue layout workloads and discovery `anyvalue-bulk-copy` become correct in
both modes and gain eight permanent lanes. Frozen corpus v1 remains exactly 452 workloads/427
healthy references and moves from 381/385/381 to 384/388/384 O0/O3/both correctness, with zero
old-correct loss. Discovery remains exactly 82/72 and moves from 58/58/58 to 59/59/59, also with
zero loss. The selected prefix passes 427/427, and all 20 directives in the four promoted files
pass.

All thirteen representative direct-O3 gates assemble with CUDA 12.9 for SM70, SM80, and SM90. The
new AnyValue gate measures 356.5 ms and 1240-byte PTX at direct O3 SM70 versus 482.8 ms and
9850-byte PTX through NVRTC O3. Direct O0 measures 346.0 ms and emits 182309-byte PTX, reinforcing
the existing O0 output-size signal. These measurements remain exploratory.

At 89.9% both-mode correctness, frozen v1 has reached the stated approximate-90% milestone. A
subsequent bounded proposal should deduplicate useful discovery combinations into a candidate
corpus v2 and present the rationale before any baseline change. Corpus v1 and discovery remain
unchanged in this slice.

## Context and Current Pipeline

Consider this AnyValue field:

```slang
struct DoubleImpl : IValue
{
    double val;
}
```

`slang-ir-any-value-marshalling.cpp` stores 64-bit leaves as two consecutive 32-bit words. During
unmarshalling, it loads those `UInt` fields and calls `IRBuilder::emitMakeUInt64(lowBits, highBits)`.
Final linked IR retains:

```text
%low  : UInt = load(...field0...)
%high : UInt = load(...field1...)
%bits : UInt64 = makeUInt64(%low, %high)
%value : Double = bitCast(%bits)
```

The producer shape is canonical and semantically useful beyond one fixture. Direct NVVM already
supports unsigned i32/i64 integer conversion, i64 left shift, i64 bitwise-or, and typed integer
constants. Preflight currently has no `kIROp_MakeUInt64` case, so it rejects the instruction before
querying those existing operations.

The owning boundary is direct-NVVM instruction classification and legalization. It should verify
the exact result and operand types once, record the complete operation closure before provider
creation, validate both SSA operands, and emit the same bit recipe as the ordinary LLVM backend.

## Scope and Non-Goals

In scope are exact scalar `UInt32` low/high operands, scalar `UInt64` result, the finite typed
reconstruction recipe, O0/O3 validation, stable promotions, separate corpus artifacts, and durable
documentation.

Out of scope are signed or vector word inputs, arbitrary integer packing, descriptor handles,
`makeArray`, default construction, `AnyValue` layout changes, source syntax reconstruction,
provider callbacks, ABI revision, frozen-corpus identity changes, and corpus v2.

## Architecture and Invariants

- The result is exactly selected scalar `UInt64`; both operands are exactly selected scalar
  `UInt32` in low-word then high-word order.
- Both words are zero-extended to UInt64 before shifting or combining.
- The shift amount is a typed UInt64 constant 32, matching the provider's homogeneous shift
  descriptor and avoiding signedness-dependent behavior.
- The low and shifted-high bit ranges are disjoint, so bitwise-or reconstructs the exact 64 bits.
- Every recipe operation is collected and capability-checked before provider module creation.
- Frozen corpus v1 and discovery retain separate exact identities and denominators.

## Interfaces and Dependencies

`source/slang/slang-emit-nvvm.cpp` owns the exact instruction resolver, requirements collection,
SSA validation, and emission recipe. Existing `NVVMValueRecipeStep`, semantic descriptors, integer
constant construction, and revision-30 provider operations are sufficient. No public header,
provider implementation, or external dependency change is planned.

## Milestones

1. Add one resolved UInt64-word-construction descriptor that proves the exact opcode, operand
   count/order, and three semantic types, and initializes conversion/shift/or recipe steps.
2. Record the three typed operation descriptors during global preflight and validate both source
   values through the established availability/dominance path.
3. Emit both zero extensions, typed shift constant, high-word shift, and final bitwise-or; bind the
   resulting provider value directly to the canonical `IRMakeUInt64`.
4. Build and run all four targets against native NVRTC at direct O0/O3. Promote stable semantic
   representatives and capture every later first blocker.
5. Run the 427-test selected prefix, frozen v1, discovery, and SM70/80/90 measurement gates. Update
   separate artifacts, report, design, ledger, and this plan; self-review and commit.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools as required by
`AGENTS.md`. Acceptance requires:

- final linked IR proves the exact producer and ordered UInt32 operands;
- provider requirements contain only the existing UInt32-to-UInt64 conversion, UInt64 shift-left,
  and UInt64 bitwise-or descriptors;
- every promoted workload is differentially correct at direct O0 and O3;
- frozen v1 remains exactly 452/427 and discovery exactly 82/72, with separate O0/O3/both,
  classifications, Pareto, and zero old-correct regression;
- the selected prefix passes and representative O3 PTX assembles for SM70/80/90;
- provider ABI revision remains 30; and
- artifact integrity and `git diff --check` pass without staging `external/slang-binaries/`.

## Failure and Recovery

If a target reaches another operation or runtime mismatch, record its exact producer and keep only
the independently proven word reconstruction. If any admitted shape differs from the canonical
two-UInt32 producer, reject it before provider mutation rather than inserting implicit repairs.
Generated IR, PTX, logs, and corpus output under `build/` are reproducible and remain untracked.

## Artifacts and Hand-Off

Commit this completed plan with the implementation because the user explicitly requires them
together. Retain final-IR probes and measurement output under `build/`; commit stable direct lanes,
Slice 156 corpus snapshots, the five-part report, and durable design/ledger updates. The report
must trace `emitMakeUInt64` from AnyValue unmarshalling through the exact typed provider recipe.
