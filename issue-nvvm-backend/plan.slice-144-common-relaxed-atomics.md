# Establish one common relaxed scalar atomic algebra

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires this experimental branch's slice plan to be committed with its implementation, which is
an exception to the repository's default active-plan lifetime policy.

## Purpose and Observable Result

After this slice, direct NVVM accepts the canonical relaxed scalar atomic family used by ordinary
compute kernels across structured/raw global buffers and module-scope groupshared storage. One
compiler-owned classifier covers load, store, exchange, compare-exchange, add/subtract,
min/max, bitwise reductions, and increment/decrement for the selected 32/64-bit integer and
32-bit floating forms proven by the corpus. The provider receives one typed operation descriptor
rather than intrinsic- or fixture-specific callbacks.

The bounded population is the ten healthy-MVP census rows whose first blocker is a common atomic
operation: the two `atomic-reduce-methods` workloads, the three `Atomic<T>` workloads,
`byte-address-buffer-atomics`, the 32/64-bit HLSL atomic-intrinsic workloads,
`metal/atomic-byteaddressbuffer`, and `exchange-int64-byte-address-buffer`. Every workload that
becomes correct at direct O0 and O3 is promoted. A later unrelated blocker or an unproven atomic
subfamily remains measured rather than expanding the slice.

## Progress

- [x] (2026-08-30) Committed Slice 143 as `ccdcf7bcd` with five both-mode gains, zero losses, and
  421/421 selected tests.
- [x] (2026-08-30) Decomposed the atomic census cohort and traced canonical global structured/raw
  and groupshared address producers plus the exact operand/order shapes.
- [x] (2026-08-30) Defined ABI revision 30's generic scalar atomic descriptor and proved the
  LLVM 14-to-libNVVM textual forms, including the libNVVM rejection of atomic load/store syntax.
- [x] (2026-08-30) Shared one compiler resolver between preflight, value validation, requirement
  collection, and emission; added provider and fake-provider contract tests for the full algebra.
- [x] (2026-08-30) Probed the bounded ten rows, discovered one additional int64 byte-address gain,
  promoted all eleven both-mode successes, regenerated the fixed census/Pareto and representative
  metrics, completed self-review, formatted, validated, documented, and prepared the commit.

## Surprises and Discoveries

- `RWByteAddressBuffer.Interlocked*` does not survive as an opaque byte-address intrinsic. Resource
  legalization produces `getEquivalentStructuredBuffer` followed by the same canonical
  `rwstructuredBufferGetElementPtr` consumed by structured-buffer atomics. The element type and
  byte-to-element index conversion are therefore already established upstream.
- The three `Atomic<T>` fixtures exercise the whole common family in one kernel, including load,
  store, compare-exchange, and inc/dec. The two reduction-method fixtures already execute their
  reduction calls correctly and stop only at the final canonical `atomicLoad`.
- ABI revision 29 can express atomic read-modify-write but cannot express atomic load, store, or
  compare-exchange correctly through its generic construction operations. These are real memory
  semantics, not ordinary load/store plus a flag, so the provider boundary has a concrete gap.
- libNVVM NVVM IR 2.0 rejects LLVM textual `load atomic` and `store atomic` even after removing the
  LLVM 14 explicit alignment suffix. A monotonic compare-exchange of zero with zero is the
  value-preserving atomic-read idiom accepted by libNVVM; monotonic exchange implements store while
  discarding the old SSA value. The strict serializer rejects raw atomic load/store instructions.
- HLSL whole-array groupshared initialization remains one canonical `store` rooted at the shared
  array global. Admitting that exact producer/consumer pair was required before its later atomic
  element operations could be reached. No other consumer receives a whole-array pointer.
- A function-local `static groupshared Atomic<int>` becomes an anonymous synthesized module-scope
  global. Since it has no mangled name, physical naming is derived deterministically from canonical
  shared-global order and still participates in collision rejection.
- The generic algebra unlocked `slang-extension/atomic-int64-byte-address-buffer` in addition to
  the bounded ten-row probe. Its exact O0/O3 differential success was independently rerun and the
  workload was promoted rather than hidden as an incidental census gain.

## Decision Log

- Decision: Treat the ten rows as one bounded common-atomic cohort while promoting only workloads
  that reach correct differential execution in both optimization modes.
  Rationale: They share canonical scalar atomic instructions and exact global/shared pointer
  producers. The 64-bit fixture is included as a probe because it applies the same algebra to
  selected widths, but it may expose a later independent operation or provider restriction.
  Date/author: 2026-08-30, Codex.
- Decision: Replace the fixed two-operand RMW provider call with one generic typed atomic operation
  descriptor and operand array at ABI revision 30.
  Rationale: This forward-only experimental ABI needs one economical contract for RMW, load,
  store, and compare-exchange. Ordinary builder loads/stores cannot encode atomic ordering, and no
  current callback can encode compare-exchange. One operation enum scales without per-intrinsic
  callbacks.
  Date/author: 2026-08-30, Codex.
- Decision: Keep memory-order classification in compiler preflight and initially admit only the
  canonical relaxed literals proven by this corpus.
  Rationale: Slang IR carries orders as semantic operands while LLVM carries them as instruction
  metadata. Erasing non-literal or stronger orders would be incorrect; widening them needs its own
  memory-model evidence.
  Date/author: 2026-08-30, Codex.
- Decision: Lower subtract and decrement through the existing atomic-add form with a compiler-side
  integer-negation recipe; lower increment through add-one.
  Rationale: These canonical operations have identical fetch-add semantics after exact integer
  modular negation and do not require a new provider primitive. The existing generic value builder
  already expresses the transformation.
  Date/author: 2026-08-30, Codex.
- Decision: Lower relaxed load and store to value-preserving compare-exchange and exchange at the
  isolated provider boundary.
  Rationale: libNVVM rejects the LLVM atomic load/store spellings. These established CUDA atomic
  idioms retain atomicity and exact stored/result bits for the admitted relaxed device-scope
  family, while ordinary non-atomic load/store is not a valid substitute.
  Date/author: 2026-08-30, Codex.
- Decision: Treat `Atomic<T>` as a physical `T` storage leaf and widen equivalent raw-buffer views
  only to the signed/unsigned 32/64-bit and Float32 leaves produced by byte-address legalization.
  Rationale: The wrapper is the canonical semantic marker for atomic access, not an extra memory
  field. The raw view's exact typed producer already establishes byte-to-element addressing; the
  compiler neither reconstructs source syntax nor rediscovers offsets.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

All eleven newly reachable workloads compile and compare correctly at direct O0 and O3. The fixed
452-workload census moves from 346 to 357 O0 successes and from 351 to 362 O3 successes, with no
old-correct identity loss. Against 427 healthy MVP references, O0/O3/both correctness is
355/359/355 (83.1%/84.1%/83.1%). The selected regression prefix passes 422/422.

The full post-slice classification is 357 correct, eight runtime mismatches, 80 preflight failures,
and seven provider failures at O0; O3 is 362 correct, eight mismatches, 80 preflight failures, and
two provider failures. All 22 newly added repository lanes pass. The representative resource,
parameter-block, and shared-control gates remain correct; their direct O3 PTX assembles for SM70,
SM80, and SM90 with CUDA 12.9. CUDA 13 and physical SM70/SM80/SM90 runtime workers remain explicit
productionization gaps.

Self-review inventory:

- The unified atomic resolver survives. Every accepted operand/order/result relation comes from a
  canonical atomic instruction and is shared by preflight and emission.
- `Atomic<T>` physical storage survives. It preserves the source semantic wrapper while lowering
  exactly one physical scalar leaf at the CUDA/provider boundary.
- Signed/unsigned/Float32 equivalent raw-buffer views survive. The canonical producer is byte-
  address legalization and the int64/Float32 raw atomic workloads prove this layer owns the view.
- Whole shared-array pointer admission survives only for canonical whole-array initialization
  stores; the broader intermediate admission was narrowed during self-review.
- Anonymous shared-global naming survives as a deterministic target-private detail, with global
  collision checking retained.
- Provider load-via-CAS, store-via-exchange, floating bit transport, and cmpxchg suffix rewriting
  survive because real libNVVM compilation and all eleven runtime comparisons require them.
- The serializer's intermediate acceptance of floating cmpxchg was removed: provider-generated
  floating exchange/CAS is deliberately represented by same-width integer operations.

The common-atomic cluster is no longer the leading priority. The next selection should decompose
the ten-row residual-marker population and the tied eight-row helper ABI, aggregate/layout,
preflight-other, and wave/reconvergence populations by exact canonical producer before choosing a
new reusable vertical slice.

## Context and Current Pipeline

Consider:

```slang
RWStructuredBuffer<Atomic<uint>> values;
uint old = values[0].compareExchange(4, 5);
uint now = values[0].load();
```

Intrinsic lowering produces `atomicCompareExchange(ptr, 4, 5, relaxed, relaxed)` and
`atomicLoad(ptr, relaxed)`. `ptr` is produced by `rwstructuredBufferGetElementPtr`. For a raw
buffer method, resource legalization first produces `getEquivalentStructuredBuffer` and then the
same element-pointer instruction. For groupshared arrays, the producer is `getElementPtr` rooted
at the canonical module-scope groupshared global.

`slang-emit-nvvm.cpp::_resolveNVVMAtomicOperation` currently accepts only selected `atomicAdd` and
`atomicMax` forms. `_validateNVVMFunction` rejects the other canonical ops before provider
discovery. Accepted RMW operations cross
`NVVMIRBuilder::emitAtomicOperation` into
`slang-llvm-nvvm.cpp::_emitAtomicOperation`, which constructs LLVM `atomicrmw`. The same resolver
must own all operation/type/address/order classification, requirement collection, value-graph
validation, and emission.

## Scope and Non-Goals

In scope:

- relaxed scalar load/store/RMW/compare-exchange on exact supported global and shared pointer
  producers;
- 32/64-bit signed/unsigned integer forms and the 32-bit floating forms concretely exercised by
  the bounded population, subject to successful libNVVM verification and runtime proof;
- add, min, max, and, or, xor, exchange, compare-exchange, load, and store; compiler recipes for
  subtract, increment, and decrement;
- structured-buffer and legalized byte-address-buffer element pointers plus established direct
  global/shared scalar/element pointer producers;
- ABI revision 30 with one generic atomic operation descriptor and operand array;
- focused positive/negative contract tests, real O0/O3 differential tests, fixed census/Pareto,
  and representative `ptxas` metrics.

Out of scope:

- acquire/release/sequentially-consistent semantics, scopes other than the established system sync
  scope, weak compare-exchange, vector/aggregate atomics, or arbitrary pointer graph traversal;
- texture/surface atomics, half/double exchange/CAS, FP8, advanced reductions, or new source
  intrinsics not represented by the canonical operations above;
- fixture-name checks, syntax reconstruction, compatibility shims, ordinary non-atomic fallback
  loads/stores, or downstream repair of malformed pointer provenance.

## Architecture and Invariants

- The canonical Slang atomic instruction is the semantic source of truth. Its pointer producer,
  value type, result type, operand count, and literal order operands must agree exactly before any
  provider mutation.
- Pointer admission is producer-based: established global/shared globals and parameters,
  module-scope shared-array element pointers, and writable structured-buffer element pointers.
  Legalized raw buffers are valid because their canonical producer is the same typed element
  pointer; the compiler does not rediscover byte offsets.
- One resolved descriptor records operation, semantic scalar type, physical address space, success
  order, and failure order. Preflight requirements and emission consume that same descriptor.
- Provider operands contain only SSA values. Memory-order literals stay in the descriptor and are
  never lowered as runtime operands.
- Load and RMW/CAS return the original typed value; store returns void. Compare-exchange returns
  LLVM's original-value field and does not expose its success flag because canonical Slang IR does
  not request it.
- Subtract/decrement use exact modular integer negation plus fetch-add. Increment uses an exact
  typed one constant plus fetch-add. The recipe is collected before provider discovery.
- Unsupported types, orders, address spaces, operand shapes, or provider overloads fail
  deterministically at preflight; emission has no fallback.

## Interfaces and Dependencies

`source/compiler-core/slang-nvvm-ir-builder-api.h` advances to ABI revision 30. The existing atomic
operation enum gains load/store/compare-exchange, the descriptor gains failure order, and
`SlangNVVMBuilderAtomicOperationsAPI::emitOperation` takes an operand array/count plus an optional
result. The wrapper, fake provider, semantic catalog, LLVM 14 provider, and builder unit tests move
forward together; no compatibility structure-size or old callback remains.

Compiler changes remain concentrated in `source/slang/slang-emit-nvvm.cpp`. Existing generic value
operations provide typed constants and negation. The isolated provider uses LLVM 14 atomic
instructions, while its established serializer must continue producing libNVVM-compatible text.

## Milestones

1. Add focused builder/provider tests for integer RMW/exchange, atomic load/store, and
   compare-exchange in global/shared storage. Confirm serialized text passes libNVVM verification
   and compilation.
2. Generalize the semantic catalog and ABI 30 atomic interface. Preserve exact operand-count,
   result, type, address-space, dominance, and order validation in both the wrapper and provider.
3. Replace the narrow compiler resolver with one operation-shape classifier. Reuse it in preflight,
   selected-value validation, requirement collection, and emission; compose inc/dec/sub recipes
   through generic value operations.
4. Build provider/host and run focused fake/provider/emitter tests. Probe all ten census rows at O0
   and O3, recording every later first blocker without speculative widening.
5. Promote exact both-mode successes, regenerate the fixed census and MVP metrics, complete the
   input-shape/self-review inventory, format, run selected and representative validation, update
   durable documents, and commit.

## Validation and Acceptance

All builds and tests run outside the sandbox with
`SLANG_NVVM_BUILDER_PATH=C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release`.
Acceptance requires:

- provider and host Release builds;
- focused builder/provider tests that serialize and compile the selected global/shared operations;
- focused emitter fake-graph positives plus negatives for non-relaxed order, unsupported type,
  malformed operand/result relations, invalid pointer producer, and missing provider support;
- O0/O3 differential comparison of all ten bounded rows and promotion only for exact both-mode
  successes;
- the selected `slang-unit-test-tool/nvvm` prefix, regression/promoted groups, fixed 452-row census,
  representative workload metrics, and `ptxas` validation for SM70/80/90;
- no old-correct census identity loss and documented compile/runtime/PTX deltas.

## Failure and Recovery

ABI 30 changes are forward-only and atomic: host, provider, wrapper, and fake provider must be
rebuilt together. A stale provider must fail discovery by revision, not crash. If one LLVM atomic
form fails serialization or libNVVM verification, retain the focused negative/diagnostic and remove
that semantic overload; do not substitute a non-atomic operation. If a bounded fixture reaches a
later unrelated blocker, record and re-cluster it. Temporary focused sources under
`build/nvvm-census` are ignored and may be discarded after their IR evidence is captured.

## Artifacts and Hand-Off

Keep temporary probes and targeted census output under `build/nvvm-census`. Commit the updated
fixed census TSV/cluster JSON, coverage manifest, MVP design metrics, promoted test lanes, and this
plan with the implementation. Record exact gains/losses, later blockers, provider ABI revision,
serialized-form findings, and the special-case inventory here and in the durable NVVM design
document.
