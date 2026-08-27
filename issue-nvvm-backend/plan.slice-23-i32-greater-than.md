# Slice 23: Compare signed i32 values for greater-than through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to ship with its implementation, so this plan will be committed
with Slice 23 rather than left as an uncommitted working log.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts canonical greater-than comparison between two
signed `i32` values.

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left > right ? 1 : 0;
}
```

The final linked Slang IR must retain one exact Boolean comparison producer. The provider must
emit the matching LLVM signed-integer predicate only after complete validation. Direct NVVM and
NVRTC must agree on raw parameter widths, ordered comparison, global storage, `ptxas` acceptance,
and representative runtime results including signed extremes.

## Progress

- [x] (2026-08-27) Started from committed Slice 22 `dd63c773d` with focused NVVM 156/156 and the
  preservation matrix 10/10.
- [x] (2026-08-27) Re-read `.agent/PLANS.md`, traced `BuiltinOperationKind::Greater` to
  `kIROp_Greater`, and selected greater-than as the smallest adjacent incomplete Bucket 2 slice.
- [x] (2026-08-27) Measured final linked `cmpGT(left, right) : Bool`, the pre-change E52017
  `'cmpGT'` boundary, and NVRTC's `[64,32,32]`/`setp.gt.s32`/global-store reference PTX.
- [x] (2026-08-27) Appended and implemented the exact provider/host signed-greater-than capability
  and strict-C probes.
- [x] (2026-08-27) Extended direct preflight, capability gating, canonical value emission, and
  adjacent unsigned/wide/float/pointer negative coverage.
- [x] (2026-08-27) Added ABI, invalid/no-mutation, fake topology, exact Slice 22 provider, PTX,
  both `ptxas` lanes, and RTX 5090 runtime evidence.
- [x] (2026-08-27) Applied pinned clang-format 17.0.6, rebuilt the provider and Release host,
  passed focused 164/164 and preservation 10/10, updated the design and ledger, inspected the DLL,
  removed probes, and completed the input-shape self-review.

## Surprises and Discoveries

- Observation: `-dump-ir` itself continues through a CUDA-source/NVRTC diagnostic path even when
  the target option selects direct NVVM, so its successful PTX is not the direct-route boundary.
  Evidence: the dump retains `cmpGT(left, right) : Bool`; the same explicit command without
  `-dump-ir` and with the Slice 22 provider fails at E52017 `'cmpGT'` and writes no output file.
  Consequence: use the dump only for final linked-shape evidence, and use ordinary code generation
  with the explicit builder path for the before/after direct-route contract.

- Observation: signed greater-than remains a distinct final linked producer.
  Evidence: every late general IR dump retains exact `cmpGT(left, right) : Bool`; no reversed
  `cmpLT(right, left)` appears, and ordinary direct code generation stops on that exact opcode.
  Consequence: implement exact `kIROp_Greater`/`ICMP_SGT`; do not reuse less-than by reversing
  operands in the consumer.

- Observation: the appended callback grows the complete V2 table from 344 to 352 bytes on x64 and
  from 192 to 196 bytes on x86.
  Evidence: the strict ABI tests prove the new field begins at the former complete minimum, every
  intermediate byte count is malformed, and both complete layouts pass the C and C++ probes.
  Consequence: exact Slice 22 providers retain their established capabilities but report
  `scalar-integer-signed-greater-than=0` and gate `kIROp_Greater` before module creation.

- Observation: NVVM may spell the signed comparison as the logically inverse PTX predicate while
  preserving the selected result.
  Evidence: differential classification accepts token-safe `setp.gt.s32` or the equivalent
  inverted `setp.le.s32` selection, and runtime covers both sign directions and integer extremes.
  Consequence: compare observable signed semantics and dataflow rather than requiring one
  optimizer-specific PTX spelling.

## Decision Log

- Decision: implement exact signed-i32 greater-than before less-equal/greater-equal, resources, or
  exceptional arithmetic.
  Rationale: Bucket 2 remains the lowest incomplete bucket. Greater-than is a distinct canonical
  opcode with ordinary signed semantics and reuses the proven comparison-result/Boolean-consumer
  boundary. Resources cross a substantially broader ABI boundary; division and shifts retain
  exceptional-input or signedness policy questions.
  Date/author: 2026-08-27, Codex.
  Revisit when: final linking intentionally canonicalizes `>` to reversed signed less-than rather
  than retaining `kIROp_Greater`.

## Outcomes and Retrospective

Slice 23 accepts exact final-linked `kIROp_Greater(left, right) : Bool` only when both operands are
available signed `i32` values and the result is canonical Boolean. The shape is correct and
principled: ordinary source `left > right` produces that exact opcode through final linking, no
upstream phase creates a reversed less-than spelling, and removing the consumer case restores the
pre-change E52017 boundary. A producer repair, operand reversal, custom equivalence, syntax
reconstruction, or graph walk would therefore obscure the source of truth instead of fixing it.

The append-only V2 ABI adds `emitIntegerSignedGreaterThan` at the former Slice 22 minimum and grows
the complete table to 352 bytes on x64 and 196 bytes on x86. Exact older providers remain valid;
partial suffixes and null complete callbacks are invalid. Host dispatch sanitizes output handles,
and the LLVM 14 provider reuses the shared binary integer validator before its sole mutation,
exact `ICMP_SGT` construction. Invalid ownership, function, type, availability, dominance, and
insertion-point cases leave the module unchanged.

Unsigned, wide, and floating cases stop at their unsupported entry parameters; pointer
greater-than reaches the exact signed-i32 operand boundary. All stop before provider discovery.
Direct NVVM and NVRTC expose `[64, 32, 32]` parameter widths, signed 32-bit ordered comparison, and
one global `u32` store; CUDA 12.9 `ptxas` accepts both. RTX 5090 results agree for equal zero,
equal negative, both sign directions, and both orderings of `INT_MIN`/`INT_MAX`.

The rebuilt Release focused NVVM suite passes 164/164. The parser, routing/hash, unsupported,
sampler, CUDA compile/pass-through, and runtime-dispatch preservation lanes pass 10/10. The
provider exports exactly the V1/V2 getters; it depends ordinarily only on `KERNEL32.dll`, delay
loads `SHELL32.dll` and `ole32.dll`, and exposes no process-visible LLVM DLL dependency.

## Context and Current Pipeline

Slices 21 and 22 accept exact `kIROp_Eql` and `kIROp_Neq` with two signed-i32 operands and a
canonical Boolean result. The private provider shares one binary comparison validator among signed
less-than, equality, and inequality, then selects exact LLVM predicates. The Boolean handle feeds
established conditional control flow; no Boolean parameter, memory, return, or phi ABI exists.

Ordinary `>` lowering maps `BuiltinOperationKind::Greater` to `kIROp_Greater`. This slice first
measures the post-link/post-optimization shape. If it remains exact `kIROp_Greater`, preflight and
emission own that opcode directly. If optimization intentionally canonicalizes it to reversed
less-than, the slice must follow the measured producer rather than accepting both spellings.

## Scope and Non-Goals

In scope are exact two-operand signed-i32 greater-than with canonical Boolean result, one
append-only provider callback, stable identity/capability gating, provider validation before
mutation, direct fake topology, old-provider compatibility, adjacent negative types,
direct/NVRTC PTX, both `ptxas` lanes, and GPU runtime cases.

Out of scope are changes to less-than/equality/inequality; less-equal/greater-equal; unsigned,
wide, floating-point, pointer, vector, matrix, aggregate, or resource comparisons; Boolean ABI or
storage; shifts, division, remainder, new address spaces, builtins, waves, and optimization claims.

## Architecture and Invariants

The V2 table remains append-only. Slice 23 appends one complete callback after the 344-byte x64 /
192-byte x86 Slice 22 prefix. An exact Slice 22 provider remains valid for inequality and older
programs but gates greater-than after discovery and before module creation. Partial pointer sizes,
null complete callbacks, and failed provider outputs follow the established strict negotiation and
sanitization rules.

Slang preflight owns exact canonical opcode, signed-i32 policy, Boolean result, and operand
availability. The provider owns same-module/same-function/equal-scalar-integer-type validation and
the exact LLVM signed predicate. Validation must precede its sole instruction mutation. No source
syntax, operand reversal, arbitrary graph walk, fallback, or serialized-text repair belongs here.

## Interfaces and Dependencies

Append a dedicated `SlangNVVMEmitIntegerSignedGreaterThan_2` callback and corresponding
minimum-size macro, host support predicate/wrapper, identity bit, and terminal
`NVVMIRCapability`. The production baseline remains exact LLVM 14.0.6 with negotiated NVVM IR 2.0
text, CUDA 12.9 libNVVM/NVRTC/`ptxas`, and the available RTX 5090. No public Slang API changes.

## Milestones

1. Probe ordinary signed greater-than after final linking and record exact direct/NVRTC evidence.
2. Append the strict-C ABI field/minimum and host negotiation, identity, and sanitized wrapper.
3. Add the provider predicate after shared validation, plus positive and invalid/no-mutation tests.
4. Extend direct preflight/emission and fake graph evidence for the measured canonical producer.
5. Prove exact Slice 22 provider compatibility, adjacent negative boundaries, differential PTX,
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
before committing the completed plan and implementation with first commit line `slice 23`.
