# Promote a specialized generic entry point

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, `tests/compute/int-generic.slang` has permanent optimized direct-CUDA runtime and
PTX lanes. Its generic `computeMain<M>` is specialized to the existing test-declared
`Material<1,2>` before direct emission, proving that the backend consumes canonical specialized IR
without accepting an invalid unspecialized generic entry point or reconstructing generic syntax.

## Progress

- [x] (2026-08-30) Completed Slice 127 as `ec145f225`; Release provider/host builds, focused
  resource/vector/libdevice coverage, both promoted fixture paths, PTX assembly, and the complete
  NVVM prefix passed 397/397.
- [x] (2026-08-30) Enumerated the seven remaining `tests/compute` files with an active CUDA lane and
  no direct-NVVM lane, then compiled all seven through the Release direct compiler.
- [x] (2026-08-30) Probed six candidate harness lanes. Only `int-generic.slang` is already inside
  the complete direct contract; the other five expose distinct final-IR boundaries and their
  temporary directives were removed.
- [x] (2026-08-30) Added and passed exact direct runtime and PTX/FileCheck lanes for the
  `Material<1,2>` specialization.
- [x] (2026-08-30) Inspected and assembled the PTX, passed Release provider/host builds and the
  complete 397/397 NVVM prefix, updated durable status, formatted, and self-reviewed.

## Surprises and Discoveries

- Bare command-line probes of five non-generic candidates produced code because no runtime shader
  object inputs were bound. The compute harness creates the actual specialized/layout-bearing
  program, where `array-existential-parameter.slang`, `loop-unroll.slang`, and
  `parameter-block.slang` stop at distinct `struct field address` producers,
  `dynamic-dispatch-7.slang` stops at `helper function parameter`, and
  `struct-default-init.slang` stops at `makeArray`. Runtime harness evidence, not an unbound source
  compile, is authoritative for fixture promotion.
- An unspecialized command-line compile of `int-generic.slang` correctly emits E38014 because its
  entry point is generic. Its existing `TEST_INPUT:type Material<1,2>` instructs the compute harness
  to specialize the entry point, so the direct lane must be measured there rather than weakening
  the compiler to accept an invalid unspecialized entry point.
- `bound-check-zero-index.slang` is the seventh census result and also compiles. Earlier direct
  runtime evidence recorded in Slice 113 reproduces the fixture's documented CUDA result mismatch,
  so it is not an honest promotion candidate.
- The specialized `int-generic.slang` program reduces completely to the established conventional
  global-parameter buffer, dispatch-thread index, and one constant UInt32 store of `3`. Its generic
  material/BRDF declarations do not survive as a second backend representation.

## Decision Log

- Decision: narrow the initial six-fixture promotion plan to the one fixture with passing harness
  evidence, and leave each independent final-IR boundary for its owning implementation slice.
  Rationale: the test harness proved that unbound command-line compilation was not representative
  for five fixtures. Combining resource arrays, parameter blocks, an aggregate helper ABI, and
  default-initialized aggregate arrays would violate the coherent-slice policy. The three field-
  address fixtures form the next larger cluster for Slice 129.
  Date/author: 2026-08-30, Codex.
- Decision: retain `int-generic.slang`'s generic entry point and rely on its existing declared test
  specialization.
  Rationale: E38014 is the correct compiler contract for an unspecialized generic entry point. The
  harness is the canonical producer of the concrete program the CUDA lanes already execute.
  Date/author: 2026-08-30, Codex.
- Decision: exclude `bound-check-zero-index.slang` despite successful direct compilation.
  Rationale: a comparison lane must preserve the fixture's expected runtime result. Registering a
  known CUDA mismatch would turn an unrelated behavior discrepancy into noisy backend coverage.
  Date/author: 2026-08-30, Codex.

## Context and Current Pipeline

`int-generic.slang` declares `computeMain<M : IMaterial>` and supplies
`TEST_INPUT:type Material<1,2>`. Both the established CUDA runtime lane and the new direct lane use
the compute harness, which resolves that type and specializes the entry point through the normal
frontend API. The direct route receives the same linked, target-specialized program, runs CUDA
varying legalization and optimization, and sees no unresolved interface or generic operation.

A bare compile without `-specialize` correctly stops at E38014 before target lowering. An explicit
PTX compile with `-specialize Material<1,2>` produces the same canonical program as the harness and
is the artifact used for static inspection and `ptxas` validation.

## Scope and Non-Goals

In scope are one optimized direct-CUDA runtime lane and one specialized direct PTX/FileCheck lane in
`int-generic.slang`; its existing `Material<1,2>` specialization and expected result; PTX inspection
and SM70 assembly; durable capability status; Release provider/host builds; the complete NVVM unit-
test prefix; and this plan.

Out of scope are the five independently unsupported harness fixtures, changing fixture inputs or
expected results, accepting unspecialized generic entry points, the known bound-zero-index runtime
discrepancy, unrelated HLSL intrinsic fixtures, production or builder changes without a failing
selected lane, compatibility aliases, source-name matching, and broad refactoring.

## Architecture and Invariants

- The direct lane uses the same source, inputs, specialization, entry point, and expected result as
  the established CUDA lane; only `-emit-cuda-via-nvvm` and the explicit SM/O3 policy differ.
- Specialization remains owned by the test harness and compiler frontend. Direct emission sees only
  the resulting canonical linked IR and never recovers generic arguments from source syntax.
- A successfully compiling unbound probe does not replace runtime comparison. The promoted lane
  must execute and match the established result.
- Any newly exposed backend operation must be selected from final IR and expressed through the
  existing typed descriptor/generic builder system. A fixture-specific bypass is not acceptable.
- The optional provider path remains explicit, and `external/slang-binaries/` remains untracked.

## Interfaces and Dependencies

Committed areas are the `int-generic.slang` directive/FileCheck block,
`docs/design/nvvm-backend.md`, the capability ledger, and this plan. Production code and builder ABI
remain unchanged.

## Milestones

1. Probe all six initial candidates through the compute harness, retain only passing promotion
   directives, and record each independent stop.
2. Add exact runtime and static PTX lanes for the specialized generic fixture.
3. Inspect the explicit specialized PTX and assemble it with CUDA 12.9 `ptxas` at SM70.
4. Run Release builds and the complete NVVM prefix, update docs and this log, format, perform the
   input-shape audit, and commit.

## Validation and Acceptance

Acceptance requires the new direct runtime and PTX/FileCheck lanes to pass with unchanged
specialization/input/result; explicit inspectable direct PTX; CUDA 12.9
`ptxas -arch=sm_70`; Release provider/compiler/unit-test builds; the complete
`slang-unit-test-tool/nvvm` prefix; pinned formatting for changed source files; and
`git diff --check`.

The self-review inventories both new directives and confirms no production helper, fallback, or
special case was added. Remove any direct-lane option drift, duplicated test expectation, source-
name match, syntax reconstruction, inferred backend specialization, provider fallback, or
compatibility shim.

## Failure and Recovery

If the specialized runtime, PTX, assembly, or full regression gate fails, retain artifacts under
ignored `build/slice128-*` and trace the exact specialization-to-linked-IR path. Do not weaken the
fixture, modify expected output, accept an unspecialized generic entry point, silently fall back to
NVRTC, reset unrelated work, or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep generated PTX, cubin, IR, and logs under ignored `build/slice128-*`. Distill the specialized
fixture, exact runtime/PTX evidence, exclusions, and next measured boundary into
`docs/design/nvvm-backend.md`, then commit this plan with the fixture changes as explicitly
requested.

## Outcomes and Retrospective

The unchanged compute harness specializes `computeMain<M>` to `Material<1,2>` before the direct
route. Both established CUDA and new direct CUDA runtime lanes produce `3`, and the explicit direct
PTX/FileCheck lane proves the selected entry and constant UInt32 global store. The 645-byte PTX
contains no generic, interface, material, or BRDF representation; it computes the dispatch index
and stores `3` through the established conventional global resource. CUDA 12.9.86
`ptxas -arch=sm_70` assembles it to a 2,792-byte cubin. Release provider/compiler/unit-test builds
pass, the fixture prefix passes 3/3, and the complete NVVM prefix remains 397/397.

The initial broad probe was useful precisely because the harness contradicted the unbound
command-line result for five candidates. Their temporary directives were removed, leaving no
content changes. Three stop at `struct field address` through distinct resource-array/parameter-
block producers and form the next audit cluster; `dynamic-dispatch-7.slang` needs another aggregate
helper ABI, and `struct-default-init.slang` needs an array-construction contract. The known
`bound-check-zero-index.slang` CUDA mismatch remains excluded.

The self-review inventories only two permanent directives. The runtime lane uses the existing
harness specialization and expected output; the static lane names the same specialization
explicitly. No production helper, fallback, source-name match, generic syntax reconstruction,
builder callback, ABI revision, compatibility shim, or alternate representation survives in the
diff.
