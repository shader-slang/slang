# Slice 35: Add exact scalar float32 division

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs `*destination = left / right` for an AS1
`Ptr<float>` and two scalar float parameters. DIVIDE extends the existing V3 floating-binary family
and the Slice 34 production/test descriptors without a callback, table growth, or copied harness.

## Progress

- [x] (2026-08-27) Recorded the Slice 34 baseline: 221 names, SHA-256
  `c24e6b4e82e289c2533444b0b0c0dab6cc44064a1df02d75a79928de94c2afa8`, Release 221/221, Debug
  10/10, and 19,512 measured test/support lines.
- [x] (2026-08-27) Confirmed canonical `kIROp_Div`, the generic floating-binary ABI, and the closed
  emitter/test mappings are the owning extension points.
- [x] (2026-08-27) Added compatible semantic negotiation, unflagged provider `fdiv`, and typed
  direct lowering; preserved integer division as unsupported.
- [x] (2026-08-27) Added one descriptor and thin registered wrappers for negotiation, text,
  topology, PTX, `ptxas`, and runtime evidence.
- [x] (2026-08-27) Formatted, built, ran focused/full Release and Debug validation, updated docs,
  completed the self-review, and prepared the slice for its required commit.

## Surprises and Discoveries

- The existing descriptor runners accepted division without a copied runner. Seven registered names
  added only 48 physical lines across the five measured test/support files (19,512 to 19,560).
- LLVM and libNVVM accepted ordinary unflagged `fdiv`. PTX used a legal 32-bit division-family
  spelling, so the existing token-aware instruction-family summary was the stable assertion layer.

## Decision Log

- Decision: append `SCALAR_FLOAT32_DIVIDE` as feature 23 and `FLOATING_BINARY_OP_DIVIDE` as
  operation 3 without changing the V3 table.
  Rationale: the semantic is same-type, two-operand float32 and already fits the generic callback.
  Date/author: 2026-08-27, Codex.

- Decision: emit ordinary unflagged LLVM `CreateFDiv` and validate exactly representable finite,
  nonzero-denominator cases.
  Rationale: this preserves the Slice 18 FP policy and avoids claims about reciprocal transforms,
  division by zero, NaN, denormals, signed zero, or fast-math accuracy.
  Date/author: 2026-08-27, Codex.

## Outcomes and Retrospective

Feature 23 and wire operation 3 extend the V3 semantic prefix without changing its 464-byte x64 or
280-byte x86 table, so exact older providers remain compatible. The direct graph is
`[FloatPointer, Float, Float]` with ordered operands and a store of the division result. The LLVM
provider emits exactly one unflagged `fdiv float`, retains alignment and kernel metadata, and emits
no other floating-binary operation.

Differential PTX agrees on parameter widths `[64, 32, 32]`, one token-safe 32-bit division family,
and the result store, with no load/add/subtract/multiply family. Both routes pass `ptxas`; CUDA
runtime comparison on an RTX 5090 produced exact results `4`, `-16`, and `-4`. The descriptor added
seven names, taking the exact Release prefix from 221 to 228 with SHA-256
`99dec82e0909050b0dc909113dad988369dfe9b2666e5385faaec947c6c29bc7`.

The focused matrix passed 9/9, the full Release prefix passed 228/228, and the established Debug
lane passed 10/10. Standalone provider, Release, and Debug builds succeeded. Integer division still
fails with E52017 and is the next honest division boundary.

## Context and Current Pipeline

Numeric lowering produces canonical `kIROp_Div` for both integer and floating division. Direct
preflight currently falls through to E52017 for all spellings. Slice 34's
`_getNVVMFloat32BinaryInfo` is the single source for accepted float opcode feature, wire operation,
and diagnostic; first-pass capability collection and emission both consume it. The second pass
validates canonical float operands through `_validateFloat32Value`.

`NVVMIRBuilder::emitFloatingBinary`, `_emitFloatingBinaryV3`, and the fake dispatch stable wire
operations generically. `NVVMFloat32BinaryTestCase` supplies the same-shape layer runners with
feature, operation, source, kernel/text tokens, and exact runtime cases.

## Scope and Non-Goals

In scope are exact scalar float32 division of entry parameters, AS1 destination store, independent
feature negotiation, unflagged `fdiv`, descriptor-backed fake/text/PTX/assembler/runtime evidence,
and preservation of the integer-division negative boundary.

Out of scope are integer division support or its divide-by-zero/overflow policy, constants, casts,
helpers, phis, remainder, reciprocal approximation, fast math, division by zero, NaN/Inf/denormal/
signed-zero claims, half/double, vectors/aggregates, resources/atomics, and text rewrites.

## Architecture and Invariants

Exact Slice 34 feature sets remain valid. Advertising DIVIDE requires the complete existing float
prefix, and the facade checks feature 23 before provider dispatch. Unknown operations clear output
and fail without mutation. The provider accepts only the established LLVM `float` ownership,
availability, dominance, function, and insertion contract, then emits unflagged `CreateFDiv`.

Only canonical float `kIROp_Div` joins the closed mapping and second-pass float validation. Integer
`kIROp_Div` remains E52017 before provider discovery. One descriptor row supplies all same-shape
test facts; existing ADD/SUBTRACT/MULTIPLY names and evidence remain unchanged.

## Interfaces and Dependencies

Append the V3 feature and floating-operation constants. Extend existing host/provider/fake switches,
emitter mapping/cases, and test descriptor/wrappers. Update design, ledger, and this plan. Add no
field, export, dependency, provider wrapper, build target, or packaging rule.

## Milestones

1. Append feature 23/operation 3 and prove old table sizes/feature sets remain compatible.
2. Extend generic host/provider/fake dispatch and emit exact unflagged `CreateFDiv`.
3. Admit canonical float `kIROp_Div` through the closed emitter mapping while retaining integer DIV
   as unsupported.
4. Add one source/kernel/runtime descriptor row, PTX evidence bit, and seven thin registered names.
5. Verify missing feature E52016, ordered fake topology, exactly one `fdiv float`, `[64,32,32]`,
   token-safe float-divide PTX, assembler acceptance, and exact runtime agreement.
6. Format, build provider and Release/Debug targets, run focused/full lanes, hash names, measure
   lines, update docs, self-review, and commit `slice 35`.

## Validation and Acceptance

Run the four-operation descriptor wrappers plus invalid-provider and unsupported-boundary tests,
then the full Release NVVM prefix and established Debug 10/10 outside the sandbox. Build standalone
Release provider and Release/Debug test targets outside the sandbox. Acceptance requires unchanged
V3 x64/x86 sizes and V2, no lost names, exact LLVM/NVVM/PTX semantics, runtime agreement, preserved
integer DIV rejection, formatted code, completed input-shape audit, and clean diff checks.

## Self-Review and Input-Shape Audit

Audit every helper, fallback, mapping, and special case. Expected production change is one new row
in the closed canonical-op mapping plus stable enum cases. Confirm canonical Float result and
operands intentionally own this path, integer DIV remains rejected, no syntax/graph reconstruction
appears, and test descriptors retain all per-layer assertions.

Completed audit: no new helper, fallback, or structural equivalence was introduced. The sole
production mapping addition is canonical Float `kIROp_Div`; first-pass type discrimination keeps
integer division on E52017, and the second-pass assertion records that invariant rather than
repairing malformed IR. Emission uses the existing typed operands and value graph directly. One
test descriptor row drives the existing runners while preserving every layer-specific assertion.

## Failure and Recovery

If libNVVM rejects `fdiv` or PTX differs by legal spelling, inspect LLVM/audited NVVM text and
token-safe PTX evidence before changing policy. If source introduces another producer, narrow or
split it. Removing DIVIDE enum/mapping/descriptor entries restores Slice 34. Never stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

The exact compatibility, graph, text, PTX, runtime, count/hash, 19,560-line measurement,
Release/Debug, negative, and self-review evidence is recorded above and distilled into the design
and capability ledger. Commit this plan with Slice 35 and continue unless blocked.
