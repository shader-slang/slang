# Add native integer surface l-values and arrays

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM compiles the existing `tests/compute/texture-subscript.slang` fixture.
PTX legalization converts its canonical partial image l-values into explicit typed image
load/store read-modify-write operations. The direct compiler maps native signed/unsigned 32-bit
surface payloads, retained 2D arrays, and the fixture's device-memory barriers through generic
builder semantics, and the fixture permanently checks the resulting PTX.

## Progress

- [x] (2026-08-29) Completed Slice 105 as `859edac7d`, with 376/376 NVVM tests, 26/26 affected
  surface tests, direct runtime parity, and accepted PTX.
- [x] (2026-08-29) Reproduced E52017 `helper function parameter` for the next existing fixture and
  captured its 1,994,221-byte direct IR dump plus 6,266-byte ordinary CUDA source.
- [x] (2026-08-29) Identified the canonical image-l-value legalization, four retained Int4 surface
  load helpers, eight partial stores, eight device barriers, and the exact 1D/2D/3D/2D-array shapes.
- [x] (2026-08-29) Enabled the existing image-subscript legalization for PTX and verified its
  image-load/store output before extending direct emission.
- [x] (2026-08-29) Generalized the surface descriptor from dimension count to shape plus arrayness,
  added the selected native 32-bit signed/unsigned matrix, and emitted exact 2D-array intrinsics.
- [x] (2026-08-29) Resolved and emitted canonical IR image loads/stores through the existing typed
  surface interface, retaining exact collected-field storage provenance.
- [x] (2026-08-29) Added the exact device-wide memory-barrier semantic and provider intrinsic under
  ABI revision 16, then enabled permanent direct fixture checks and focused negative boundaries.
- [x] (2026-08-29) Formatted, built, ran focused and complete validation, ran `ptxas`, self-reviewed,
  updated durable documentation, recorded outcomes, and prepared the slice commit.

## Surprises and Discoveries

- `-target cuda` succeeds because the CUDA source emitter understands image-subscript l-values, but
  the ordinary `-target ptx` path currently gives NVRTC invalid pointer/subscript expressions. PTX
  is absent from the target list that runs `legalizeImageSubscript`; GLSL, SPIR-V, and Metal already
  use the pass.
- The legalizer is the correct producer boundary. It recognizes a store rooted at exactly one
  `IRImageSubscript`, emits a four-lane `IRImageLoad` when a partial update needs the old value,
  applies `IRSwizzleSet`, emits `IRImageStore`, and removes the dead l-value. Reimplementing that
  transformation in the direct emitter would duplicate canonical target legalization.
- The unlegalized final entry contains six two-lane swizzled stores and two scalar stores through a
  one-level `IRGetElementPtr`. All eight are rooted directly at an image subscript from a canonical
  collected-global resource load. The verification half contains eight calls across four Int4 load
  helpers: exact 1D, 2D, 3D, and 2D-layered GenericAsm strings.
- `AllMemoryBarrier` finalizes as one no-argument void helper with exact GenericAsm
  `__threadfence()`, called after every partial update. LLVM 7 and LLVM 14 expose
  `llvm.nvvm.membar.gl`, the direct semantic equivalent of CUDA's device-wide thread fence.
- The current surface descriptor carries only `dimensionCount`; it cannot distinguish 3D from a
  2D array because both consume three coordinate lanes. The sampled-texture interface already uses
  the scalable representation: shape plus independent arrayness.
- LLVM exposes exact zero-boundary `suld.2d.array` and `sust.b.2d.array` i32 scalar/v2/v4
  intrinsics. Signed integer, unsigned integer, and Float32 semantic lanes all share those physical
  i32 payload rows.
- The fixture's source-level `RWTexture*<int4>` declarations produce inferred `rgba32i` field-key
  decorations even though no explicit format attribute appears in the Slang source. This is native
  signed-32-bit storage, not formatted integer conversion. Reusing `ImageFormatInfo` for scalar kind
  and channel count admits that canonical decoration without duplicating an image-format table.
- After the surface operations and barrier were admitted, the final result expression exposed one
  `intCast` from Boolean to UInt. The existing generic integer-convert provider already zero-extends
  when its source is not signed; admitting selected Boolean source lanes in the shared semantic
  family completed that existing abstraction without a new callback.
- Adding PTX to the canonical image-subscript legalization list changes the linked-IR form consumed
  by direct NVVM. The ordinary PTX route still hands a different, earlier shape to the CUDA source
  emitter and NVRTC rejects its invalid `CUsurfObject` l-value syntax, so that neighboring failure
  is unchanged and remains outside this slice.

## Decision Log

- Decision: run the existing image-subscript legalization for PTX instead of teaching the direct
  emitter to walk l-value address chains.
  Rationale: the IR shape is canonical input but is intentionally target-legalized. The existing
  pass already owns partial update semantics and yields ordinary typed image operations consumed by
  multiple target backends.
  Date/author: 2026-08-29, Codex.
- Decision: replace surface `dimensionCount` with the same shape-plus-arrayness vocabulary used by
  sampled textures and advance the exact forward-only ABI to revision 16.
  Rationale: 3D and 2D array have the same coordinate width but different NVVM intrinsics. Encoding
  shape explicitly prevents inference from helper text or operand count and scales to future array
  rows without another interface family.
  Date/author: 2026-08-29, Codex.
- Decision: admit native selected Int32/UInt32/Float32 scalar/v2/v4 rows for 1D, 2D, 3D, and 2D
  array, while retaining existing Half and formatted-Float16 restrictions.
  Rationale: all 32-bit semantic kinds share the same physical i32 intrinsic table. Capability
  negotiation and focused negatives keep Float3, other widths, and unmeasured array shapes closed.
  Date/author: 2026-08-29, Codex.
- Decision: add device-wide memory barrier as one catalog value operation.
  Rationale: it is a typed zero-operand/void semantic like the established workgroup barrier and
  maps exactly to one LLVM intrinsic. No dedicated callback or GenericAsm passthrough is needed.
  Date/author: 2026-08-29, Codex.
- Decision: classify decorated native surface storage through the existing `ImageFormatInfo` table.
  Rationale: field-key format is the semantic source of truth, and Slang already owns the mapping
  from every format to channel count and scalar kind. The selected integer rows require exact
  32-bit signedness/width, while the existing Float32-to-Float16 path remains explicit.
  Date/author: 2026-08-29, Codex.
- Decision: allow Boolean sources in the generic integer-convert family.
  Rationale: the canonical final IR uses `intCast(Bool) -> UInt`, and LLVM represents the operation
  as the same lane-preserving zero extension already implemented by that descriptor family.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

Slices 99-101 established typed native-Half and formatted-Float surface operations. Slice 105
generalized the provider's physical-width handling and added native Float32 1D/2D/3D rows. The
interface still identifies a surface by dimension count, and compiler resolution occurs only in
retained CUDA-prelude GenericAsm helper functions.

The new fixture reaches two forms of surface operation. Its verification reads retain exact
GenericAsm helpers. Its partial writes remain canonical l-value IR because PTX does not currently
run the shared image-subscript legalization. After legalization, those writes become explicit
`IRImageLoad`, `IRSwizzleSet`, and `IRImageStore` instructions. Direct preflight must consume both
the retained helper representation and the legalized core-IR representation without conflating
their producers.

## Scope and Non-Goals

In scope:

- PTX use of the existing canonical image-subscript store legalization;
- native selected signed-i32, unsigned-i32, and Float32 scalar/v2/v4 surface payloads;
- non-arrayed 1D/2D/3D and arrayed 2D read-write surfaces with zero boundary behavior;
- exact signed-coordinate read helpers and legalized core image load/store operations;
- shape-aware LLVM i32 intrinsic selection, byte scaling, and semantic bitcasts;
- exact CUDA `__threadfence()` to `llvm.nvvm.membar.gl` semantics;
- the existing fixture's direct compile/PTX and runtime if the existing harness/backend supports it.

Out of scope:

- Float3/Int3/UInt3, 8/16/64-bit integer payloads, normalized or packed conversion, and atomics;
- 1D arrays, 3D arrays, cubes, multisample, rasterizer-ordered, sparse/status, or non-zero boundary;
- formatted integer conversion or inferring physical resource formats from test-harness metadata;
- arbitrary resource forwarding, arbitrary address-chain recovery, or source GenericAsm forwarding;
- changes to source-level partial-store semantics or CUDA-source emission.

## Architecture and Invariants

- The established legalizer owns image l-values. Direct preflight receives explicit image
  load/store operations and never reconstructs their source address chain.
- The surface descriptor carries operation, base shape, arrayness, complete semantic element type,
  boundary mode, and storage format. Coordinate width derives from shape plus arrayness.
- General surface classification accepts exact read-write texture flags and selected element types.
  Field classification remains the only source of native versus converting storage.
- Retained helper resolution checks complete GenericAsm text and signedness. Core image operation
  resolution checks exact opcode/operand/result identity and canonical resource-field provenance.
- Provider capability rejects invalid descriptors before mutation. One physical i16/i32 intrinsic
  table owns direction/shape/array/lane mapping; semantic signedness affects LLVM value type but not
  physical 32-bit surface instruction selection.
- The barrier catalog row is exact zero-operand void `__threadfence()` and emits only
  `llvm.nvvm.membar.gl`.

## Interfaces and Dependencies

Advance `SLANG_NVVM_BUILDER_ABI_REVISION` to 16. Move the shared texture-shape vocabulary before the
surface descriptor, replace surface dimension count with shape plus `isArray`, and append the
device-memory-barrier value operation. Update facade validation, compiler classifiers/resolvers,
real and fake providers, unit tests, the PTX legalization target list, the existing shader fixture,
focused negatives, and `docs/design/nvvm-backend.md`.

## Validation and Acceptance

Acceptance requires exact image legalization evidence; fake capability coverage across the selected
shape/type matrix; real-provider LLVM assembly checks for 2D-array load/store and global membar;
the existing fixture's direct PTX checks and any supported direct runtime lane; focused array/type
negatives; retained native/formatted Half and Float surface regressions; the complete
`slang-unit-test-tool/nvvm` prefix; LLVM verification; libNVVM; CUDA 12.9 `ptxas`; pinned
clang-format 17; and `git diff --check`.

Record exact test counts, generated instruction rows, PTX/cubin sizes, any ordinary-PTX improvement,
self-review, and the next measured fixture boundary.

## Self-Review and Input-Shape Audit

Inventory the PTX pass-list change, shared shape enum, surface classifier, image-op resolver,
requirement ownership, integer provider types, array intrinsic table, device barrier catalog row,
and negatives. For each, identify its exact producer, canonicality, semantic source of truth, and
failing test. Revert or move upstream any direct-emitter l-value recovery, arbitrary root-address
walk, coordinate-shape inference, hidden format guessing, consumer-side repair, or fallback.

## Failure and Recovery

If the shared legalization changes non-direct PTX semantics, or LLVM verification, libNVVM,
runtime, or `ptxas` rejects the selected intrinsic mapping, preserve diagnostics under ignored
`build/` and stop the loop. Never reset unrelated work or stage `external/slang-binaries/`.

## Outcomes and Retrospective

Slice 106 advances the exact forward-only builder ABI to revision 16. The surface descriptor now
uses the shared base-shape enum plus independent arrayness, and the real/fake providers negotiate
native signed-i32, unsigned-i32, and Float32 scalar/v2/v4 rows across 1D, 2D, 3D, and arrayed 2D.
LLVM 14 emits the exact arrayed-2D i32 intrinsic rows. The same ABI revision adds the catalog's
zero-operand device-memory-barrier semantic, which emits `llvm.nvvm.membar.gl`.

PTX runs the existing image-subscript legalization. The direct emitter receives its canonical
`imageLoad`/`swizzleSet`/`imageStore` read-modify-write form and resolves each operation from the
typed resource plus its exact collected-global field provenance. No l-value address recovery,
format guessing, or source GenericAsm forwarding was added. The new classifier reuses
`ImageFormatInfo`; the only adjacent value-family change is lane-preserving Boolean-to-integer
conversion through the existing parameterized conversion operation.

`texture-subscript.slang` passes its new direct CUDA runtime lane with output `1` and its permanent
direct PTX check. The 4,787-byte optimized PTX contains eight `sust`, sixteen `suld`, and eight
`membar.gl` instructions across 1D, 2D, 3D, and arrayed-2D shapes. CUDA 12.9.86
`ptxas -arch=sm_70` accepts it and emits a 4,968-byte cubin. The native Float surface prefix passes
5/5, the formatted/native Half surface prefix passes 10/10, the texture-subscript prefix passes
13/13, focused Float3/Int3/1D-array boundaries pass 3/3, and the complete Release NVVM unit prefix
passes 376/376.

Self-review retained two named helpers: one maps the already-validated surface base shape to its
semantic coordinate width, and one resolves the canonical explicit image operation. Both consume
producer-owned type/field data and are exercised by the existing full shader; neither walks
arbitrary operands or introduces an alternative representation. The pass-list change is at the
same target-legalization boundary used by GLSL, SPIR-V, and Metal. Removing it restores the eight
image-subscript l-values that direct preflight rejects; removing the resolver restores E52017 on
the legalized image operations. The ordinary PTX/NVRTC source path still fails on invalid surface
l-value syntax and was not repaired in the direct backend.
