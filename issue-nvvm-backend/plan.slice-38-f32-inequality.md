# Slice 38: Add exact scalar float32 unordered inequality

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs
`*destination = left != right ? 1 : 0` for two scalar float parameters and an AS1 `Ptr<int>`
destination. It reuses the generic V3 floating-comparison callback established in Slice 37,
including source-language unordered NaN behavior, without growing the V3 table.

## Progress

- [x] (2026-08-27) Recorded the Slice 37 baseline: 242 names, SHA-256
  `7bdb7df316f95767ad79c76e2f802dc08504dfd06fbdfd5208a9c0eafd4ca670`, Release 242/242, Debug
  10/10, a 480-byte x64/288-byte x86 V3 table, and 20,503 measured test/support lines.
- [x] (2026-08-27) Selected the existing floating-inequality unsupported fixture as the next exact
  boundary and chose unordered inequality as the explicit source-language NaN policy.
- [x] (2026-08-27) Appended feature/operation negotiation and provider/fake unordered-inequality dispatch while
  retaining the Slice 37 table layout.
- [x] (2026-08-27) Admitted canonical float32 `kIROp_Neq` through the shared typed classifier without changing
  signed-i32 inequality or adjacent comparison diagnostics.
- [x] (2026-08-27) Added the second floating-comparison descriptor row and thin registered wrappers for provider
  text, direct topology, PTX, `ptxas`, runtime, capability, and negative evidence.
- [x] (2026-08-27) Formatted, built standalone/Release/Debug targets, ran focused/full lanes,
  hashed registered names, measured marginal test growth, updated durable docs, completed
  self-review, and prepared the exact slice commit.

## Surprises and Discoveries

- Observation: Slice 37 already paid for the stable callback, Float argument ABI, Boolean-result
  consumer, PTX summarization, assembler runner, CUDA runner, and comparison descriptor family.
  Consequence: use Slice 38's marginal diff and measured lines as evidence for whether that family
  scales economically.

- Observation: the existing `kDirectNVVMFloatingNotEqualSource` lowers to canonical Bool
  `kIROp_Neq` with two canonical Float operands and currently reaches the signed-i32 validator.
  Consequence: extend the same bounded operand-type classifier used by equality; do not introduce
  a new representation or search the IR graph.

- Observation: feature 26 reuses the exact Slice 37 suffix: x64 stays 480 bytes, x86 stays 288
  bytes, and the semantic minimum stays 284 bytes before x86 tail padding.
  Consequence: use one generic floating-compare minimum-size name and retain the equality-specific
  spelling as a source-compatible alias; do not append a callback or semantic-specific size.

- Observation: provider and NVRTC PTX may use either the source predicate or its branch complement,
  so `setp.eq.f32` and `setp.neu.f32` are both valid token-safe evidence for this result/control-flow
  shape.
  Consequence: rename the PTX summary field to the comparison family and rely on runtime truth
  tables for ordered-versus-unordered semantics.

- Observation: adding the second row and seven independently registered tests grows the five
  measured test/support files by 185 lines, versus 662 lines for the first row and family harness.
  Consequence: the descriptor/helper transition is materially reducing marginal test growth while
  keeping failures layer-specific.

## Decision Log

- Decision: append `SCALAR_FLOAT32_NOT_EQUAL` as feature 26 and operation 1
  `UNORDERED_NOT_EQUAL`, while reusing `emitFloatingCompare` and its existing suffix.
  Rationale: semantic availability remains independently negotiable, but the stable operation
  family already carries the new predicate and therefore needs no callback or table growth.
  Date/author: 2026-08-27, Codex.

- Decision: lower Slang `!=` to unflagged LLVM `fcmp une`, not ordered `one`.
  Rationale: source inequality is the logical complement of ordered equality and must be true when
  either operand is NaN. Encoding unorderedness in the wire operation makes the contract explicit.
  Date/author: 2026-08-27, Codex.

- Decision: convert the first-row-only registered test bodies into descriptor-driven helpers or
  macros where that removes duplication, while retaining separately registered layer tests.
  Rationale: Slice 38 should demonstrate that adding a predicate is data plus thin wrappers rather
  than another copy of the layer harness.
  Date/author: 2026-08-27, Codex.

## Outcomes and Retrospective

Feature 26 and floating-compare operation 1 use the unchanged 480-byte x64/288-byte x86 V3 table,
with the existing 284-byte x86 semantic minimum. A Slice 37 feature set remains valid without the
new bit, and disabling equality while retaining inequality proves that the facade maps operation 1
to its independent feature before dispatch.

Both generic LLVM and negotiated NVVM-2.0 text contain exactly one unflagged `fcmp une float` and
no other comparison. Direct topology stays `[Pointer, Float, Float]`; parameters 1 and 2 feed the
floating comparison, whose Bool result controls the established four-block zero/one, integer-phi,
and aligned-i32-store graph. Signed-i32 inequality is unchanged, the floating-inequality negative
advances to supported, and adjacent relational/pointer/unsigned/wide boundaries remain stable.

NVVM and NVRTC agree on `[64, 32, 32]`, token-safe float32 comparison evidence, one global i32
store, and no global load, float arithmetic, or integer predicate. CUDA 12.9 `ptxas` accepts both.
RTX 5090 results are zero for `3.75 != 3.75` and `+0 != -0`, and one for `-8 != 0.5` and quiet
`NaN != NaN`.

Seven names raise the Release prefix from 242 to 249 with sorted LF-terminated SHA-256
`529af4d3eba39ba0aabd6ca881ca3ac66b5f30c5f272c75a54a3b5cdc15156ea`; removing those names
reproduces the Slice 37 count and hash exactly. Focused tests pass 13/13, full Release passes
249/249, Debug preservation passes 10/10, and standalone/Release/Debug builds succeed. The five
measured test/support files grow 185 physical lines, from 20,503 to 20,688. Equality-only test
bodies are now descriptor-driven helpers, and the production emitter deletes duplicated inequality
validation/emission in favor of the shared closed classifier.

## Context and Current Pipeline

Normal lowering turns the motivating ternary into canonical Bool `kIROp_Neq` over two Float entry
parameters, followed by the established conditional branches, integer zero/one constants, i32 phi,
and aligned global i32 store. `_getNVVMFloat32CompareInfo` recognizes only equality today; first-
pass validation therefore classifies `kIROp_Neq` as signed-i32 inequality, and second-pass operand
validation rejects the Float values. Emission likewise routes every surviving `kIROp_Neq` through
the integer comparison family.

The V3 API already ends with `emitFloatingCompare`; `NVVMFloat32ComparisonTestCase` owns the first
ordered-equality row and the provider, fake, direct, PTX, assembler, and CUDA utilities already
accept its stable operation field.

## Scope and Non-Goals

In scope are exact scalar float32 unordered inequality of entry parameters, Boolean result/control
flow, independent feature negotiation, operation 1 on the existing generic callback, unflagged
`fcmp une`, unchanged Slice 37 layout, and fake/text/PTX/assembler/runtime evidence including quiet
NaN.

Out of scope are ordered/unordered relational predicates, integer/pointer behavior changes, float
constants/casts/helpers/phis, direct Bool entry parameters or memory, half/double, vectors/
aggregates, fast/constrained math, resources, atomics, and performance claims. Runtime proves the
quiet-NaN truth value but makes no payload or signaling-exception claim.

## Architecture and Invariants

Feature 26 requires the already-complete floating-compare suffix and float-type callback. A Slice
37 provider remains valid without feature 26 and its table size is unchanged. The facade maps each
known comparison operation to its exact feature before provider dispatch; unknown operations clear
output and fail. The provider validates standard handles, ownership, availability, dominance,
function, insertion point, identical exact LLVM-float types, and a stable operation before emitting
the matching LLVM predicate.

First-pass direct validation recognizes only two Float operands plus canonical Bool result as a
floating comparison and requests the predicate-specific feature. Second-pass validation uses the
existing float-value rule for both operands. Emission consumes the same closed classifier. Signed-
i32 equality/inequality and every unsigned, wide, pointer, and adjacent floating comparison retain
their established paths or diagnostics.

The second descriptor row owns source, feature, wire operation, kernel name, LLVM opcode, PTX
semantic evidence, and runtime truth table. Registered wrappers remain separate so failures still
identify their layer, but shared bodies take a descriptor operation.

## Interfaces and Dependencies

Append one feature and one operation value. Extend facade availability mapping, provider/fake
dispatch, direct classification/validation/emission, the existing invalid-operation/provider tests,
one comparison descriptor row, registered wrappers, design, capability ledger, and this plan. Add
no callback, table field, minimum-size constant, ABI version, V2 change, export, dependency, build
target, or textual compatibility rewrite.

## Milestones

1. Append feature 26 and operation 1 without changing V3 offsets or sizes; prove a Slice 37 feature
   set remains valid and the new feature independently gates dispatch.
2. Implement facade, provider, and fake unordered-not-equal dispatch with output sanitization and
   invalid operation/handle/type/ownership/availability/dominance/insertion coverage.
3. Route canonical float32 `kIROp_Neq` through the existing comparison classifier, while keeping
   signed-i32 inequality unchanged and advancing only the existing floating-inequality fixture.
4. Add one descriptor row plus thin negotiation, provider-text, direct-topology, capability,
   differential-PTX, `ptxas`, and runtime wrappers. Prove finite, signed-zero, and quiet-NaN cases
   against NVRTC.
5. Format and build standalone/Release/Debug targets, run focused/full lanes, hash names, measure
   marginal growth, update durable docs, complete the input-shape audit, and commit `slice 38`.

## Validation and Acceptance

Run the new wrappers plus invalid-provider, signed-i32-inequality, ordered-equality, adjacent
floating-relation, and unsupported-boundary tests; then run the full Release NVVM prefix and the
established Debug 10/10 outside the sandbox. Build the standalone Release provider and Release/
Debug test targets outside the sandbox.

Acceptance requires independent feature negotiation, unchanged V3 layout, no lost registered
names, exactly one unflagged `fcmp une float` in generic and NVVM-2.0 text, ordered fake operands,
matching token-safe float-inequality PTX, `ptxas` acceptance, matching finite/signed-zero/quiet-NaN
runtime truth tables, unchanged adjacent diagnostics, formatted code, a completed input-shape audit,
and clean diff checks.

## Self-Review and Input-Shape Audit

The production inventory adds one feature, one stable operation row in existing facade/provider
switches, and one row in the existing closed direct classifier. No callback, provider field,
representation, rewrite, fallback, or new production helper is added. The facade maps operation to
feature before dispatch; the provider consumes exact LLVM Float operands and selects
`CreateFCmpUNE` only after the shared ownership, availability, dominance, function, insertion, and
type checks.

The exact input reaching `_getNVVMFloat32CompareInfo` is canonical Bool `kIROp_Neq` with two
canonical Float operands, produced by normal lowering of
`*destination = left != right ? 1 : 0`. That shape is intentional: comparison results are Bool, so
the already-checked operand type is the semantic source of truth distinguishing Float from signed
i32. First-pass feature collection, second-pass value validation, and emission consume the same
bounded mapping. Removing its `kIROp_Neq` row restores the motivating `signed i32 value` failure,
proving the emitter owns type-family dispatch. No syntax is rebuilt, operand graph searched,
alternate Bool created, or producer accident repaired.

The test inventory adds one descriptor row and seven thin registered wrappers. Existing equality
bodies for provider construction, direct topology, capability, differential PTX, assembler, and
runtime now accept the descriptor operation, and the PTX summary name reflects the comparison
family rather than one row. Runtime remains the semantic oracle distinguishing ordered equality
from unordered inequality when PTX uses a branch complement. No structural equivalence, semantic
fallback, syntax reconstruction, arbitrary graph-search helper, or test-specific provider path
survives in the diff.

## Failure and Recovery

If LLVM 14 or libNVVM rejects `fcmp une`, inspect exact generic and NVVM-2.0 text before changing
semantics. If PTX expresses inequality as a branch complement, retain token-safe predicate-family
evidence and rely on `ptxas` plus runtime truth tables. Removing feature 26, operation 1, the
classifier row, and the descriptor row restores Slice 37 without changing ABI layout. Never stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

The retained evidence is: unchanged 480-byte x64/288-byte x86 complete tables and 284-byte x86
semantic suffix; direct `[Pointer, Float, Float]` graph; exact `fcmp une` text; matching
`[64, 32, 32]` PTX summaries; `ptxas` acceptance; finite/signed-zero/quiet-NaN RTX results; focused
13/13, Release 249/249, Debug 10/10; sorted-name hash
`529af4d3eba39ba0aabd6ca881ca3ac66b5f30c5f272c75a54a3b5cdc15156ea`; and a 185-line marginal
delta. Durable facts are in the design and capability ledger; this completed plan ships with Slice
38.
