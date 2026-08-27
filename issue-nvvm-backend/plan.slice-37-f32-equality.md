# Slice 37: Add exact scalar float32 ordered equality

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs
`*destination = left == right ? 1 : 0` for two scalar float parameters and an AS1 `Ptr<int>`
destination. A generic floating-compare callback is appended to V3, and exact Slice 36 providers
remain loadable when they do not advertise the new semantic.

## Progress

- [x] (2026-08-27) Recorded the Slice 36 baseline: 235 names, SHA-256
  `2b79918702a9b21110af8251944e4428001a4ea69a2ff79b7a18e488cd13b4ba`, Release 235/235, Debug
  10/10, and a 472-byte x64/280-byte x86 V3 table.
- [x] (2026-08-27) Selected the existing floating-equality unsupported fixture as the next exact
  type/result-shape boundary and chose ordered equality as its explicit NaN policy.
- [x] (2026-08-27) Appended compatible feature/callback negotiation and provider/fake
  ordered-equality dispatch, including every partial suffix size.
- [x] (2026-08-27) Admitted canonical float32 `kIROp_Eql` through typed direct validation and emission without
  changing signed-i32 equality or adjacent comparison diagnostics.
- [x] (2026-08-27) Added scalable negotiation, provider text, direct topology, PTX, `ptxas`, runtime, and negative
  evidence for the first floating-comparison row.
- [x] (2026-08-27) Formatted, built standalone/Release/Debug targets, passed focused 12/12, full
  Release 242/242, and Debug 10/10, updated durable docs, completed self-review, and prepared the
  exact slice commit.

## Surprises and Discoveries

- Observation: the existing `kDirectNVVMFloatingEqualSource` already lowers to canonical Boolean
  `kIROp_Eql` with two Float operands, then fails honestly at the signed-i32 operand validator.
  Consequence: Slice 37 advances a measured boundary rather than inventing a synthetic producer.

- Observation: integer and floating comparisons share Boolean control-flow consumers but not
  operand types, provider predicates, runtime argument ABI, or PTX semantics.
  Consequence: keep a small floating-comparison descriptor family and reuse common infrastructure;
  do not add flags to the established integer descriptor until it becomes a mini-language.

- Observation: appending a four-byte callback at x86 offset 280 grows the structure to 284 bytes
  of content and 288 bytes after existing eight-byte structure alignment; x64 grows naturally from
  472 to 480 bytes.
  Consequence: the feature requires the exact 284-byte minimum suffix, while compatibility tests
  separately assert the 288-byte complete x86 `sizeof` and every partial size.

- Observation: both libNVVM and NVRTC preserve a token-safe float32 equality predicate and accept
  the same `[64, 32, 32]` launch ABI; the existing PTX summarizer can recognize either ordered
  equality or its unordered-not-equal branch complement.
  Consequence: assert the semantic predicate family plus runtime truth table rather than exact PTX
  branch orientation.

## Decision Log

- Decision: append `SCALAR_FLOAT32_EQUAL` and a generic `emitFloatingCompare` callback to V3.
  Rationale: comparison is a distinct result-shape family returning `i1`. One stable callback can
  carry equality and later ordered/unordered predicates without adding one API per emit call.
  Date/author: 2026-08-27, Codex.

- Decision: name operation 0 `ORDERED_EQUAL` and lower it to unflagged LLVM `fcmp oeq`.
  Rationale: Slang `==` is false when either operand is NaN. Encoding orderedness in the wire
  operation makes that contract explicit and prevents a future provider from choosing `ueq`.
  Date/author: 2026-08-27, Codex.

- Decision: distinguish integer and float `kIROp_Eql` by canonical operand type, not result type.
  Rationale: both produce canonical Bool, while the already-checked operands are the semantic
  source of truth. No graph walk, syntax reconstruction, or alternate comparison is needed.
  Date/author: 2026-08-27, Codex.

- Decision: give floating comparisons their own compact descriptor family while sharing existing
  compile, PTX parsing, assembler, and CUDA launch utilities.
  Rationale: the first row pays the family scaffolding cost; later predicates should require one
  descriptor row and thin registered wrappers, while the integer descriptor stays simple.
  Date/author: 2026-08-27, Codex.

## Outcomes and Retrospective

Feature 25 and floating-compare operation 0 append a 480-byte x64/288-byte x86 V3 table, with a
284-byte x86 semantic minimum because the complete callback precedes tail padding. An exact
472-byte x64/280-byte x86 Slice 36 provider remains valid without the bit; every partial callback
size and a null complete callback fail when equality is advertised.

Both generic LLVM and negotiated NVVM-2.0 text contain exactly one unflagged `fcmp oeq float` and
need no dialect rewrite. Direct topology is `[Pointer, Float, Float]`; parameter values 1 and 2
feed one floating comparison whose Bool result controls the established four-block zero/one,
integer-phi, aligned-i32-store graph. Signed-i32 equality is unchanged, the floating-equality
negative advances to supported, and floating inequality plus pointer/unsigned/wide comparisons
retain their deterministic boundaries.

NVVM and NVRTC agree on `[64, 32, 32]`, a token-safe float32 equality predicate, one global i32
store, and no global load, float arithmetic, or integer predicate. Both pass CUDA 12.9 `ptxas`.
RTX 5090 results are one for `3.75 == 3.75` and `+0 == -0`, and zero for `-8 != 0.5` and quiet
`NaN == NaN`.

Seven names raise the Release prefix from 235 to 242 with sorted LF-terminated SHA-256
`7bdb7df316f95767ad79c76e2f802dc08504dfd06fbdfd5208a9c0eafd4ca670`; removing those names
reproduces the Slice 36 hash exactly. Focused tests pass 12/12, full Release passes 242/242, Debug
preservation passes 10/10, and standalone/Release/Debug builds succeed. The five measured
test/support files grow 662 physical lines, from 19,841 to 20,503; that cost establishes the family
and later floating predicates reuse its descriptors and runners.

## Context and Current Pipeline

Canonical Float entry parameters and AS1 pointers are centrally legalized. Add/subtract/multiply/
divide and unary negate already consume Float values through generic V3 families. Comparisons
produce Bool, which the established direct path already accepts as an internal condition for
conditional branches; signed-i32 comparison results drive the same two-arm constant/phi/store
shape used by the motivating source.

The first validation pass currently classifies every Boolean `kIROp_Eql` as signed-i32 equality.
The second pass then rejects Float operands as `signed i32 value`. Emission similarly always calls
`emitIntegerCompare`. The V3 table ends at `emitFloatingUnary`.

## Scope and Non-Goals

In scope are exact scalar float32 ordered equality of entry parameters, Boolean result/control
flow, independent feature negotiation, one generic floating-compare operation, unflagged
`fcmp oeq`, exact Slice 36 prefix compatibility, and fake/text/PTX/assembler/runtime evidence.

Out of scope are floating inequality or ordered relational predicates, integer/pointer behavior
changes, float constants/casts/helpers/phis, direct Bool entry parameters or memory, half/double,
vectors/aggregates, fast/constrained math, resources, atomics, and performance claims. Runtime
cases may include quiet NaN to prove ordered equality, but this slice makes no NaN payload or
signaling-exception claim.

## Architecture and Invariants

Feature 25 requires the complete appended compare callback suffix and the established float-type
callback. A table ending exactly after Slice 36 remains valid without feature 25. Unknown compare
operations clear output and fail before dispatch. The provider validates output, module ownership,
availability, dominance, function, insertion point, identical exact LLVM-float operand types, and
the stable ordered-equal operation before `CreateFCmpOEQ`.

First-pass direct validation recognizes only two Float operands plus canonical Bool result as the
new case and requests feature 25. Second-pass validation uses the existing float-value rule for
both operands. Emission dispatches by canonical operand type. Signed-i32 equality and every
unsigned, wide, pointer, and adjacent floating comparison fixture retain their established path or
diagnostic.

The floating-comparison descriptor owns source, feature, wire operation, kernel name, LLVM opcode,
PTX predicate evidence, and explicit runtime values. Each layer keeps its own assertion and stable
registered wrapper. The descriptor is test data, not a second production opcode mapping.

## Interfaces and Dependencies

Append one feature, operation type/value, callback typedef, table field, minimum-suffix constant,
and facade method. Extend initialization, provider/fake dispatch, direct validation/emission, the
existing invalid-float provider test, layout/compatibility tests, floating-comparison descriptors
and runners, design, capability ledger, and this plan. Add no export, dependency, ABI version, V2
field, build target, textual compatibility rewrite, or general test generator.

## Milestones

1. Append feature 25 and `emitFloatingCompare`; prove an exact Slice 36 table remains valid without
   the bit and partial/null new suffixes fail when it is advertised.
2. Implement facade and provider ordered-equal dispatch with invalid operation, handle, type,
   ownership, availability, dominance, insertion, and output-sanitization coverage.
3. Route canonical float32 `kIROp_Eql` through float validation and compare emission while keeping
   signed-i32 equality unchanged and advancing only the existing floating-equality fixture.
4. Add one floating-comparison descriptor and thin negotiation, provider-text, direct-topology,
   capability, differential-PTX, `ptxas`, and runtime wrappers. Prove finite equal/unequal and quiet
   NaN cases against NVRTC.
5. Format and build standalone/Release/Debug targets, run focused/full lanes, hash registered names,
   measure test growth, update durable docs, complete the input-shape audit, and commit `slice 37`.

## Validation and Acceptance

Run the new wrappers plus invalid-provider, signed-i32-equality, floating-inequality, and unsupported
boundary tests; then run the full Release NVVM prefix and established Debug 10/10 outside the
sandbox. Build the standalone Release provider and Release/Debug test targets outside the sandbox.

Acceptance requires old-prefix compatibility, complete new suffix negotiation, no lost registered
names, exactly one unflagged `fcmp oeq float` in both LLVM and NVVM-2.0 text, ordered fake topology,
matching token-safe float-equality PTX, `ptxas` acceptance, matching finite/NaN runtime truth tables,
unchanged adjacent diagnostics, formatted code, a completed input-shape audit, and clean diff checks.

## Self-Review and Input-Shape Audit

The production inventory contains one append-only callback/facade method, one closed comparison
classifier, and one provider operation. The callback follows the established V3 generic-family
contract: feature 25 requires the complete suffix plus float-type callback, callers without the bit
may end at the exact Slice 36 prefix, and unknown operations clear output and fail before provider
dispatch. There is no text rewrite or fallback.

The exact input reaching `_getNVVMFloat32CompareInfo` is canonical Bool `kIROp_Eql` with two
canonical Float operands, produced by normal lowering of
`*destination = left == right ? 1 : 0`. That shape is intentional: equality always produces Bool,
so operand type is the existing semantic source of truth distinguishing it from signed-i32
equality. First-pass classification, second-pass typed operand validation, and emission consult the
same bounded helper. The result continues through the existing branch/constant/phi/store graph;
no syntax is rebuilt, graph is searched, alternate Bool is created, or producer accident is
repaired. Removing this classifier restores the motivating `signed i32 value` failure, proving the
direct emitter owns the type-family dispatch.

The provider consumes the canonical operands directly and emits exact unflagged `CreateFCmpOEQ`
after the standard ownership, availability, dominance, function, insertion, operation, and
identical-float checks. Ordered equality is the source-language NaN contract, not a test-specific
special case; the quiet-NaN runtime row proves both downstream routes agree.

The test inventory adds a floating-comparison descriptor because its Float argument ABI and
runtime oracle differ materially from integer comparisons. `_emitNVVMBooleanResultAsI32` extracts
the already-common provider result consumer, and the generic float `ptxas` runner accepts source
and feature without operation flags. Direct topology, text, PTX, assembler, runtime, suffix, and
negative assertions remain in independently registered layers. No structural equivalence,
semantic fallback, syntax reconstruction, or arbitrary graph-search helper survives in the diff.

## Failure and Recovery

If LLVM 14 or libNVVM rejects `fcmp oeq`, inspect exact generic and NVVM-2.0 text before changing
semantics. If PTX folds the comparison/control flow, retain token-safe semantic evidence where
stable and rely on `ptxas` plus the runtime truth table. Removing the feature/callback/type split/
descriptor restores Slice 36. Never stage `external/slang-binaries/`.

## Artifacts and Hand-Off

The retained evidence is: 472/280-byte old and 480/288-byte new complete tables, 284-byte x86
semantic suffix, direct `[Pointer, Float, Float]` graph, exact `fcmp oeq` text, matching
`[64, 32, 32]` float-equality PTX, `ptxas` acceptance, finite/signed-zero/quiet-NaN RTX results,
focused 12/12, Release 242/242, Debug 10/10, sorted-name hash
`7bdb7df316f95767ad79c76e2f802dc08504dfd06fbdfd5208a9c0eafd4ca670`, and a 662-line first-family
delta. Durable facts are in the design and capability ledger; this completed plan ships with Slice
37.
