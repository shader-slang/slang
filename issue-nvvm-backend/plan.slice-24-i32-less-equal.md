# Slice 24: Compare signed i32 values for less-than-or-equal through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to ship with its implementation, so this plan will be committed
with Slice 24 rather than left as an uncommitted working log.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts canonical less-than-or-equal comparison between two
signed `i32` values.

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left <= right ? 1 : 0;
}
```

The final linked Slang IR must retain one exact Boolean comparison producer. The provider must
emit the matching LLVM signed-integer predicate only after complete validation. Direct NVVM and
NVRTC must agree on raw parameter widths, ordered comparison, global storage, `ptxas` acceptance,
and representative runtime results including signed extremes.

## Progress

- [x] (2026-08-27) Started from committed Slice 23 `0c59498f4` with focused NVVM 164/164 and the
  preservation matrix 10/10.
- [x] (2026-08-27) Traced ordinary `<=` through `BuiltinOperationKind::Leq` to `kIROp_Leq` and
  selected it as the smallest adjacent incomplete Bucket 2 slice.
- [x] (2026-08-27) Measured final linked `cmpLE(left, right) : Bool`, the pre-change E52017
  `'cmpLE'` boundary, and NVRTC's `[64,32,32]`/`setp.le.s32`/global-store reference PTX.
- [x] (2026-08-27) Appended and implemented the exact provider/host signed-less-equal capability and
  strict-C probes.
- [x] (2026-08-27) Extended direct preflight, capability gating, canonical value emission, and
  adjacent unsigned/wide/float/pointer negative coverage.
- [x] (2026-08-27) Added ABI, invalid/no-mutation, fake topology, exact Slice 23 provider, PTX,
  both `ptxas` lanes, and RTX 5090 runtime evidence.
- [x] (2026-08-27) Applied pinned clang-format 17.0.6, rebuilt the provider and Release host,
  passed focused 172/172 and preservation 10/10, updated the design and ledger, inspected the DLL,
  removed probes, and completed the input-shape self-review.

## Surprises and Discoveries

- Observation: signed less-than-or-equal remains a distinct final linked producer.
  Evidence: every late general IR dump retains exact `cmpLE(left, right) : Bool`, followed by the
  established conditional/constant/phi/store graph. Ordinary direct code generation with the
  Slice 23 provider stops at E52017 `'cmpLE'` and writes no direct output file.
  Consequence: implement exact `kIROp_Leq`/`ICMP_SLE`; do not reconstruct it as less-than or
  equality, negate greater-than, or reverse operands in the consumer.

- Observation: the NVRTC reference preserves signed 32-bit ordered semantics.
  Evidence: its 8,626-byte PTX has parameter widths `[64, 32, 32]`, `setp.le.s32`, and
  `st.global.u32`.
  Consequence: differential evidence can reuse the established signed-comparison and global-store
  classifiers while runtime distinguishes equality from strict less-than.

- Observation: the appended callback grows the complete V2 table from 352 to 360 bytes on x64 and
  from 196 to 200 bytes on x86.
  Evidence: the strict ABI tests prove the new field begins at the former complete minimum, every
  intermediate byte count is malformed, and both complete layouts pass the C and C++ probes.
  Consequence: exact Slice 23 providers retain their established capabilities but report
  `scalar-integer-signed-less-equal=0` and gate `kIROp_Leq` before module creation.

- Observation: direct NVVM and NVRTC may choose inverse PTX predicates while preserving selection
  semantics.
  Evidence: differential classification accepts token-safe signed 32-bit `lt`, `ge`, `gt`, or `le`
  predicates, while runtime distinguishes equality, strict ordering in both directions, and signed
  extremes.
  Consequence: compare observable signed semantics and dataflow rather than requiring one
  optimizer-specific PTX spelling.

## Decision Log

- Decision: implement exact signed-i32 less-than-or-equal before greater-equal, resources, or
  exceptional arithmetic.
  Rationale: Bucket 2 remains the lowest incomplete bucket. Less-than-or-equal has ordinary signed
  semantics and can reuse the proven comparison-result/Boolean-consumer boundary. Resources cross
  a substantially broader ABI boundary; division and shifts retain exceptional-input or
  signedness policy questions.
  Date/author: 2026-08-27, Codex.
  Revisit when: final linking intentionally canonicalizes `<=` to another exact producer rather
  than retaining `kIROp_Leq`.

## Outcomes and Retrospective

Slice 24 accepts exact final-linked `kIROp_Leq(left, right) : Bool` only when both operands are
available signed `i32` values and the result is canonical Boolean. The shape is correct and
principled: ordinary source `left <= right` produces that exact opcode through final linking, no
upstream phase reconstructs it from other comparisons, and removing the consumer case restores
the pre-change E52017 boundary. A producer repair, Boolean negation, operand reversal, custom
equivalence, syntax reconstruction, or graph walk would therefore obscure the source of truth.

The append-only V2 ABI adds `emitIntegerSignedLessEqual` at the former Slice 23 minimum and grows
the complete table to 360 bytes on x64 and 200 bytes on x86. Exact older providers remain valid;
partial suffixes and null complete callbacks are invalid. Host dispatch sanitizes output handles,
and the LLVM 14 provider reuses the shared binary integer validator before its sole mutation,
exact `ICMP_SLE` construction. Invalid ownership, function, type, availability, dominance, and
insertion-point cases leave the module unchanged.

Unsigned, wide, and floating cases stop at their unsupported entry parameters; pointer less-equal
reaches the exact signed-i32 operand boundary. All stop before provider discovery. Direct NVVM and
NVRTC expose `[64, 32, 32]` parameter widths, signed 32-bit ordered comparison, and one global
`u32` store; CUDA 12.9 `ptxas` accepts both. RTX 5090 results agree for equal zero, equal negative,
both sign directions, and both orderings of `INT_MIN`/`INT_MAX`.

The rebuilt Release focused NVVM suite passes 172/172. The parser, routing/hash, unsupported,
sampler, CUDA compile/pass-through, and runtime-dispatch preservation lanes pass 10/10. The
provider exports exactly the V1/V2 getters; it depends ordinarily only on `KERNEL32.dll`, delay
loads `SHELL32.dll` and `ole32.dll`, and exposes no process-visible LLVM DLL dependency.

## Context and Current Pipeline

Slices 21 through 23 accept exact `kIROp_Eql`, `kIROp_Neq`, and `kIROp_Greater` with two signed-i32
operands and a canonical Boolean result. The private provider shares one binary comparison
validator among these operations and signed less-than, then selects exact LLVM predicates. The
Boolean handle feeds established conditional control flow; no Boolean parameter, memory, return,
or phi ABI exists.

Ordinary `<=` lowering maps `BuiltinOperationKind::Leq` to `kIROp_Leq`. This slice first measures
the post-link/post-optimization shape. If it remains exact `kIROp_Leq`, preflight and emission own
that opcode directly. If optimization intentionally canonicalizes it, the slice must follow the
measured producer rather than accepting multiple spellings.

## Scope and Non-Goals

In scope are exact two-operand signed-i32 less-than-or-equal with canonical Boolean result, one
append-only provider callback, stable identity/capability gating, provider validation before
mutation, direct fake topology, old-provider compatibility, adjacent negative types,
direct/NVRTC PTX, both `ptxas` lanes, and GPU runtime cases.

Out of scope are changes to existing comparisons; greater-equal; unsigned, wide, floating-point,
pointer, vector, matrix, aggregate, or resource comparisons; Boolean ABI or storage; shifts,
division, remainder, new address spaces, builtins, waves, and optimization claims.

## Architecture and Invariants

The V2 table remains append-only. Slice 24 appends one complete callback after the 352-byte x64 /
196-byte x86 Slice 23 prefix. An exact Slice 23 provider remains valid for greater-than and older
programs but gates less-than-or-equal after discovery and before module creation. Partial pointer
sizes, null complete callbacks, and failed provider outputs follow the established strict
negotiation and sanitization rules.

Slang preflight owns exact canonical opcode, signed-i32 policy, Boolean result, and operand
availability. The provider owns same-module/same-function/equal-scalar-integer-type validation and
the exact LLVM signed predicate. Validation must precede its sole instruction mutation. No source
syntax, operand reversal, arbitrary graph walk, fallback, or serialized-text repair belongs here.

## Interfaces and Dependencies

Append a dedicated `SlangNVVMEmitIntegerSignedLessEqual_2` callback and corresponding minimum-size
macro, host support predicate/wrapper, identity bit, and terminal `NVVMIRCapability`. The production
baseline remains exact LLVM 14.0.6 with negotiated NVVM IR 2.0 text, CUDA 12.9
libNVVM/NVRTC/`ptxas`, and the available RTX 5090. No public Slang API changes.

## Milestones

1. Probe ordinary signed less-than-or-equal after final linking and record exact direct/NVRTC
   evidence.
2. Append the strict-C ABI field/minimum and host negotiation, identity, and sanitized wrapper.
3. Add the provider predicate after shared validation, plus positive and invalid/no-mutation tests.
4. Extend direct preflight/emission and fake graph evidence for the measured canonical producer.
5. Prove exact Slice 23 provider compatibility, adjacent negative boundaries, differential PTX,
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
before committing the completed plan and implementation with first commit line `slice 24`.
