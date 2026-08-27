# Slice 30: Consolidate NVVM scalar tests around recorded operations

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user requires each
completed slice plan to ship with its implementation, so this plan will be committed with Slice 30.

## Purpose and Observable Result

After this slice, established scalar unary, binary, and comparison coverage is declared through a
small set of case descriptors and exercised by shared negotiation, invalid-operation,
direct-emitter, differential-PTX, `ptxas`, and runtime runners. The fake V3 provider records generic
operations instead of owning a storage type, counter, callback, operand list, and failure switch
for every opcode.

Existing test names referenced by the capability ledger remain registered unless a documented
one-to-one generated wrapper replaces them. The full focused and preservation evidence remains
green, while the total NVVM test/support line count decreases and adding a same-shaped scalar
operation requires one descriptor plus only genuinely operation-specific assertions.

## Progress

- [x] (2026-08-27) Audited repeated per-operation fake storage/callbacks, negotiation tests,
  invalid-operation tests, direct topology tests, capability tests, PTX checks, `ptxas` bodies, and
  runtime bodies in the Slice 26 baseline.
- [x] (2026-08-27) Deferred semantic consolidation until after test decomposition, V3 generic
  operations, and centralized type lowering provide stable owners.
- [x] (2026-08-27) Captured the post-Slice-29 193-name hash, per-file physical/nonblank line counts,
  assertion inventory, and the eight-layer repeated scalar evidence matrix.
- [x] (2026-08-27) Replaced per-operation fake state with generic recorded operations and defined
  separate unary, binary, and comparison descriptors.
- [x] (2026-08-27) Consolidated V2 negotiation, invalid-operation, real-builder, direct-emitter,
  capability, differential-PTX, `ptxas`, and runtime bodies behind stable named wrappers.
- [x] (2026-08-27) Proved exact name/coverage preservation, material source reduction, full focused
  and preservation behavior, documented the extension contract, and completed the self-review.

## Surprises and Discoveries

- Observation: runtime dispatch is already partially table-driven, while the surrounding compile,
  PTX, `ptxas`, and registered-test bodies are repeated.
  Evidence: `ScalarRuntimeOperation` and local runtime case arrays share launch/result logic, but
  individual operation tests still repeat source compilation, provider setup, output parsing, and
  environment gates.
  Consequence: this slice can promote an existing tested pattern rather than inventing an external
  data-driven framework.

- Observation: many test names are durable documentation keys.
  Evidence: `docs/design/nvvm-backend-capability-ledger.md` names individual builder, direct,
  differential, assembler, and runtime tests.
  Consequence: use generated or thin named wrappers over shared runners where granularity remains
  useful; do not collapse evidence into one opaque loop merely to reduce line count.

- Observation: generated wrappers remain visible to the test registry but not to a simple
  `SLANG_UNIT_TEST(...)` source regular expression.
  Evidence: Release `-dry-run` enumerates 193 unique names and hashes to the exact baseline, while
  the wrapper invocations intentionally use layer-specific macros.
  Consequence: compare registered names from the built test binary, not source spellings, whenever
  this pattern is extended.

- Observation: two historical test bodies relied on incidental fixture shape rather than their
  stated contract.
  Evidence: the first consolidated run showed that wide unary integer values are valid inputs (the
  invalid mismatch dimension applies only to binary/compare pairs), and that add/sub stores in the
  combined conditional fixture have no required record order. Restricting the former assertion to
  two-operand families and accepting either exact add/sub store permutation restored the intended
  checks; the full suite then passed.
  Consequence: shared runners should encode family invariants and semantic relationships, not copy
  incidental per-body ordering assumptions.

## Decision Log

- Decision: one declarative scalar case list is the source of truth for repeated family metadata.
  Rationale: operation name, V3 enum/predicate, Slang source, expected recorded operation, PTX
  instruction family, and representative runtime values otherwise drift across separate tests.
  Date/author: 2026-08-27, Codex.
  Revisit when: an operation has a genuinely different ABI, result shape, or environment contract;
  keep it in a separate family rather than adding flags until the descriptor becomes a mini-language.

- Decision: retain layer-specific assertions and test names while sharing setup/runners.
  Rationale: a verifier failure, direct-topology failure, `ptxas` failure, and runtime mismatch are
  different evidence and should remain independently diagnosable.
  Date/author: 2026-08-27, Codex.
  Revisit when: the test framework gains native parameterized subtests with stable reported names.

- Decision: line-count reduction is a consequence, not the sole design criterion.
  Rationale: compressed macros can be harder to review than explicit tests. Acceptance requires
  removal of repeated ownership/state and one-source mappings, plus an actual aggregate reduction;
  it does not reward dense or cryptic code.
  Date/author: 2026-08-27, Codex.
  Revisit when: clear generated wrappers cause a small net increase while demonstrably eliminating
  future per-operation growth; record the measurement and obtain agreement before accepting it.

- Decision: real scalar builder modules use the generic V3 facade and share one family-aware module
  constructor; frozen V2 callback/layout tests remain explicit adapters and compatibility data.
  Rationale: all production scalar emission already crosses V3, while V2 byte offsets and callback
  names are historical ABI facts that future V3-only operations must not extend.
  Date/author: 2026-08-27, Codex.
  Revisit when: V2 compatibility is removed, or a new scalar result shape cannot use the unary,
  binary, or comparison module topology.

## Outcomes and Retrospective

Slice 30 replaces the repeated post-control-flow scalar test representation without touching
production. Physical line counts change as follows: builder 9,823 to 5,935; compiler 1,173 to
1,173; emitter 3,817 to 2,464; integration 3,917 to 1,831; and support 7,623 to 6,415. The total
falls from 26,353 to 17,818, a reduction of 8,535 lines (32.4%). The corresponding nonblank total
falls from 24,476 to 16,613. Repeated shared assertions account for the lower textual check count;
the 88 operation/layer wrappers still invoke all eight runners independently.

The fake builder no longer owns per-operation storage kinds, counters, operand lists, or failure
booleans for multiply, AND, OR, XOR, NOT, negate, equality, inequality, signed-greater,
signed-less-equal, or signed-greater-equal. It records unary, binary, and comparison identities in
one ordered stream. Frozen V2 callbacks are adapters to that recorder, and V3 callbacks use it
directly. Add/subtract/signed-less-than now use the same stream in their combined control-flow
fixtures. Atomic, resource, pointer, and array records remain separate because their ABIs and
result shapes are genuinely different.

Separate unary, binary, and comparison descriptor arrays own shared source, wire operation, kernel,
LLVM/PTX classification, runtime operation, and diagnostic metadata. Provider negotiation,
invalid/no-mutation checks, generic real-builder modules, direct topology, capability gating,
differential PTX, `ptxas`, and runtime execution each have one runner plus thin stable wrappers.
Adding a synthetic same-shaped V3 case now requires one descriptor row, explicit runtime values,
any unique semantic PTX assertion, and the applicable wrapper registrations. It requires no fake
state/callback implementation or repeated compile, serialization, assembly, or launch harness.
Frozen V2 layout rows are changed only for an operation that actually belongs to that immutable ABI.

Release build succeeds and the full NVVM prefix passes 193/193, including every real differential
PTX, `ptxas`, and RTX 5090 runtime lane. Debug build succeeds and preservation passes 10/10. Release
`-dry-run` reports 193 unique registered names; the sorted LF-terminated name set keeps pre/post
SHA-256 `1f35f717b93e1cb62c3f872e99b819386ab9c5474b203256e58ee1bdb41c97b7`.
`git diff --check` passes. The self-review found no production helper, fallback, representation
repair, custom semantic equivalence, or new input-shape special case: every new helper is test-only,
and family branches correspond to the declared unary/binary/comparison arity and result contract.

## Context and Current Pipeline

After Slice 27, test infrastructure is physically separated. After Slice 28, established scalar
operations enter the provider through generic unary, binary, and comparison callbacks and are
advertised by feature bits. After Slice 29, their types are lowered through one cache context.
Nevertheless, the migrated tests may still preserve the historical V2 shape: one fake storage
kind, counter, callback, invalid matrix, direct test, capability test, PTX test, `ptxas` test, and
runtime body per operation.

Slice 30 changes the test representation to match the new production abstraction. V2 compatibility
continues to need coverage, but its specialized callbacks should be compact adapters into the same
recorded operation stream rather than a second fake semantic implementation.

## Scope and Non-Goals

In scope are generic recorded fake operations, declarative unary/binary/comparison case lists,
shared layer-specific test runners, generated or thin stable test wrappers, compact V2 adapter
coverage, deletion of obsolete per-operation fake state, and capability-ledger name updates only
where unavoidable.

Out of scope are production behavior changes, new operations or types, changing provider ABI,
weakening negative/no-mutation checks, replacing semantic PTX classification with textual equality,
removing `ptxas` or runtime evidence, changing CUDA environment gates, and a general-purpose test
generation system for unrelated backends.

## Architecture and Invariants

The fake builder records values conceptually as:

```text
RecordedNVVMOperation {
    family: unary | binary | compare | dedicated,
    operation: stable V3 wire enum or dedicated identifier,
    result: fake value reference,
    operands: ordered fake value references,
    insertion block: fake block reference,
}
```

One validator owns operand availability/type checks common to a family. Failure injection is keyed
by family/operation and whether failure occurs before or after a deliberately invalid output write;
the host wrapper must still clear failed outputs. V2 adapters translate specialized callbacks to
the same recorded V3 operation identity.

A scalar case descriptor contains only metadata shared across at least two layers. Keep a separate
descriptor family when arity, result type, ABI, PTX semantics, or runtime oracle differs. Expected
values remain explicit and readable. PTX checks remain token-safe and semantic; text equality is
not introduced.

The case declaration is test data, not a second production mapping. Production continues to map
Slang IR operations to V3 enums explicitly, and tests compare that output to independently stated
expectations.

## Interfaces and Dependencies

Expected changes are confined to the Slice 27 test support/fake/test files, the capability ledger,
backend design, and this plan. A small multi-include definition file or typed constexpr arrays may
declare cases. Prefer ordinary C++ and existing Slang containers/macros; do not add a code generator,
Python build step, third-party parameterization library, or runtime file parser.

Production provider and emitter files should not change. If a production change appears necessary,
stop and determine whether Slice 28 or 29 left an abstraction incomplete rather than masking it in
tests.

## Milestones

1. Record the post-Slice-29 registered name set, per-file/aggregate non-generated line counts, fake
   state inventory, and assertion matrix for unary/binary/comparison operations.
2. Add `RecordedNVVMOperation` and generic family validation/failure injection. Route V3 fake
   callbacks and frozen V2 adapters to it, proving exact ordered operands, result kinds, blocks, and
   no-mutation behavior.
3. Define separate unary, binary, and comparison case lists with readable Slang source, wire
   operation, expected topology/PTX classification, and runtime oracle data. Do not force atomic or
   resource cases into scalar descriptors.
4. Replace repeated negotiation/invalid/direct/capability bodies with shared runners and stable
   named wrappers. Preserve assertions for unknown enums, wrong types, cross-module values,
   dominance, terminated blocks, failure-after-write, and missing feature bits.
5. Replace repeated differential PTX, `ptxas`, and runtime setup with layer-specific runners while
   retaining independent reported names and environmental skips. Keep operation-specific PTX and
   edge-value assertions in the descriptors or short callbacks.
6. Delete superseded per-operation fake storage, counters, lists, callback implementations, and
   duplicate source constants. Compare test names/assertions and line counts, run all evidence, and
   update durable documentation.

## Validation and Acceptance

Build Release/Debug test targets outside the sandbox. Run the complete focused NVVM prefix and
Debug preservation 10/10. Run real direct/NVRTC differential PTX, every established scalar
`ptxas` lane, and the full GPU scalar runtime matrix. Compare sorted registered test names against
the pre-slice list; any name change must have a documented ledger update and equivalent reported
granularity.

Focused fake tests must prove generic recording and every prior invalid/no-mutation dimension.
Coverage inventory must show each established unary/binary/comparison operation still crosses the
provider negotiation, provider validation, direct-emitter topology, real compilation/PTX, assembler,
and runtime layers where it did before.

Acceptance requires deletion of the specialized scalar fake-state pattern, one declarative mapping
per scalar family, no duplicated compile/`ptxas`/runtime harness bodies per operation, a net decrease
in aggregate NVVM test/support source lines relative to the recorded start, readable formatted code,
and `git diff --check` success.

## Failure and Recovery

Convert one operation family at a time and keep the old tests until the new runner proves the same
assertions. If generated test names do not register stably, retain thin explicit wrappers rather
than weakening reporting. If a descriptor accumulates operation-specific flags, split the family
or keep that test explicit. If a test fails only after consolidation, compare its recorded fake
topology and real artifacts against the pre-slice output before changing production.

Do not delete or stage `external/slang-binaries/`. Remove temporary name lists, line-count reports,
generated PTX, and probe binaries before committing.

## Artifacts and Hand-Off

The retained evidence is the exact 193-name hash
`1f35f717b93e1cb62c3f872e99b819386ab9c5474b203256e58ee1bdb41c97b7`, physical/nonblank line
reductions of 8,535/7,863, 88 stable operation/layer wrappers, Release 193/193, and Debug
preservation 10/10. `docs/design/nvvm-backend.md` records the settled extension pattern, and the
capability ledger records that no selector or claim changed. Commit this completed plan with Slice
30; the next slice may build on the generic fake and descriptor architecture.
