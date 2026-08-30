# Lower canonical ephemeral values without source reconstruction

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires this experimental branch's slice plan to be committed with its implementation, which is
an exception to the repository's default active-plan lifetime policy.

## Purpose and Observable Result

After this slice, the direct NVVM path handles three canonical IR forms that are ordinary input to
code generation but do not correspond to source-level CUDA expressions: a chosen value for
`LoadFromUninitializedMemory`, the semantic-free `DebugNoScope` marker when the backend emits no
debug information, and a compile-time hash for a validated literal `getStringHash`. One compiler-
owned classification is shared by preflight, SSA validation, and emission. No source syntax is
reconstructed and no fixture identity influences lowering.

The bounded population is the eight healthy-MVP Slice 144 census rows whose first blocker is one
of those three forms: three undefined-value rows, four debug-marker rows, and `bugs/string-inline`.
Every row that becomes correct at direct O0 and O3 is promoted. Later independent blockers remain
measured. `RequirePrelude` and `RequireComputeDerivative` are deliberately excluded: both carry
target behavior, so discarding either as a no-op would be semantically incorrect.

## Progress

- [x] (2026-08-30) Committed Slice 144 as `5f6731eb6d` with eleven both-mode atomic gains, zero
  losses, 422/422 selected tests, and a 452-workload post-slice census.
- [x] (2026-08-30) Decomposed the leading ten-row healthy-MVP residual cluster by exact canonical
  shape and selected the eight-row ephemeral-value cohort.
- [x] (2026-08-30) Audited the exact producers, result types, uses, and reference behavior of all
  eight rows.
- [x] (2026-08-30) Added one compiler-side classifier shared by both validation passes and
  emission, using only revision-30 generic provider operations, with focused contract coverage.
- [x] (2026-08-30) Probed and promoted all eight rows at O0 and O3; each became a correct
  differential result and no later blocker remained in the bounded cohort.
- [x] (2026-08-30) Ran formatting, builds, 423/423 selected regressions, all promoted lanes, the
  complete fixed census, representative metrics, self-review, and durable documentation.

## Surprises and Discoveries

- The Slice 144 `residual-target-marker-or-undefined-value` bucket is not one implementation
  cluster. Its ten healthy-MVP rows split into four `DebugNoScope`, three
  `LoadFromUninitializedMemory`, one literal `getStringHash`, one `RequirePrelude`, and one
  `RequireComputeDerivative` first blocker.
- `LoadFromUninitializedMemory` is defined by `slang-ir-insts.lua` as one arbitrary value chosen
  per instruction; an optimization may select a concrete value and replace all uses. A fixed
  canonical zero of the same complete type is therefore legal and preserves per-instruction
  consistency, unlike rebuilding source initialization syntax.
- `DebugNoScope` is introduced by inlining to close an inlined debug scope. The established LLVM
  emitter consumes it only when debug emission is enabled and otherwise gives it no provider
  representation.
- `getStringHash` reaches GPU-like emitters only after type inlining, dead-code elimination, and
  `checkGetStringHashInsts`; its operand is therefore already a canonical `IRStringLit`. Both the
  LLVM and C-like emitters compute `getStableHashCode32` directly from that literal.
- The first literal-hash prototype selected the templated overload for the
  `UnownedStringSlice` object instead of the byte-range overload. The real differential probe
  exposed the resulting runtime mismatch even though the focused pipeline compiled. Passing
  `begin()` and `getLength()` makes the semantic byte sequence explicit.
- Whole-file runs for some promoted tests also exercised unrelated WGPU lanes that fail with an
  existing invalid bind-group-layout diagnostic on this machine. All sixteen explicit direct
  NVVM lanes passed, and the fixed CUDA census was unaffected.

## Decision Log

- Decision: Treat selected undefined values, ignored debug markers, and validated literal hashes
  as one bounded ephemeral-value slice.
  Rationale: All three are canonical post-link IR inputs whose meaning must be consumed at the
  compiler/emitter boundary rather than represented as source syntax or a CUDA runtime operation.
  They cover eight tied-leading healthy-MVP workloads and share the same preflight/value/emission
  consistency requirement.
  Date/author: 2026-08-30, Codex.
- Decision: Exclude `RequirePrelude` and `RequireComputeDerivative` from this slice.
  Rationale: The former injects target-language definitions used by source `GenericAsm`; the
  latter requests compute-derivative execution behavior. Neither is semantic-free metadata, and
  accepting either without its effect would violate the producer-side methodology.
  Date/author: 2026-08-30, Codex.
- Decision: Prefer a compiler-selected concrete zero value for
  `LoadFromUninitializedMemory` if all observed complete types can be constructed through the
  existing generic builder operations.
  Rationale: The IR contract explicitly permits choosing one value per instruction. Recursive
  construction from canonical type structure preserves the same chosen value for every use and
  avoids an ABI revision. Add a provider callback only if a concrete observed canonical type
  cannot be represented correctly through revision 30.
  Date/author: 2026-08-30, Codex.
- Decision: Hash the canonical literal byte range and map the uint32 result to the signed i32
  provider constant with explicit modulo-2^32 arithmetic.
  Rationale: The bytes are the established semantic source of truth. Explicit arithmetic avoids
  implementation-defined unsigned-to-signed conversion while preserving the exact i32 bit
  pattern expected by existing emitters.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

Slice 145 promoted all eight bounded workloads at direct O0 and O3. The selected regression prefix
passes 423/423, and identity comparison against Slice 144 finds zero losses. The complete fixed
census remains 448 eligible sources and 452 workloads. Direct results are 365 correct, 8 runtime
mismatches, 72 preflight failures, and 7 provider failures at O0; and 370 correct, 8 runtime
mismatches, 72 preflight failures, and 2 provider failures at O3. On the 427-workload healthy-MVP
denominator, both-mode correctness rises from 355/427 (83.1%) to 363/427 (85.0%).

The accepted shapes remain narrow and producer-owned. `_resolveNVVMEphemeralValue` recognizes the
three exact op/type relations and is reused at every consumption point.
`_emitNVVMChosenUndefinedValue` follows canonical complete type structure and stores one result per
SSA instruction. The debug no-op is owned by this emitter's explicit lack of debug output, and
literal hashing reads the already-validated semantic operand. No provider callback, compatibility
fallback, fixture-name condition, arbitrary operand walk, source reconstruction, or downstream
repair was retained.

Representative direct O3 SM70 compile medians remain 270.3 ms, 251.5 ms, and 255.2 ms for the
resource/aggregate/helper, parameter-block, and shared-control gates, compared with NVRTC medians
of 390.0 ms, 369.7 ms, and 372.9 ms. Their direct O3 PTX sizes are 919, 793, and 1404 bytes versus
8889, 8839, and 9190 bytes from NVRTC. CUDA 12.9 assembles the generated PTX for SM70, SM80, and
SM90. CUDA 13 and physical SM70/SM80/SM90 runtime validation remain productionization gaps.

The next choice is deliberately Pareto-driven: aggregate/pointer/layout transport, helper ABI/type
contracts, other exact preflight shapes, and common wave/reconvergence GenericAsm each block eight
healthy-MVP workloads. Their internal canonical shapes and representative importance must be
compared before selecting Slice 146.

## Context and Current Pipeline

Consider:

```slang
[ForceInline]
Result makeResult()
{
    Result result;
    // Every observed element is overwritten on the paths used by the kernel.
    return result;
}
```

SSA construction in `slang-ir-ssa.cpp` emits `LoadFromUninitializedMemory` when a read is not
proven initialized. The instruction definition permits an optimization to choose a concrete value
for that instruction. `_validateNVVMFunction` currently reaches its default rejection before the
ordinary value pass; direct emission consequently has no mapping for it.

Inlining in `slang-ir-inline.cpp` emits `DebugNoScope` when leaving an inlined debug scope. The
LLVM emitter tracks it only if debug output is active. Direct NVVM currently produces no source-
level debug information, so the canonical marker has no executable effect, but preflight rejects
it before that invariant can be applied.

For:

```slang
int hash = getStringHash("Hello World!");
```

`emitEntryPointsSource` runs GPU type inlining and DCE, then `checkGetStringHashInsts` verifies that
the live operand is an `IRStringLit`. Existing LLVM and C-like emitters compute the stable 32-bit
hash from that canonical literal. Direct NVVM currently rejects the instruction instead of using
the same semantic source of truth.

## Scope and Non-Goals

In scope:

- the exact complete value types produced by the three bounded
  `LoadFromUninitializedMemory` workloads;
- one compiler-selected value per undefined instruction, reused by all SSA consumers;
- canonical `DebugNoScope` with no debug provider output;
- canonical `getStringHash(IRStringLit)` lowered with `getStableHashCode32`;
- shared classification across preflight, value-graph validation, and emission;
- focused fake-provider tests, real O0/O3 differential probes, promotions, fixed census/Pareto,
  representative metrics, and durable design/capability updates.

Out of scope:

- poison semantics, arbitrary undefined pointer/resource values, or debug-info emission;
- `RequirePrelude`, `RequireComputeDerivative`, reconvergence/quad requirements, or generic source
  assembly;
- speculative acceptance of other debug or target-marker instructions;
- fixing unrelated blockers discovered after the bounded cohort advances;
- any fixture-name check, syntax reconstruction, compatibility fallback, or downstream repair for
  malformed upstream IR.

## Architecture and Invariants

1. `LoadFromUninitializedMemory` is a canonical SSA producer. The direct backend may choose one
   concrete same-typed value for it, but every use of that instruction must observe the same
   provider handle.
2. The chosen-value constructor follows canonical IR type structure and uses only existing generic
   scalar constants and aggregate/vector construction. It does not infer source declarations or
   traverse arbitrary use graphs.
3. `DebugNoScope` is accepted only because the direct backend currently has no debug output. It
   does not enter `valueMap` and must have no executable consumers.
4. `getStringHash` is accepted only with the canonical literal operand guaranteed by the existing
   GPU validation pipeline. The stable hash implementation remains the single semantic source of
   truth.
5. Preflight, SSA availability validation, and emission agree exactly on every admitted shape.
   Unsupported types or malformed operands fail deterministically before provider discovery.
6. LLVM provider ABI revision 30 remains unchanged unless an observed canonical type proves that
   the selected value cannot be expressed with existing generic builder operations.

## Interfaces and Dependencies

Expected compiler work is in `source/slang/slang-emit-nvvm.cpp`, with reusable type queries from
`slang-emit-nvvm-type-lowering.*`. Stable string hashing comes from the existing
`slang-ir-string-hash` implementation already used by established emitters. The builder/provider
contract changes only if the prototype criterion in the previous section requires it.

Focused coverage belongs in `tools/slang-unit-test/unit-test-nvvm-emitter.cpp` and the fake provider
support. Successful real workloads receive O0/O3 direct-NVVM lanes in their existing test files.

## Milestones

1. Record the exact type and use shape for every bounded undefined instruction, and confirm that
   debug markers and string hashes have the expected canonical producer chain.
2. Add focused fake-provider tests that fail on the three current preflight shapes and negative
   tests for malformed literal/type relations.
3. Add one compiler-owned classifier and chosen-value construction, reuse it in both preflight
   passes and emission, and keep revision 30 when existing operations suffice.
4. Build and run the focused tests, then run all eight bounded workloads in both direct modes.
   Promote only exact differential successes; classify every later blocker.
5. Format, rebuild, run the full selected NVVM prefix and promoted file tests, regenerate the fixed
   corpus census/Pareto and representative metrics, perform the required special-case inventory,
   and update durable docs plus this plan's outcomes.

## Validation and Acceptance

All builds and tests run outside the sandbox, as required by `AGENTS.md`.

- Provider build if the provider changes:
  `cmake.exe --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release`
- Host build:
  `cmake.exe --build build --config Release --target slang-unit-test`
- Selected regression prefix with `SLANG_NVVM_BUILDER_PATH` set:
  `.\build\Release\bin\slang-test.exe slang-unit-test-tool/nvvm`
- Bounded probe:
  `python.exe issue-nvvm-backend\run-compute-census.py --output build/nvvm-census/slice145-probe --match-regex <bounded-expression>`
- Complete fixed census and summary using the established Slice 144 scripts.
- Representative workload metrics using `measure-compute-mvp.py`, including CUDA 12.9 PTX
  assembly for SM70, SM80, and SM90 on this machine.
- `git diff --check`, formatter, focused negative tests, and zero losses among every Slice 144
  correct workload identity.

Acceptance requires focused contract coverage, correct O0/O3 differential execution for every
promoted row, no previous correct identity loss, updated explicit denominators/clusters, and a
documented explanation for each retained widening or special case.

## Failure and Recovery

All changes are additive until the bounded probe succeeds. If recursive concrete-value
construction encounters a type not expressible through revision 30, stop and record that exact
type before revising the ABI; do not substitute a zero of another physical type. If a row advances
to an unrelated failure, preserve the deterministic diagnostic and leave that later cluster for a
future slice. Generated census and metric directories under `build/nvvm-census/` are disposable
and rerunnable.

## Artifacts and Hand-Off

Retain the final fixed census TSV and cluster JSON in `issue-nvvm-backend/`. Distill stable
ephemeral-value contracts and updated denominators into `docs/design/nvvm-backend.md` and
`docs/design/nvvm-backend-capability-ledger.md`. Complete this plan and a five-part Slice 145 report
with the producer/use audit, rejected alternatives, validation evidence, gains, losses, remaining
clusters, and productionization gaps.
