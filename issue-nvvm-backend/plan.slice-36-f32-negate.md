# Slice 36: Add exact scalar float32 negation with a compatible V3 suffix

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs `*destination = -value` for an AS1
`Ptr<float>` and one scalar float parameter. A generic floating-unary callback is appended to the
V3 table, while providers ending at Slice 35's floating-binary field remain loadable when they do
not advertise the new semantic.

## Progress

- [x] (2026-08-27) Recorded the Slice 35 baseline: 228 names, SHA-256
  `99dec82e0909050b0dc909113dad988369dfe9b2666e5385faaec947c6c29bc7`, Release 228/228, Debug
  10/10, and a 464-byte x64/280-byte x86 V3 table.
- [x] (2026-08-27) Identified canonical float `kIROp_Neg`, the existing negative fixture, and an
  append-only V3 floating-unary callback as the producer, evidence, and ABI ownership points.
- [x] (2026-08-27) Added feature negotiation, a stable unary operation, facade/provider/fake dispatch, and exact
  unflagged LLVM `fneg`.
- [x] (2026-08-27) Admitted only canonical float32 negation through direct preflight and emission while preserving
  signed-i32 negation and all other negate diagnostics.
- [x] (2026-08-27) Added shared descriptor-backed negotiation, text, topology, PTX, `ptxas`, and
  runtime evidence without a copied unary layer harness.
- [x] (2026-08-27) Formatted, built standalone/Release targets, passed focused 11/11 and full
  Release 235/235, updated durable docs, and recorded the exact name hash and line delta.
- [x] (2026-08-27) Built Debug targets, passed the established Debug 10/10, completed the
  input-shape/self-review audit, and prepared the exact slice for commit.

## Surprises and Discoveries

- The appended pointer grows x64 V3 from 464 to 472 bytes. On x86 it occupies four bytes of prior
  tail padding, so the table remains 280 bytes; feature absence, not padding contents, keeps an old
  provider from dispatching the new callback.
- CUDA 12.9 libNVVM rejects LLVM 14's `fneg` in NVVM-2.0 text with `parse expected instruction
  opcode`. An audited, semantic-count-matched writer rewrite to `fsub float -0.0, value` is accepted
  and produces `neg.f32` through both libNVVM and NVRTC.
- Generalizing the four-row binary descriptor and its layer runners to five-row float arithmetic
  avoided a copied unary harness. The five measured files grow 281 physical lines, from 19,560 to
  19,841, including the new ABI-prefix and legacy-writer contracts.

## Decision Log

- Decision: append `SCALAR_FLOAT32_NEGATE` as feature 24 and one `emitFloatingUnary` callback to
  V3 rather than encoding negation as subtraction from a synthesized zero.
  Rationale: `kIROp_Neg` is a canonical unary producer and LLVM has an exact `fneg`; preserving
  that shape avoids invented constants, graph reconstruction, and the signed-zero semantics of a
  different operation.
  Date/author: 2026-08-27, Codex.

- Decision: keep ABI version 3 and make feature 24 require the new complete suffix, while accepting
  an exact Slice 35-sized table when the feature is absent.
  Rationale: V3 explicitly uses append-only generic callbacks plus independent semantic bits. Its
  size-bounded copy contract already supports older providers; a new ABI version would duplicate
  the frozen V2 compatibility core without a representation break.
  Date/author: 2026-08-27, Codex.

- Decision: retain canonical `CreateFNeg` in the provider module and rewrite only exact validated
  fneg lines in negotiated NVVM-2.0 text to `fsub float -0.0, value`.
  Rationale: the dialect limitation belongs at the existing compatibility writer, not Slang IR or
  provider graph construction. Semantic instruction and rewritten-line counts must agree, generic
  LLVM text remains unchanged, and finite test cases avoid unsupported NaN-payload claims.
  Date/author: 2026-08-27, Codex.

- Decision: replace the binary-only test descriptor with an operand-counted float-arithmetic
  descriptor and reuse every same-layer runner for unary negation.
  Rationale: operand arity changes the provider callback and launch ABI but not the surrounding
  negotiation, topology, text, PTX, assembler, or runtime contracts. One descriptor remains the
  source of truth and prevents another copied layer harness.
  Date/author: 2026-08-27, Codex.

## Outcomes and Retrospective

Feature 24 and floating-unary operation 0 append a 472-byte x64/280-byte x86 V3 suffix while an
exact 464-byte x64/280-byte x86 Slice 35 provider remains valid without the bit. The direct graph is
`[FloatPointer, Float]`, with parameter 1 as the unary operand and its result stored through
parameter 0. Signed-i32 negation is unchanged; the old float-negate-plus-cast fixture now reaches
E52017 `castFloatToInt`.

Generic LLVM text has exactly one unflagged `fneg float`; negotiated NVVM-2.0 text has exactly one
audited `fsub float -0.000000e+00, value` and no `fneg`. NVVM and NVRTC agree on `[64, 32]`,
`neg.f32`, one store, and no load or binary float operation. Both pass `ptxas`; RTX 5090 runtime
results are `-1.5`, `8`, and `-1024`.

Seven names raise the Release prefix from 228 to 235 with sorted LF-terminated SHA-256
`2b79918702a9b21110af8251944e4428001a4ea69a2ff79b7a18e488cd13b4ba`. Focused tests pass 11/11,
full Release passes 235/235, standalone and Release builds succeed, and the shared evidence files
measure 19,841 lines. Debug targets build and the established cross-backend regression set passes
10/10.

## Context and Current Pipeline

Slang lowers unary floating negation to canonical `kIROp_Neg`, the same opcode used by signed-i32
negation but with canonical Float result and operand types. `_validateNVVMFunction` currently owns
signed-i32 admission and diagnoses the existing floating fixture as `signed i32 arithmetic
negation`. Emission later dispatches accepted signed-i32 negation through the generic V3 integer
unary family.

`SlangNVVMBuilderAPI_V3` currently ends at `emitFloatingBinary`. The provider copies only the
caller's capacity, the host retains only the provider's reported size, and feature validation
already ties an appended callback prefix to advertised semantics. Slice 36 extends that mechanism
by one suffix rather than modifying the established callback layout.

## Scope and Non-Goals

In scope are exact scalar float32 negation of an entry parameter, AS1 destination store, independent
feature negotiation, one generic floating-unary operation, unflagged `fneg`, compatibility with an
exact Slice 35 V3 prefix, and descriptor-backed fake/text/PTX/assembler/runtime evidence.

Out of scope are unsigned/wide/integer behavior changes, constants, casts, helpers, phis, absolute
value, reciprocal, remainder, fast/constrained math, NaN/Inf/denormal/signed-zero claims beyond
ordinary unflagged `fneg`, half/double, vectors/aggregates, resources/atomics, and other text
rewrites.
The sole text conversion in scope is the exact semantic-count-matched fneg downgrade required by
the negotiated LLVM-7-era NVVM-2.0 dialect.

## Architecture and Invariants

Feature 24 requires the complete appended callback suffix and the established float type callback.
An older table ending after `emitFloatingBinary` remains valid if it does not advertise feature 24.
Unknown unary operations clear output and fail before provider dispatch. The provider applies the
existing ownership, availability, dominance, function, insertion, and exact LLVM-float contracts.

First-pass direct validation distinguishes canonical Float and signed-i32 `kIROp_Neg` by result
type. Second-pass validation uses the matching typed operand path. Emission consumes the already
lowered operand without creating zero or rebuilding IR. The existing signed-i32 path and
unsigned/wide negative fixtures retain their behavior.

## Interfaces and Dependencies

Append the V3 feature, floating-unary operation type/value, callback typedef, table field, suffix
size constant, and facade method. Extend host/provider/fake dispatch, direct validation/emission,
test descriptors/runners, design, ledger, and this plan. Extend the existing audited NVVM-2.0
writer with one bounded fneg downgrade. Add no export, dependency, ABI version, V2 field, build
target, packaging rule, or general text-rewrite framework.

## Milestones

1. Append feature 24 and the floating-unary callback, then prove exact Slice 35 tables remain valid
   without the feature and partial/absent new suffixes fail when it is advertised.
2. Add stable NEGATE operation dispatch and exact unflagged provider `CreateFNeg` with invalid-
   operation and invalid-handle coverage.
3. Route canonical float32 `kIROp_Neg` through float validation/emission while keeping signed-i32
   negation on its existing generic family and other types unsupported.
4. Generalize the float-arithmetic descriptor by operand count and add one row plus thin registered
   wrappers for negotiation, provider text, direct topology/capability, differential PTX, `ptxas`,
   and exact runtime comparison.
5. Format, build standalone provider and Release/Debug targets, run focused/full lanes, measure
   table sizes/test growth, hash names, update docs, complete the input-shape audit, and commit
   `slice 36`.

## Validation and Acceptance

Run the new unary wrappers plus invalid-provider, signed-i32-negate, and unsupported-boundary tests,
then the full Release NVVM prefix and established Debug 10/10 outside the sandbox. Build the
standalone Release provider and Release/Debug test targets outside the sandbox. Acceptance requires
old-prefix compatibility, a complete new suffix, no lost names, exactly one unflagged `fneg float`
in generic LLVM text and one audited legacy fsub in NVVM-2.0 text,
ordered fake topology, matching PTX/assembler/runtime behavior, unchanged old diagnostics, formatted
code, a completed input-shape audit, and clean diff checks.

## Self-Review and Input-Shape Audit

The production inventory contains one append-only callback/facade method, one canonical result-type
branch in direct validation/emission, and one NVVM-dialect writer rewrite. The callback follows the
existing generic-family suffix contract: feature 24 requires both the float-type callback and the
complete unary suffix, and callers without the bit can retain the exact Slice 35 prefix. Unknown
operation values clear output and fail before dispatch.

The exact input reaching the direct branch is canonical Float `kIROp_Neg`, produced by normal Slang
lowering for `-value`. This is an intentionally valid spelling shared with signed-i32 negation, so
the canonical result type is the existing semantic source of truth. First-pass admission and
second-pass operand validation use that type; emission consumes the original operand and neither
walks arbitrary graphs nor synthesizes a zero or alternate expression. The existing signed-i32
path remains intact, while removing the float branch restores the motivating unsupported-type
failure.

The text rewrite is the only flagged special case. The provider deliberately builds canonical,
unflagged LLVM `fneg`; a direct CUDA 12.9 libNVVM probe rejects that LLVM 14 spelling at the
LLVM-7-era NVVM-2.0 dialect boundary. Therefore the producer graph is not malformed and changing
it would corrupt generic LLVM output. The negotiated writer owns this compatibility conversion,
counts semantic `fneg` instructions, rewrites only exact printed instruction lines to `fsub float
-0.000000e+00, operand`, requires the two counts to agree, and fails closed otherwise. Generic LLVM
text remains `fneg`, and the finite runtime corpus avoids claiming untested NaN payload behavior.

The test inventory replaces the binary-only descriptor with a one-or-two-operand descriptor rather
than adding a parallel unary harness. The descriptor remains the single source of truth, and each
shared runner retains its layer-specific callback ordering, text, topology, PTX, assembler, or
runtime assertions. No new structural equivalence relation, semantic fallback, syntax
reconstruction, or graph-search helper survives in the diff.

## Failure and Recovery

If LLVM 14, the audited NVVM writer, or libNVVM rejects `fneg`, isolate exact LLVM/NVVM text before
changing semantics. If PTX expresses negation through another legal instruction, assert the
semantic instruction family token-safely and preserve runtime evidence. Removing the appended
feature/callback/mapping/descriptor restores Slice 35. Never stage `external/slang-binaries/`.

## Artifacts and Hand-Off

The retained evidence is: 464-byte old and 472-byte new x64 prefixes (both 280 bytes on x86), direct
graph `[FloatPointer, Float]`, generic `fneg` plus audited NVVM `fsub`, matching `[64, 32]`
`neg.f32` PTX, `ptxas` acceptance on both routes, RTX 5090 results `-1.5`, `8`, and `-1024`, focused
11/11, Release 235/235, Debug 10/10, sorted-name hash
`2b79918702a9b21110af8251944e4428001a4ea69a2ff79b7a18e488cd13b4ba`, and a 281-line shared-test
delta. Durable facts are in the design and capability ledger; this completed plan ships with Slice
36.
