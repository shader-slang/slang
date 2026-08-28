# Slice 61: Add public signed-integer wave-active-all-equal

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs public `WaveActiveAllEqual(int)`. CUDA's canonical
`WaveMaskAllEqual(activeMask, value)` helper remains the semantic provider boundary, and the
provider lowers its exact signed-32-bit form to `llvm.nvvm.match.all.sync.i32p` and returns the
intrinsic's Boolean predicate result.

## Progress

- [x] (2026-08-28) Recorded the Slice 60 baseline: 397 names, SHA-256
  `d5daef5d6db4caa82e5dd8039a8b0f5e095d13cdb819a81f7ea69a30ab873b0d`, Release 397/397,
  Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 27,500 measured lines.
- [x] (2026-08-28) Audited source selection, exact final linked IR, the pre-provider E52017
  boundary, LLVM 7/provider intrinsic contracts, CUDA prelude semantics, and NVRTC PTX.
- [x] (2026-08-28) Appended feature 46/operation 12 and lowered the exact signed-i32 helper through
  the native aggregate-result intrinsic while returning only its predicate.
- [x] (2026-08-28) Added provider/direct/capability/PTX/`ptxas`/RTX evidence through the public
  source path, including mixed and uniform runtime cases.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, audited, and removed
  temporary probes for the completed slice.

## Surprises and Discoveries

- Observation: final linked IR contains exact `Func(Bool, UInt, Int)` ending in
  `GenericAsm("_waveAllEqual($0, $1)")`; direct NVVM reaches this canonical helper and reports
  E52017 `GenericAsm`.
  Consequence: match exact assembly plus complete semantic signature. Do not match helper names,
  bypass active-mask synthesis, or reconstruct the public operation in the emitter.

- Observation: the same public probe instantiated with `uint` first fails earlier on unsupported
  UInt load-result lowering, while the signed-i32 form reaches the intended GenericAsm boundary.
  Consequence: keep this slice on the already-supported signed-i32 load/value path. UInt, Float,
  wider scalars, vectors, and matrices remain independently auditable future work.

- Observation: CUDA's `_waveAllEqual(mask, value)` calls `__match_all_sync`; NVRTC emits one
  `match.all.sync.b32` whose outputs are a matching-lane mask and a predicate, then uses the
  predicate as the helper result. LLVM 7 and the provider LLVM expose
  `llvm.nvvm.match.all.sync.i32p` as `{i32, i1} (i32, i32)` with convergent and
  inaccessible-memory semantics.
  Consequence: the generic semantic operation returns only Bool, while the provider privately
  extracts aggregate element 1. Do not expose the incidental match-mask result through the ABI.

- Observation: the direct fake-graph test initially expected one Bool function parameter, but the
  final specialized graph has none; its two Boolean values are helper call results. The all-equal
  helper and public wrapper take only integer mask/value parameters.
  Consequence: assert zero Bool parameters and two Bool call results. Do not infer parameter types
  from the helper's result type.

## Decision Log

- Decision: make signed-i32 public `WaveActiveAllEqual()` the next bounded wave operation.
  Rationale: it is the next source-visible vote primitive, uses already-supported source loads and
  scalar types, and proves that a semantic one-result builder operation can lower through a native
  aggregate-result intrinsic without widening the public provider ABI.
  Date/author: 2026-08-28, Codex.

- Decision: append one V3 feature and intrinsic operation for the exact signed-i32 helper.
  Rationale: the helper is the canonical target-specialized boundary. Independent negotiation
  preserves exact Slice 60 compatibility and leaves other overloads unsupported rather than
  conflating type-specific contracts.
  Date/author: 2026-08-28, Codex.

- Decision: discard the native match-mask result inside the provider and return only the native
  i1 predicate.
  Rationale: Slang's semantic helper returns Bool, the CUDA prelude consumes only the predicate,
  and exposing an aggregate or second result would make the ABI mirror an LLVM implementation
  detail instead of the operation's stable contract.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Feature 46 and operation 12 now negotiate the exact canonical `Func(Bool, UInt, Int)` masked
helper. The change adds no type role, callback field, or table member. V3 remains 528 bytes on x64
and 308 bytes on x86, and exact Slice 60 providers load with feature 46 clear.

The provider validates both i32 operands before mutation and emits
`llvm.nvvm.match.all.sync.i32p(i32, i32) -> {i32, i1}` followed by one `extractvalue` of element 1.
Both normal LLVM assembly and LLVM-7-compatible NVVM IR contain the same call, aggregate result,
predicate extraction, declaration, and convergent/inaccessible-memory/nounwind attributes. The
legacy writer therefore needs only an exact semantic audit, not a rewrite. The standalone Release
provider and Release/Debug main targets build successfully.

All seven new names pass 7/7, the Slice 46-61 wave/ABI matrix passes 106/106, the Release NVVM
prefix passes 404/404, and Debug preservation passes 10/10. The complete sorted LF-terminated
Release name set hashes to
`40f3eba7cfb2602716a16b54d942cf09e34e9f2171835889a1dea43cb1e10d0a`; removing the seven Slice 61
names gives 397 names and exactly Slice 60's
`d5daef5d6db4caa82e5dd8039a8b0f5e095d13cdb819a81f7ea69a30ab873b0d`. The five measured
test/support files grew by 169 physical lines, from 27,500 to 27,669.

NVVM and NVRTC agree on the `[64, 64]` entry ABI, one 32-bit global load/store pair, two
synchronized ballots, and one `match.all.sync.b32`. CUDA 12.9 `ptxas` accepts both. On the RTX
5090, distinct signed values make all 32 lanes store zero, while uniform `-17` makes all lanes store
one through both routes. Sharing the predicate-intrinsic fixture and direct/PTX setup kept marginal
growth bounded without merging any argument-type or operation-specific assertion. The only initial
test correction changed the expected Bool function-parameter count from one to the observed zero;
generated code and runtime semantics were already correct.

## Context and Current Pipeline

Consider this kernel:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAllEqual(source[laneIndex]) ? 1 : 0;
}
```

CUDA target selection in `source/slang/hlsl.meta.slang` implements the public generic operation as
`WaveMaskAllEqual(WaveGetActiveMask(), value)`. Active-mask synthesis and control-flow mask
maintenance retain the established synchronized ballot graph. Specialization produces one exact
`Bool(UInt, Int)` helper whose body is `_waveAllEqual($0, $1)`.

`source/slang/slang-emit-nvvm.cpp` recognizes supported GenericAsm helpers by exact assembly and
semantic function signature and calls `NVVMIRBuilder::emitIntrinsic`. The facade in
`source/compiler-core/slang-nvvm-ir-builder.cpp` enforces feature-to-operation negotiation. The
provider in `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` validates opaque values, creates the native
LLVM call, and serializes LLVM-7-compatible textual IR for libNVVM.

The provider intrinsic returns `{i32, i1}` even though the source helper returns Bool. That
aggregate is an LLVM/NVPTX encoding of the PTX instruction's two outputs; extracting element 1 at
the provider boundary preserves the source helper's single semantic result.

## Scope and Non-Goals

In scope are one append-only feature/operation, exact `Bool(UInt, Int)` descriptor selection,
provider `match.all.sync.i32p` emission plus predicate extraction and declaration audit, one public
runtime row, provider/direct/capability/PTX/`ptxas`/RTX evidence, design, ledger, and this plan.

Out of scope are UInt/Float/i64/vector/matrix all-equal overloads, new type roles, UInt source
loads, aggregate values in the provider ABI, match-mask exposure, `WaveMatch`, reductions,
divergence stress, new callback fields, performance claims, or unrelated text rewrites.

## Architecture and Invariants

The source library and active-mask synthesis remain the sole producers of the canonical graph.
Direct NVVM recognizes the exact signed-i32 masked helper by complete result/parameter shape plus
assembly text, requires the new feature, and forwards the existing UInt mask and Int value.

The facade maps the new operation only to the new feature. The provider validates two i32 operands
before module mutation, calls `llvm.nvvm.match.all.sync.i32p(i32, i32)`, verifies the native
aggregate result shape, extracts predicate element 1, and returns it as the opaque Bool handle. The
legacy writer accepts only the exact LLVM-7-compatible declaration signature and semantic
attributes.

Tests may share scaffolding only where the graph shape is genuinely identical. Every registered
test must retain operation-specific feature/op, helper, declaration, PTX mnemonic, source, and
runtime assertions.

## Interfaces and Dependencies

Append feature 46, operation 12, and a minimum-size alias to V3 unless implementation evidence
shows those next ordinal values changed. Extend the facade, provider, exact descriptor, fake Bool
intrinsic classification, public fixture/runtime expectation, tests, design, ledger, and plan.
Do not change table layout, ABI version, type-role enum, V2, exports, LLVM components, or formats.

This slice depends on CUDA SM 7.0 match instructions, LLVM's native
`nvvm_match_all_sync_i32p` intrinsic, installed libNVVM/NVRTC/`ptxas`, and the existing RTX runtime
fixture. LLVM 7 source under `build/nvvm-builder-deps/llvm7-project` is compatibility evidence, not
a production dependency.

## Milestones

1. Append feature 46/operation 12 with unchanged V3 sizes and exact Slice 60 compatibility.
2. Match exact `Bool(UInt, Int)` all-equal GenericAsm and emit/extract the native aggregate result.
3. Add provider/direct/capability/PTX/`ptxas`/RTX evidence through the public source path.
4. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, remove probes, and
   commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
new names, complete wave matrix, generic-function and intrinsic compatibility/invalid tests,
unsupported boundary, full Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged 528/308-byte V3; exact Slice 60 compatibility; one native aggregate match-all
call, one predicate extraction, and one exact declaration in both assembly forms; independent
feature-46 E52016 before module construction; public `[64, 64]` ABI with one load/store, two
ballots, and one `match.all.sync.b32`; CUDA 12.9 `ptxas`; mixed per-lane inputs returning false and
uniform inputs returning true through both RTX routes; hash continuity; economical marginal
growth; formatted code; completed audit; removed probes; clean diffs.

## Self-Review and Input-Shape Audit

Inventory the feature/operation mappings, exact descriptor/provider case, aggregate extraction,
declaration audit, fake classification, fixtures/runtime row, and evidence. Prove all provider
validation precedes mutation, the extracted predicate is selected by the intrinsic's documented
result contract rather than graph rediscovery, semantic types remain authoritative, and no helper
name matching, syntax reconstruction, fallback, custom equivalence, or duplicate bridge was added.

The `Bool(UInt, Int)` helper is the canonical target-specialized shape produced by the source
library for this overload. The LLVM aggregate is also canonical at the provider boundary, but it is
not a second Slang value representation: it is consumed immediately and only the semantic Bool is
returned to the facade. This is the layer that owns the adaptation because LLVM defines the native
intrinsic result shape and upstream Slang intentionally defines a single-result helper.

## Failure and Recovery

If final IR differs from the audited helper or exposes another type role, stop rather than broaden
the slice. If libNVVM rejects the native declaration, compare exact normal and compatible text
before adding a rewrite. If LLVM's aggregate cannot remain private to the provider, stop rather
than change the ABI. Removing feature 46/operation 12 and Slice 61 evidence restores Slice 60.
Never stage `external/slang-binaries/` or `tmp-slice-61-*` artifacts.

## Artifacts and Hand-Off

Retain exact helper semantics, LLVM 7/provider intrinsic declaration and call, NVVM/NVRTC PTX,
`ptxas`/RTX results, sizes, hashes, line growth, and audit. Distill durable evidence into the
design/ledger and commit this completed plan with Slice 61.
