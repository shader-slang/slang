# Complete scalar Float64 transport and bit reinterpretation

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM carries canonical scalar `Double`, Int64, and UInt64 values through
ordinary helpers and emits exact same-width bit reinterpretation. The existing
`tests/compute/bitcast-64bit.slang` fixture should pass its unchanged four-result CUDA runtime and
PTX lanes. The generic value-operation path should also support the coherent scalar Float64
arithmetic, comparison, conversion, selection, constant, and helper family without admitting
Float64 vectors or new launch/storage ABIs.

## Progress

- [x] (2026-08-29) Completed Slice 119 as `033d007cf`; the full NVVM prefix passed 386/386.
- [x] (2026-08-29) Reproduced E52017 `helper function parameter` and captured optimized final IR
  for `bitcast-64bit.slang`.
- [x] (2026-08-29) Extended the canonical scalar Float64 value family through classification,
  operation resolution, constants, helper transport, and the LLVM provider/fake provider.
- [x] (2026-08-29) Mapped canonical `IRBitCast` to generic bit reinterpretation and proved all four
  signed/unsigned/floating combinations.
- [x] (2026-08-29) Added focused operation-family and boundary coverage, promoted the real fixture,
  validated PTX, runtime, `ptxas`, and the 387/387 full NVVM prefix, documented, formatted, and
  self-reviewed the slice.

## Surprises and Discoveries

- The optimized fixture preserves four one-parameter helpers. Their signatures are
  `Int64(Double)`, `Int64(UInt64)`, `UInt64(Double)`, and `UInt64(Int64)`; each body contains one
  canonical `bitCast` and a return. The entry stores through the already established
  `RWStructuredBuffer<UInt64>` route.
- The first failure is the Double helper parameter, not resource storage or bit reinterpretation.
  Int64/UInt64 value types, calls, results, and resource stores already compose.
- Generic value-operation descriptors and the LLVM provider's parameterized family already model
  same-width bit reinterpretation and floating operations. The missing pieces are bounded Float64
  admission, Double materialization, and mapping `IRBitCast`; no new callback is needed.
- The existing scalar floating classifier is also used by vector classification. Adding Double
  without separating that decision would silently admit two- through four-lane Float64 vectors,
  which are outside this measured boundary and existing negative coverage.
- The direct emitter has separate structural and provider-availability validation switches. The
  first focused run exposed that `IRBitCast` had been added to structural preflight and emission
  but not availability validation; adding it to the same canonical operation family restored the
  invariant that both checks resolve the identical descriptor.
- libNVVM preserves the semantic Float64 family but is free to expand an operation. In the emitted
  PTX, remainder becomes an absolute-value/divide/round/multiply/subtract/select sequence rather
  than one `rem.f64` instruction. Runtime and `ptxas` evidence therefore test the semantic result
  and accepted module, not an unnecessarily rigid instruction spelling.

## Decision Log

- Decision: extend the generic scalar floating family to IEEE Float64 while keeping vector
  floating elements restricted to the established Float16/Float32 set.
  Rationale: the provider operation algebra is dimensioned, but this fixture and current runtime
  evidence establish only scalar Double. A scalar/vector distinction prevents accidental vector,
  alignment, and storage expansion.
  Date/author: 2026-08-29, Codex.
- Decision: admit the coherent generic scalar Float64 arithmetic, comparison, conversion,
  selection, and bit-reinterpret families in one slice rather than adding four fixture-specific
  cast signatures.
  Rationale: all operations share the same typed descriptor, LLVM scalar type, constant, and
  helper transport contract. The larger family is economical and directly testable without new
  API surface.
  Date/author: 2026-08-29, Codex.
- Decision: keep CUDA entry parameters, conventional parameter-group fields, device pointers,
  byte-address payloads, Float64 vectors/arrays, and texture/surface values unchanged.
  Rationale: those are distinct ABI, layout, or resource contracts not proven by a first-class
  helper-value fixture.
  Date/author: 2026-08-29, Codex.
- Decision: map canonical `IRBitCast` through `SLANG_NVVM_VALUE_OP_BIT_REINTERPRET`.
  Rationale: final IR already states the semantic operation and exact result/operand types. Source
  builtin matching or a dedicated builder callback would duplicate that source of truth.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

The slice establishes scalar Float64 as a first-class direct-NVVM value without changing any CUDA
storage or vector contract. Type lowering, exact constants, helper transport, generic arithmetic,
comparison, conversion, selection, and same-width reinterpretation all compose through the
dimensioned builder interface. `IRBitCast` uses the semantic operation catalog rather than source
builtin recognition or LLVM text editing.

The existing `bitcast-64bit.slang` fixture passes direct runtime and PTX, and the broader
file-backed Float64 family test proves dynamic operations rather than constants that an optimizer
could erase. Both generated modules assemble with CUDA 12.9.86 `ptxas -arch=sm_70`; focused
negative coverage preserves Float64-vector and raw-entry boundaries; the complete NVVM prefix
passes 387/387.

The self-review inventory contained the scalar/vector classifier split, the deliberately narrower
mutable numeric classifier, Float64 type and exact-constant materialization, the dimensioned
semantic-catalog extension, the canonical `IRBitCast` mapping, provider/fake support, and focused
and file-backed tests. Every surviving branch consumes canonical final-IR types or opcodes. No
custom equivalence, operand-graph recovery, source syntax reconstruction, provider-only fallback,
size-changing reinterpretation, operation-table duplication, LLVM text rewrite, vector leak, or
launch/storage widening remains.

## Context and Current Pipeline

Optimized final IR contains these helpers:

    func icast(Double x) -> Int64  { return bitCast(x); }
    func icast(UInt64 x) -> Int64  { return bitCast(x); }
    func ucast(Double x) -> UInt64 { return bitCast(x); }
    func ucast(Int64 x) -> UInt64  { return bitCast(x); }

The kernel calls them with `-1.0`, `2`, `3.0`, and `-4`, converts the first two signed results to
UInt64 where source assignment requires it, and stores four UInt64 values. The synthesized global
parameter block contains only the established output view. There is no Double launch parameter,
Double memory access, vector, aggregate, or libdevice operation in the motivating final IR.

## Scope and Non-Goals

In scope are canonical scalar Double values; exact IEEE-754 64-bit constants; ordinary helper
parameters/results/calls/returns; generic scalar Float64 negate, arithmetic, comparisons,
integer/Float64 and floating-width conversions, typed select, and same-lane bit reinterpretation;
`IRBitCast` mapping; focused fake/provider evidence; and runtime/PTX promotion of the existing
fixture.

Out of scope are Float64 vectors, matrices, arrays, conventional global fields, constant buffers,
parameter blocks, raw Double buffers, byte-address Double payloads, CUDA entry parameters, device
pointers, mutable Float64 storage, texture/surface Double, atomics, libdevice transcendental
functions, relaxed/fast-math policy changes, reinterpretation across different total bit sizes,
aggregate bitcasts, a new builder callback, and LLVM text rewriting.

## Architecture and Invariants

- Scalar Float64 is one canonical first-class value type with semantic descriptor
  `{FloatingPoint, 64, 1}`; Float64 lane counts above one remain invalid.
- Helper signature validation and type lowering make the same decision before consulting caches.
- A Double constant carries the exact 64-bit `DoubleAsInt64` pattern to the provider; decimal text
  round-tripping is not part of the contract.
- `IRBitCast` is accepted only when semantic-family resolution proves one operand, equal bit width
  and lane count, and a different integer/floating signedness kind.
- Provider operation support and emission use the same semantic-family resolver as preflight.
- Existing Float16/Float32 vectors and Float32-oriented launch/storage/resource contracts do not
  broaden as a consequence of adding the scalar width.

## Interfaces and Dependencies

No builder ABI revision is planned. The semantic catalog, type lowering, direct emitter, LLVM
provider, fake provider and focused source/test, existing compute fixture, durable design status,
and this plan are the expected committed areas. CUDA 12.9 runtime and `ptxas` provide real semantic
and assembly evidence.

## Milestones

1. Separate scalar and vector floating-width admission and add exact Float64 type/constant support.
2. Extend the bounded semantic resolver and both providers for scalar Float64 descriptors.
3. Map, preflight, and emit canonical `IRBitCast` through generic bit reinterpretation.
4. Add focused coverage for the complete scalar family and preserve Float64 vector/entry/storage
   rejection boundaries.
5. Promote the real fixture, validate runtime/PTX and `ptxas`, run the full prefix, update durable
   status and this plan, self-review, format, and commit.

## Validation and Acceptance

Acceptance requires Release provider and host builds; focused fake evidence for Float64 type and
constant width/pattern, helper parameter/result/call/return transport, each selected generic
operation family, and exact 64-bit reinterpret descriptors; existing Float64-vector and raw-entry
negative coverage before provider mutation; direct runtime/PTX lanes for the existing fixture;
CUDA 12.9 `ptxas -arch=sm_70`; the complete `slang-unit-test-tool/nvvm` prefix; pinned formatting;
and `git diff --check`.

The self-review inventories the scalar/vector classifier split, helper/type-lowering branches,
constant bit materialization, semantic-family expansion, IR opcode mapping, provider/fake changes,
and tests. For each, record the exact final-IR producer, why the shape is canonical, and which test
fails without it. Remove any Float64-vector leak, launch/storage widening, source builtin matching,
size-changing reinterpret fallback, duplicated operation table, provider-only acceptance, or text
rewrite.

## Failure and Recovery

If libNVVM rejects a valid LLVM 7 textual Double form or an operation family differs at runtime,
preserve IR, LLVM assembly, PTX, cubin, and logs under ignored `build/slice120-*`; narrow to the
independently proven Float64 helper/bit-reinterpret subset and record the next operation boundary.
Do not synthesize integer operations in the emitter, fold source constants to hide missing
transport, widen vectors/storage, patch LLVM text, reset unrelated work, or stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

Keep generated IR, PTX, cubin, and logs under ignored `build/slice120-*`. Distill the final scalar
Float64 contract, validation evidence, and next measured corpus boundary into
`docs/design/nvvm-backend.md`, then commit this plan with the implementation as explicitly
requested.
