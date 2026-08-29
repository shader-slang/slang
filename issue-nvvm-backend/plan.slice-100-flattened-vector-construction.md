# Flatten vector constructors and complete the native-Half surface fixture

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM compiles the complete `tests/compute/half-texture.slang` fixture. The
generic vector-construction path accepts canonical constructors whose operands are any ordered mix
of same-element scalars and vectors, flattens them to exact scalar lanes, and still rejects lane
count or element-type mismatches before provider discovery. The existing fixture then exercises
scalar Half, Half2, and Half4 2D surface loads and stores through the typed Slice 99 resource API.

## Progress

- [x] (2026-08-29) Compiled all four remaining Half texture fixtures through direct NVVM and
  captured their finalized `-O3` IR and first diagnostics.
- [x] (2026-08-29) Selected `half-texture.slang`: its first and only measured preflight stop is the
  canonical `half4(half2, half, half)` constructor after all six surface helpers are retained.
- [x] (2026-08-29) Generalized the vector-construction classifier and emission data without adding a
  texture-specific special case.
- [x] (2026-08-29) Added focused positive coverage, revalidated the existing malformed provider-construction cases, and enabled direct PTX coverage on the
  existing fixture.
- [x] (2026-08-29) Formatted, built, ran focused/full/changed-shader validation, assembled PTX, self-reviewed, updated
  durable docs, and commit the completed slice.

## Surprises and Discoveries

- `half-texture.slang` passes Slice 99's resource-type and helper-signature gates unchanged. Its
  scalar, two-lane, and four-lane 2D load/store helpers all have the already supported canonical
  GenericAsm bodies.
- The selected constructor has result `Vec(Half, 4)` but three operands: `Vec(Half, 2)`, `Half`, and
  `Half`. The existing classifier incorrectly assumes operand count must equal result lane count,
  although the IR explicitly preserves the ordered lane contribution of every operand.
- The two formatted RW texture fixtures retain semantic Float/Float2/Float4 resource elements and
  require formatted Half storage/conversion semantics. The sampled-texture fixture retains seven
  1D/2D/3D/cube, arrayed and non-arrayed SampleLevel helpers. Those are distinct resource-operation
  families and are not folded into this vector-construction slice.
- No subsequent compiler boundary appeared: after flattening the mixed constructor,
  `half-texture.slang` compiled directly to the expected six native surface instructions and
  CUDA 12.9 assembled the PTX unchanged.

## Decision Log

- Decision: flatten canonical same-element scalar and vector constructor operands in the existing
  generic vector classifier.
  Rationale: the source shape is an ordinary vector constructor, not a surface workaround. Its
  final IR contains all ordered components, so the current scalar-only restriction is the wrong
  abstraction boundary.
  Date/author: 2026-08-29, Codex.
- Decision: retain a fixed four-lane bound and materialize each vector operand as provider-side
  constant-index extraction followed by the existing vector constructor.
  Rationale: the shield already exposes generic typed vector extraction and construction. No new
  LLVM callback or texture-aware operation is needed, and every lane remains explicit.
  Date/author: 2026-08-29, Codex.
- Decision: use `half-texture.slang` as the slice integration fixture and leave formatted surfaces
  and sampled textures for separately measured slices.
  Rationale: it is the largest existing fixture unlocked by one general representation fix and it
  expands real surface coverage to Half2 loads/stores and Half4 stores without broadening resource
  semantics.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

The finalized entry-point IR contains:

    %h2  : Vec(Half, 2) = call %loadHalf2(...)
    %h   : Half         = call %loadHalf(...)
    %out : Vec(Half, 4) = makeVector(%h2, %h, %h)
    call %storeHalf4(..., %out)

`_getNVVMVectorConstruction` currently accepts `makeVector` only when operand count equals result
lane count and every operand is the scalar element type. Preflight therefore reports `selected
value construction or extraction` before builder discovery. Emission already consumes a flattened
`NVVMVectorConstruction::elements` array and can lower extracted vector lanes through
`emitVectorElementExtract`, so the correction belongs in the classifier's construction of that
array rather than in surface emission.

The same fixture also contains Half4-to-Half2 swizzle construction and scalar Half2 lane extracts;
those already use the generic vector classifier successfully. Once the concatenating constructor
is classified, the calls reach the typed surface path established in Slice 99.

## Scope and Non-Goals

In scope:

- `makeVector` operands that are same-element scalars or fixed vectors and whose flattened lane
  total exactly equals a supported two-, three-, or four-lane result;
- exact ordered lane flattening using constant provider extraction;
- focused positive coverage for mixed scalar/vector operands and negative coverage for mismatched
  lane totals or element types;
- complete direct-NVVM PTX compilation and `ptxas` assembly of `half-texture.slang`;
- real Half, Half2, and Half4 2D surface load/store instruction coverage.

Out of scope:

- nested arrays, matrices that survive legalization, vectors wider than four lanes, dynamic lane
  sequences, bit reinterpretation, or mixed element types;
- new builder ABI or LLVM provider callbacks;
- formatted Float-to-Half surface conversion, sampled textures, sampler semantics, arrays, 3D or
  cube resources, mip levels, or SampleLevel;
- arbitrary GenericAsm or backward compatibility.

## Architecture and Invariants

- The result vector's canonical scalar element type is the sole element type. Each source operand
  must be exactly that scalar or an accepted vector of exactly that scalar.
- Operands are flattened left to right and vector lanes in increasing index order. The accumulated
  lane count may never exceed the result count and must equal it at the end.
- A scalar lane keeps its source value. A vector lane keeps its base plus constant source index;
  emission uses the existing typed extraction operation before constructing the result.
- Preflight and emission share the same resolved `NVVMVectorConstruction`; emission does not
  rediscover or reinterpret operand structure.
- The fixed-size four-element resolved array remains sufficient and bounds all loops.
- The surface API and resource classifier remain unchanged; this fixture is evidence for existing
  descriptor rows, not authorization to broaden them.

## Interfaces and Dependencies

Change only the generic vector resolver and its focused fake/compiler tests unless measurement
exposes a subsequent principled boundary. Add a direct PTX FileCheck lane to
`tests/compute/half-texture.slang` for scalar, v2, and v4 2D loads and stores. Update
`docs/design/nvvm-backend.md` and this plan with final evidence.

Validation uses the configured Release host build, standalone LLVM provider, CUDA 12.9, and
`ptxas -arch=sm_70`. CMake builds, tests, and native formatting run outside the sandbox per
repository instructions.

## Milestones

1. Teach `_getNVVMVectorConstruction` to flatten exact scalar/vector operands while preserving its
   bounded resolved representation.
2. Add focused compiler coverage proving mixed constructors lower and malformed adjacent shapes
   stop before provider mutation.
3. Enable the direct PTX fixture lane, compile and assemble optimized PTX, run the complete NVVM
   prefix, audit the diff, update docs and this plan, then commit.

## Validation and Acceptance

Acceptance requires the new focused mixed-constructor compiler test and the existing provider
negative construction cases; the complete
`slang-unit-test-tool/nvvm` prefix; every enabled `half-texture.slang` lane including the new direct
PTX check; standalone optimized direct PTX; CUDA 12.9 `ptxas` assembly; pinned clang-format 17; and
`git diff --check`.

The Release `slangc` and `slang-unit-test` targets built successfully. The focused mixed-constructor
test passed 1/1, the complete NVVM prefix passed 373/373, and all five enabled
`half-texture.slang` compile lanes passed. Its optimized direct PTX is 1,659 bytes and contains
scalar, v2, and v4 `suld.b.2d.*.zero` instructions followed by scalar, v2, and v4
`sust.b.2d.*.zero` instructions. CUDA 12.9.86 `ptxas -arch=sm_70` emitted a 3,176-byte cubin.
Pinned clang-format 17 and `git diff --check` completed cleanly apart from expected checkout
line-ending notices. Final-tree probes confirmed both the formatted-surface and sampled-texture
fixtures still stop at their distinct helper-parameter boundaries.

## Self-Review and Input-Shape Audit

The only new helper is `_appendNVVMVectorConstructOperand`. It survives because it owns the one new
generic invariant: append either one exact scalar lane or every ordered lane of one exact
same-element vector while remaining within the result width. The concrete producer is the final
`makeVector(%half2Call, %half, %half)` instruction from `half4(h2, h, h)`. This is canonical and
intentional—the IR already represents a vector constructor as an ordered sequence of scalar or
vector contributions—so rejecting it downstream was an overly narrow classifier, not malformed
input. The focused compiler test fails at `selected value construction or extraction` without the
change and proves the first two provider constructor operands are constant-index extracts from the
same helper call.

Emission consumes the resolved four lanes and does not walk the source operands again. The helper
requires canonical IR type equality, rejects overflow while appending, and the classifier rejects
an incomplete final lane total. Existing real-provider construction tests still reject wrong lane
counts, wrong element types, foreign-module types and values, and unavailable operands. No
surface-specific type, operation, helper name, or GenericAsm appears in the generic vector code.

## Failure and Recovery

If the constructor generalization reveals a new unrelated stop, preserve the diagnostic and final
IR. Continue within this slice only if it is necessary to complete the same bounded Half surface
fixture and has a generic, independently tested representation; otherwise record it as the next
slice boundary. Generated dumps, PTX, and cubins stay under ignored `build/`. Never reset unrelated
work or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Record the exact flattened lane map, focused fake records, emitted PTX, ptxas result, test counts,
next exact fixture stop, and completed self-review inventory. Distill durable generic vector and
surface coverage into `docs/design/nvvm-backend.md`.

## Outcomes and Retrospective

The representation fix completed the intended fixture without any resource-API change. Direct
NVVM now handles concatenating two- through four-lane constructors generically, and
`half-texture.slang` expands native surface evidence to every scalar/v2/v4 2D load and store row.

The next measured resource boundary remains the formatted RW surface family:
`half-rw-texture-convert.slang` and `half-rw-texture-convert2.slang` stop on helper parameters whose
semantic Float/Float2/Float4 resource types carry R16F/RG16F/RGBA16F formats. The other available
fixture, `half-texture-simple.slang`, is substantially broader: it stops on sampled texture helper
parameters and retains seven SampleLevel helpers across 1D, 2D, 3D, cube, and arrayed shapes.
