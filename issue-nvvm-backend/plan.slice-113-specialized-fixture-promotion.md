# Promote specialized real-suite compute fixtures

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the optimized direct-libNVVM route is permanent runtime and PTX coverage for a
larger group of existing compute-suite shaders whose generics, interfaces, existential dispatch,
and nested type constructs specialize to the already-supported canonical IR. The shader bodies,
inputs, and expected outputs remain unchanged.

## Progress

- [x] (2026-08-29) Completed Slice 112 as `00bab52a1` with structured-matrix runtime/PTX coverage
  and a 381/381 NVVM unit prefix.
- [x] (2026-08-29) Probed every compute fixture with an existing CUDA directive but no direct-NVVM
  directive, separating latent passing fixtures from distinct production capability buckets.
- [x] (2026-08-29) Added direct runtime and PTX lanes to fourteen retained specialized/value
  fixtures and passed all 28 exact new test IDs.
- [x] (2026-08-29) Inspected and assembled all fourteen PTX modules, retained stable typed
  load/store checks, and recorded exact outputs and artifact sizes.
- [x] (2026-08-29) Passed Release compiler/provider build sanity and the complete NVVM prefix
  381/381; the full affected prefixes pass 83/87 with four unrelated synthesized WebGPU failures.
- [x] (2026-08-29) Completed the test-only self-review and `git diff --check`, updated durable
  status, and prepared the requested slice commit.

## Surprises and Discoveries

- Fifteen useful existing fixtures compile to PTX without a production change: zero-index bounds,
  one-lane generic dot, nine statically resolvable dynamic-dispatch cases, basic dynamic generics,
  kernel-context threading, a nested struct in a generic, and transitive interface inheritance.
  The zero-index-bounds runtime probe reproduces the file's documented CUDA defect, so that fixture
  is excluded rather than retaining compile-only evidence.
- The corpus probe cleanly groups remaining failures into unrelated future work: richer collected
  global/parameter-block fields, helper aggregate/resource ABI, atomics, select, fixed-array value
  construction, transcendental GenericAsm/libdevice operations, padded non-square matrix storage,
  and harness-provided specialization/type-conformance inputs.
- `default-major.slang` also compiles, but all of its runtime lanes are intentionally disabled
  because the gfx harness forces row-major layout. It is not a valid promotion candidate.
- The 2026 interface-qualifier SIMPLE test compiled only because the generic probe omitted its
  language-mode arguments. It is compile-output coverage, not a runtime fixture, and remains out
  of this slice.

## Decision Log

- Decision: make Slice 113 a broad promotion gate rather than immediately adding another builder
  operation.
  Rationale: after three matrix capability slices, the prototype needs evidence that high-level
  Slang specialization consistently reduces real shaders to the generic contracts already built.
  Promoting all coherent latent passers gives more coverage without API growth.
  Date/author: 2026-08-29, Codex.
- Decision: require exact runtime success before retaining each candidate.
  Rationale: a standalone optimized PTX compile proves only code generation. Existential lowering,
  resource binding, launch ABI, and test-harness specialization must preserve each fixture's
  existing result before the lane becomes permanent.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

Slices 107-112 moved the backend from focused bring-up tests into existing-suite coverage and
closed aggregate helper, fixed-array, and matrix storage boundaries. The current corpus probe now
finds a substantial class of generic/interface-heavy shaders whose linked optimized IR contains
only selected scalar/vector values, internal helper calls, ordinary control flow, and structured
buffer traffic. Their source-level features do not require corresponding NVVM builder concepts.

The selected fixtures are:

- `dot1-generic.slang`;
- `dynamic-dispatch-{1,2,3,4,5,6,8,9,10}.slang`;
- `dynamic-generics-simple.slang`;
- `kernel-context-threading.slang`;
- `struct-in-generic.slang`; and
- `transitive-interface.slang`.

## Scope and Non-Goals

In scope are optimized direct CUDA runtime directives, direct PTX directives with stable semantic
checks, unchanged fixture inputs/outputs, independent CUDA 12.9 `ptxas` assembly, and exact test
IDs for all selected fixtures.

Out of scope are production emitter/provider changes, fixture simplification, disabled
default-layout tests, tests needing additional type-conformance inputs, and every unsupported
bucket identified by the probe. If any selected fixture needs production code, remove it from this
promotion slice and record its first stop instead of weakening a contract.

## Architecture and Invariants

- Source generics, interfaces, existentials, and nested declarations must be specialized/lowered
  before direct NVVM preflight; the provider receives no new high-level dispatch representation.
- Existing expected buffers are the semantic source of truth. A passing PTX compile cannot replace
  runtime parity.
- PTX checks identify entry/global-memory or other stable semantic instructions, never optimizer
  register names.
- No production helper, fallback, structural equivalence, or source-feature recognition is added.

## Interfaces and Dependencies

No builder ABI or production interface change is planned. The selected existing test files,
`docs/design/nvvm-backend.md`, the capability ledger, and this plan are the intended committed
files. CUDA 12.9 libNVVM, the CUDA runtime, and `ptxas` provide external evidence.

## Milestones

1. Add one optimized direct runtime directive and one direct PTX directive to each selected
   fixture, preserving every existing directive and shader input.
2. Run all exact new IDs. Remove and document any candidate whose runtime or harness-specific
   linked program differs from the standalone probe.
3. Generate PTX files, inspect stable kernel/memory behavior, and assemble every module with
   `ptxas -arch=sm_70`.
4. Run the complete NVVM unit prefix and affected fixture prefixes, self-review the test-only diff,
   update durable status, and commit the slice.

## Validation and Acceptance

Acceptance requires every retained direct runtime/PTX lane, unchanged existing lanes where broad
prefixes are reliable, CUDA 12.9 `ptxas` for every retained module, Release host/provider build
sanity, the complete `slang-unit-test-tool/nvvm` prefix, `git diff --check`, and an explicit
test-only self-review. Record exact pass counts, outputs, PTX/cubin sizes, exclusions, and any
environment-only failures.

## Failure and Recovery

If a runtime lane fails, preserve its diagnostic/output under ignored `build/slice113-*`, remove
only the new directive for that fixture, and record the distinct future boundary. Do not change an
expected buffer, add fixture-specific production behavior, reset unrelated work, or stage
`external/slang-binaries/`.

## Outcomes and Retrospective

Fourteen fixtures retain direct runtime and PTX lanes; the consolidated exact run passes 28/28.
Their unchanged outputs are:

- generic dot: `20`;
- dynamic dispatch 1/8/9: `1, 2, 5, 10`;
- dynamic dispatch 2 and basic dynamic generics: `0, 1, 4, 9`;
- dynamic dispatch 3: `2, 3, 6, 11`;
- dynamic dispatch 4 and 6: four copies of `4`;
- dynamic dispatch 5: `3, 4, 5, 6`;
- dynamic dispatch 10: `2, 4, 10, 20`;
- kernel-context threading: the four unchanged input matrix rows
  `(1,0,0,0)`, `(0,1,0,0)`, `(0,0,1,0)`, and `(10,20,30,1)`;
- nested generic struct: Float32 `0, 1, 2, 3`; and
- transitive interface: four copies of `3`.

PTX/cubin sizes in the selected fixture order are 478/2,664; 688/2,792; 685/2,792;
688/2,792; 645/2,792; 680/2,792; 645/2,792; 688/2,792; 688/2,792; 712/2,792;
685/2,792; 1,233/3,048; 674/2,792; and 645/2,792 bytes. CUDA 12.9.86
`ptxas -arch=sm_70` accepts all fourteen.

The zero-index bounds candidate produced valid 2,163-byte PTX but failed runtime with the same
CUDA result mismatch already documented by that file. Its added runtime/PTX lines were reverted,
leaving the fixture unchanged and making runtime parity, rather than successful compilation, the
promotion gate.

Release `slangc` and isolated-provider builds pass, and the complete NVVM unit prefix remains
381/381. The full fourteen affected fixture prefixes pass 83/87 with five ignored lanes. The four
failures are automatically synthesized WebGPU runs for generic dot, kernel-context threading,
nested generic struct, and transitive interface; all fail at Dawn bind-group-layout creation on
this machine, matching the previously recorded environment issue. Every explicit native and direct
lane in those prefixes passes.

The final self-review inventory contains only fourteen runtime directives, fourteen PTX directives
with stable entry/load/store patterns, and documentation. Every runtime directive fails to provide
direct semantic evidence if removed; every PTX directive fails to prove the explicit compiler
route if removed. No shader body, input, expected result, production helper, fallback, special
case, ABI, or source-feature recognizer changed. The temporary bounds candidate, including its
newline-only artifact, was fully reverted after the runtime mismatch. The linked optimized outputs
themselves demonstrate the intended input-shape audit: front-end specialization removes the
generic/interface/existential representations before direct NVVM sees the program.

## Artifacts and Hand-Off

Keep corpus logs, PTX, and cubins under ignored `build/slice113-*`. Commit this completed plan with
the promoted fixtures and durable status as explicitly requested by the user.
