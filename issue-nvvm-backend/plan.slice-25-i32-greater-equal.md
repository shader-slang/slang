# Slice 25: Compare signed i32 values for greater-than-or-equal through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to ship with its implementation, so this plan will be committed
with Slice 25 rather than left as an uncommitted working log.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts canonical greater-than-or-equal comparison between
two signed `i32` values.

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left >= right ? 1 : 0;
}
```

The final linked Slang IR must retain one exact Boolean comparison producer. The provider must emit
the matching LLVM signed-integer predicate only after complete validation. Direct NVVM and NVRTC
must agree on raw parameter widths, ordered comparison, global storage, `ptxas` acceptance, and
representative runtime results including signed extremes.

## Progress

- [x] (2026-08-27) Started from committed Slice 24 `d1f2a743a` with focused NVVM 172/172 and the
  preservation matrix 10/10.
- [x] (2026-08-27) Traced ordinary `>=` through `BuiltinOperationKind::Geq` to `kIROp_Geq` and
  selected it as the smallest adjacent incomplete Bucket 2 slice.
- [x] (2026-08-27) Measured final linked `cmpGE(left, right) : Bool`, the pre-change E52017
  `'cmpGE'` boundary, and NVRTC's `[64,32,32]`/`setp.ge.s32`/global-store reference PTX.
- [x] (2026-08-27) Appended and implemented the exact provider/host signed-greater-equal
  capability, including strict-C layout and callback-type probes.
- [x] (2026-08-27) Extended direct preflight, capability gating, canonical value emission, and
  signed/unsigned/wide/floating-point/pointer negative coverage.
- [x] (2026-08-27) Added ABI, invalid/no-mutation, fake topology, exact-Slice-24-provider, PTX,
  both-`ptxas`, and runtime evidence.
- [x] (2026-08-27) Applied pinned formatting, rebuilt the provider and Slang tests, passed the
  focused suite 180/180 and preservation matrix 10/10, updated durable docs, inspected the DLL,
  removed probes, and completed the input-shape/diff audit.

## Surprises and Discoveries

- Observation: signed greater-than-or-equal remains a distinct final linked producer.
  Evidence: every late general IR dump retains exact `cmpGE(left, right) : Bool`, followed by the
  established conditional/constant/phi/store graph. Ordinary direct code generation with the
  Slice 24 provider stops at E52017 `'cmpGE'` and writes no direct output file.
  Consequence: implement exact `kIROp_Geq`/`ICMP_SGE`; do not reconstruct it as greater-than or
  equality, negate less-than, or reverse operands in the consumer.

- Observation: the NVRTC reference preserves signed 32-bit ordered semantics.
  Evidence: its 8,626-byte PTX has parameter widths `[64, 32, 32]`, `setp.ge.s32`, and
  `st.global.u32`.
  Consequence: differential evidence can reuse the established signed-comparison and global-store
  classifiers while runtime distinguishes equality from strict greater-than.

- Observation: the append grows the complete V2 table to 368 bytes on x64 and 204 bytes on x86.
  Evidence: strict-C and C++ compile-time probes agree; an exact 360-byte Slice 24 prefix remains
  accepted for older operations, while sizes 361 through 367 and a null complete callback are
  rejected before module creation.
  Consequence: greater-than-or-equal has one unambiguous append-only capability boundary without
  changing any prior callback offset.

- Observation: optimized PTX is not required to spell the source predicate identically.
  Evidence: the differential classifier observes signed 32-bit ordered comparison behavior on both
  routes, while the optimizer may invert a predicate together with its select/branch use.
  Consequence: exact `ICMP_SGE` is asserted at the provider boundary; PTX tests assert observable
  signed ordered behavior and runtime equality/sign/extreme cases instead of textual identity.

## Decision Log

- Decision: implement exact signed-i32 greater-than-or-equal before resources or exceptional
  arithmetic.
  Rationale: Bucket 2 remains the lowest incomplete bucket. Greater-than-or-equal completes the
  ordinary signed comparison family and reuses the proven Boolean-consumer boundary. Resources
  cross a substantially broader ABI boundary; division and shifts retain exceptional-input or
  signedness policy questions.
  Date/author: 2026-08-27, Codex.
  Revisit when: final linking intentionally canonicalizes `>=` to another exact producer rather
  than retaining `kIROp_Geq`.

## Outcomes and Retrospective

Slice 25 completes the ordinary signed-i32 comparison family by owning exact final-linked
`kIROp_Geq`. Slang validates two available signed-i32 operands and a canonical Boolean result;
the provider revalidates module/function/type/availability/dominance invariants before its only
mutation, one exact `ICMP_SGE`. Removing this direct case restores the measured E52017 `cmpGE`
failure, so the case is required at this consumer boundary. The producer shape is canonical and
intentional: no custom equivalence, graph walk, operand reversal, negated comparison, reconstructed
syntax, fallback, or producer-side repair survives the diff audit.

The append-only ABI is 368 bytes on x64 and 204 bytes on x86. Exact 360/200-byte Slice 24 providers
remain compatible for older operations and gate this capability before module construction;
partial sizes and null callbacks are rejected without mutation. Adjacent signedness, width,
floating-point, pointer, and malformed-provider cases all remain outside the support contract.

Direct NVVM and NVRTC agree on `[64,32,32]` raw parameters, signed 32-bit ordered comparison
behavior, and global 32-bit storage. CUDA 12.9 `ptxas` accepts both outputs. RTX 5090 execution
agrees for equal zero, equal negative, both unequal-sign directions, and both orderings of
`INT_MIN`/`INT_MAX`. The focused NVVM prefix passes 180/180 and the preservation matrix passes
10/10. The provider DLL exports only `slang_getNVVMBuilderAPI_V1` and
`slang_getNVVMBuilderAPI_V2`; its ordinary dependencies remain `KERNEL32.dll` with delayed
`SHELL32.dll` and `ole32.dll`, and no process-visible LLVM DLL.

## Context and Current Pipeline

Slices 21 through 24 accept equality, inequality, strict greater-than, and less-than-or-equal with
two signed-i32 operands and a canonical Boolean result. The private provider shares one binary
comparison validator among these operations and signed less-than, then selects exact LLVM
predicates. The Boolean handle feeds established conditional control flow; no Boolean parameter,
memory, return, or phi ABI exists.

Ordinary `>=` lowering maps `BuiltinOperationKind::Geq` to `kIROp_Geq`. This slice first measures
the post-link/post-optimization shape. If it remains exact `kIROp_Geq`, preflight and emission own
that opcode directly. If optimization intentionally canonicalizes it, the slice must follow the
measured producer rather than accepting multiple spellings.

## Scope and Non-Goals

In scope are exact two-operand signed-i32 greater-than-or-equal with canonical Boolean result, one
append-only provider callback, stable identity/capability gating, provider validation before
mutation, direct fake topology, old-provider compatibility, adjacent negative types,
direct/NVRTC PTX, both `ptxas` lanes, and GPU runtime cases.

Out of scope are changes to existing comparisons; unsigned, wide, floating-point, pointer, vector,
matrix, aggregate, or resource comparisons; Boolean ABI or storage; shifts, division, remainder,
new address spaces, builtins, waves, and optimization claims.

## Architecture and Invariants

The V2 table remains append-only. Slice 25 appends one complete callback after the 360-byte x64 /
200-byte x86 Slice 24 prefix. An exact Slice 24 provider remains valid for less-than-or-equal and
older programs but gates greater-than-or-equal after discovery and before module creation. Partial
pointer sizes, null complete callbacks, and failed provider outputs follow the established strict
negotiation and sanitization rules.

Slang preflight owns exact canonical opcode, signed-i32 policy, Boolean result, and operand
availability. The provider owns same-module/same-function/equal-scalar-integer-type validation and
the exact LLVM signed predicate. Validation must precede its sole instruction mutation. No source
syntax, operand reversal, arbitrary graph walk, fallback, or serialized-text repair belongs here.

## Interfaces and Dependencies

Append a dedicated `SlangNVVMEmitIntegerSignedGreaterEqual_2` callback and corresponding
minimum-size macro, host support predicate/wrapper, identity bit, and terminal `NVVMIRCapability`.
The production baseline remains exact LLVM 14.0.6 with negotiated NVVM IR 2.0 text, CUDA 12.9
libNVVM/NVRTC/`ptxas`, and the available RTX 5090. No public Slang API changes.

## Milestones

1. Probe ordinary signed greater-than-or-equal after final linking and record exact direct/NVRTC
   evidence.
2. Append the strict-C ABI field/minimum and host negotiation, identity, and sanitized wrapper.
3. Add the provider predicate after shared validation, plus positive and invalid/no-mutation tests.
4. Extend direct preflight/emission and fake graph evidence for the measured canonical producer.
5. Prove exact Slice 24 provider compatibility, adjacent negative boundaries, differential PTX,
   `ptxas`, and signed-boundary runtime behavior.
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
equivalence, producer repair, operand-reversal fallback, syntax reconstruction, or failure-driven
special case.

## Failure and Recovery

Probes, builds, tests, formatting, and binary inspection are safe to repeat. If final IR uses a
different canonical shape, update this plan before implementation rather than accepting multiple
spellings. Provider invalid calls must leave no partial instruction. Do not delete or stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

Keep final IR, ABI sizes, provider assembly, PTX classification, assembler/runtime results, test
counts, binary surface, and the self-review in this plan. Distill durable architecture into
`docs/design/nvvm-backend.md` and durable coverage into the capability ledger. Remove probe files
before committing the completed plan and implementation with first commit line `slice 25`.
