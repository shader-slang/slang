# Slice 33: Add exact scalar float32 subtraction

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs the exact raw CUDA kernel
`*destination = left - right` for an AS1 `Ptr<float>` and two scalar float parameters. The V3
floating-binary family gains SUBTRACT and a semantic feature bit without adding a callback or
changing the V3 table size. LLVM/NVVM text, NVRTC differential PTX, `ptxas`, and runtime agreement
prove the result.

## Progress

- [x] (2026-08-27) Recorded the Slice 32 baseline: 207 names, SHA-256
  `5e9c007c59d45c4db5bf9724e6b76c039455d342330f06b8aa68cd2e5eb2316b`, Release 207/207, Debug
  10/10.
- [x] (2026-08-27) Confirmed `kIROp_Sub` is the canonical linked-IR producer and the existing
  floating-binary callback is the correct provider family.
- [x] (2026-08-27) Added compatible feature/operation negotiation and provider/fake `fsub`.
- [x] (2026-08-27) Added direct topology, capability, LLVM/NVVM, PTX, `ptxas`, runtime, and
  negative evidence; focused coverage passes 9/9.
- [x] (2026-08-27) Full Release NVVM coverage passes 214/214; the Release provider/main builds and
  Debug main build succeed; the established Debug compatibility lane passes 10/10; durable docs
  and the input-shape self-review are complete.

## Surprises and Discoveries

- Observation: the V3 floating prefix already has the exact operand/result shape subtraction needs.
  Evidence: `emitFloatingBinary(module, operation, left, right, outValue)` owns two same-type float
  operands and one result, while the facade/provider/fake currently accept only ADD.
  Consequence: extend the enum and feature set; do not add `emitFloat32Subtract` anywhere.

- Observation: float parameters and type construction currently require the Slice 31 ADD feature.
  Consequence: subtraction requires the established ADD/type prefix plus its independent SUBTRACT
  bit. This preserves compatibility with exact Slice 31/32 providers and keeps type construction's
  existing negotiated owner.

## Decision Log

- Decision: append `SCALAR_FLOAT32_SUBTRACT` as feature 21 and `FLOATING_BINARY_OP_SUBTRACT` as
  operation 1, with no table growth.
  Rationale: feature bits express semantic availability; the generic callback already expresses
  the operation ABI.
  Date/author: 2026-08-27, Codex.
  Revisit when: a floating operation changes arity/result or constrained-FP policy.

- Decision: emit ordinary unflagged LLVM `CreateFSub` and use exactly representable finite runtime
  values.
  Rationale: this matches Slice 18 policy and avoids claiming fast-math, NaN, denormal, signed-zero,
  or rounding behavior.
  Date/author: 2026-08-27, Codex.

## Outcomes and Retrospective

Feature 21 and floating operation 1 reuse the exact 464-byte x64/280-byte x86 prefix. A provider
without SUBTRACT returns unavailable before dispatch; a subtraction-only advertised partial prefix
is malformed; the complete fake records ordered operands generically. The real provider validates
the established float value contract and emits `CreateFSub` without flags.

The canonical graph is three parameters, one float `kIROp_Sub`, and one aligned store. Direct fake
evidence records operation SUBTRACT with parameters 1/2 and result-store destination 0. Verified
LLVM/NVVM text has exactly one `fsub float`, store/alignment/kernel metadata, no `fadd`, and no fast
flag. NVVM/NVRTC agree on `[64, 32, 32]`, `sub.f32`, store, no load/add; `ptxas` accepts both; RTX
5090 results are `7.5`, `-8.5`, `1280`.

Seven names raise the prefix from 207 to 214; sorted LF-terminated SHA-256 is
`6ba1df40ff963723a866c61cbf8518aba7596e23213d5743015397547c90af9d`. Focused tests pass 9/9.
The standalone Release provider and Release/Debug main test targets build successfully. Full
Release NVVM coverage passes 214/214 and the established Debug compatibility lane passes 10/10.

## Context and Current Pipeline

The first direct preflight pass currently routes float `kIROp_Add` through the float feature and
falls `kIROp_Sub` into signed-i32 validation. The second pass and emission similarly special-case
float ADD before the shared integer ADD/SUB path. The new branch must select semantics from the
canonical result type and exact opcode, then use the existing generic floating facade.

V3 initialization requires the complete float prefix only when ADD is advertised. It must require
that same prefix when SUBTRACT is advertised. `getFloatingPointType` must be available for either
feature, while `emitFloatingBinary` checks the feature corresponding to the requested operation.

## Scope and Non-Goals

In scope are scalar float32 entry parameters/device destination, exact two-operand subtraction,
unflagged `fsub`, feature negotiation, fake generic operation evidence, LLVM/NVVM text, differential
PTX, `ptxas`, runtime, and migrated unsupported-operation expectations.

Out of scope are constants, loads beyond Slice 32, helpers, phis, casts, multiply/divide/remainder,
negation, comparisons, FMA/contraction, half/double, vectors/aggregates, resources/atomics, new API
callbacks, text rewrites, and performance claims.

## Architecture and Invariants

An exact Slice 31/32 table without SUBTRACT remains valid and ADD continues working. Advertising
SUBTRACT requires the complete existing float prefix and both callbacks. Unknown enum values fail
in the facade before provider mutation. The provider accepts available/dominating same-module,
same-function LLVM `float` operands at an unterminated insertion point and emits ADD or SUBTRACT by
the stable enum with no flags.

Preflight requires float parameters/type prefix plus SUBTRACT only for canonical float `kIROp_Sub`.
Integer subtraction remains unchanged. Fake evidence uses the existing `FloatingBinary` family,
ordered operands, and generic failure maps.

## Interfaces and Dependencies

Change the V3 feature/operation enums, existing facade/provider dispatch, direct emitter, generic
fake, and decomposed NVVM tests. Update the design, ledger, and this plan. Do not add a struct field,
export, dependency, build target, or packaging rule.

## Milestones

1. Append feature/operation constants and prove old feature sets/table sizes remain compatible.
2. Extend facade, provider, and fake dispatch with output clearing and unknown-op no mutation.
3. Admit/emit exact float `kIROp_Sub` while preserving integer SUB and all adjacent float stops.
4. Prove generic fake topology and missing-SUBTRACT E52016 before module construction.
5. Verify LLVM/NVVM `fsub float`/store/no fast flags; compare `[64,32,32]`, `sub.f32`, store/no load;
   assemble and run exact signed cases through both routes.
6. Format, build provider and Release/Debug tests, run focused/full lanes, hash names, update docs,
   self-review, and commit `slice 33`.

## Validation and Acceptance

Run focused negotiation, invalid-provider, builder, direct, capability, PTX, `ptxas`, runtime, and
unsupported-boundary tests, then the full Release NVVM prefix and established Debug 10/10 outside
the sandbox. Acceptance requires unchanged table sizes/V2, verified LLVM/NVVM text, matching PTX
semantics, runtime agreement, formatted code, completed input-shape audit, and clean diff checks.

## Self-Review and Input-Shape Audit

No new production helper, fallback, custom equivalence, or representation repair is introduced.
The surviving branches are stable enum dispatch in the existing facade/provider family and typed
dispatch for canonical `kIROp_Sub`. The exact final linked shape is intentional: the front end
produces the same opcode for numeric subtraction and preserves canonical Float result/operand
types; the emitter owns backend subset selection and does not rebuild syntax or search graphs.

The test helper rename to `_populateFloat32BinaryKernel` and `_runFloat32BinaryKernel` consolidates
ADD/SUBTRACT shapes rather than adding per-operation provider wrappers. Thin registered tests retain
layer-specific diagnosis. Removing the typed subtraction branch restores the prior E52017 and
breaks the direct/integration evidence at the owning layer.

## Failure and Recovery

If libNVVM rejects `fsub`, inspect generic LLVM and audited text before changing the writer. If the
source introduces another producer, narrow it. Removing the new feature/enum branches restores
Slice 32 without disturbing ADD/load/provider layout. Never stage `external/slang-binaries/`.

## Artifacts and Hand-Off

The retained evidence is: unchanged 464-byte x64/280-byte x86 V3 layout; exact graph/text/PTX and
runtime results above; 214 names with hash
`6ba1df40ff963723a866c61cbf8518aba7596e23213d5743015397547c90af9d`; focused 9/9, full Release
214/214, and Debug 10/10; negative negotiation/shape evidence; and the completed self-review.
Durable facts are distilled into the design and capability ledger. Commit this plan with Slice 33
and continue unless blocked.
