# Slice 41: Add exact scalar float32 ordered greater-than-or-equal

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs
`*destination = left >= right ? 1 : 0` for two scalar float parameters and an AS1 `Ptr<int>`
destination, using ordered NaN semantics through the existing V3 comparison family.

## Progress

- [x] (2026-08-27) Recorded Slice 40 baseline: 263 names, SHA-256
  `f93467f3b27def96040db05fca0fec79c5e22a5010ae6a3226fab4d249d860a1`, Release 263/263,
  Debug 10/10, unchanged 480-byte x64/288-byte x86 table, and 20,789 measured lines.
- [x] (2026-08-27) Selected the existing floating greater-equal fixture and canonical Bool
  `kIROp_Geq` as the next exact measured boundary.
- [x] (2026-08-27) Added independent feature/operation/provider/fake dispatch through the fixed
  comparison family.
- [x] (2026-08-27) Routed exact float32 `kIROp_Geq` through shared classification while preserving
  signed-i32 greater-equal.
- [x] (2026-08-27) Added one descriptor row and seven thin registered evidence wrappers.
- [x] (2026-08-27) Formatted, built, ran focused/full/Debug lanes, hashed names, measured marginal
  growth, updated docs, completed self-review, and prepared the exact slice commit.

## Surprises and Discoveries

- Observation: normal lowering preserves canonical `kIROp_Geq` with original Float operand order
  before the established Boolean consumer.
  Consequence: map it directly to ordered greater-equal; do not complement, swap, or reconstruct it.

- Observation: Slice 40 reduced the fourth predicate to 41 marginal measured lines.
  Consequence: preserve the same descriptor/helper shape and record Slice 41's marginal cost.

- Observation: feature 29 and operation 4 leave the complete and semantic suffix sizes unchanged;
  the comparison-family suffix validator gains only one feature predicate.
  Consequence: exact Slice 40 providers remain compatible without feature 29.

- Observation: the fifth row plus seven names adds 52 measured lines, retaining the low marginal
  cost of the descriptor-driven comparison family.
  Consequence: preserve independent evidence names without copying layer implementations.

- Observation: PTX uses direct `setp.ge.f32` or an unordered-less branch complement, while the
  runtime lane distinguishes ordered semantics.
  Consequence: keep token-safe relation-family recognition and runtime as the semantic oracle.

## Decision Log

- Decision: append feature 29 `SCALAR_FLOAT32_ORDERED_GREATER_EQUAL` and comparison operation 4
  `ORDERED_GREATER_EQUAL` without changing the table.
  Rationale: semantic capability remains independently negotiable on the stable family callback.
  Date/author: 2026-08-27, Codex.

- Decision: lower original-order operands to unflagged LLVM `fcmp oge`.
  Rationale: it exactly represents canonical `kIROp_Geq` and is false for NaN operands.
  Date/author: 2026-08-27, Codex.

## Outcomes and Retrospective

Feature 29 and floating-compare operation 4 use the unchanged 480-byte x64/288-byte x86 V3 table
and existing 284-byte x86 semantic suffix. Feature removal fails before provider dispatch; the
complete provider accepts operation 4 through the same callback.

Generic LLVM and negotiated NVVM-2.0 text contain exactly one unflagged `fcmp oge float`. Direct
topology is `[Pointer, Float, Float]` with original parameters 1 and 2 feeding one comparison whose
Bool drives the established four-block zero/one, integer-phi, aligned-i32-store graph. Signed-i32
greater-equal remains unchanged; only the matching floating negative advances to supported.

NVVM and NVRTC agree on `[64, 32, 32]`, token-safe float32 relation evidence, one global i32 store,
and no load, float arithmetic, or integer predicate. CUDA 12.9 `ptxas` accepts both. RTX 5090
returns one for `3.75 >= 1.5` and `+0 >= -0`, and zero for `-8 >= 0.5` and quiet `NaN >= -1`.

Seven names raise Release from 263 to 270 with sorted LF-terminated SHA-256
`5358536da56531d08b93bd3e2f55d25d3d8cc42a21e461b3a905b1425a1f1fc4`; removing them reproduces
Slice 40 exactly. Focused tests pass 14/14, full Release passes 270/270, Debug preservation passes
10/10, and all standalone/Release/Debug targets build. The five measured test/support files grow
52 lines from 20,789 to 20,841; production direct-emitter comparison code shrinks by combining the
old greater-equal block with the shared classifier.

## Context and Current Pipeline

The motivating ternary lowers to Bool `kIROp_Geq` over two Float entry parameters, then the shared
branch/constants/i32-phi/global-store graph. The floating classifier currently stops at less-equal,
so first-pass feature collection and second-pass value validation use signed-i32 greater-equal,
and Float operands fail. Emission likewise uses integer `SIGNED_GREATER_EQUAL`.

All facade/provider/fake families and descriptor-driven provider/direct/PTX/assembler/runtime
layers already exist and accept a stable comparison operation.

## Scope and Non-Goals

In scope are exact scalar float32 ordered greater-equal, feature 29/operation 4, `fcmp oge`,
canonical Bool control flow, unchanged V3 layout, and established evidence including quiet NaN.

Out of scope are less-than, integer/pointer behavior changes, Float constants/casts/helpers/phis,
Bool ABI, half/double, vectors/aggregates, fast/constrained math, resources, atomics, and performance
claims.

## Architecture and Invariants

Feature 29 requires the existing complete comparison suffix and Float-type callback. Slice 40
providers load without it. Facade maps operation to exact feature; provider validates exact Float
operands before `CreateFCmpOGE`; all failures clear output.

The closed direct classifier accepts only Bool result plus two canonical Float operands. Its
`kIROp_Geq` row drives feature collection, Float value validation, and emission. Signed-i32
greater-equal and adjacent fixtures remain unchanged. The descriptor row remains test data, not a
second production opcode mapping.

## Interfaces and Dependencies

Append one feature and operation. Extend existing family switches, classifier, descriptor, thin
registrations, design, ledger, and plan. Add no callback/field/suffix/ABI/V2/export/dependency/
target/text rewrite.

## Milestones

1. Add feature 29/operation 4 with unchanged layout and independent negotiation.
2. Emit exact provider `fcmp oge` through existing validation.
3. Admit canonical Float `kIROp_Geq`, preserve signed-i32 greater-equal, and advance only its
   fixture.
4. Add one descriptor row plus seven layer registrations and finite/signed-zero/NaN runtime cases.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, commit `slice 41`.

## Validation and Acceptance

Run seven new wrappers plus layout, unknown/invalid provider, earlier Float comparisons,
signed-i32 greater-equal, and the unsupported matrix. Run full Release NVVM and Debug 10/10
outside sandbox; build standalone Release provider and Release/Debug test targets.

Accept unchanged layout, independent gating, one unflagged `fcmp oge float` per text dialect,
original fake operands, token-safe Float relation PTX, `ptxas`, matching finite/signed-zero/NaN RTX
truth tables, exact name continuity, stable adjacent diagnostics, formatted code, completed audit,
and clean diff checks.

## Self-Review and Input-Shape Audit

The production inventory adds one feature, one operation row to existing facade/provider switches,
and one row to the closed direct classifier. No callback, representation, fallback, rewrite, or new
production helper is added. Comparison-family suffix validation stays shared.

Normal lowering of `*destination = left >= right ? 1 : 0` produces canonical Bool `kIROp_Geq`
with two canonical Float operands in original order. Operand type is the existing semantic source
of truth because signed-i32 and Float comparisons share Bool result type. Feature collection, value
validation, and emission consume the same bounded mapping. Removing the row restores the motivating
`signed i32 value` diagnostic. The emitter therefore owns family dispatch; the producer shape is
canonical and needs no repair. No operand swap, complement, syntax reconstruction, structural
equivalence, graph walk, alternate Bool, or text rewrite survives.

The test inventory adds one descriptor row and seven macro registrations. Provider text, direct
graph, PTX, assembler, and runtime bodies remain shared. Runtime proves ordered NaN semantics where
PTX may use a branch complement. No test-specific provider fallback is introduced.

## Failure and Recovery

Inspect exact generic/NVVM text if `fcmp oge` fails. PTX may use direct or unordered-less branch
complement; runtime is semantic oracle. Removing feature/operation/classifier/descriptor rows
restores Slice 40. Never stage `external/slang-binaries/`.

## Artifacts and Hand-Off

The retained evidence is: unchanged 480-byte x64/288-byte x86 table and 284-byte x86 semantic
suffix; original-order direct `[Pointer, Float, Float]` graph; exact `fcmp oge` text; matching
`[64, 32, 32]` PTX; `ptxas`; finite/signed-zero/quiet-NaN RTX results; focused 14/14, Release
270/270, Debug 10/10; name hash
`5358536da56531d08b93bd3e2f55d25d3d8cc42a21e461b3a905b1425a1f1fc4`; and 52 marginal lines.
Durable evidence is in design and ledger; this completed plan ships with Slice 41.
