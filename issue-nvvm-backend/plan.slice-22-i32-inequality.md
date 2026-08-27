# Slice 22: Compare signed i32 values for inequality through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to ship with its implementation, so this plan will be committed
with Slice 22 rather than left as an uncommitted working log.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts canonical inequality between two signed `i32`
values.

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left != right ? 1 : 0;
}
```

The final linked Slang IR must retain one exact Boolean comparison producer rather than an
equality-plus-Boolean-NOT spelling. The provider must emit the matching LLVM integer predicate only
after complete validation. Direct NVVM and NVRTC must agree on raw parameter widths, inequality
selection, global storage, `ptxas` acceptance, and equal/not-equal runtime results.

## Progress

- [x] (2026-08-27) Started from committed Slice 21 `3f7bb9b9e` with focused NVVM 148/148 and the
  preservation matrix 10/10.
- [x] (2026-08-27) Re-read `.agent/PLANS.md`, confirmed `kIROp_Neq` is a distinct canonical Slang
  opcode, and selected inequality as the smallest adjacent Bucket 2 slice with stable semantics.
- [x] (2026-08-27) Measured final linked `cmpNE(left, right) : Bool`, the pre-change E52017
  `'cmpNE'` boundary, and NVRTC's `[64,32,32]`/`setp.ne.s32`/global-store reference PTX.
- [x] (2026-08-27) Appended and implemented the exact provider/host inequality capability and
  strict-C minimum-size/order probes.
- [x] (2026-08-27) Extended direct preflight, capability gating, canonical Boolean value emission,
  and unsigned/wide/floating/pointer negative coverage.
- [x] (2026-08-27) Added ABI, invalid/no-mutation, fake topology, exact Slice 21 provider,
  differential PTX, `ptxas`, and RTX 5090 runtime evidence.
- [x] (2026-08-27) Formatted with clang-format 17.0.6, rebuilt Debug and Release hosts plus the
  Release provider outside the sandbox, passed focused 156/156 and preservation 10/10, inspected
  the provider binary, updated docs/ledger, removed probes, and completed the self-review.

## Surprises and Discoveries

- Observation: ordinary inequality survives final linking as its own canonical comparison.
  Evidence: `-dump-ir -stage compute` shows `cmpNE(left, right) : Bool`, followed by the existing
  conditional, integer constants, phi, and store; it does not become equality plus Boolean NOT.
  Consequence: admit exact `kIROp_Neq` and emit exact `ICMP_NE`; no spelling fallback or producer
  repair is needed.

- Observation: optimized PTX may express equality-family control flow with either `setp.ne.s32`
  or the inverse `setp.eq.s32` predicate plus reversed selection.
  Evidence: the explicit NVRTC probe emitted `setp.ne.s32`, while the existing token-safe PTX
  classifier intentionally recognizes both predicates and executable runtime cases prove the
  truth table.
  Consequence: retain the established semantic equality-predicate classifier and pair it with
  exact provider-IR, `ptxas`, and runtime evidence instead of imposing textual PTX identity.

- Observation: the append-only ABI sizes follow the expected pointer-width increment.
  Evidence: the exact Slice 21 prefix is 336 bytes on x64 and 188 bytes on x86; the complete Slice
  22 prefix is 344/192, and all interior sizes plus a null complete callback reject.
  Consequence: freeze those predecessor/new sizes in the negotiation test and leave older tables
  valid for every previously supported program.

## Decision Log

- Decision: implement exact signed-i32 inequality before shifts, division/remainder, resources, or
  broader comparison families.
  Rationale: Bucket 2 remains the lowest incomplete bucket. Inequality is a distinct canonical
  opcode with stable semantics and reuses the now-demonstrated comparison result/consumer boundary;
  shifts and division inherit unresolved exceptional-input policy, while resources cross a much
  broader ABI boundary.
  Date/author: 2026-08-27, Codex.
  Revisit when: final linked IR rewrites `!=` into another already-supported canonical graph.

## Outcomes and Retrospective

Slice 22 is complete. The final producer is exact two-operand `kIROp_Neq` over signed-i32 values
with canonical Boolean result. Slang preflight owns that shape and rejects unsigned, i64, float,
and pointer adjacency before provider discovery. The provider reuses the established binary
integer ownership/type/availability/dominance validator, then performs the sole mutation as exact
`ICMP_NE`; its verified module contains one `icmp ne i32` and no i64 comparison.

The ABI appends one callback after the frozen 336-byte x64 / 188-byte x86 Slice 21 prefix. The
complete prefix is 344/192; interior byte counts and a null callback reject, future tables clamp,
and wrapper failures sanitize outputs. An exact Slice 21 provider still compiles equality but gates
inequality with E52016 before module creation.

Direct NVVM and NVRTC expose matching `[64,32,32]` parameters, a token-safe 32-bit equality-family
predicate, and global i32 storage. Both CUDA 12.9 `ptxas` lanes pass. On the RTX 5090, both routes
produce zero for equal zero/negative pairs and one for unequal signs/extremes. Release focused
tests pass 156/156; preservation passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary, 3/3
sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch.

The helper/special-case inventory contains only the append-only ABI predicate/wrapper, provider
callback, fake value/callback tracking, provider module builder, and test fixtures. All survive:
they exercise existing canonical boundaries and add no fallback, custom semantic equivalence,
operand-graph walk, syntax reconstruction, or producer-side patch. The exact input shape is
correct and principled because normal lowering and final linking deliberately preserve
`kIROp_Neq`; the consumer owns translating that canonical opcode. Removing its case restores the
measured E52017 `'cmpNE'` failure, so this emitter/provider boundary is the responsible layer.

Binary inspection reports exactly `slang_getNVVMBuilderAPI_V1` and
`slang_getNVVMBuilderAPI_V2`; the ordinary dependency is only `KERNEL32.dll`, with delayed
`SHELL32.dll` and `ole32.dll`, and no process-visible LLVM DLL.

## Context and Current Pipeline

Slice 21 accepts exact `kIROp_Eql` with two signed-i32 operands and a canonical Boolean result. The
private provider shares one binary comparison validator between signed less-than and equality, then
selects `ICMP_SLT` or `ICMP_EQ`. The Boolean handle feeds established conditional control flow; no
Boolean parameter, memory, return, or phi ABI exists.

Ordinary `!=` lowering names `kIROp_Neq` as a distinct semantic operation. This slice first measures
the post-link/post-optimization shape. If it remains exact `kIROp_Neq`, preflight and emission own
that opcode directly. If optimization intentionally canonicalizes it to equality plus an existing
consumer, the slice must follow the measured producer rather than adding a spelling fallback.

## Scope and Non-Goals

In scope are exact two-operand signed-i32 inequality with canonical Boolean result, one append-only
provider callback, stable identity/capability gating, provider validation before mutation, direct
fake topology, old-provider compatibility, adjacent negative types, direct/NVRTC PTX, both `ptxas`
lanes, and GPU runtime cases.

Out of scope are equality changes; ordered greater/less-equal/greater-equal comparisons; unsigned,
wide, floating-point, pointer, vector, matrix, aggregate, or resource comparisons; Boolean ABI or
storage; shifts, division, remainder, new address spaces, builtins, waves, and optimization claims.

## Architecture and Invariants

The V2 table remains append-only. Slice 22 appends one complete callback after the 336-byte x64 /
188-byte x86 Slice 21 prefix. An exact Slice 21 provider remains valid for equality and older
programs but gates inequality after discovery and before module creation. Partial pointer sizes,
null complete callbacks, and failed provider outputs follow the established strict negotiation and
sanitization rules.

Slang preflight owns exact canonical opcode, signed-i32 policy, Boolean result, and operand
availability. The provider owns same-module/same-function/equal-scalar-integer-type validation and
the exact LLVM predicate. Validation must precede its sole instruction mutation. No source syntax,
comparison inversion, arbitrary operand walk, fallback, or serialized-text repair belongs here.

## Interfaces and Dependencies

Append a dedicated `SlangNVVMEmitIntegerNotEqual_2` callback and corresponding minimum-size macro,
host support predicate/wrapper, identity bit, and terminal `NVVMIRCapability`. The production
baseline remains exact LLVM 14.0.6 with negotiated NVVM IR 2.0 text, CUDA 12.9 libNVVM/NVRTC/
`ptxas`, and the available RTX 5090. No public Slang API changes.

## Milestones

1. Probe ordinary inequality after final linking and record exact direct/NVRTC evidence.
2. Append the strict-C ABI field/minimum and host negotiation, identity, and sanitized wrapper.
3. Add the provider predicate after shared validation, plus positive and invalid/no-mutation tests.
4. Extend direct preflight/emission and fake graph evidence for the measured canonical producer.
5. Prove exact Slice 21 compatibility, adjacent negative boundaries, differential PTX, `ptxas`, and
   equal/not-equal runtime behavior.
6. Apply pinned formatting, rebuild/test outside the sandbox, inspect the DLL, update durable docs,
   remove probes, self-review, and commit only intended files.

## Validation and Acceptance

Run Windows-native CMake builds and tests outside the sandbox:

```text
cmake.exe --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release --target slang-llvm-nvvm -- /m
cmake.exe --build build --config Release --target slang-unit-test slang-test -- /m
$env:SLANG_NVVM_BUILDER_PATH='C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Release\bin\slang-test.exe slang-unit-test-tool/nvvm
```

Then run the established parser 1/1, routing/hash 2/2, unsupported 1/1, sampler 3/3, CUDA
compile/pass-through 2/2, and runtime-dispatch 1/1 preservation prefixes. Acceptance also requires
the two-export DLL allowlist, no process-visible LLVM DLL, exact input-shape evidence, and no custom
equivalence, producer repair, syntax reconstruction, or failure-driven fallback.

## Failure and Recovery

Probes, builds, tests, formatting, and binary inspection are safe to repeat. If final IR uses a
different canonical shape, update this plan before implementation rather than accepting multiple
spellings. Provider invalid calls must leave no partial instruction. Do not delete or stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

Keep final IR, ABI sizes, provider assembly, PTX classification, assembler/runtime results, test
counts, binary surface, and the self-review in this plan. Distill durable architecture into
`docs/design/nvvm-backend.md` and durable coverage into the capability ledger. Remove probe files
before committing the completed plan and implementation with first commit line `slice 22`.
