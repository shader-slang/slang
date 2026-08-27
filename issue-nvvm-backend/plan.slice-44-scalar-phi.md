# Slice 44: Generalize scalar phis and add float32 SSA merging

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs a conditional that merges two scalar
float32 parameters through canonical Slang block-parameter SSA before storing the selected value.
An append-only generic typed-phi suffix replaces the need for a future emit callback per scalar
type, while frozen V2 signed-i32 phis remain compatible.

## Progress

- [x] (2026-08-27) Recorded Slice 43 baseline: 284 names, SHA-256
  `3e78b6b3069dd0a12cbde4d78e4d804e5eeace161cdbf86d620262b5e9d9a72d`, Release 284/284,
  Debug 10/10, 488-byte x64/288-byte x86 V3 table, and 21,445 measured lines.
- [x] (2026-08-27) Audited canonical block parameters/branch arguments, the signed-i32 V2 phi
  callbacks, provider validation, fake storage, and direct placeholder/incoming phases.
- [x] (2026-08-28) Appended generic typed phi creation/incoming negotiation and shared
  provider/facade/fake dispatch; V3 is 504 bytes on x64 and 296 bytes on x86.
- [x] (2026-08-28) Admitted canonical float32 block parameters and arguments while preserving the
  frozen V2 signed-i32 SSA path.
- [x] (2026-08-28) Added seven independently named provider/direct/PTX/assembler/runtime evidence
  layers around a real two-predecessor Float merge.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, completed the input-shape
  audit, and prepared the complete slice for commit.

## Surprises and Discoveries

- Observation: Slang block parameters and positional branch arguments already are the canonical SSA
  source of truth; only the provider calls and type checks are integer-specific.
  Consequence: generalize at the typed phi boundary without reconstructing local variables or
  walking operand graphs.

- Observation: float32 phis need both creation and incoming-edge callbacks.
  Consequence: append one generic pair rather than a floating-only pair, and use one scalar fake
  phi representation for integer and Float values.

- Observation: this slice adds 709 measured test/support lines, from 21,445 to 22,154, because it
  establishes the generic callback pair, typed fake family, and reusable Float callable runtime
  harness as well as seven evidence names.
  Consequence: later scalar types can reuse those families instead of adding per-type callbacks or
  duplicating runtime setup; marginal evidence should return to descriptor-sized growth where the
  behavior permits it.

## Decision Log

- Decision: append feature 32 `SCALAR_PHI` plus generic
  `emitPhi(module, block, type, outValue)` and
  `addPhiIncoming(module, phi, value, predecessorBlock)` callbacks.
  Rationale: the type handle already carries the scalar type; duplicating callbacks for every
  scalar width/family would repeat the scaling problem the V3 families were introduced to solve.
  Date/author: 2026-08-27, Codex.

- Decision: retain frozen V2 integer phi facade/provider paths and use the generic V3 pair only for
  canonical Float block parameters in this slice.
  Rationale: exact old providers and existing signed-i32 feature semantics must remain valid.
  Date/author: 2026-08-27, Codex.

## Outcomes and Retrospective

Feature 32 appends one complete generic typed-phi pair. The x64 V3 table grows from 488 to 504
bytes and x86 grows from 288 to 296 bytes; exact Slice 43 tables remain accepted when they do not
advertise the feature. Both callbacks are required together, and null/short advertised suffixes
are rejected without mutation.

The direct fake graph has entry topology `[FloatPointer, Integer, Float, Float]`. Parameters 2 and
3 arrive on the two actual predecessor branches, feed one typed `ScalarPhi`, and reach the sole
Float store. Generic LLVM and negotiated NVVM-2.0 text each contain exactly one `phi float`.
NVVM/NVRTC PTX agree on `[64, 32, 32, 32]`, one global 32-bit store, and no global load, Float
arithmetic, or Float predicate; matching CUDA 12.9 `ptxas` accepts both. RTX 5090 runs agree for
finite choices and preserve the selected `-0.0` versus `+0.0` bit pattern.

The reusable generic phi/fake/runtime-family base plus seven evidence names adds 709 physical
lines across the five measured files, from 21,445 to 22,154. The focused matrix passes 14/14,
Release passes 291/291 with sorted LF-terminated name-set SHA-256
`c18462cd303630788566c59409f369ef57a46614652571a97663acf0ffb01690`, and removing the seven new
names reproduces Slice 43's 284-name hash exactly. Debug preservation passes 10/10.

## Context and Current Pipeline

Preflight registers every non-entry block parameter before value validation because emission first
creates all phi placeholders, emits bodies, then attaches incoming values from actual predecessor
edges. The representation is already principled and supports loops. Current checks require every
parameter and argument to be signed i32, and emission always calls the frozen V2 integer phi pair.

The LLVM provider likewise hardcodes `IntegerType` checks even though the remaining ownership,
placement, CFG, dominance, duplicate-edge, and exact-type validation applies equally to scalar
Float. The fake records `IntegerPhi` as a type-specific value kind, which would scale poorly if
copied for Float.

## Scope and Non-Goals

In scope are canonical scalar Float block parameters, matching Float branch arguments, one
append-only generic typed-phi callback pair/feature, one shared fake scalar-phi representation, a
finite conditional merge, and provider/direct/PTX/assembler/runtime evidence.

Out of scope are helper Float parameters/returns/calls, Float loop-carried arithmetic, Bool phis,
half/double, pointer/vector/aggregate phis, select, switch, critical-edge rewriting, noncanonical
predecessors, and performance claims.

## Architecture and Invariants

Feature 32 requires the exact complete two-callback suffix. Slice 43-sized providers remain valid
without it. Generic provider creation accepts only negotiated scalar integer/Float types from the
module context, inserts before the first non-phi, and returns a typed phi. Incoming attachment
preserves the established same-module/function/type, unique predecessor, complete-CFG, exact edge,
and dominance checks.

Direct preflight accepts signed i32 through feature 6/V2 exactly as before. Canonical Float block
parameters and arguments request feature 32 and validate through the existing Float value rule.
Emission chooses the legacy integer or generic scalar pair from the canonical parameter type; no
alternate SSA form is introduced.

## Interfaces and Dependencies

Append one feature, two callback typedefs/table fields/suffix macro, and two facade methods. Refactor
provider validation into shared typed helpers and fake phi storage into one scalar representation.
Extend direct validation/emission, tests, design, ledger, and plan. Add no ABI version, V2 field,
export, dependency, target, text rewrite, local-variable reconstruction, or per-type callback.

## Milestones

1. Append feature 32 and the generic phi pair with exact Slice 43 compatibility.
2. Share provider and fake scalar-phi mechanics while retaining frozen V2 adapters.
3. Admit canonical Float block parameters/arguments and dispatch emission by their semantic type.
4. Add seven named negotiation, provider, direct, capability, differential, `ptxas`, and runtime
   tests around a two-predecessor Float merge.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, commit `slice 44`.

## Validation and Acceptance

Run the seven new tests plus V3 layout, old-prefix, provider invalid/no-mutation, signed-i32
merge/loop preservation, exact constants, adjacent Float arithmetic, and unsupported matrix. Run
full Release NVVM and Debug 10/10 outside sandbox; build standalone Release provider and
Release/Debug test targets.

Accept old-prefix compatibility, rejected partial/null suffixes, one exact `phi float` in both text
dialects, direct predecessor/value topology, matching `[64, 32, 32, 32]` PTX with a global store and
no Float arithmetic/load/predicate, `ptxas`, matching finite RTX/NVRTC selections, exact name
continuity, formatted code, completed audit, and clean diff checks.

## Self-Review and Input-Shape Audit

The new-helper/special-case inventory is the generic callback/facade pair, provider shared phi
helpers, direct semantic-type branches, fake typed `ScalarPhi` storage, and reusable Float callable
runtime template. All survive this audit:

- Slang lowering produces the exact valid input shape: a non-entry block with a canonical Float
  block parameter and positional Float arguments on its actual predecessor branches. The block
  parameter's semantic IR type is the single source of truth. Direct preflight validates each
  argument against that type, creates all placeholders before bodies, and attaches incoming values
  only after the CFG is complete. Removing the Float branches restores the prior
  basic-block-parameter rejection, so this is the layer that must admit the already-canonical
  representation.
- The direct emitter chooses the generic or frozen-V2 pair from the canonical block-parameter type.
  It neither reconstructs a local variable nor walks the operand graph to rediscover a type or
  predecessor. Positional branch arguments and actual CFG predecessor blocks remain the mapping
  consumed by the provider.
- The provider owns opaque LLVM construction and therefore owns phi placement, exact LLVM type and
  module/function identity, predecessor-edge, duplicate, and dominance validation. Its shared
  helpers express those type-independent invariants. V2 adapters require Integer exactly as
  before; generic V3 admits negotiated scalar Integer/Float types without inventing a custom type
  equivalence.
- The fake's `ScalarPhi` is an ABI-path representation shared across generic scalar types, not a
  Float-specific duplicate. The separate `IntegerPhi` record deliberately preserves evidence for
  the frozen V2 path. The runtime template factors route setup and callable launch comparison; it
  does not alter compilation semantics.

No syntax conversion, arbitrary graph walk, alternate SSA form, custom equivalence, fallback,
silent default, text rewrite, or downstream repair of malformed IR was introduced.

## Failure and Recovery

If the source optimizes away the block parameter, inspect linked IR and choose a canonical
two-predecessor fixture rather than disabling optimization globally. If LLVM/libNVVM rewrites the
phi in PTX, retain LLVM/NVVM text plus runtime as semantic evidence. Removing the appended suffix
and Float type branches restores Slice 43. Never stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Retain old/new table sizes, exact generic/NVVM phi text, direct typed predecessor graph, matching
PTX ABI, `ptxas`, RTX/NVRTC results, counts/hashes, line growth, and audit. Distill durable evidence
to design/ledger and ship this completed plan with Slice 44.
