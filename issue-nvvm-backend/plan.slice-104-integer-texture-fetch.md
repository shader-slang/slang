# Add integer-coordinate texture fetches

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs both existing CUDA texture-subscript fixtures:
`tests/compute/texture-subscript-cuda.slang` and
`tests/compute/texture-subscript-uint-cuda.slang`. One typed texture fetch-level operation covers
the retained 2D, 3D, and 2D-array helpers with scalar, two-lane, and four-lane Float, Int, or UInt
results. The compiler recognizes complete finalized helpers; the shielded LLVM provider owns the
fixed PTX spelling needed for the operation.

## Progress

- [x] (2026-08-29) Reproduced the Slice 103 baseline and rebuilt `slangc` so probes use the current
  direct route.
- [x] (2026-08-29) Captured the finalized helper signatures, GenericAsm strings, and ordinary PTX
  rows for both existing texture-subscript fixtures.
- [x] (2026-08-29) Compared LLVM 7.0.1 and LLVM 14.0.6 texture intrinsic catalogs and established
  that neither exposes the integer-coordinate fetch form with an explicit integer mip level.
- [x] (2026-08-29) Generalized read-only texture classification to selected 32-bit numeric
  scalar/v2/v4 element
  types without weakening SampleLevel's scalar-Float contract.
- [x] (2026-08-29) Added the typed fetch-level descriptor operation, compiler
  resolver/preflight/emission, and
  provider-owned fixed inline PTX implementation.
- [x] (2026-08-29) Added real/fake API coverage, direct compiler/runtime/PTX coverage for both
  fixtures, and a
  focused unsupported-shape regression.
- [x] (2026-08-29) Formatted, built, ran focused and complete NVVM validation, ran `ptxas`,
  self-reviewed, updated durable design documentation, and recorded outcomes.

## Surprises and Discoveries

- `slangc` CLI target options are committed to the requested output. A probe without `-o` took the
  legacy CUDA-source route even with `-emit-cuda-via-nvvm`; adding an explicit PTX output reproduced
  the direct E52017 boundary. The first dump was therefore discarded as pipeline evidence.
- The direct boundary is currently `helper function parameter` because sampled-texture
  classification admits only scalar Float elements. The finalized helpers themselves are simple
  two-parameter functions: a read-only texture and a signed integer location vector.
- A 2D location is `int3(x, y, mip)`, a 3D location is `int4(x, y, z, mip)`, and a 2D-array
  location is `int4(x, y, layer, mip)`. Ordinary CUDA emits `tex.level.2d`, `tex.level.3d`, and
  `tex.level.a2d` with `.s32` coordinates.
- LLVM's `llvm.nvvm.tex.unified.*.v4*.s32` intrinsics represent integer-coordinate texture reads
  without an explicit mip operand. Its explicit-level intrinsics accept floating coordinates and a
  floating level. Substituting either family would change `Texture.Load` semantics.
- The existing provider already uses descriptor-selected fixed inline PTX for formatted surface
  stores where LLVM lacks a usable intrinsic. Integer fetch can use the same encapsulation: the
  public builder sees operation, shape, arrayness, element type, and typed operands, never a source
  assembly string.
- The first end-to-end fetch probe produced a scalar descriptor with `laneCount=0`. The shared
  vector classifier clears its lane-count out-parameter before returning false for a scalar, so
  passing the scalar default variable directly to that probe destroyed the producer's default of
  one. Using a separate vector-probe result fixes descriptor construction at its source; no
  consumer accepts zero lanes or adds a scalar fallback.

## Decision Log

- Decision: add one `FETCH_LEVEL` operation to the existing texture descriptor family.
  Rationale: the operation's capability key remains texture shape, arrayness, and complete semantic
  element type. Reusing the typed query/emit interface avoids a fixture-specific callback or raw
  assembly escape.
  Date/author: 2026-08-29, Codex.
- Decision: pass texture, semantic integer coordinate, and integer level as separate builder
  operands.
  Rationale: this mirrors the established SampleLevel contract and keeps the packed Slang helper
  location out of the provider API. The compiler can use existing generic vector extract/construct
  callbacks to split the canonical helper parameter.
  Date/author: 2026-08-29, Codex.
- Decision: implement fetch with fixed provider-owned inline PTX rather than an approximate LLVM
  intrinsic mapping.
  Rationale: neither selected LLVM exposes the exact integer-coordinate explicit-level operation.
  Fixed descriptor-owned assembly preserves ordinary CUDA semantics and follows the already proven
  formatted-surface-store boundary without forwarding source GenericAsm.
  Date/author: 2026-08-29, Codex.
- Decision: remain fixture-driven and admit retained 2D, 3D, and 2D-array forms only.
  Rationale: CUDA's 1D implementation is deliberately disabled and no retained cube helper exists.
  Supporting unmeasured shapes would have no failing test and would overstate the prototype.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

Slice 102 established scalar Float SampleLevel operations. Slice 103 added integer switches and
dimension queries. The next two adjacent suite fixtures now get past their entry control flow and
stop while collecting helpers because their texture element types include numeric vectors and
integers.

Each retained `_Texture.Load` helper ends in one exact CUDA-selected GenericAsm terminator. Calls
pass a texture loaded from the conventional global block and a signed integer location vector.
The entry points already use established switch, raw structured-buffer, vector construction and
extraction, numeric conversion, arithmetic, and texture-handle transport paths.

## Scope and Non-Goals

In scope:

- read-only, non-multisampled, non-shadow, non-combined textures with Float/Int/UInt 32-bit
  scalar, two-lane, or four-lane elements;
- exact 2D, 3D, and 2D-array integer fetch-level helpers and signed integer locations;
- separate typed coordinate and mip operands across the builder interface;
- four physical PTX output registers reconstructed into the requested semantic lane count;
- ordinary/direct runtime comparison, exact PTX rows, LLVM verification, libNVVM, and `ptxas`.

Out of scope:

- 1D fetch, 1D arrays, cube/cube arrays, multisample fetch, sparse/status overloads, offsets,
  gradients, gathers, sampling, comparison, feedback, or writable textures;
- Half, Float64, Boolean, three-lane, matrix, aggregate, or other result families;
- floating-coordinate substitution for integer fetch or forwarding arbitrary GenericAsm;
- generic user-authored inline assembly or backward compatibility.

## Architecture and Invariants

- General texture classification establishes the physical read-only texture handle plus semantic
  element descriptor. Each operation resolver then applies its narrower contract: SampleLevel and
  the already demonstrated dimension queries remain scalar Float, while fetch accepts selected
  32-bit numeric scalar/v2/v4 elements.
- Fetch resolution checks the complete GenericAsm spelling, result identity with the texture
  element type, exact shape/arrayness, two helper parameters, and signed integer location length.
  Preflight persists the descriptor and canonical location parameter.
- Emission splits only the final mip lane from the canonical helper location. The preceding lanes
  form the semantic coordinate, including the array layer for an array texture. Existing typed
  vector construction and extraction callbacks perform this structural conversion.
- Provider validation requires one usable i64 texture, exact signed-i32 scalar/vector coordinate,
  and signed i32 mip. It selects a fixed descriptor-owned PTX template and output constraints
  before mutating LLVM.
- PTX always writes four scalar registers. The provider returns lane zero for scalar elements or
  constructs the requested two-/four-lane LLVM vector. Signedness affects semantic classification
  and PTX data type but not the physical i32 register type.

## Interfaces and Dependencies

Extend `SlangNVVMTextureOperation` without changing the texture interface layout. Update texture
type lowering, facade operand validation, real and fake providers, provider/compiler tests,
requirements and GenericAsm resolution, the two existing compute fixtures, a focused negative,
and `docs/design/nvvm-backend.md`. The forward-only ABI remains revision 15 because the queried
interface layout is unchanged and unsupported operations are already capability-negotiated.

Validation uses the configured Release host build, standalone LLVM provider, CUDA 12.9, and
`ptxas -arch=sm_70`. CMake builds, tests, and native formatting run outside the sandbox per
repository instructions.

## Validation and Acceptance

Acceptance requires focused real/fake builder and compiler tests; both exact texture-subscript
runtime lanes through ordinary CUDA and direct libNVVM; direct PTX checks covering all retained
shape/data-kind rows; a negative adjacent fetch shape; the complete `slang-unit-test-tool/nvvm`
prefix; LLVM verification; CUDA 12.9 `ptxas`; pinned clang-format 17; and `git diff --check`.

Record exact counts, runtime values, PTX/cubin sizes, instruction counts, self-review, and the next
measured fixture boundary as work completes.

## Self-Review and Input-Shape Audit

Inventory generalized texture classification, operation-specific restrictions, the exact helper
resolver, packed-location splitting, provider template selection, physical-result reconstruction,
and any new diagnostic special case. For each, identify its exact producer, canonicality, semantic
source of truth, and failing test. Reject approximate intrinsic substitution, source-text
forwarding, arbitrary operand recovery, hidden fallback, or a shape absent from measured IR.

The final inventory keeps each named addition and no temporary diagnostic. The generalized
classifier consumes the canonical `IRTextureTypeBase` and selected element type; separate operation
resolvers retain the narrower SampleLevel/query contracts. The fetch resolver consumes only the
three finalized CUDA-prelude helper bodies, exact helper types, and signed packed locations.
Emission structurally splits that canonical location with established vector callbacks. The
provider selects one fixed template solely from the checked descriptor, and reconstructs only the
descriptor-selected result lanes. The scalar lane-count failure was fixed in descriptor production,
not masked in preflight or the provider. No syntax reconstruction, arbitrary graph walk, source
GenericAsm forwarding, intrinsic approximation, hidden fallback, or unmeasured shape survives.

## Failure and Recovery

If LLVM verification, libNVVM, runtime comparison, or `ptxas` rejects the fixed fetch operation,
preserve the exact IR/PTX/diagnostic under ignored `build/` and stop the loop. Never reset unrelated
work or stage `external/slang-binaries/`.

## Outcomes and Retrospective

Slice 104 keeps forward-only builder ABI revision 15 and adds a capability-negotiated fetch-level
operation to the existing texture interface. Read-only texture classification now carries complete
selected Float32/Int32/UInt32 scalar, v2, or v4 element descriptors, while SampleLevel and dimension
queries explicitly retain their scalar-Float contract. Exact 2D, 3D, and 2D-array fetch helpers
split their packed signed location into semantic coordinate and mip operands.

The real provider validates the complete descriptor and operands before emitting fixed
descriptor-owned inline PTX. All three physical texture forms and all nine data-kind/lane-width
families verify in normal LLVM 14 and NVVM-2.0-compatible assembly. A retained 1D fetch stops as
unsupported `GenericAsm`, and the earlier vector SampleLevel/query negatives continue to stop at
their operation-specific boundary.

Validation evidence:

- standalone Release provider and Release `slangc`/`slang-test` builds pass;
- focused real/fake builder coverage, both three-lane shader fixture matrices, and all three
  adjacent negative fixtures pass;
- the complete `slang-unit-test-tool/nvvm` prefix passes 376/376;
- ordinary and direct mixed-resource runtime produce
  `0, 0, 40E00000, 40E00000, 40E00000, 4FDE4000, 4FDE4000, 4FDE4000, 4FDE4000, 0, 0`, while the
  UInt fixture produces `FE000000, FE000000, FE000000`;
- optimized direct PTX is 8,138 and 2,997 bytes and contains 21 and 9 integer-coordinate fetch rows;
- CUDA 12.9.86 `ptxas -arch=sm_70` accepts both modules and emits 5,800-byte and 3,432-byte cubins;
- pinned clang-format 17 and `git diff --check` pass.

The next existing-suite probe is `tests/compute/rw-texture-simple.slang`. With its compute stage and
an explicit direct PTX output, it stops at E52017 `helper function parameter`; its native Float32
read-write 1D/2D/3D surface family is the next coherent resource boundary to measure.
