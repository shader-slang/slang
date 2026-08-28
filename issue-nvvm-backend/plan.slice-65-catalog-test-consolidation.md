# Slice 65: Consolidate semantic catalogs and family tests

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, one declarative catalog is the source of truth for recognized GenericAsm
semantics, typed V4 signatures, diagnostics, and provider operation requirements. The Cartesian
signature enum and repeated feature/op mappings disappear. Tests verify operation families through
shared fixtures and generated cases, and adding a same-shaped semantic has bounded, measured work.

## Progress

- [x] (2026-08-28) Scoped this cleanup after the V4 architecture so it can remove legacy
  duplication rather than build another abstraction over V3.
- [x] (2026-08-28) Re-audited the post-Slice-64 catalogs, test families, and extension points.
- [x] (2026-08-28) Established one 40-row declarative production catalog and generic exact
  signature matcher.
- [x] (2026-08-28) Routed facade, provider, fake, canonical IR, and GenericAsm semantics through
  the catalog while preserving independently registered tests.
- [x] (2026-08-28) Proved bounded extension cost, validated Release and Debug lanes, documented
  the result, and completed the self-review.

## Surprises and Discoveries

- Slice 64 described the same 40 typed overloads independently in facade feature synthesis,
  provider signature validation and dispatch, fake support and dispatch, emitter operation
  metadata, and the GenericAsm whole-signature enum. The fake was also more permissive than the
  real provider for most malformed signatures.
- The five test/support files were 28,508 physical lines after Slice 64, but the scalar, float, and
  wave behavioral tests were already organized around shared descriptors and named registration
  macros. Replacing those registrations with another generator would save little and make isolated
  failures harder to read. The repeated fake semantic switch was the test-side duplication worth
  removing in this slice.
- The complete Release run initially exposed a diagnostic-order regression for unsupported
  unsigned and pointer comparisons. Exact lookup during feature discovery rejected the operation
  before the existing operand validator could identify the unsupported value. Feature discovery
  now defers only structurally valid unmatched comparisons; the operand validator rejects them
  before builder discovery, and exact catalog resolution remains asserted after successful
  validation.
- This clone has only Release LLVM libraries for the optional provider build. The Debug Slang host
  therefore used the freshly built Release provider for its preservation sample, as in the
  established lane; the Debug host itself was rebuilt from the changed sources.

## Decision Log

- Decision: preserve independently registered public test names even when their implementation is
  generated from family descriptors.
  Rationale: the name set is an external regression ledger and supports exact baseline hashing.
  Date/author: 2026-08-28, Codex.

- Decision: use arrays of result/operand type roles instead of enums that encode whole signatures.
  Rationale: signature dimensions then grow linearly and can be validated by one matcher.
  Date/author: 2026-08-28, Codex.

- Decision: keep the canonical `IROp`-to-semantic-operation mapping in the emitter, and keep LLVM
  handle and insertion-state validation in the provider.
  Rationale: those mappings describe their respective representations. The shared catalog owns
  only the semantic signature and frozen compatibility route, so it does not become a serialized
  IR or leak LLVM policy into Slang.
  Date/author: 2026-08-28, Codex.

- Decision: retain the V2 and V3 compatibility callbacks and their family-level test helpers.
  Rationale: they remain reachable for older provider exports. V4 callers use the generic
  descriptor path, while the compatibility adapter is the only production path that maps a
  catalog row back to a frozen callback family.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

`slang-nvvm-semantic-catalog.h` now owns 40 exact rows: 14 signed-i32, 11 float32, and 15 wave
overloads, including 14 canonical GenericAsm spellings. It replaces the Cartesian signature enum,
the facade's feature/operation descriptor construction, provider and fake signature switches, and
the emitter's repeated semantic metadata. The provider and fake dispatch only by the row's frozen
callback family.

Across the three changed production implementations plus the new catalog, physical LOC falls from
8,844 to 7,707 (-1,137, including the new 560-line table). The five NVVM test/support files fall
from 28,508 to 28,396 (-112). A same-shaped overload over existing kinds now needs one catalog row
and behavioral tests; it needs no ABI callback, V4 operation ID when reusing a semantic operation,
legacy feature, facade wrapper, provider matcher branch, or fake dispatch branch. An ordinary new
IR operation may additionally need one canonical `IROp` mapping, while a GenericAsm spelling is a
field in its catalog row.

The focused Release catalog/family sample passes 12/12. The complete Release prefix passes 419/419,
and its sorted LF-terminated registered-name ledger remains SHA-256
`c634caa999f2b191c85b37cc7885d39462bcef55406ef64dc04bd1a1d02590c9`. The rebuilt Debug host passes
all 11 selected preservation tests using the validated Release provider. Full Release coverage
retains compatible NVVM assembly, CUDA 12.9 `ptxas`, and RTX/NVRTC runtime parity.

## Context and Current Pipeline

Slice 64 introduces typed semantic signatures at the provider boundary, but compatibility adapters
and historical tests may still retain separate descriptor arrays, feature mappings, operation
switches, wrapper calls, and bespoke fake state. This slice makes the typed catalog authoritative
from final canonical GenericAsm recognition through facade negotiation.

## Scope and Non-Goals

In scope are production descriptor/matcher consolidation, capability collection, V3 adapter
mapping, fake operation records, shared family fixtures, generated registrations where supported,
preserved test names, bounded-growth measurements, design, and this plan.

Out of scope are new CUDA semantics merely to justify a framework, deleting compatibility tests,
changing test assertions, broad unrelated unit-test cleanup, and introducing code generation that
requires an extra build tool.

## Architecture and Invariants

One catalog row owns exact canonical assembly spelling, semantic operation, result type descriptor,
operand descriptors, and diagnostic name. Matching compares the complete canonical function shape
against those descriptors. Provider support is queried with the same typed signature. V3 fallback
mapping is isolated in the compatibility adapter.

Family tests share setup and assertion mechanics but retain operation-specific expected source,
signature, provider record, IR/PTX mnemonic, capability failure, and runtime behavior. Consolidation
must not weaken invalid/no-mutation, malformed-provider, fallback, or compatibility evidence.

## Interfaces and Dependencies

Primarily modify `source/slang/slang-emit-nvvm.cpp`, the facade compatibility adapter, fake support,
the five NVVM unit-test translation units as justified, design, and this plan. Avoid ABI changes
unless Slice 64 evidence proves its catalog descriptor incomplete.

## Milestones

1. Inventory each mapping and repeated fixture with before counts.
2. Introduce the generic type-role signature matcher and single semantic catalog.
3. Route capability collection and emission through the catalog; isolate V3 mapping.
4. Consolidate fake records and tests while preserving exact registered names/assertions.
5. Demonstrate bounded extension cost, run all lanes, measure, document, audit, and commit.

## Validation and Acceptance

Run focused catalog/matcher invalid tests, every scalar/floating/wave family, malformed V4 and V3
fallback tests, Release full prefix, Debug preservation, real provider compatible assembly,
`ptxas`, and representative runtime parity outside the sandbox.

Accept only if the sorted registered-name set and hash are unchanged from Slice 64; the Cartesian
signature enum is gone; no production semantic is described in two catalogs; a same-shaped row
requires no new wrapper/provider callback/feature ID or matcher branch; assertions remain semantic;
measured support/test LOC decreases materially or a documented stronger metric demonstrates the
cleanup; and formatting/audit pass.

## Self-Review and Input-Shape Audit

Inventory all helper extraction, table lookup, compatibility mapping, and generated-test machinery.
Prove each recognized GenericAsm shape is canonical target-selected IR. Do not match helper names,
reconstruct syntax from semantic IR, combine rows that differ in behavior, or retain a fallback
mapping made unreachable by V4.

The helper inventory is: semantic type conversion, canonical `IROp` mapping, exact catalog lookup,
legacy-family lookup, and provider/fake family dispatch. All survive. The emitter receives either
an ordinary canonical value instruction or the exact GenericAsm spelling and function type chosen
by CUDA target lowering. It derives type descriptors directly from those IR values; it does not
walk operand graphs, recreate syntax, or infer helper identity from a source name. The catalog
keeps rows separate wherever signatures or frozen behavior differ.

The one deferred comparison shape has a canonical Boolean result and two values of an unsupported
type. It is intentionally not accepted: `_validateI32Value` diagnoses the first operand before any
builder discovery, and exact lookup is release-asserted if both operands validate. Removing the
deferral reproduces the diagnostic regression in
`nvvmSlangUnsupportedIRStopsBeforeEmission`; removing the exact post-validation lookup cannot make
a supported canonical operation valid. V2/V3 family switches survive only in reachable older-
provider adapters. No new equivalence relation, syntax reconstruction, graph search, silent
default, or target-specific producer repair was introduced.

## Failure and Recovery

If one row needs graph inspection beyond exact assembly and signature, audit its producer before
generalizing the matcher. If test generation obscures failures or loses stable names, retain
explicit registrations over shared descriptor-driven bodies. Reverting catalog routing restores
Slice 64. Never stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Retain mapping inventory, before/after LOC and duplication counts, exact test name/hash continuity,
extension-cost demonstration, validation logs, and self-review. Commit this completed plan with
Slice 65.
