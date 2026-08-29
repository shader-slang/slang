# Add formatted Half surface conversion

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs the existing
`tests/compute/half-rw-texture-convert2.slang` fixture. Typed surface descriptors distinguish a
native semantic representation from Float/Float2/Float4 values backed by R16F/RG16F/RGBA16F
storage. Formatted reads load native i16 Half bits and widen them to Float; formatted writes emit
fixed typed `sust.p` zero-boundary operations. Resource format is resolved from every direct call
site's canonical field decoration, and inconsistent or smuggled resource values remain closed.

## Progress

- [x] (2026-08-29) Reproduced the final Slice 100 boundary and captured finalized IR for both
  formatted RW texture fixtures.
- [x] (2026-08-29) Verified expanded CUDA semantics: reads use byte-scaled ordinary surface loads
  plus Half-to-Float conversion, while formatted writes use unscaled `sust.p.*.b32.zero`.
- [x] (2026-08-29) Verified LLVM 7 and LLVM 14 declare native i16 surface loads but expose only
  trap-boundary `sust.p` intrinsic IDs; fixed zero-boundary LLVM inline asm is therefore the
  provider experiment gate.
- [x] (2026-08-29) Revised the typed surface descriptor and provider for native versus
  Float16-formatted storage; LLVM, libNVVM, and `ptxas` accepted the fixed typed store assembly.
- [x] (2026-08-29) Resolved exact resource formats from direct canonical call sites and carried the resolved helper
  semantics from preflight into emission.
- [x] (2026-08-29) Added focused descriptor/compiler tests and enabled direct runtime/PTX coverage on the existing
  formatted fixtures.
- [x] (2026-08-29) Formatted, built, ran focused/full/changed-shader validation, assembled PTX,
  self-reviewed, updated durable docs, and prepared the completed slice commit.

## Surprises and Discoveries

- The finalized helper GenericAsm text intentionally retains `$C` and `$E`; CUDA source expansion
  decides `_convert` and coordinate scaling from the actual resource argument's `IRFormatDecoration`
  at each call site. Helper parameter type alone does not encode the backing format.
- `half-rw-texture-convert2.slang` expands stores with x scaling of one and reads with x scaling of
  two, four, or eight bytes. This asymmetry is the `sust.p` contract, not an optimizer artifact.
- Slang's existing intrinsic expander explicitly documents that format attributes can be lost when
  a resource is smuggled through an arbitrary helper. Direct NVVM can remain deterministic by
  accepting only direct load-from-formatted-field arguments and requiring every call to one
  retained surface helper to resolve to the same descriptor.
- LLVM 14 verified and serialized descriptor-owned inline assembly for every 1D/2D scalar, v2, and
  v4 formatted store. libNVVM emitted the expected `sust.p.*.b32.zero` PTX without a name rewrite or
  GenericAsm transport, and CUDA 12.9 `ptxas` accepted the result.

## Decision Log

- Decision: add a storage-format field to the generic surface descriptor rather than add separate
  formatted callbacks or overload GenericAsm transport.
  Rationale: operation, dimensions, semantic value type, backing representation, and boundary mode
  form one complete surface semantic. The existing interface remains economical as formats grow.
  Date/author: 2026-08-29, Codex.
- Decision: emit fixed zero-boundary `sust.p` LLVM inline asm inside the shield if libNVVM accepts
  it.
  Rationale: both supported LLVM versions omit the needed zero variants from their intrinsic tables,
  while CUDA's documented fixed PTX instruction is the producer's actual semantic. Keeping the
  exact template inside the typed provider preserves the LLVM shield and does not transport user or
  GenericAsm text.
  Date/author: 2026-08-29, Codex.
- Decision: resolve formatted storage from exact direct call arguments and persist each helper's
  descriptor in preflight requirements.
  Rationale: the canonical source of truth is the struct-field format decoration. Reusing the
  source emitter's bounded load/field producer shape is preferable to guessing from Float types;
  persisting the result avoids a second provenance walk during emission.
  Date/author: 2026-08-29, Codex.
- Decision: reject multiple incompatible call-site formats or resource parameters whose format
  provenance is unavailable.
  Rationale: one emitted helper body cannot honestly implement two backing formats. Cloning or
  upstream type specialization is a future general solution; silently choosing one is invalid.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

The fixture retains six helpers:

    Float  load2D(Texture2D<Float  backed by R16F>,    Int2)
    Float2 load2D(Texture2D<Float2 backed by RG16F>,   Int2)
    Float4 load2D(Texture2D<Float4 backed by RGBA16F>, Int2)
    void store2D(the same resource/value shapes with UInt2 coordinates)

All helper bodies use the same canonical `surf2Dread$C...$E` or `surf2Dwrite$C...$E` GenericAsm
text as native surfaces. At each entry-point call, argument zero is a load from a conventional
global field whose key has the exact R16F, RG16F, or RGBA16F decoration. CUDA intrinsic expansion
uses that call argument to choose `_convert`, byte-scale reads, and leave formatted-write x
coordinates unscaled.

Slice 99's descriptor carries semantic element type but assumes native storage. The provider maps
Half lanes to i16 `suld`/`sust.b`. This slice must preserve that path while adding Float32 semantic
lanes backed by Float16 channels. Formatted reads can reuse i16 `suld` and widen each reconstructed
Half. Formatted stores consume Float32 lanes directly in `sust.p.*.b32.zero`.

## Scope and Non-Goals

In scope:

- non-arrayed, non-multisampled, non-shadow, non-combined read-write 1D/2D resources;
- scalar, two-lane, and four-lane Float32 semantic values backed by matching Float16 channels;
- exact R16F, RG16F, and RGBA16F conventional-global field decorations;
- zero-boundary formatted loads and stores, direct call-site consistency, typed preflight, fixed
  provider inline PTX, runtime/PTX fixture coverage, and `ptxas` assembly;
- the two existing formatted Half RW texture fixtures.

Out of scope:

- unformatted Float32 surfaces, other image formats, Half3/Float3, arrays, 3D, cube, multisample,
  shadow, combined, raster-ordered, non-zero-boundary, or atomic surfaces;
- resources passed through arbitrary user helpers, inconsistent call-site formats, helper cloning,
  or an upstream global format-specialization redesign;
- sampled textures, samplers, mip levels, or SampleLevel;
- arbitrary inline assembly, GenericAsm transport/parsing, or backward compatibility.

## Architecture and Invariants

- A descriptor's semantic element type is the load result/store value type. Its storage format says
  whether those lanes are represented natively or converted to Float16 channels.
- Native format remains exact Half/Half2/Half4. Float16-formatted storage requires exact
  Float/Float2/Float4 and matching R/RG/RGBA field decoration.
- Every call to one surface helper must resolve to the same complete descriptor. Preflight stores
  that descriptor with the helper identity and emission performs a direct lookup.
- Formatted resource provenance is exactly `load(get_field_addr(conventionalGlobal, key))`; the key
  supplies the format decoration. No arbitrary operand graph or helper chain is searched.
- Reads scale x by storage bytes (two times lane count), load i16 lanes, reconstruct Half, and widen
  to Float32. Native stores retain byte scaling; formatted stores do not scale x.
- Fixed provider inline asm is selected only from descriptor enums and lane/dimension counts. No
  source text crosses the builder ABI.
- Capability queries remain pure and finish before module creation. Provider operand validation
  finishes before any LLVM mutation.

## Interfaces and Dependencies

Revise builder ABI 13 with one surface storage-format enum/field. Extend facade/fake/provider
validation. Generalize the internal surface classifier, resolve field decorations at direct calls,
and persist helper descriptors in `NVVMOperationRequirements`. Add exact negative coverage for
wrong formats and unsupported formatted descriptors. Update the two compute fixtures and
`docs/design/nvvm-backend.md`.

Validation uses the configured Release host build, standalone LLVM provider, CUDA 12.9, and
`ptxas -arch=sm_70`. CMake builds, tests, and native formatting run outside the sandbox per
repository instructions.

## Milestones

1. Extend the descriptor/provider and prove formatted inline PTX survives LLVM verification,
   libNVVM compilation, and `ptxas` assembly.
2. Classify exact semantic resource types, resolve direct field formats consistently, and reuse the
   persisted descriptor in helper emission.
3. Add focused fake/compiler tests, enable existing runtime/PTX lanes, run the complete NVVM prefix,
   audit the diff, update docs and this plan, then commit.

## Validation and Acceptance

Acceptance requires API negotiation and positive/negative formatted descriptor tests; focused
compiler format/provenance tests; the complete `slang-unit-test-tool/nvvm` prefix; every enabled
lane of both formatted fixtures; optimized direct PTX with scalar/v2/v4 native reads and formatted
stores; CUDA 12.9 `ptxas` assembly; pinned clang-format 17; and `git diff --check`.

Record exact counts, runtime output, emitted LLVM inline asm/PTX, PTX/cubin sizes, and the next
measured fixture boundary as work completes.

## Self-Review and Input-Shape Audit

Inventory the format classifier, direct provenance resolver, helper consistency check, provider
inline-asm selector, and every new branch. Record the exact load/field producer and why it is a
valid canonical input; confirm malformed provenance is rejected rather than searched through
arbitrary graphs. Verify that type alone never guesses a backing format, semantic source-of-truth
decorations are preserved, preflight owns provenance, emission only looks up resolved data,
provider validation precedes mutation, and no texture logic leaks into scalar value operations.

## Failure and Recovery

If LLVM verification or libNVVM rejects fixed inline `sust.p.*.zero`, preserve the exact generated
NVVM IR and diagnostic and stop the loop. Do not substitute trap boundary behavior, transport
GenericAsm, or add a serializer name rewrite. Generated dumps, PTX, and cubins stay under ignored
`build/`. Never reset unrelated work or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Record exact descriptors, resolved resource-field decorations, inline LLVM assembly, emitted PTX,
runtime output, ptxas result, test counts, next exact fixture stop, and completed self-review. Distill
durable formatted-surface architecture and coverage into `docs/design/nvvm-backend.md`.

## Outcomes and Retrospective

Builder ABI revision 13 now distinguishes native Half semantic/storage rows from Float32 semantic
values backed by Float16 surface channels. The provider validates the complete descriptor and
operands before LLVM mutation. Formatted reads issue native i16 `suld`, reconstruct Half, and
`fpext` to Float32. Formatted stores select one of six fixed descriptor-owned inline-assembly
templates and preserve pixel coordinates; native stores retain byte scaling.

The generalized resource classifier admits only exact selected read-write 1D/2D surface types.
The field classifier then requires the matching R16F/RG16F/RGBA16F key decoration for Float
semantics. During GenericAsm preflight, every direct call must pass the exact
`load(fieldAddress(conventionalGlobal, key))` producer and resolve to one consistent storage row.
The resulting descriptor is stored beside the helper identity; emission performs a direct lookup
and does not repeat the provenance walk. The missing-format diagnostic test fails at the
conventional-global field boundary, while the helper-smuggling test fails at GenericAsm. Removing
either check would make one of those tests incorrectly proceed by semantic type alone.

The self-review inventory contains five intentional pieces: the generalized surface type
classifier, the field/storage classifier, the exact call-site resolver, the persisted helper
requirement lookup, and the provider inline-assembly selector/conversion branches. The first two
operate at canonical type/field construction boundaries. The call-site resolver examines one
documented producer shape and rejects arbitrary graphs. The requirement lookup prevents a second
structural interpretation during emission. The provider mapping is one bounded source of truth
selected only from enums and dimensions. None reconstruct checked syntax, redo substitution, add a
fallback, or patch malformed IR downstream; all survive the audit.

Final validation used the standalone Release provider and Release host build. The full NVVM unit
prefix passes 373/373. The combined changed-fixture and native-surface regression set passes 19/19
with 26 intentionally ignored platform lanes. Both formatted fixtures pass Vulkan,
ordinary CUDA, and direct CUDA runtime; `half-rw-texture-convert2.slang` additionally passes direct
PTX FileCheck and returns 96, 100, 104, and 108. The optimized PTX is 1,956 bytes and contains
scalar/v2/v4 `sust.p.2d.*.b32.zero` followed by v4/v2/scalar `suld.b.2d.*.b16.zero`; CUDA 12.9.86
`ptxas -arch=sm_70` produces a 3,304-byte cubin. The focused 1D fixture proves the corresponding
six 1D rows. Pinned clang-format 17 and `git diff --check` pass.

The next measured fixture boundary is `tests/compute/half-texture-simple.slang`: direct preflight
reaches sampled Texture/Sampler helper parameters and `SampleLevel` helper bodies. Sampled texture
coordinates, sampler handles, dimensionality, and result widths form the next coherent resource
family; they are not folded into the surface operation added here.
