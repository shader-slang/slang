# Slice 39: Add exact scalar float32 ordered greater-than

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs
`*destination = left > right ? 1 : 0` for two scalar float parameters and an AS1 `Ptr<int>`
destination. It reuses the generic V3 comparison callback and descriptor-driven layer tests,
including ordered NaN behavior, without growing the V3 table.

## Progress

- [x] (2026-08-27) Recorded the Slice 38 baseline: 249 names, SHA-256
  `529af4d3eba39ba0aabd6ca881ca3ac66b5f30c5f272c75a54a3b5cdc15156ea`, Release 249/249,
  Debug 10/10, a 480-byte x64/288-byte x86 table, and 20,688 measured test/support lines.
- [x] (2026-08-27) Selected the existing floating greater-than unsupported fixture and canonical
  Bool `kIROp_Greater` as the next exact measured boundary.
- [x] (2026-08-27) Appended feature/operation negotiation and provider/fake ordered-greater-than dispatch through
  the unchanged floating-compare family.
- [x] (2026-08-27) Admitted exact float32 `kIROp_Greater` through shared classification without changing signed-i32
  greater-than or adjacent diagnostics.
- [x] (2026-08-27) Added one descriptor row and seven thin registered wrappers for all established evidence layers.
- [x] (2026-08-27) Formatted, built, ran focused/full/Debug lanes, hashed names, measured marginal
  growth, updated docs, completed self-review, and prepared the exact slice commit.

## Surprises and Discoveries

- Observation: the existing `kDirectNVVMFloatingGreaterThanSource` lowers to canonical Bool
  `kIROp_Greater` with two Float operands, then reaches the signed-i32 validator.
  Consequence: add one closed classifier row; do not normalize by swapping operands or invent an
  alternative less-than spelling.

- Observation: Slice 38 made every comparison layer accept a descriptor operation.
  Consequence: this slice should add test data and thin names, with no copied provider, PTX,
  assembler, direct, or runtime body.

- Observation: feature 27 and operation 2 leave the complete and semantic suffix sizes unchanged;
  the comparison-family suffix validator gains only one feature predicate.
  Consequence: exact Slice 38 providers remain compatible without feature 27.

- Observation: the third row plus seven names adds 60 measured lines, down from 185 for Slice 38
  and 662 for the first family row in Slice 37.
  Consequence: descriptor-driven tests are reaching the intended low marginal cost while retaining
  independent layer registrations.

- Observation: PTX uses a stable `setp.gt.f32` or unordered-less-equal branch complement for this
  graph, while runtime distinguishes ordered semantics.
  Consequence: extend token-safe relation-family recognition and keep runtime as semantic oracle.

## Decision Log

- Decision: append `SCALAR_FLOAT32_ORDERED_GREATER_THAN` as feature 27 and
  `ORDERED_GREATER_THAN` as operation 2 on `emitFloatingCompare`.
  Rationale: availability stays independently negotiable while callback/table layout remains fixed.
  Date/author: 2026-08-27, Codex.

- Decision: lower to unflagged LLVM `fcmp ogt` with the original operand order.
  Rationale: canonical `kIROp_Greater` is the measured producer, and ordered comparison must be
  false if either operand is NaN.
  Date/author: 2026-08-27, Codex.

## Outcomes and Retrospective

Feature 27 and floating-compare operation 2 use the unchanged 480-byte x64/288-byte x86 V3 table
and existing 284-byte x86 semantic suffix. Feature removal fails before provider dispatch; the
complete provider accepts operation 2 through the same callback.

Generic LLVM and negotiated NVVM-2.0 text contain exactly one unflagged `fcmp ogt float`. Direct
topology is `[Pointer, Float, Float]` with original parameters 1 and 2 feeding one comparison whose
Bool drives the established four-block zero/one, integer-phi, aligned-i32-store graph. Signed-i32
greater-than remains unchanged; only the matching floating negative advances to supported.

NVVM and NVRTC agree on `[64, 32, 32]`, token-safe float32 relation evidence, one global i32 store,
and no load, float arithmetic, or integer predicate. CUDA 12.9 `ptxas` accepts both. RTX 5090
returns one for `3.75 > 1.5` and zero for `-8 > 0.5`, `+0 > -0`, and quiet `NaN > -1`.

Seven names raise Release from 249 to 256 with sorted LF-terminated SHA-256
`f8b9a58433982e2583a7310c3e2bc43c82767adee115d121a13147783a8a6fcf`; removing them reproduces
Slice 38 exactly. Focused tests pass 14/14, full Release passes 256/256, Debug preservation passes
10/10, and all standalone/Release/Debug targets build. The five measured test/support files grow
60 lines from 20,688 to 20,748; production direct-emitter comparison code shrinks by combining the
old greater-than block with the shared classifier.

## Context and Current Pipeline

Normal lowering turns the motivating ternary into canonical Bool `kIROp_Greater` over two Float
entry parameters, followed by the established branch/zero-one/i32-phi/global-store consumer.
`_getNVVMFloat32CompareInfo` currently accepts equality and inequality only. First-pass validation
therefore requests signed-i32 greater-than, and second-pass operand validation rejects the Float
values. Emission routes surviving `kIROp_Greater` to integer operation `SIGNED_GREATER_THAN`.

Feature-specific facade dispatch, exact provider Float validation, comparison descriptors, fake
operation recording, PTX summarization, `ptxas`, and CUDA runtime utilities already exist.

## Scope and Non-Goals

In scope are exact scalar float32 ordered greater-than of entry parameters, canonical Bool control
flow, feature 27/operation 2, exact unflagged `fcmp ogt`, unchanged V3 layout, and the established
fake/text/PTX/assembler/runtime evidence including quiet NaN.

Out of scope are less-than, less-equal, greater-equal, integer/pointer changes, constants/casts/
helpers/phis for Float, Bool ABI, half/double, vectors/aggregates, fast/constrained math, resources,
atomics, and performance claims. Quiet-NaN cases prove ordered truth value only.

## Architecture and Invariants

Feature 27 requires the existing complete floating-compare suffix and Float-type callback. Slice 38
providers remain loadable without it. Facade and provider switches accept only stable operations,
clear failed outputs, and preserve exact feature gating. Provider validation remains shared before
`CreateFCmpOGT`.

The direct classifier accepts only canonical Bool results with two canonical Float operands and
maps `kIROp_Greater` directly; the same helper drives feature collection, value validation, and
emission. Signed-i32 greater-than retains its current feature/operation and every adjacent fixture
retains its deterministic diagnostic.

The descriptor row owns source, feature, operation, kernel, LLVM opcode, PTX evidence, and runtime
truth table. Thin independently registered wrappers preserve layer-local failure names.

## Interfaces and Dependencies

Append one feature and one operation value. Extend existing facade/provider/fake/direct switches,
one descriptor row, registered wrappers, design, ledger, and this plan. Add no callback, field,
suffix size, ABI version, V2 change, export, dependency, build target, or text rewrite.

## Milestones

1. Append feature 27/operation 2 with unchanged table sizes and independent negotiation.
2. Emit exact provider `fcmp ogt` after existing validation, including invalid-operation and output
   sanitization preservation.
3. Route canonical Float `kIROp_Greater` through the shared classifier while preserving signed-i32
   greater-than and removing only the matching unsupported fixture.
4. Add one comparison descriptor row plus seven thin registered wrappers; prove finite,
   signed-zero, and quiet-NaN behavior against NVRTC.
5. Format/build, run focused/full/Debug lanes, hash names, measure growth, update durable docs,
   complete the input-shape audit, and commit `slice 39`.

## Validation and Acceptance

Run the seven new wrappers plus V3 layout, invalid provider, ordered equality/inequality,
signed-i32 greater-than, adjacent floating comparisons, and unsupported-boundary tests. Run the
complete Release NVVM prefix and established Debug 10/10 outside the sandbox. Build the standalone
Release provider and Release/Debug test targets outside the sandbox.

Acceptance requires unchanged V3 layout, independent feature gating, exactly one unflagged
`fcmp ogt float` in both text dialects, original ordered fake operands, matching token-safe
float-relation PTX, `ptxas`, finite/signed-zero/quiet-NaN runtime truth tables, exact name/hash
continuity, unchanged adjacent diagnostics, formatted code, completed audit, and clean diff checks.

## Self-Review and Input-Shape Audit

The production inventory adds one feature, one operation row to existing facade/provider switches,
and one row to the closed direct classifier. No callback, representation, fallback, rewrite, or new
production helper is added. Comparison-family suffix validation stays shared.

Normal lowering of `*destination = left > right ? 1 : 0` produces canonical Bool
`kIROp_Greater` with two canonical Float operands in original order. Operand type is the existing
semantic source of truth because signed-i32 and Float comparisons share Bool result type. Feature
collection, value validation, and emission consume the same bounded mapping. Removing the row
restores the motivating `signed i32 value` diagnostic. The emitter therefore owns family dispatch;
the producer shape is canonical and needs no repair. No operand swap, syntax reconstruction,
structural equivalence, graph walk, alternate Bool, or text rewrite survives.

The test inventory adds one descriptor row and seven macro registrations, while converting
post-Slice-37 comparison feature negotiation to the descriptor helper. Provider text, direct graph,
PTX, assembler, and runtime bodies are unchanged. Runtime proves ordered NaN semantics where PTX
may use a branch complement. No test-specific provider fallback is introduced.

## Failure and Recovery

If LLVM 14 or libNVVM rejects `fcmp ogt`, inspect exact generic/NVVM-2.0 text before changing
semantics. PTX may use an ordered predicate or a branch complement; retain token-safe family
evidence and use runtime as the semantic oracle. Removing feature 27, operation 2, and their
classifier/descriptor rows restores Slice 38. Never stage `external/slang-binaries/`.

## Artifacts and Hand-Off

The retained evidence is: unchanged 480-byte x64/288-byte x86 table and 284-byte x86 semantic
suffix; original-order direct `[Pointer, Float, Float]` graph; exact `fcmp ogt` text; matching
`[64, 32, 32]` PTX; `ptxas`; finite/signed-zero/quiet-NaN RTX results; focused 14/14, Release
256/256, Debug 10/10; name hash
`f8b9a58433982e2583a7310c3e2bc43c82767adee115d121a13147783a8a6fcf`; and 60 marginal lines.
Durable evidence is in design and ledger; this completed plan ships with Slice 39.
