# Add native Half surface-resource operations

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs the complete
`tests/compute/half-rw-texture-simple.slang` fixture. The compiler preserves read-write texture
handles as a resource kind while lowering their physical ABI to `i64`, recognizes the exact CUDA
prelude surface-read/write helpers by complete signature, and asks the shielded LLVM provider for
typed surface operations. The provider emits LLVM NVVM surface intrinsics for zero-boundary 1D/2D
Half-family loads and stores. Arbitrary GenericAsm and adjacent unsupported texture shapes remain
closed.

## Progress

- [x] (2026-08-29) Reproduced the post-Slice-98 boundary and dumped the final linked IR for all
  remaining Half texture fixtures.
- [x] (2026-08-29) Identified the exact 1D/2D scalar-Half and 2D Half4 read helpers plus the 2D
  scalar-Half write helper retained by `half-rw-texture-simple.slang`.
- [x] (2026-08-29) Verified in the local LLVM 7 and LLVM 14 sources that matching
  `llvm.nvvm.suld.*.i16.zero`, `llvm.nvvm.suld.*.v2i16.zero`,
  `llvm.nvvm.suld.*.v4i16.zero`, and `llvm.nvvm.sust.b.*.i16.zero` intrinsic families exist.
- [x] (2026-08-29) Added one generic surface-operation interface and provider implementation, then proved its
  descriptor validation independently.
- [x] (2026-08-29) Added resource-aware compiler type lowering, exact helper recognition, preflight capability
  discovery, and emission.
- [x] (2026-08-29) Added focused positive/negative tests and enabled direct runtime/PTX lanes on the existing
  shader fixture.
- [x] (2026-08-29) Formatted, built, ran focused/full/changed-shader validation, self-reviewed, updated durable docs,
  and commit the completed slice.

## Surprises and Discoveries

- CUDA texture and surface objects are already represented as 64-bit handles in the CUDA source
  emitter. The missing direct-NVVM contract is therefore resource identity and operation semantics,
  not a new physical aggregate ABI.
- Surface-load x coordinates are byte offsets. The selected CUDA GenericAsm producer multiplies the
  logical x coordinate by `$E`; the typed provider must preserve that semantic conversion rather
  than transporting or parsing the text.
- The needed zero-boundary surface intrinsic families exist unchanged in both local LLVM versions.
  This permits exact typed intrinsic construction without extending the LLVM-7 compatibility
  serializer by name rewriting.
- An earlier pre-optimization IR dump represented the 1D coordinate as a one-lane vector, while
  the final `-O3` linked IR simplifies it to signed scalar `Int`. A temporary one-lane-vector
  accommodation was removed; the compiler now matches the finalized canonical scalar shape.
- Depending on CFG simplification staging, a surface helper can be a single GenericAsm block or an
  empty entry block branching without arguments to the GenericAsm block. Validation admits exactly
  those two equivalent producer shapes and no general control flow.
- LLVM 14 assembled the native surface intrinsic declarations without any compatibility rewrite,
  and CUDA 12.9 libNVVM accepted the resulting textual NVVM IR.

## Decision Log

- Decision: introduce a separate, descriptor-driven surface-operations interface rather than add
  callbacks per intrinsic or overload scalar value operations.
  Rationale: resource access has a handle, dimensional coordinates, an element shape, and a
  boundary contract. Keeping those semantics together scales to more shapes and formats while the
  scalar value-operation descriptor remains honest.
  Date/author: 2026-08-29, Codex.
- Decision: preserve texture/resource identity in compiler classification and lower only the
  physical provider type to `i64`.
  Rationale: treating the handle as an ordinary unsigned integer would let unrelated integer
  values satisfy resource helper signatures and erase the source-level invariant preflight needs.
  Date/author: 2026-08-29, Codex.
- Decision: recognize helpers from exact GenericAsm text plus their complete result, resource,
  coordinate, and value signature.
  Rationale: helper names and placeholder parsing are not semantic sources of truth. A full exact
  signature admits only the canonical CUDA-prelude producer shape.
  Date/author: 2026-08-29, Codex.
- Decision: revise the forward-only builder ABI and require the new interface at initialization.
  Rationale: this experiment intentionally carries no compatibility surface; compiler and provider
  should fail discovery rather than silently omit required resource semantics.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

The selected shader uses three read-write surface objects and one output buffer. Its retained
helpers have these semantics:

    Half  load2D(RWTexture2D<Half>,  int2)
    Half  load1D(RWTexture1D<Half>,  int)
    Half4 load2D(RWTexture2D<Half4>, int2)
    void  store2D(RWTexture2D<Half>, uint2, Half)

After specialization and `-O3`, each selected fixture helper is a one-block function terminated by
the exact selected `surf1Dread`, `surf2Dread`, or `surf2Dwrite` GenericAsm body. The validator also
accepts the equivalent trivial entry-to-GenericAsm branch produced before that simplification.
Before this slice, direct preflight rejected the resource parameter before any builder module was
created. The global-parameter layout already places the three CUDA handles at offsets 0, 8, and 16
and the structured-buffer view at offset 24.

The provider includes LLVM's NVPTX intrinsic definitions. Surface loads return integer bits (or an
LLVM aggregate of integer lanes), while stores take integer bits. The shield must bitcast Half lanes
at the operation boundary, scale logical x by element byte size, and keep the rest of the compiler
independent of LLVM types and intrinsic IDs.

## Scope and Non-Goals

In scope:

- non-arrayed, non-multisampled, non-shadow read-write 1D/2D surface handles;
- zero-boundary scalar Half, Half2, and Half4 surface loads and stores supported by LLVM's native
  i16 families;
- exact canonical prelude helper signatures and text;
- forward-only interface negotiation, provider validation, fake-boundary recording, compiler
  preflight, direct runtime/PTX validation, and `ptxas` assembly;
- the complete `half-rw-texture-simple.slang` fixture.

Out of scope:

- sampled read-only textures, samplers, combined textures, arrays, multisampling, shadow compare,
  mip levels, normalized coordinates, gradients, gathering, or texture-size queries;
- non-Half surface formats, Half3, non-zero boundary modes, raster ordered resources, or atomics;
- arbitrary GenericAsm, helper-name recognition, placeholder parsing, or textual LLVM rewriting;
- backward-compatible interface discovery.

## Architecture and Invariants

- A surface descriptor carries operation kind, dimensionality, boundary mode, and complete semantic
  element type. Operands remain an ordered semantic sequence: handle and coordinate for load;
  handle, coordinate, and value for store.
- Only scalar Half, Half2, and Half4 descriptors are supported here. Half3 remains unsupported
  because NVVM exposes scalar, v2, and v4 native surface families.
- A 1D coordinate lowers to `i32`; a 2D coordinate lowers to `<2 x i32>`. Signed and unsigned Slang
  coordinates share those signless physical types but exact helper matching still verifies the
  canonical source signature.
- A resource helper parameter lowers physically to `i64` only after exact resource classification.
  Ordinary integer parameters do not become surface handles.
- Preflight gathers every required surface descriptor and queries provider support before creating
  a module. Unsupported descriptors and malformed helpers stop deterministically before mutation.
- The provider computes byte x from the element's storage width and emits only exact zero-boundary
  LLVM intrinsics. It never receives GenericAsm text or a Slang IR type.

## Interfaces and Dependencies

Revise `slang-nvvm-ir-builder-api.h` with a surface operation descriptor and required interface;
cache and validate it in `NVVMIRBuilder`; implement it in `slang-llvm-nvvm.cpp`; and extend the C
compile probe. Extend NVVM type lowering with an exact native Half surface classifier. Add compiler
helper resolution, capability collection, and emission in `slang-emit-nvvm.cpp`. Mirror the API in
the fake builder and focused unit tests, update `docs/design/nvvm-backend.md`, and register the
existing compute fixture for direct CUDA runtime and PTX.

Validation uses the configured Release host build, standalone LLVM provider, CUDA 12.9, and
`ptxas -arch=sm_70`. CMake builds, tests, and native formatting run outside the sandbox per
repository instructions.

## Milestones

1. Define and negotiate the generic surface descriptor/API; implement exact provider support and
   focused builder tests for valid and invalid descriptors/operands.
2. Classify the exact native Half surface types, preserve resource identity, and lower their
   physical handles and coordinates.
3. Recognize the canonical helpers, collect provider requirements in preflight, and emit typed
   surface operations through the facade.
4. Add fake/compiler tests, enable and run direct shader lanes, assemble PTX, run the complete NVVM
   prefix, audit the diff, update docs and this plan, then commit.

## Validation and Acceptance

Acceptance requires API-negotiation and provider surface-operation tests; focused compiler tests
covering exact positive helpers and malformed/adjacent negative shapes; the complete
`slang-unit-test-tool/nvvm` prefix; every enabled `half-rw-texture-simple.slang` lane; standalone
optimized direct PTX; CUDA 12.9 `ptxas` assembly; pinned clang-format 17; and `git diff --check`.

The Release build completed for the standalone provider, `slangc`, and `slang-unit-test`. The
focused malformed-helper test passed 1/1. The complete NVVM prefix passed 372/372. The fixture
passed all three enabled lanes (existing CUDA source, direct-NVVM CUDA runtime, and direct-NVVM PTX
FileCheck), with seven unrelated lanes ignored. Optimized direct PTX is 1,681 bytes and contains
the expected scalar 1D/2D loads, four-lane 2D load, and scalar 2D store. CUDA 12.9.86
`ptxas -arch=sm_70` produced a 3,176-byte cubin. Pinned clang-format 17 and `git diff --check`
completed cleanly apart from expected checkout line-ending notices.

## Self-Review and Input-Shape Audit

The final inventory contains the surface descriptor validator and intrinsic selector, the native
Half texture classifier, the exact GenericAsm/signature resolver, and requirement deduplication.
They survive because each owns one boundary invariant: the provider's exact instruction family,
the compiler's canonical resource type, the producer's complete helper semantic, and preflight's
unique capability set. Resource handles never pass through integer semantic matching; only their
physical lowering is `i64`. Helper text is compared exactly and never parsed, and no helper name
participates. Half3 and adjacent texture attributes remain closed in focused descriptor tests. The
provider owns byte-coordinate scaling because it is part of the surface instruction contract.

The audit removed two accidental accommodations. Entry-point parameter validation no longer
admits surface resources because this slice establishes only conventional-global resource fields.
The fake provider now records a distinct typed surface-operation result instead of manufacturing a
floating unary scalar operation, preserving honest operation counts. The temporary one-lane 1D
coordinate alternative was also reverted after the final linked IR established scalar `Int` as the
canonical shape. The malformed-helper test proves exact text with an extra parameter still stops at
GenericAsm before builder discovery.

## Failure and Recovery

If LLVM cannot construct or libNVVM cannot compile the locally declared surface intrinsics, retain
the exact emitted assembly and diagnostic and stop rather than transporting GenericAsm or adding a
blind serializer rewrite. Generated dumps, PTX, and cubins stay under ignored `build/`. Never reset
unrelated work or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Record exact fake descriptors, emitted LLVM/NVVM IR, runtime output, PTX/cubin sizes, focused/full
test counts, next exact fixture stop, and the completed self-review inventory. Distill durable
surface-resource architecture and coverage into `docs/design/nvvm-backend.md`.

## Outcomes and Retrospective

Slice 99 establishes the first complete resource path through direct NVVM: semantic texture
classification, physical handle ABI, preflight capability negotiation, typed shield emission,
real libNVVM compilation, PTX assembly, and GPU runtime behavior. The existing
`half-rw-texture-simple.slang` test is enabled rather than duplicated, and malformed signatures
remain closed before provider discovery.

The next slice should probe the remaining Half texture fixtures from this final tree and choose the
largest coherent family at their first measured stop. The likely boundary is the CUDA prelude's
Half conversion surface helpers, but that must be confirmed from compiler diagnostics and final IR
rather than inferred from source names.
