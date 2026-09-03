# Slice 187: Separate NVVM plan ownership from provider emission

## Motivation

Consider one `InterlockedAdd` followed by a floating-point comparison. Preflight already records an
atomic plan and a numeric-truthiness plan, but `emitNVVMIRFromLinkedIR` used to construct and own one
dictionary for each record family. The schema lived in the emitter's public-facing header while
the indexing invariant was a local implementation detail. This obscured the boundary between
deciding what canonical IR means and performing provider operations.

The decomposed NVVM tests presented a related-looking but importantly different boundary. Their
shared support header owns fake-provider state in an anonymous namespace, so each test translation
unit receives an independent fixture. Moving those definitions into one `.cpp` file would change
state ownership rather than merely reduce header size.

## Proposed solution

Give immutable emission plans a dedicated internal header and implementation. Build one checked
`NVVMEmissionPlanIndex` before provider module creation, then expose family-specific typed lookups
to emission. Retain the fake-provider support header and document why it is intentionally
header-only.

## Change summary

- Moved all emission-plan and provider-requirement records to `slang-emit-nvvm-plan.h`.
- Added `NVVMEmissionPlanIndex`, which validates non-null, unique source keys for every family.
- Replaced nine emitter-local dictionaries and their repeated indexing calls with typed lookups.
- Documented the per-translation-unit ownership contract in `unit-test-nvvm-support.h`.
- Preserved provider ABI revision 34 and every accepted or rejected canonical IR shape.

## Concepts and vocabulary

**Emission plan** is the immutable result of canonical preflight classification. **Plan index** is
the checked mapping from a final linked-IR source instruction to its one family-specific plan
record. **Fake-provider state** is the call trace and fault-injection data used by one unit-test
translation unit.

## Process report

For a canonical instruction such as `IRAtomicAdd`, `_resolveNVVMAtomicOperation` still runs only in
the first preflight walk and appends one source-keyed record. The later validation walk checks that
record, and emission now calls `NVVMEmissionPlanIndex::findAtomicOperation`. The same trace applies
to ordinary scalar operations, UInt64 construction, truthiness, floating remainder, bitfields,
default resources, ephemeral values, surfaces, and atomics. `initialize` asserts that every source
is non-null and unique before `createModule`, so malformed plans cannot partially mutate provider
state.

The exact incoming shapes are unchanged and remain canonical outputs of their named resolvers.
This slice does not rediscover syntax, accept an adjacent spelling, or add a fallback. The new
index is representation plumbing for the existing plan, not another semantic classifier.

The test-support dependency audit found that `unit-test-nvvm-support.h` places its fake builder and
compiler state in an anonymous namespace and that the split builder/compiler/emitter/integration
tests inspect that state directly. A shared implementation unit would merge those independent
states. The proposed wholesale extraction was therefore rejected; the header now states its
ownership contract so a future extraction must first introduce an explicit fixture context.

The self-review inventory contains the plan header, the checked index class, two generic private
index helpers, nine typed lookup methods, the emitter consumer conversions, and the test ownership
comment. All survive because they either establish the single plan/index boundary or document a
dependency that prevented an unsafe move. No compatibility path, provider callback, shape guard,
or downstream repair was added.

The Release `slang-unit-test`, `slangc`, and `slang-test` targets build successfully. The selected
NVVM prefix passes 437/437 and the permanent NVVM category passes 92/92. Frozen corpus v1 remains
452 workloads/427 healthy references at 418/418/418 O0/O3/both with zero old-correct regression;
all-row direct results remain 432 correct, three runtime mismatches, and 17 preflight failures per
mode. Discovery remains 82 workloads/72 healthy references at 72/72/72; classifications remain 72
correct, seven infrastructure failures, one runtime mismatch, and two preflight failures per mode.
Exact per-workload comparison against Slice 186 found zero classification changes in either
corpus.
