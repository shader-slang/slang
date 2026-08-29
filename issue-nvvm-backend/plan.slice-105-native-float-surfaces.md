# Add native Float32 read-write surfaces

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs `tests/compute/rw-texture-simple.slang`. One existing
typed surface-operation interface covers native Float32 scalar/v2/v4 loads and stores for retained
1D, 2D, and 3D read-write textures. The provider emits exact NVVM surface intrinsics and the source
fixture permanently checks all eighteen shape/width/direction rows.

## Progress

- [x] (2026-08-29) Completed and committed Slice 104 as `d2466617e`, with 376/376 NVVM unit tests
  and both integer-coordinate texture-subscript fixtures green.
- [x] (2026-08-29) Reproduced the direct E52017 `helper function parameter` boundary for the next
  existing fixture with an explicit stage and PTX output.
- [x] (2026-08-29) Captured the finalized helper signatures, all six 1D/2D/3D GenericAsm bodies,
  and the eighteen ordinary CUDA surface PTX rows.
- [x] (2026-08-29) Generalized the selected surface type/storage classifier for native Float32 and
  retained 3D
  shapes without weakening formatted-Float16 provenance.
- [x] (2026-08-29) Extended compiler helper resolution, real/fake provider capability, intrinsic
  selection,
  coordinate emission, and payload conversion for native Float32 1D/2D/3D scalar/v2/v4 rows.
- [x] (2026-08-29) Enabled the existing runtime/PTX fixture, promoted the obsolete unformatted-Float
  negative,
  and retained focused unsupported-width and formatted-provenance boundaries.
- [x] (2026-08-29) Formatted, built, ran focused and complete validation, ran `ptxas`, self-reviewed,
  updated durable
  design documentation, and recorded outcomes plus the next measured fixture boundary.

## Surprises and Discoveries

- The first command-line probe omitted `-stage compute`, producing the front-end's normal missing
  stage diagnostic and an unrelated internal-error cascade. The corrected probe is the only direct
  boundary evidence used by this plan.
- Final linked IR retains exactly eighteen helpers: load and store for Float32 scalar, v2, and v4
  across 1D, 2D, and 3D. Loads use signed Int coordinates; stores use unsigned UInt coordinates.
  The helper bodies are the existing `surf*read$C`/`surf*write$C` strings with byte-scaled x.
- Ordinary CUDA emits `suld.b` and `sust.b` `.b32` rows. Scalar, v2, and v4 are present for each
  dimension; the 3D PTX spelling physically supplies a fourth coordinate slot, which the NVVM
  intrinsic lowering owns rather than the public builder interface.
- Both LLVM 7.0.1 and LLVM 14.0.6 expose the exact `llvm.nvvm.suld.*.i32.zero` and
  `llvm.nvvm.sust.b.*.i32.zero` families for all retained dimensions and lane widths. Unlike Slice
  104's fetch operation, native Float32 surfaces need no inline PTX escape.
- The earlier `nvvm-formatted-surface-requires-format.slang` negative encoded a temporary boundary:
  absent `[format]` can now select the CUDA prelude's native Float32 payload, while an explicit R16F
  format still selects conversion and still requires canonical collected-field provenance.

## Decision Log

- Decision: extend the existing surface descriptor instead of adding a native-Float callback or
  operation family.
  Rationale: operation, dimension count, semantic element type, boundary mode, and storage format
  already form the complete capability key. Native Float32 is one additional supported descriptor
  matrix, not a new interface.
  Date/author: 2026-08-29, Codex.
- Decision: interpret an undecorated selected Float32 read-write texture as native Float32 storage;
  keep matching R16F/RG16F/RGBA16F decorations as explicit Float16 conversion.
  Rationale: the finalized ordinary CUDA helper instantiation and PTX derive a 32-bit native payload
  from the semantic template type. `[format]` remains the source of truth only when conversion is
  requested.
  Date/author: 2026-08-29, Codex.
- Decision: admit 3D only for native Float32 in this slice.
  Rationale: the motivating fixture measures all native Float32 3D rows. Native Half and formatted
  Float-to-Half 3D resources are unmeasured neighboring shapes and remain closed.
  Date/author: 2026-08-29, Codex.
- Decision: keep semantic Float values across the ABI and bitcast only inside the LLVM provider.
  Rationale: the NVVM intrinsics spell physical 32-bit payloads as i32, while the Slang resource
  contract is Float32. The shielded provider already owns the analogous Half payload conversion.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

Slices 99-101 established typed read-write surfaces for native Half and formatted Float-to-Half
conversion in 1D/2D. The descriptor-driven surface interface, resource-handle lowering, call-site
format provenance, helper requirements, and load/store emission already exist. Slice 104 broadened
read-only textures but intentionally did not change read-write surfaces.

The new fixture's conventional global block contains nine read-write texture handles followed by
the established output buffer. Its entry point constructs signed load and unsigned store
coordinates from dispatch index, calls each retained helper, accumulates selected Float lanes, and
stores one final value. Existing conventional-global, vector, conversion, arithmetic, and raw
structured-buffer paths already cover everything outside the surface helpers.

## Scope and Non-Goals

In scope:

- non-arrayed, non-multisampled, non-shadow, non-combined read-write 1D/2D/3D textures;
- native Float32 scalar, v2, and v4 elements with zero boundary behavior;
- exact finalized signed-load and unsigned-store helper coordinate signatures;
- byte-scaled x coordinates using four bytes per semantic lane;
- i32 intrinsic payloads bitcast to/from semantic Float32 inside the provider;
- ordinary/direct runtime comparison, exact PTX rows, LLVM verification, libNVVM, and `ptxas`.

Out of scope:

- Float3, Half3, Float64, integer, normalized, packed, matrix, or aggregate elements;
- native Half 3D or formatted Float-to-Half 3D resources;
- arrays, multisample, rasterizer-ordered, write-only, sparse/status, atomics, mip/sample indices,
  or non-zero boundary modes;
- arbitrary resource provenance, source GenericAsm forwarding, or backward compatibility.

## Architecture and Invariants

- General surface classification consumes the canonical `IRTextureTypeBase`, exact access/shape
  flags, and selected scalar/vector element. It admits dimension three only with semantic Float32.
- Field classification treats no format decoration as native semantic storage. A format decoration
  must exactly match the existing Float16 conversion rows; any other decoration remains rejected.
- Helper resolution checks one complete finalized GenericAsm string, exact result/value identity,
  exact signedness and lane count of the coordinate, and canonical call-site resource loads. It
  persists one complete descriptor per helper.
- Provider capability restricts the descriptor matrix before mutation. Intrinsic selection uses
  direction, dimension, physical bit width, and lane count as one source of truth.
- The provider scales x by semantic lane count times physical scalar byte width, appends y/z from
  the semantic coordinate, bitcasts store lanes to i32, and reconstructs Float32 load lanes from
  i32 results. LLVM owns the physical 3D coordinate spelling.

## Interfaces and Dependencies

Builder ABI revision 15 and the surface interface layout remain unchanged. Update surface type and
field classification, helper resolution, real/fake provider capability and emission, API tests,
the existing compute fixture, focused surface tests, and `docs/design/nvvm-backend.md`.

Validation uses the configured Release host build, standalone LLVM provider, CUDA 12.9, and
`ptxas -arch=sm_70`. CMake builds, tests, and native formatting run outside the sandbox per
repository instructions.

## Validation and Acceptance

Acceptance requires focused fake/real surface capability coverage; ordinary and direct runtime
lanes for `rw-texture-simple.slang`; direct PTX checks for all eighteen rows; focused unsupported
Float3 and formatted-provenance boundaries; the complete `slang-unit-test-tool/nvvm` prefix; LLVM
verification; CUDA 12.9 `ptxas`; pinned clang-format 17; and `git diff --check`.

Record exact counts, runtime values, PTX/cubin sizes, instruction counts, self-review, and the next
measured fixture boundary as work completes.

## Self-Review and Input-Shape Audit

Inventory the surface classifier, absent-format interpretation, 3D helper strings, signed/unsigned
coordinate checks, provider descriptor matrix, intrinsic selector, byte scaling, payload bitcasts,
and any changed negative test. For each, identify its exact producer, canonicality, semantic source
of truth, and failing test. Reject source-text forwarding, arbitrary resource recovery, hidden
format guessing, consumer-side shape repair, unmeasured resource families, or fallback.

## Failure and Recovery

If LLVM verification, libNVVM, runtime comparison, or `ptxas` rejects an intrinsic mapping, preserve
the exact IR/PTX/diagnostic under ignored `build/` and stop the loop. Never reset unrelated work or
stage `external/slang-binaries/`.

## Outcomes and Retrospective

Builder ABI revision 15 was sufficient. The compiler now classifies undecorated selected Float32
read-write fields as native storage and admits their exact 1D/2D/3D helper signatures. Explicit
matching Float16-format decorations retain conversion storage and exact collected-field
provenance. Float3, native Half 3D, formatted Float-to-Half 3D, and all unmeasured resource
families remain closed.

The provider uses one table indexed by direction, dimension, physical width, and lane width for
the selected i16/i32 intrinsic rows. Its shared emitter derives byte scaling and payload bitcasts
from physical width and appends every semantic coordinate dimension. No callback, ABI field,
source-text forwarding path, fallback, or consumer-side representation repair was added.

`rw-texture-simple.slang` passes CPU, Vulkan, ordinary CUDA, direct-libNVVM CUDA, and direct PTX
checks: 5/5 executed lanes pass and the runtime output is `3, 24, 45, 66`. The affected native and
formatted surface family passes 26/26 executed tests with 31 unsupported-backend lanes ignored.
The complete `slang-unit-test-tool/nvvm` prefix passes 376/376. LLVM verification and libNVVM both
accept all eighteen unique surface rows. The 3,642-byte optimized PTX contains 21 surface
instructions and CUDA 12.9.86 `ptxas -arch=sm_70` emits a 4,328-byte cubin.

Self-review found no new helper, fallback, arbitrary graph walk, or special-case repair. The exact
3D GenericAsm strings are produced by retained CUDA-prelude helpers; coordinate type and signedness
come from those finalized helper signatures; native-versus-converting storage comes from the
canonical collected field; and the provider's physical bitcasts directly implement the selected
NVVM intrinsic ABI. Removing any of those gates reproduces the focused positive, Float3 negative,
formatted-provenance negative, or full fixture failure at the layer that owns the invariant.

The next existing fixture, `tests/compute/texture-subscript.slang`, stops at E52017 `helper function
parameter`. It retains native Int4 read-write surfaces over 1D, 2D, 3D, and 2D-array shapes plus
partial-vector updates and barriers. Slice 106 should measure that complete finalized IR before
choosing the next coherent capability boundary.
