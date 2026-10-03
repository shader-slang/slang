# NVVM feature and evidence matrix

This matrix describes qualified combinations, not a claim of complete Slang or CUDA support. The
[architecture](nvvm-backend.md) owns representations and invariants. [STATUS](../../issue-nvvm-backend/STATUS.md)
owns the current accepted checkpoint and loaded compiler identity; its referenced manifests own exact
inventories and outcomes. Source test links below identify durable contracts; the accepted evidence
records which compiler and inputs were actually tested. Deeper historical evidence is available through the
[archive guide](../../issue-nvvm-backend/HISTORY.md).

Entry and helper parameter/result admission and provider lowering share provider-independent
`NVVMTypeInfo` role analysis.
Argument provenance, exported ABI restrictions and physical layout compatibility remain separate
requirements; sharing type classification does not broaden any qualified role.

## Reading the evidence

- **GPU** means an independently expected output was qualified on the recorded platform. It does not
  mean every type/role combination, host layout or optimization has been tested.
- **Compile/assembly** means compilation and fresh cubin assembly, without a runtime conclusion.
- **Unit/IR** means provider/emitter/representation checks. Fake-provider success is not GPU execution.
- **Boundary** means a deliberate unsupported or failing case whose outcome remains visible.
- **Focused research** means a bounded experiment; its sources may be in archived evidence rather than
  the permanent corpus. It does not silently expand the implementation or main corpus.

The maintained runtime comparison is NVRTC O3, NVVM O0 and NVVM O3. NVRTC O3 is a comparison, not a
universal oracle. Explicit expected values, complete buffers, guards and execution counts establish
correctness. An unsupported compile has no GPU result. A deliberate corruption rejected by an oracle
is successful test infrastructure evidence, not a passing shader.

Current broad qualification is on native Ubuntu 24.04, L4 SM89, driver 580.126.09, target SM80,
CUDA 12.9.2/NVRTC 12.9.86 and LLVM 14. Other historical toolkits/devices retain their own provenance.
The installed compiler can be older than Git HEAD; source revision alone does not identify loaded code.

## TensorView, references and canonical numeric storage

TensorView/DiffTensorView use a typed 56-byte descriptor with the existing host offsets and alignment.
Query, address, load/store, reference and public atomic methods compose ordinary NVVM operations.
Scalar indexing covers ranks 1–5 and vector indexing widths 1–4; noncontiguous byte strides and
nested helper/entry descriptors are qualified. Address products widen before multiplication, avoiding
the CUDA-text route's 32-bit product wrap. Torch-dependent MakeTensorView is unqualified because the
optional dependency is unavailable; this is not a claim of complete Torch interoperability.

The sibling `tensor-view-descriptor`, `tensor-view-numeric-storage`, `tensor-view-atomic` and
`structured-numeric-child-storage` fixtures retain independent descriptor, numerical and byte oracles.
Numeric storage covers 204 cells per family at O0/O3, both matrix orders, standard scalar widths,
width3 packing, Bool byte0x80, padding/guards, immutable input and two mutable aliases. Tensor atomics
cover the complete public overload family in 16 cells. [Reference returns](../../tests/compute/ref-accessor-return.slang)
execute on CPU, NVRTC and NVVM; [invalid returns](../../tests/diagnostics/ref-accessor-return.slang)
reject values/coercions instead of manufacturing a temporary address. General reference lifetime
analysis is outside this qualification.

Canonical integer and floating atomics retain their operation/type/order restrictions while admitting
checked generic, global and shared pointers. [Shared floating atomics](../../tests/cuda/nvvm-shared-floating-atomics.slang)
cover shared helper transport and nested resource children. Known local allocation roots reject before
provider creation; unknown generic helper pointers require global/shared backing at runtime.
The maintained focused manifest owns exact build identities, comparisons and validation histories.

## Pointer-bearing entry values and checked local addresses

Finite helper records and fixed arrays now decode canonical UserPointer leaves from CUDA launch
storage into generic executable pointers. Nested numeric/Half fields retain CUDA packing and the
integer Half call transport. Pointer-bearing parameter-group aggregate loads use the same checked
decoder. The sibling `pointer-aggregate-entry-abi` fixture exercises nested arrays, pointer-to-record,
pointer-to-array, pointer-to-pointer, value/out/inout/readonly forwarding, Half3/Bool/float3 launch
fields, cbuffer loads and guarded writes at O0/O3 on both routes (155 assertions per route).

Ordinary typed offsets retain the CUDA/LLVM layout proof. Shared storage legalization now makes
standard numeric scalar/vector/matrix, nested record/array and pointer-to-pointer storage match
CUDA packing, including float3 stride12, Half3 padding and Bool byte truthiness. Local and device
aliases use the same recipe without pointer-call copies. The sibling `compact-pointer-storage`
fixture covers 204 shape/optimization cells with independent byte guards; `compact-pointer-record`
covers nested arrays, records and Packet** aliases at O0/O3. Exact pointer qualification retains
readonly and explicit-layout rejection. BF16/FP8 and explicit layouts retain their existing owners.
OptiX callable payloads use the same selected storage on both sides; register payloads retain
logical matrix order. [Static context addressing](../../tests/cuda/nvvm-static-context-array-addressing.slang)
qualifies deep field/index chains and per-thread isolation on three routes. Static tests cover
layout/cache ownership, denied launch roles and readonly forwarding; existing SPIR-V address-space
specialization controls remain passing.

The NVRTC comparison passes the nested record fixture. Its tag increment uses ordinary access;
only the NVVM branch qualifies coherent access through a bare scalar entry pointer. Its numeric fixture reads Bool byte0x80
as -128 when converted to float, whereas NVVM preserves nonzero truthiness and returns1. NVRTC
then rejects a column-major Bool3x3 local pointer passed to its lowered helper; subsequent numeric
cells and O3 are unrun. These exact differences remain in the RHI manifest; CUDA output does not
replace the independent byte and logical-value oracle.

## Local resource helper transport

Existing buffer/texture/surface/sampler leaves, records and fixed arrays support value parameters
and results plus local mutable and readonly references. Checked child addresses preserve local
provenance and access. [Live buffers](../../tests/cuda/nvvm-resource-helper-transport.slang) pass
NVRTC O3 and NVVM O0/O3; [handle transport](../../tests/cuda/nvvm-resource-helper-handles.slang)
passes all 20 selected shapes at NVVM O0/O3 using synthetic bits that are never sampled.
The affected SlangPy selection supplies live 2D/3D texture evidence: 9 failures resolved and all
37 prior passing controls preserved. Six static checks cover role/cache order, descriptor layout,
readonly rejection before mutation and retained resource-format/storage restrictions.

This does not itself admit arbitrary pointer-bearing entries, external helper ABI or surface
formats or acceleration handles. Later application qualification below adds opaque acceleration
handle composition. The original combined generic handle failure remains recorded; per-shape success
is not qualification of their coexistence. Shader cache reload subsequently exposed
a shared RHI PTX-termination failure, now repaired by owning the exact byte span as terminated text.
The deterministic CUDA cache fixture and raygen/triangle controls pass on both routes; the SlangPy
cache node also passes. OptiX controls do not qualify persistent OptiX cache roundtrips. Original
failures and fixes remain in the maintained manifests.

## Application value and AD update qualification

The full CUDA-selected SlangPy inventory passes 1,591 with zero failures,807 skips and3 expected failures
across2,401 nodes plus14 module skips. All 29 previous failures resolve; all 1,561 prior passes remain passing.
The extra node completes RGB Float/Sint/Uint metadata coverage, using the real texture-type factory
without allocating CUDA RGB textures. This does not establish physical three-channel texture support.

Default/CUDA pointer layout qualification preserves identical pointees and checked CUDA/LLVM layout;
atomic pointers and nested helper records retain established access and provenance restrictions.
Fixed resource arrays use the checked entry aggregate path. Opaque acceleration handles now compose
inside records/arrays and parameter-group storage, without integer construction or a returned-handle
ABI. Default uniform storage uses canonical CUDA numeric packing and preserves its collected-record
marker through physical lowering. Finite uniform/local/structured values share the same producer
cache; readonly uniform borrows require identical pointee/storage representations and layout proof.
Pointer-bearing record borrows retain their rejection. BF16/FP8 and ByteAddress storage keep separate admission.

[Imported generic pointers](../../tests/cuda/nvvm-specialized-pointer-identity.slang) qualify distinct
canonical specialization identities at NVVM O0/O3 and NVRTC O3. The earlier combined generic handle
failure remains historical evidence; that separate combined fixture has not been rerun.
[Aggregate updates](../../tests/cuda/nvvm-aggregate-value-updates.slang) qualify reverse AD through
field/vector/array/matrix chains while retaining the original value. [Array snapshots](../../tests/cuda/nvvm-dynamic-array-snapshots.slang)
qualify high-bit uint8 dynamic reads, repeated mutation and padded nested-record values at O0/O3.
Boolean2/3/4 leaves, every dynamic record index, nested arrays and numeric sibling fields also pass
NVVM O0/O3 and NVRTC. Provider-private byte lanes preserve Boolean snapshots; the original RHI
aggregate-entry regression and differentiated-array application control pass.
Captured LLVM confirms i8-to-i64 zero extension; captured CUDA12.9 PTX qualifies a fixed local frame.
The vendor optimizer stall and rejected entry-hoisting/select-chain attempts remain recorded.

The [bitfield boundary fixture](../../tests/cuda/nvvm-bitfield-boundaries.slang) qualifies zero/full,
top-bit and interior extraction/insertion for signed/unsigned 8/16/32/64-bit scalars and vectors2–4
at NVVM O0/O3. Canonical bitfield operations now lower to ordinary IR; exact signature negatives and
module-expression placement are checked before provider mutation. Runtime behavior is preserved;
LLVM-poison intermediate shifts are avoided. Invalid ranges remain outside the contract.

## Explicit OptiX versions

`OptixVersion` / `-optix-version 80000|80100|90000` selects an SDK contract independent of CUDA
and GPU architecture. RHI supplies its actual context version. Existing target/link hashes include
the selection; explicit NVRTC selection checks the SDK headers. Default NVVM remains90000.
Optional provider interface12 configures an empty module; ABI46/module43/container2 are unchanged.

| SDK | NVVM common runtime                          | Selected NVRTC controls | HitObject status                                               |
| --- | -------------------------------------------- | ----------------------- | -------------------------------------------------------------- |
| 8.0 | 9 pass / 1 inherited skip, 1,401 assertions  | 2 pass,24 assertions    | Rejected; legacy contract pending                              |
| 8.1 | 9 pass / 1 inherited skip, 1,401 assertions  | 2 pass,24 assertions    | Rejected; legacy contract pending                              |
| 9.0 | 10 pass / 1 inherited skip, 2,527 assertions | 2 pass,24 assertions    | Retained supported lifecycle passes; arbitraryMakeHit excluded |

Common coverage is raygen/bindings, triangle tracing, recursive payload layout and callable families;
the latter payload/callable fixtures exerciseO0/O3. SDK 9 includes the retained lifecycle. The
entry-parameter write/cache fixture is a runtime skip on all three, not an executed pass.16 version/
routing cells,13 existing stdout rejection controls and all 16 smoke cells pass. This is focused
qualification on L4/driver595.71.05/CUDA12.9/SM80, not complete SDK or full-suite acceptance.
The new no-o source rejection fixture also repairs CLI target-option ownership: target creation
now copies options independently of output-file association. The13 existing stdout fixtures are
freshly requalified; their historical successes alone could have exercisedNVRTC.

Raw SDK probes show native 8.x Traverse/Invoke and explicit constructor/Invoke both expose zero
incoming RayFlags, while9 preserves flags. Ordinary Trace independently preserves flags on all three.
For flag 8, native 8 Invoke executes CH while9Invoke suppresses CH. Treat this as empirical SDK behavior,
not an inferred Vulkan/D3D standaloneInvoke rule. Modern flag visibility inside invoked CH/MS
cannot be recovered merely by storing flags alongside an older snapshot. A private context ABI
preserving 32 payload words or an explicit native 8semantic exception remains a maintainer decision.
The [D3D HitObject contract](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#hitobject-invoke)
and [SPIR-V EXT contract](https://github.khronos.org/SPIRV-Registry/extensions/EXT/SPV_EXT_shader_invocation_reorder.html)
remain comparison references; oldNV/OptiX8 API shape is not automatic modern semantic parity.

## HitObject lifecycle and boundaries

The OptiX9 implementation retains independent noncopyable objects in caller-owned storage,
including helper returns and references, and restores the selected object before queries,
invocation and reordering. The private snapshot is 392 bytes with 8-byte alignment and owns up to
31 transform handles. Arbitrary MakeHit/MakeMotionHit remains explicitly unsupported: all four
constructors diagnose, separately from restoration of a previously traced hit. The supported family passes 27 RHI registrations with 2,265 assertions, six static units, six provider/stage
units, 22 source control cells and 15 smoke cases. This resolves 26 original application failures;
the arbitrary MakeHit registration remains intentionally unsupported. The full checkpoint is unchanged.

The sibling `ray-tracing-hitobject-lifecycle.cuda` oracle passes at NVVM O0/O3 with 1,127 assertions.
It checks all 533 guarded words, independent hit/miss/nop objects, helper transport, two asymmetric
affine instances, full forward/inverse matrices and object rays, SBT selection and repeated
invocation with an evolving current payload. The unchanged NVRTC comparison aborts during OptiX
module creation because its incoming ObjectRayOrigin getter is illegal in raygeneration. CUDA's
single implicit outgoing object and ignored constructor arguments are not an oracle for independent
objects; scene-derived expected values remain authoritative.

The independent host for the [motion fixture](../../tests/pipeline/ray-tracing/nvvm-hitobject-motion.slang)
passes all 309 guarded words at NVVM O0/O3. It checks a nested instance/SRT/matrix graph at times
0, .5 and 1, all 24 forward/inverse coefficients, world/object rays, attributes and repeated invocation
with an evolving payload. The oracle uses exact finite values with signed-zero equivalence. Its
motion interpolates translation with fixed nonidentity quaternion and scales; this does not qualify
changing-quaternion interpolation, maximum graph depth or every motion geometry. SDK-only controls
retain separate identities and do not establish compiler correctness. The fixture's eight maintained
compile cells each select one entry at O0/O3; they do not run the host.

The sibling RHI adapter now selects the conservative `ALLOW_ANY` graph option. On the recorded
SDK9/driver combination, the previous single-level specialization yielded zero instance identity
and a linear-swept-sphere failure after replay; the isolated corrected instance-ID and LSS cases pass 70 assertions.
This is observed compatibility evidence, not a universal API prohibition. Performance is unmeasured,
and this option does not extend the RHI scene-depth qualification.

The retained compatibility gate compiles the two explicit SDK8.1 constructors through NVRTC, but
OptiX9 rejects both at module creation7204 (maximum supported ABI102, current105). A NOP control
passes module/program/pipeline creation; no GPU launch or constructor correctness is claimed.
SDK9 documents restoration from prior opaque traversal data, not arbitrary hit construction.
[The SDK9 construction contract](https://raytracing-docs.nvidia.com/optix9/guide/optix_guide.250130.A4.pdf)
and [DXR object semantics](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#hitobject)
explain the distinction. The maintainer approved the supported OptiX9 lifecycle with arbitrary
MakeHit explicitly rejected. FromRayQuery, HitObject use in callable stages and general pointer payloads remain
separate work. GeometryIndex exposes the SBT GAS index, which equals a geometry ordinal only under
the one-record-per-build-input convention without per-primitive SBT offsets.

## OptiX callable shaders

The typed callable path admits dynamic UInt32 indices and copy-in/out of recursive copyable
numeric/bool values, vectors, positive fixed arrays, records and legalized matrices. Raygen,
closest-hit, miss and callable callers share the same checked operation. Callable entries accept
one canonical mutable inout/out payload, or no parameter after canonical empty-type erasure.
Pointer/resource payloads and arbitrary function pointers remain excluded. This private NVVM ABI
uses the existing value representation on both sides; cross-backend callable interoperability is
unqualified.

The sibling `ray-tracing-callable-family.cuda` fixture observes all 27 aggregate leaves at O0/O3,
including float3/float4 padding, arrays, row/column and singleton matrices, Half2, Double, Boolean
and a nonzero high word in Int64. Dynamic nested calls run from raygen/closest-hit/miss; empty,
recursively empty and out payloads have independent effects. Its deferred pipeline changes the
application's original stack descriptor before binding and checks that owned settings survive.
The original callable test and the imported nested-call stack regression remain separate controls.
NVRTC rejects the singleton matrix shape; its failure does not invalidate the independent oracle.

The RHI stack setup reuses commit `81d5ded4` with the current motion-graph depth bound and owned
deferred settings. Default one-level direct calls and explicit depth-two calls are exercised;
arbitrary recursion depth, traversal-stage direct calls and cross-SDK runtime coverage are not
claimed. Static checks retain rejected stages/signatures, and the real provider checks rejection
without module mutation. Exact current acceptance is owned by the maintained focused/RHI manifests.

## OptiX ray generation and triangle tracing

The direct PTX route admits ray-generation entries, launch index/dimensions, conventional launch
parameters and SBT uniforms. [The permanent fixture](../../tests/cuda/nvvm-optix-raygen.slang) compiles
at NVVM O0/O3. The `nvvmOptixRaygenBindings` unit uses the installed OptiX runtime with validation
and an independent host ABI/output oracle: each mode reuses one pipeline/SBT allocation for two
asymmetric launches, changes launch parameters and SBT data, and checks all 170 output words,
including guards and untouched trailing storage. Reflection must agree with the fixed host layout.

The [triangle fixture](../../tests/pipeline/ray-tracing/nvvm-optix-triangle.slang) qualifies TraceRay
from raygen into miss/closest-hit at NVVM O0/O3, with UInt and Float4 payloads and NVRTC O3 controls.
Each of six cases checks four hits, four misses and all 34 output words, including untouched storage.
Both triangle attributes are consumed with unequal values (.125 and .25); Float4 lanes use independent
exact IEEE bit expectations. The existing render-test scene provides real acceleration structures.

Type admission includes Int32/UInt32/Float32 scalar/vector/matrix/record/fixed-array
payloads of 1..32 physical words, including padding. CUDA payload matrices store logical rows
independently of buffer annotations; singleton rows/columns use canonical scalar legalization. The sibling RHI flat and nested array tests add runtime qualification for
Float32[12] and nested records totaling exactly 32 words (Float2 arrays, signed and unsigned arrays).
Both execute caller/AnyHit/ClosestHit/miss at NVVM O0/O3: 293 assertions pass, with all 26/66 output
words and guards checked. A prior-compiler NVRTC control passes the same oracle. Stateful AnyHit
arithmetic uses explicit no-duplicate geometry and forced nonopaque traversal. The expanded static
unit checks 23 admission/preflight cases; all 15 smoke cells pass. Its initial test-only build error
used an abstract IR opcode instead of the existing poison builder; the corrected static build and
unit pass without changing production. UInt and Float4 retain their earlier runtime qualification. Opaque acceleration handles have
value, local-storage and helper-parameter roles, without integer semantics or recursive resource
aggregate admission. Provider tests cover 1/4/32 live words, exact SDK adaptation and rejection
without mutation. Preflight enforces stages and rejects missing optional trace support before
module creation. Static tests retain SBT layout/load and role/cache boundaries.

The sibling RHI layout and singleton-matrix fixtures execute padded record arrays and row/column
annotations through caller/AnyHit/ClosestHit/miss at O0/O3. Padded procedural attributes qualify
all eight dense words in a 48-byte record through ReportHit, saved GetAttributes and Invoke.
Current sphere/LSS data and predicates plus cluster ID execute in ClosestHit; AnyHit admission
has static coverage. Recursive conventional pointer records/arrays retain checked storage roles.
Unused RO/RW typed-buffer bindings preserve eight-byte slots and neighboring uniforms, including
null bindings; this is not typed-buffer access support.
NVRTC passes the padded-layout oracle but rejects singleton matrices and its sizeof-based saved
attribute helper rejects the 48-byte record. These are recorded comparison differences, with the
independent NVVM oracle unchanged. [Focused evidence](../../issue-nvvm-backend/focused-evidence.json)
and the [RHI manifest](../../issue-nvvm-backend/rhi-cuda-status.json) own exact outcomes.

The original raygen/triangle fixtures target SM80 with CUDA12.9/OptiX SDK9 on the recorded
L4/595.71.05 host; sibling RHI targets follow the CUDA device. Pointer payloads,
explicitly strided/unsized arrays, subword payloads, callables and recursive
callback tracing remain outside this trace contract. Typed procedural ReportIntersection now admits
zero through eight attributes, including variadic ReportHitOptix, with separate family validation. The shared payload-termination producer repair is separately
qualified by nine CUDA/NVRTC modes at O0/O3 (231 assertions), a focused IR boundary unit and
source regressions. A subsequent bounded AnyHit batch qualifies exact object-ray queries and
nullary Void ignore/accept primitives. Five selected NVVM RHI cases pass 418 assertions; object
origin/direction and the nine-mode payload oracle execute O0/O3. The affine oracle distinguishes
inverse-transformed, unnormalized directions from world-space values. Two world-ray controls add
64 passing assertions. A separate AnyHit state oracle qualifies the thirteen existing world-ray,
ray-range, flags and hit-identity queries plus both unequal triangle attributes at O0/O3. All 289
assertions pass through NVVM and a prior-compiler NVRTC control. Six isolated hits across two
instances and a miss check 114 words, both faces, flags 2/10, independent IDs, candidate distance 2
versus trace maximum 4, sentinels and guards. ClosestHit is NOP. The original direction Z 1.5 fixture
had twenty 1–4 ULP NVRTC barycentric mismatches; direction Z 2 selects a cleaner exact-input domain
without changing expected barycentrics or adding tolerance. Exact outcomes and both identities are
retained in focused evidence; this is not a general exact-intersection claim. The affected 62-case
rejection unit and 15 smoke cells pass. The preceding working checkpoint passes all 1,711 admitted
configurations on its recorded compiler; it was not repeated for this small stage extension.
Full RHI stays optional. World-ray and range queries remain qualified in miss/closest-hit as well.
The [material fixture](../../tests/pipeline/ray-tracing/nvvm-optix-material.slang) samples all four
texel centers of a 2x2 texture and evaluates the unchanged imported MaterialX dielectric BSDF.
NVVM O0/O3 and NVRTC O3 each check 202 words: 16 material lanes against the maintained finite-input
budget, all remaining state/texture/status/guard words exactly. A separate host oracle verifies the
same complete outputs. Non-unit miss directions are preserved, and hit/miss distances are distinct.
NVVM O0 differs from CUDA in 12 bounded material words; O3 matches all words for these selected inputs.
This qualifies one textured normal-incidence BSDF path, not the full generated material graph or its
packed texture-handle convention. PTX acceptance by libNVVM alone is not an OptiX execution claim.

PrimitiveIndex, InstanceIndex, InstanceID and HitKind use exact UInt32 SDK queries in
closest-hit/any-hit. RayFlags is qualified in miss/closest-hit/any-hit. The optional sibling RHI `ray-tracing-intrinsics-hit-identities.cuda` test checks two
instances, distinct custom IDs, primitive indices 0/1/2, one miss and all 44 words at NVVM O0/O3;
NVRTC controls use the same independent host oracle. Both triangle windings and incoming flags 0/1
are checked, including nonzero flags in miss. The original RHI triangle test also passes.

Pure global numeric expressions and dependent constructors now use the existing dependency-localization
pass at the final NVVM legalization boundary. The
[global-expression fixture](../../tests/cuda/nvvm-global-constant-expressions.slang) checks all 30 output
words at NVVM O0/O3; the static unit covers dependency ownership, scalar/reordered-vector projections
and direct/nested Call/Load rejection. Calls, loads, pointer/resource operations and unknown effects
remain excluded. The original global record-constructor Call variant remains unresolved; the qualified
fixture constructs records locally. Its NVRTC dynamic device-initializer failures remain unadmitted,
alongside the earlier NVVM constructor/projection failures and corrected CLI invocation history.

GetTransformListSize, GetTransformListHandle, GetTraversableTransformType,
GetTraversableInstanceId and GetTraversableChild now use typed scalar SDK calls. The dedicated
sibling RHI test passes 165 assertions at NVVM O0/O3, with a prior-compiler NVRTC control also
passing 165. Qualification covers two single-level static instances in AnyHit and ClosestHit:
53 guarded words include distinct custom IDs, a full 64-bit child comparison against the host BLAS,
a miss with preserved sentinels, and nonzero opaque instance handles consistent across stages.
Instance handle bits are not a fixed portable oracle. PTX proves the uniform list index remains
dynamic and handles remain 64-bit. Three affected units and all 15 smoke cells pass. Recorded
E41012 profile-upgrade warnings do not invalidate the outputs. This scalar-query qualification does
not establish multilevel traversal, motion or interpretation of an opaque handle as a memory address.

Instance-scoped forward and inverse matrices are qualified by an independent sibling RHI oracle:
two asymmetric affine instances, all 24 coefficients per hit, preserved miss sentinels and guards,
215 passing assertions at NVVM O0/O3 (2.280s). The prior-compiler NVRTC control passes 215 assertions.
Coefficient checks use exact finite Float32 equality with signed-zero equivalence, without an epsilon.
Fresh runtime qualification is **ClosestHit only**. Compiler static checks cover 15 grouped cases:
4 valid-stage groups,3 forbidden-stage groups and8 type/immediate/arity rejection groups. The valid
RG/Miss/ClosestHit/AnyHit groups inspect 24 checked row plans, then prove old-provider rejection
before mutation. Provider contracts cover all 6 row/direction variants, invalid descriptors/operands/
insertion, malformed optional tables and paired no-mutation serialization in both LLVM dialects.
These checks do not claim fresh runtime execution in RG/Miss/AnyHit. All15 smoke cells pass.

The instance-scoped APIs require valid instance handles, matching their existing CUDA contract.
They do not grant arbitrary SDK-pointer dereference or treat a non-instance handle as an instance.

The complete current-ray ObjectToWorld/WorldToObject family is qualified separately: all four RHI
3x4/4x3 cases pass 162 assertions. The independent nested instance/SRT/matrix motion host checks all
309 guarded words at NVVM O0/O3, including current matrices in AnyHit and ClosestHit at times 0,
0.5 and 1, and repeated saved-hit invocation with evolving payloads. It reuses the existing exact
finite matrix expectations; signed zeros compare equal, without an epsilon. The
[current-transform fixture](../../tests/pipeline/ray-tracing/nvvm-current-transform-motion.slang)
keeps the source contract reproducible. The four RHI cases retain authored/default optimization;
the motion fixture supplies explicit O0/O3 coverage. Earlier NVRTC controls retain their identity.

Typed row planning admits AnyHit/ClosestHit/Intersection and checks literal row/direction plus
Float4 result. Interface 10 preserves older provider tables and rejects absent support before
module creation. The provider shares ordered instance/static/matrix/SRT evaluation with owned
HitObjects while keeping incoming and saved lists distinct. Runtime qualification here is AH/CH,
with fixed nonidentity quaternion/scales and interpolated translation; changing-quaternion motion,
maximum depth and Intersection execution remain unqualified. Each returned row currently composes
the full list independently; no speed claim is made.
Compute/raygen calls to ray state and compute/raygen/miss calls to hit-only helpers reject before
provider module creation.
See [the optional application workflow](../../issue-nvvm-backend/RESULTS.md#optional-slang-rhi-cuda-suite)
for commands and compiler-selector scope. The separate full application checkpoint covers all 269
CUDA registrations in the tested binary: NVVM 198 pass / 61 compiler rejections / 10 runtime skips;
NVRTC 259 pass / 0 failures / the same 10 skips. All 61 rejected NVVM cases pass their NVRTC controls.
The [current application manifest](../../issue-nvvm-backend/rhi-cuda-status.json) retains exact names,
diagnostics, route exceptions and complete-family work candidates. This is a measured application
baseline with open gaps, not full NVVM suite acceptance or an admission to the Slang working corpus.

## Compute, values and memory

| Region                          | Qualified domain / evidence                                                                                                                                                                                                          | Boundaries and durable anchors                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| ------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Direct PTX routing              | Opt-in direct route, target-scoped selector, explicit architecture; `case nvvm` refines CUDA for direct PTX                                                                                                                          | NVRTC remains default. [Compiler units](../../tools/slang-unit-test/unit-test-nvvm-compiler.cpp)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| Compute launch and builtins     | Conventional and raw compute entry contracts; explicit LLVM register reads for thread/block/grid dimensions; group barrier; GPU                                                                                                      | OptiX raygen has a separate qualification below; no graphics-stage claim. [Core execution](../../tests/cuda/nvvm-core-execution.slang), [ordinary entry](../../tests/cuda/nvvm-ordinary-compute-entry.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| Scalar numeric values           | Selected signed/unsigned 8/16/32/64-bit integers, Bool and IEEE Half/Float/Double; typed transport/conversion and admitted operations                                                                                                | Exact operation descriptors govern overloads; type admission alone grants no operation. [Mixed numeric](../../tests/cuda/nvvm-mixed-numeric.slang), [Half](../../tests/cuda/nvvm-half-values.slang), [Double](../../tests/cuda/nvvm-float64-values.slang)                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Vectors                         | Selected widths 2–4; construction, extraction, swizzle updates, typed selection and operation families                                                                                                                               | Boolean lane references need local SSA normalization; arbitrary pointer escape remains rejected. [Typed select](../../tests/cuda/nvvm-typed-select.slang), [Boolean lanes](../../tests/cuda/nvvm-local-boolean-lanes.slang), [negative](../../tests/cuda/nvvm-boolean-lane-reference-unsupported.slang)                                                                                                                                                                                                                                                                                                                                                                            |
| Matrices                        | Selected logical numeric matrices lowered to aggregate values and physical storage; GPU and layout checks                                                                                                                            | Physical row/column layout and host packing remain explicit. [Float matrices](../../tests/cuda/nvvm-float-matrix-values.slang), column-major qualification below                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| Helpers and control flow        | Typed calls/results, phi values, loops/switches, finite copyable records/arrays, noinline functions, selected mutable references                                                                                                     | Internal by-value ABI and exported CUDA ABI are separate. [Helper values](../../tests/cuda/nvvm-helper-copyable-values.slang), [mutable forwarding](../../tests/cuda/nvvm-mutable-parameter-forwarding.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| Pointer-bearing helper values   | Canonical device/UserPointer and qualified recursive helper transport                                                                                                                                                                | Access/address-space/layout operands and producer provenance are checked. [Pointer forwarding](../../tests/cuda/nvvm-mutable-pointer-payload-forwarding.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| Explicit-layout pointer memory  | Std430/Scalar/C Device record roots through entry/internal-helper parameters and checked conventional constant-buffer fields; signed32 offsets, UInt64 observations, selected nested field loads/stores; allocated O0/O3 byte checks | Shared strides64/48/40; Bool1/4/4-byte and exact vector payload storage. Selected Bool, Int/UInt32/64, Float32 and 2–4-lane Int/UInt32/Float32 fields, constant vector components. No whole-record/array/matrix memory, dynamic components, field-pointer helper escape, scoped accesses, exports, pointer results, general storage or reconstruction. CUDA layout differences remain recorded. [Field memory](../../tests/cuda/nvvm-layout-pointer-fields.slang), [Reinterpret](../../tests/cuda/nvvm-pointer-reinterpret.slang), [Parameter groups](../../tests/cuda/nvvm-parameter-group-layout-pointers.slang), [Helpers](../../tests/cuda/nvvm-layout-pointer-helpers.slang). |
| Parameter groups and resources  | Selected conventional globals (including direct Int/UInt/Float32 vectors2–4), uniforms, constant/parameter blocks, structured and byte-address buffers, resource-bearing aggregates                                                  | Separate launch, parameter-group and structured-buffer representation; no universal aggregate ABI. [Multiple resources](../../tests/cuda/nvvm-conventional-global-multi-resource.slang), [global vectors](../../tests/cuda/nvvm-conventional-global-vectors.slang) (exact lanes, compact width3 and neighboring fields), [compact storage](../../tests/cuda/nvvm-compact-vector-storage.slang)                                                                                                                                                                                                                                                                                     |
| Borrowed float3 storage         | Readonly borrowed fields/array elements preserve native storage across mutable use; GPU and provider-memory regression                                                                                                               | Readonly access must not imply compact parameter-group storage. [Borrowed vector storage](../../tests/cuda/nvvm-borrowed-vector-storage.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| Shared/local memory and atomics | Selected finite shared storage, typed integer atomic families and admitted memory orders; unit/integration/corpus gates                                                                                                              | Every element type, pointer role and atomic overload still needs admission. [Emitter units](../../tools/slang-unit-test/unit-test-nvvm-emitter.cpp), [integration units](../../tools/slang-unit-test/unit-test-nvvm-integration.cpp)                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| Thread-local context            | Selected per-invocation global/context values and explicitly initialized finite copyable aggregates pass repeated dispatch                                                                                                           | The fixture now uses `static State state = {};` to express its zero expectation; three launches per mode pass. Bare uninitialized storage has no zero guarantee, and its earlier failure remains recorded. Does not imply arbitrary global initialization or host ABI. [Context](../../tests/cuda/nvvm-thread-local-global-context.slang), [copyable context](../../tests/cuda/nvvm-copyable-kernel-context.slang)                                                                                                                                                                                                                                                                 |
| Receiver snapshots              | Canonical aggregate parameter snapshots preserve values across resource mutation; GPU and material compile evidence                                                                                                                  | Aggregate-memory ordering issue is fixed; material runtime is separately scoped below. [Snapshot](../../tests/cuda/nvvm-aggregate-param-snapshot.slang), [resource snapshot](../../tests/cuda/nvvm-aggregate-param-resource-snapshot.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| Nested integer aggregate stores | Nested records and root/wrapped/multidimensional arrays; NVVM O0/O3 GPU and 39 provider shape/alignment checks                                                                                                                       | Qualified libNVVM store workaround; focused NVRTC optimized copies still fail. [Root](../../tests/cuda/nvvm-nested-array-root.slang), [wrapper](../../tests/cuda/nvvm-nested-array-wrapped.slang), [multidimensional](../../tests/cuda/nvvm-nested-array-multidimensional.slang)                                                                                                                                                                                                                                                                                                                                                                                                   |

Scoped coherent pointer memory admits naturally aligned Int/UInt32/64 for exactly Device/global
and Workgroup/shared. The canonical memory attributes select scoped relaxed GPU/CTA operations;
coherent reads do not become read-modify-write operations or invariant loads. Existing permissions
and proven pointer roots remain required. The [guarded fixture](../../tests/cuda/nvvm-coherent-pointer-memory.slang)
uses disjoint writes, a group barrier, mirrored reads, ordinary-memory controls and both-end guards.
The [negative fixture](../../tests/cuda/nvvm-coherent-pointer-memory-unsupported.slang) keeps CUDA C++,
SM60, mismatched scopes/spaces, float/vector values, weak alignment and integer-derived roots closed.
The NVVM arm requires SM70+, with focused qualification at SM80; the Vulkan capability arm is
unchanged. Other scopes, new pointer roles and volatile semantics are not admitted. The original
physical-pointer and redundant-load corpus programs contain races and are not runtime oracles.
See [the scoped validation procedure](../../issue-nvvm-backend/RESULTS.md#scoped-coherent-pointer-memory).

Core execution helpers and varying-parameter legalization use named
`llvm.nvvm.read.ptx.sreg.*` intrinsics for all twelve thread/block/grid coordinates.
The named-intrinsic boundary admits zero-argument scalar i32 register reads, the three void
barrier/fence operations described below, and scalar integer `llvm.ctpop`, `llvm.bitreverse`,
`llvm.ctlz` and `llvm.cttz`, scalar Float32/Float64 `llvm.sqrt`, and zero-operand
`llvm.nvvm.read.ptx.sreg.clock`/`clock64` signatures. It does not admit arbitrary LLVM
assembly. ABI 46 carries explicit ordinary `__intrinsic_asm` operands; pure provider queries validate LLVM registry signatures and
immediate-constant requirements before module creation. Actual emission separately validates value
handles and rejects conflicting symbols without mutation. CUDA source and NVRTC retain CUDA target
selection, including when an explicit `nvvm` capability is supplied.
[Target/argument fixture](../../tests/cuda/nvvm-target-switch-explicit-args.slang) and
[varying composition](../../tests/cuda/nvvm-execution-register-varyings.slang) retain their contracts.

Core integer count, reverse, high-bit and low-bit APIs use named scalar primitives plus ordinary
Slang width/sign/zero handling. Live independent bit-loop oracles cover signed/unsigned scalar operations and
vectors of lengths 2/3/4 at widths [8](../../tests/cuda/nvvm-integer-intrinsics-8.slang),
[16](../../tests/cuda/nvvm-integer-intrinsics-16.slang),
[32](../../tests/cuda/nvvm-integer-intrinsics-32.slang) and
[64](../../tests/cuda/nvvm-integer-intrinsics-64.slang), with literals, completion and guard checks.
All widths pass NVVM O0/O3; widths 32/64 also pass NVRTC O3. NVRTC 8/16 scan/reverse prelude helpers
remain missing, and their original compile failures remain recorded. The separate
[small-count fixture](../../tests/cuda/nvvm-integer-count-small.slang) passes all three modes.
Public signed 8-bit population count preserves uint32 sign extension (32 for -1, 25 for -128); direct
i8 population count independently checks the same-width results (8 and 1). The
[producer check](../../tests/cuda/nvvm-integer-producers.slang) requires four named intrinsics and
explicit false scan flags. Scalar registry admission does not imply vector LLVM intrinsic support.

The [target-switch helper test](../../tests/language-feature/capability/target-switch-dead-helper.slang)
checks that discarded NVVM helper branches do not diagnose during CUDA linking, while a live
unavailable switch still rejects. [Layout controls](../../tests/language-feature/capability/target-switch-layout-lifetime.slang)
cover uniform and resource-only CUDA/HLSL/SPIR-V programs; later HLSL/SPIR-V pruning still removes
unused resources. Semantic module version 43 rejects older modules before AST/IR decoding; metadata inspection
and source fallback remain available. Retired integer IDs 46/47 are not accepted through a compatibility
shim. Numeric count45/low48 are also retired after their compound wave consumers moved to core.

Public `round` uses named `__nv_roundf`/`__nv_round` calls validated against actual definitions in
an immutable snapshot of the selected libdevice. The same bytes reach libNVVM. Half uses canonical
FloatCast widening/narrowing around Float32 round, preserving NVVM ties-away and CUDA Half ties-even.
Numeric operation64 and the old tag/text recognizers are retired. Scalar library ABI admission
does not admit vector calls; the public vector/matrix bodies map to scalar operations.
The [Float32](../../tests/cuda/nvvm-round-32.slang), [Float64](../../tests/cuda/nvvm-round-64.slang)
and [Half](../../tests/cuda/nvvm-round-half.slang) contracts cover live scalar/noinline helper,
vector2/3/4 and matrix2x2 results with independent integer/rational oracles, signed-zero checks,
NaN classification, guards and completion. Half compares both tie policies unconditionally.
The [oracle checker](../../extras/test-generators/check-nvvm-round-oracles.py) recomputes frozen
expectations without a compiler; the [removed-tag control](../../tests/cuda/nvvm-round-removed-tag.slang)
checks the source boundary. Current acceptance and compiler identity remain owned by STATUS.

Public `ceil`, `floor` and `trunc` use direct scalar Half `llvm.ceil`/`llvm.floor`/`llvm.trunc`
with signatures checked by the LLVM registry. Checked Half trunc lowers to the exact pure PTX
`cvt.rzi.f16.f16` primitive with bit-preserving i16 assembly transport because libNVVM 12.9
rejects its LLVM declaration and Half assembly operand types; ceil/floor keep their
named declarations. Their six Float32/64 names still come from selected
libdevice definitions; IDs 42/54/57 and their tag/text paths are
retired. The combined
[Float32](../../tests/cuda/nvvm-directed-rounding-32.slang),
[Float64](../../tests/cuda/nvvm-directed-rounding-64.slang) and
[Half](../../tests/cuda/nvvm-directed-rounding-half.slang) fixtures check 64 live IEEE inputs each,
with signed zero, subnormal and integer/precision neighbors, infinity and NaN class. Each input
has 45 scalar/helper/vector/matrix observations, distinct operation/shape masks, guards and completion.
The [integer-bit oracle checker](../../extras/test-generators/check-nvvm-directed-rounding-oracles.py)
validates the frozen rationally generated constants without executing a compiler.

Public `frac` uses an explicit NVVM `x - floor(x)` core body; `fract` forwards to it. Named selected
floor calls, ordinary subtraction and canonical casts replace numeric 59, its tag and text path.
Half evaluates the whole expression in Float32 before narrowing once. Every finite Half residual
is exactly Float32, proving finite RN-even equivalence with CUDA's direct Half expression. Tiny
negative inputs can round to one; finite integers and either signed zero return positive zero.
The [Half](../../tests/cuda/nvvm-frac-half.slang), [Float32](../../tests/cuda/nvvm-frac-32.slang) and
[Float64](../../tests/cuda/nvvm-frac-64.slang) fixtures have 64 live IEEE inputs per width and 26
observations per lane: scalar/noinline/vector2/3/4/matrix2x2 `frac`, plus scalar/noinline/vector2/3/4
`fract`. There is no matrix `fract` overload. Eleven shape-error bits, both scalar raw results,
per-lane completion and endpoint guards occupy 386 words per fixture. Finite results compare exact
bits; infinity and NaN inputs require NaN classification. All three modes use one mathematical
oracle, with no target-policy marker. The [standalone integer checker](../../extras/test-generators/check-nvvm-frac-oracles.py)
recomputes the independent rational expectations and validates the fixture/output contract without
compiler execution or ignored manifests. The [removed-tag test](../../tests/cuda/nvvm-frac-removed-tag.slang)
checks the frontend boundary. Real-provider tests retain the named-floor/ordered-subtract contract
in both serializers and reject reserved 59 without mutation; emitter tests verify the public
composition and Half cast ordering. Current acceptance and exact compiler identity are owned by STATUS.

Public `rsqrt` uses selected named Float32/64 library functions and canonical Half widening,
Float32 evaluation and one narrowing. Numeric 65, its tag and text route are retired. Real-provider
tests derive signatures from selected definitions, including deliberate name/width crossings,
preserve typed calls in both serializers and reject invalid insertion without mutation. Emitter
tests verify direct Half widen/call/narrow edges and live scalar/vector/matrix outputs.
The [Half](../../tests/cuda/nvvm-rsqrt-half.slang), [Float32](../../tests/cuda/nvvm-rsqrt-32.slang)
and [Float64](../../tests/cuda/nvvm-rsqrt-64.slang) fixtures contain 64 unique IEEE inputs and 15
observations per lane. Exact rational references define a test union of reference-spacing radius
and encoding-step neighborhoods (Float32 N = 2, Float64 N = 1), with empirical, non-guaranteed library
scope. Separate NVVM and CUDA Half policies use the exact narrowed images of the library union
and the PTX epsilon 2^-22.9 set, respectively. These policies agree on the selected Half inputs;
a shared selector drives both the admission table and visible policy marker.
Six shape-error bits, raw scalar low/high pairs, completion and guards occupy 259 Half or 258
other words. The [standalone checker](../../extras/test-generators/check-nvvm-rsqrt-oracles.py)
recomputes tables without a compiler or ignored manifest and exercises six negative certificate
controls. The [removed-tag test](../../tests/cuda/nvvm-rsqrt-removed-tag.slang) checks the frontend
boundary. Preserve all nine pre-migration buffers byte-for-byte, including raw NaNs; payloads remain
unpromised by numerical admission. Current acceptance and exact compiler identity are owned by STATUS.

Public `exp` selects named `__nv_expf`/`__nv_exp` through the same selected-definition boundary.
Half preserves Float32 library evaluation before one narrowing. Numeric 55, its tag and text route
are retired; real-provider tests retain both serializers and reject reserved operation 55 without mutation.
Emitter tests preserve preflight-before-output and direct Half widen/call/narrow edges.
The [Half](../../tests/cuda/nvvm-exp-half.slang), [Float32](../../tests/cuda/nvvm-exp-32.slang)
and [Float64](../../tests/cuda/nvvm-exp-64.slang) fixtures contain 80 / 62 / 62 unique inputs and 15
scalar/noinline/vector2/3/4/matrix2x2 observations per lane. Six shape-error bits, raw scalar pairs,
completion and guards occupy 323 / 250 / 250 words. Half markers `55040` (NVVM) and `1209` (CUDA) identify
policies, not the current module version; the same target selector chooses marker and admission.

The default library union uses reference-spacing radius or encoding-step distance, N = 2 for Float32
and N = 1 for Float64, as an empirical non-guaranteed test convention. Zero uses minsubnormal spacing;
maxfinite uses the conceptual next binade; infinity uses only ordered encoding neighbors. Exact
special-input classifications override the union. NVVM Half is its exact Float32-to-Half image;
CUDA Half retains encoded-FMA, ex2 input/output FTZ, Half narrowing and four correction FMAs.
All four correction inputs distinguish candidate sets; baseline outputs at `0x1f79` / `0x25cf` differ
intentionally. These bounded policies imply neither universal accuracy nor target equivalence.

The [standalone checker](../../extras/test-generators/check-nvvm-exp-oracles.py) independently
reconstructs the corpus, references, sets, stage images and fixture contracts using integers,
without compiler execution or ignored manifests. Its 38 synthetic controls cover midpoint parity,
endpoint rules, FTZ, constants and corrections. An optional ignored proposal audit checks generator
proof fields and rejects 16 mutations. The [removed-tag test](../../tests/cuda/nvvm-exp-removed-tag.slang)
checks the frontend boundary. Preserve all nine raw baseline buffers, including NaNs. Current
acceptance and exact compiler identity remain owned by STATUS.

Public `exp2` selects named `__nv_exp2f`/`__nv_exp2`; Half preserves the selected Float32 call
before narrowing once. Numeric 56, its tag and legacy text are retired. Direct real-provider
rejection/no-mutation and emitter preflight tests remain independent of the module-version gate.
The [Half](../../tests/cuda/nvvm-exp2-half.slang), [Float32](../../tests/cuda/nvvm-exp2-32.slang)
and [Float64](../../tests/cuda/nvvm-exp2-64.slang) fixtures contain 74 / 69 / 69 unique inputs,
15 live observations per lane and 299 / 278 / 278 output words. They preserve full scalar bits,
completion and guards; Half markers `56042` and `1209` identify NVVM and CUDA policies.

Library admission follows the same test-defined endpoint convention with independently certified
base-two references. CUDA Half uses ex2 input/output FTZ, Float32 FMA with multiplier `2^-24` and
then RN16; NVVM Half narrows its Float32 library result once. Candidate sets agree on this corpus,
while synthetic midpoint candidates distinguish the policies. The existing exponential checker
with `--operation exp2 --check --self-test` reconstructs 212 references and runs 57 controls:
38 retained shared checks and 19 exp2 checks. Optional preparation-proposal auditing rejects 14
mutations. Default invocation still checks exp unchanged. The [removed-tag test](../../tests/cuda/nvvm-exp2-removed-tag.slang)
checks fresh tagged source; current acceptance and exact tested identity remain owned by STATUS.

Public `log`, `log2` and `log10` use six selected Float32/Float64 library names and canonical Half
widening/evaluation/narrowing. Operations 60/61/62 and their tag/text paths are retired. The
[log](../../tests/cuda/nvvm-log-half.slang), [log2](../../tests/cuda/nvvm-log2-half.slang) and
[log10](../../tests/cuda/nvvm-log10-half.slang) families each have Half/Float32/Float64 fixtures:
91/62/62 inputs for log and log2, and 91/62/80 for log10. Each input has 15 scalar/helper/vector/matrix
observations, guarded full scalar bits and completion; all 27 mode buffers must preserve their
respective baseline bytes. Current acceptance remains owned by STATUS.

The [independent checker](../../extras/test-generators/check-nvvm-log-oracles.py) certifies signed
references using directed integer logarithm intervals. Ordinary library admission uses the
empirical test-defined spacing/encoding union reflected by sign, with Float32 radii 1/1/2 and
Float64 radius 1. NVVM Half narrows the Float32 set; CUDA Half checks ideal RN-even results on the
finite corpus. Bounded source/correction controls distinguish the installed Half paths without
claiming comprehensive PTX approximation qualification. Exact specials preserve signed input-zero
handling, positive output zero at one, infinities and NaN class; NaN payloads remain unpromised.

CUDA double-log10 checks RN32 input conversion, Float32 log10 admission and exact widening because
the existing `F64_log10(float)` wrapper narrows. This qualified preservation result is not evidence
of true-double accuracy. The corpus excludes negative tiny doubles that narrow to negative zero;
its common NaN classification does not establish that untested case. Direct retired-ID rejection
and legacy-text preflight tests remain independent of old-module version rejection.

Public `sin`, `cos`, `acos`, `asin`, `atan`, `atan2`, `pow`, `tan`, `sinh`, `cosh`, `tanh`, `fma`
and `fmod` use 26 selected Float32/Float64 library names. Shared table-driven units cover their
real-provider signatures, explicit argument order and scalar/vector public paths. Half `fma` uses
`llvm.fma` with one nearest-even rounding of the exact product and sum; the remaining operations
retain Half widen/call/narrow composition. The [Half FMA fixture](../../tests/cuda/nvvm-half-fma.slang)
checks independent exact finite bits, signed zero/Inf, NaN class, helper and vector paths, including
precise mode. This deliberately corrects the old Float32-intermediate double rounding.
[RESULTS](../../issue-nvvm-backend/RESULTS.md#native-half-fused-multiply-add-contract) records its
contract and graphics-backend limitations. Approximate Half exp2/tanh and round tie-policy changes
remain separate work. Twelve retired numeric IDs reject without module mutation; all thirteen legacy tag/text
routes and legacy `sincos` text reject before output. FMOD 58 remains for canonical `kIROp_FRem`.
Core `sincos` assigns sine then cosine, floating `mad` calls `fma`, and integer `mad` uses multiply/add.
The small [composition smoke](../../tests/cuda/nvvm-core-math-composition.slang) checks exact
runtime-loaded integer `mad` results across signed/unsigned widths, scalar Half/Float32/Float64
`sincos` at zero, aliased output order and floating `mad`, under NVVM O0/O3. Integer `mad` and scalar
Half `sincos` are new support. These focused checks establish routing and composition, without
claiming a new per-operation numerical qualification. Current acceptance remains owned by STATUS.

Public Half bit transport, packed `f16tof32`/`f32tof16`, double word conversion and
`isfinite`/`isinf`/`isnan` use core expressions. Numeric NaN operation 67 is reserved; seven old
transport spellings and nine width-specific classification shapes reject before provider mutation.
The small [core bits smoke](../../tests/cuda/nvvm-core-bits.slang) checks eight runtime-loaded rows
through CUDA and NVVM O0/O3: signed/unsigned Half bit transport including NaNs, ignored upper packed
bits, zero-extension after packing, asymmetric double word order, aliased out stores, and all three
floating widths' zero/subnormal/finite/infinity/NaN classifications. NaN numerical conversions check
classification only; raw bit transport checks exact bits. Existing Half value/narrowing and vector
classification fixtures cover the shared operations. Current focused acceptance remains in STATUS.

Public abs/min/max/sign now use core bit operations, comparisons and selected fabs/fmin/fmax
library calls. Half abs preserves raw NaN payloads, signed abs emits wrapping INT_MIN semantics, and sign
maps NaNs and both zeros to zero. Unsigned abs and signed integer sign are new support. The compact
[core values fixture](../../tests/cuda/nvvm-core-values.slang) checks runtime-loaded integer edges,
Half quiet/signaling NaN abs, zero and NaN sign, shared integer reductions and shared/global 64-bit
inc/dec wrap under NVVM O0/O3. Both cells now pass with the original wrapping expectations. The
previous O3 signed16 INT_MIN failure is resolved by provider normalization at semantic integer
consumers: comparison, division/remainder, right shift, widening and integer-to-float conversion.
An explicit signed/unsigned widen/narrow pair prevents PTX's excess high bits from changing those
observations. Shift counts normalize around width conversion; scalar and vector lanes retain their
original LLVM types. Half/BF16 transport is unchanged. The
[narrow integer fixture](../../tests/cuda/nvvm-narrow-integer-semantics.slang) passes O0/O3 and covers
overflow/underflow, signedness reinterpretation, helper/vector paths, mixed-width shifts, named bit
intrinsics and a wrapped-zero vector index. Historical failures and primitive experiments remain in
accepted evidence; this does not claim arbitrary raw pointer arithmetic qualification. Named provider
units cover all six library signatures; raw IDs 49/68 and 80/81 and retired source routes reject.
Numeric MIN/MAX is also retired after canonical wave consumers moved to core.

Both public atomic reduction APIs now produce canonical Atomic IR. Their existing Relaxed-only
memory-order and pointer admission is unchanged; HLSL inc/dec explicitly forwards order and rejects
floating inputs. Shared integer reductions and 64-bit inc/dec are newly exposed through that same
contract. The existing integer/float/Half reduction fixtures and Float32 ByteAddress add/UInt64 CAS
fixtures cover existing behavior. ByteAddress typed views require naturally aligned offsets.
Named clock signatures retain side-effecting PTX and the existing
[clock observations fixture](../../tests/cuda/nvvm-clock-observations.slang); they do not become
readnone LLVM calls. Current focused results and compiler identity remain owned by STATUS.

Public `frexp` and `modf` now call the four selected libdevice definitions with checked local
output pointers; Half promotes to Float32 and narrows floating outputs in core, retaining frexp's
integer exponent. Scalar Float32/64 modf
is newly exposed by this producer migration. Provider units cover all four exact signatures,
wrong pointee/role/address-space/attributes and rejection without mutation; compiler units keep
both result channels live, including Float64 modf. Existing
[Float32 frexp](../../tests/bugs/frexp.slang), [Float64 frexp](../../tests/bugs/frexp-double.slang)
and [Half scalar math](../../tests/hlsl-intrinsic/scalar-half.slang) own numerical runtime evidence.
No fresh Float64 modf numerical claim follows from provider/compiler checks. IDs 69/70/77/78 and
the pointer-output CUDA text recipes are retired; the LLVM registry still rejects output pointers.

Size/alignment use canonical typed queries with explicit CUDA layout. Unit checks preserve general
zero size and the CUDA unsized-array wrapper's size 16/alignment 8, while NVVM size helpers reject
zero and signed Int32 overflow. Field offset uses a typed source query with its exact field key
captured before optimization. NVVM retains the direct same-base restriction and CUDA layout;
CPP/CUDA restore the original call and bodies. Focused units cover equal-valued distinct fields,
generic record specialization, dead queries, wrong-base rejection and the removed text spelling.
The existing [array-layout](../../tests/cuda/cuda-array-layout.slang) and
[parameter-block alignment](../../tests/cuda/param-block-alignment.slang) fixtures cover both
NVVM and CUDA source paths. No active NVVM CUDA-text recognizer remains.

Public `sqrt` uses the named LLVM intrinsic with canonical Half promotion to Float32 and narrowing.
Numeric 36, its tag and text path are retired. Direct named Half/vector sqrt is outside provider
admission; public vector/matrix operations map to scalar calls. Pure queries reject floating
INTEGER_CONSTANT metadata, and typed emission preserves registry attributes and ownership checks.
Sqrt-only programs make no library query or load. The [Half](../../tests/cuda/nvvm-sqrt-half.slang),
[Float32](../../tests/cuda/nvvm-sqrt-32.slang) and [Float64](../../tests/cuda/nvvm-sqrt-64.slang)
fixtures cover 64 live IEEE inputs, 15 scalar/helper/vector/matrix observations per lane, signed
zeros, subnormal/normal boundaries, infinity, NaN classification, guards and completion. CUDA Half
uses an independently bounded approximate-root oracle; NVVM uses exact Float32 evaluation followed
by Half rounding. Distinct visible mode markers prevent accepting the wrong policy. All nine
observed output buffers retain their pre-migration bytes; NaN payloads remain unpromised.
The [standalone integer oracle](../../extras/test-generators/check-nvvm-sqrt-oracles.py) requires no
compiler or ignored manifest. The [removed-tag test](../../tests/cuda/nvvm-sqrt-removed-tag.slang)
checks the frontend diagnostic; preserved old modules separately qualify numeric-tag retirement.

Core bit reinterpretation and Half-value conversions use canonical NVVM `BitCast`/`FloatCast`
instructions. The [conversion API fixture](../../tests/cuda/nvvm-conversion-intrinsics.slang) checks
all six signed/unsigned/float reinterpretation directions against exact input bits, scalar and
heterogeneous vector2/3/4 Half conversions against independent encodings, literal rounding boundaries
and an untouched guard. Numeric NaNs require NaN class; reinterpretation preserves payload bits.
The [producer IR test](../../tests/cuda/nvvm-conversion-producers.slang) requires typed casts for
all six reinterpretation signatures and scalar/vector Half conversion paths. CUDA vector conversion
uses canonical casts and the existing emitter's per-lane conversion; CUDA's scalar intrinsics cannot
accept whole vectors. Packed uint Half conversions and Double word assembly remain separate contracts.

Ordinary Var/Load/Store decisions are now retained in the checked emission plan: allocation role,
alignment, conversion recipe, load flags and pointer ABI/provenance. BF2 identity and BF3/BF4 lane
conversion are planned before provider mutation. Native readonly borrows remain distinct from compact
storage and from immutable locations. This changes ownership, not the supported language surface.
Structured-buffer roots and direct loads also retain their checked view/access/operand facts.
Recursive Boolean/aggregate/vector3 conversions use recipes built during preflight, including
explicit-stride construction; children inherit checked parent address facts in dominance order.
The [recursive round trip](../../tests/cuda/nvvm-structured-bool-vector-roundtrip.slang) covers nested
Boolean fields and float3 storage in both directions, noncanonical Boolean bytes, nested partial
writes, whole-array copies and mutable helper-reference aliases. Default structured buffers now use
the shared CUDA storage legalizer, including Bool-byte encoding, compact width3 vectors, Half3
padding and row/column-major matrices. Child plans retain readonly access independently of value
representation. Current shared lowering treats StructuredBuffer reads as immutable; post-write alias
observations use two RWStructuredBuffer views. The exploratory RO/RW alias attempt and its stale
reads remain a recorded semantic limitation, not an accepted mutable-read contract.

Uniform groups now use that same shared CUDA numeric lowering even with explicit Scalar/CData
layout operands, matching CUDA reflection. Both former compact-vector load conversion kinds and
their emitter reconstruction are removed. Existing compact type/stride proofs and Half storage
cache roles remain; unnormalized compact loads on checked immutable group paths reject before
provider mutation, as does Std430. This also admits previously rejected explicit Bool3, Int16x3,
UInt64x3 and Double3 groups through the existing policy. The
[explicit-group byte fixture](../../tests/cuda/nvvm-explicit-group-compact-storage.slang) checks a
144-byte mixed record at default/Scalar/CData O0/O3, including integer/Half lanes, Half3 NaN padding,
noncanonical Boolean true, wider scalar alignment and adjacent sentinels. The
[compact float3 fixture](../../tests/cuda/nvvm-compact-vector-storage.slang) also covers nested
records and dynamic array reads with explicit Scalar/CData layouts. Explicit pointer/resource
layout contracts are unchanged.

Half-vector helper parameters/results use physical integer lane transport while body arithmetic and
storage retain their selected Half representations. [Parameter transport](../../tests/cuda/nvvm-half-vector-helper-parameters.slang)
and [result transport](../../tests/cuda/nvvm-half-vector-helper-results.slang) separately cover all
65,536 encodings per lane in two correlated input families, preserving earlier results across later
calls. This includes signed zeros and NaN payload bits without performing Half arithmetic; it is not
a Cartesian product of lane combinations. [Effectful composition](../../tests/cuda/nvvm-half-vector-helper-effectful.slang)
checks a Half-vector result beside an observed call effect. [Exports](../../tests/cuda/nvvm-half-vector-helper-exports.slang)
cover retained source boundaries; separate frozen-declaration PTX callers qualify the existing direct
export ABI, including semantic lane offsets. CUDA-prelude binary interoperability and return padding
contents are not qualified. [Direct static units](../../tools/slang-static-unit-test/unit-test-nvvm-type-lowering.cpp)
check exact classifier boundaries and canonical/physical/storage cache orders.

Unsupported bodies, signatures or roles fail preflight; successful lowering of an adjacent type is
not authorization to guess an ABI. Negative emitter tests also assert that rejection occurs before
provider mutation. The provider's serialization tests establish physical operation shape separately
from executable source contracts.

## BF16 and FP8: role-specific matrix

Here **record value** means internal construction/extraction/whole load/store/call/return/phi in the
qualified finite record domain. **Local reference** means the exact qualified mutable Generic root
and field provenance, not arbitrary pointers. Formats remain semantic identities even when LLVM uses
integer storage.

| Feature / role                                         | Current state and evidence                                                                                                                                                                         | Explicit limits / regression                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| ------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| BF16 scalar transport, literals, local mutable storage | Supported as physical i16; GPU includes all 65,536 bit encodings and signed literals                                                                                                               | Raw transport preserves NaN payloads; casts have separate policy. [Scalar](../../tests/cuda/nvvm-bf16-scalar.slang), [signed literals](../../tests/cuda/nvvm-bf16-signed-literals.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| BF16↔Float32                                           | Scalar and equal-width vector conversion; nearest-even narrowing and exact bit expansion on qualified SM80                                                                                         | NaN narrowing classification only; no integer, Half or Double cast admission. [Scalar](../../tests/cuda/nvvm-bf16-scalar.slang), [vector values](../../tests/cuda/nvvm-bf16-vector-values.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| BF2/BF3/BF4 register and internal helper values        | Supported `<N x i16>` construction/extraction and branch/phi transport; GPU                                                                                                                        | No external CUDA helper ABI claim or inferred explicit vector Select contract. [Vector values](../../tests/cuda/nvvm-bf16-vector-values.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| BF2/BF3/BF4 local mutable references                   | BF2 native-vector storage; BF3/BF4 component arrays; symmetric raw lane conversion; GPU with both cache orders                                                                                     | Does not grant component-pointer, device/resource, readonly or exported roles. [Local vectors](../../tests/cuda/nvvm-bf16-local-vectors.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| Flat local BF16 records including BF3/BF4              | Integer and BF16 scalar/vector fields; field load/store through proven local root; layout and exhaustive transport evidence                                                                        | BF3/BF4-containing whole record values, nesting, arrays and external storage not admitted. [Local records](../../tests/cuda/nvvm-bf16-local-records.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| CUDA BF16 layout queries                               | Canonical scalar/vector/matrix/wrapper layout matches selected CUDA forms; reflection, IR and GPU query tests                                                                                      | Layout metadata, including record-array queries, is not runtime memory admission. [Layout](../../tests/cuda/nvvm-bf16-cuda-layout.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| BF16 dot widths 2–4                                    | Core source-ordered product/sum composition using scalar BF16 FMA; GPU                                                                                                                             | No unrestricted FP32 accumulation, general BF16 arithmetic/comparisons or width-0/1 contract. [Dot](../../tests/cuda/nvvm-bf16-dot.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| FP8 scalar transport and finite literals               | E4M3/E5M2 i8 values, internal helpers, same-size bitcasts and same-format selection; GPU over all 256 encodings per format                                                                         | Transported nonfinite bytes supported; nonfinite literals rejected. [Transport](../../tests/cuda/nvvm-fp8-scalar-transport.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| FP8→Float32                                            | Both formats, exact finite values and E5M2 infinity; GPU over every byte                                                                                                                           | NaN payload/sign unspecified; no native FP8 hardware required on SM80. [Widening](../../tests/cuda/nvvm-fp8-widening.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| Shared finite FP8 folding                              | Repaired finite normal/subnormal conversion and nearest-even rounding; exhaustive independent unit evidence and source regression                                                                  | Shared overflow policy differs from CUDA SATFINITE. [Finite literals](../../tests/cuda/nvvm-fp8-finite-literals.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| Float32→FP8 runtime narrowing                          | Research only; no backend admission                                                                                                                                                                | Prototype nearest-even/saturation evidence does not qualify production conversion. Integer/Half/Double conversions, FP8 arithmetic and vectors remain excluded.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| Flat/nested mixed record values                        | Integer leaves with FP8/BF16 scalar/BF2, at least one substandard descendant; local mutable storage and internal value transport supported                                                         | No BF3/BF4 whole values or resource/shared/readonly/exported roles. [Flat](../../tests/cuda/nvvm-substandard-records.slang), [nested](../../tests/cuda/nvvm-nested-substandard-records.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| BF2 record-field component address                     | Supported from actual field of a qualified local record, preserving exact pointer/index checks                                                                                                     | Does not qualify BF3/BF4 component pointers or bare FP8 pointers. [Flat records](../../tests/cuda/nvvm-substandard-records.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| Nested dynamic AnyValue payloads                       | Two runtime-selected conformers, Natural unpack/CUDA local/mutate/repack and saved interface copy; three-mode GPU                                                                                  | Raw bit transport; no new arithmetic or arbitrary interface ABI. [Substandard](../../tests/cuda/nvvm-nested-substandard-dynamic.slang), [integer control](../../tests/cuda/nvvm-nested-integer-dynamic.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| Local mixed record arrays                              | Nonempty fixed arrays of existing integer/FP8/BF16/BF2 identity records; dynamic selection, snapshots, internal value, out/inout and readonly parameters, source returns via OutParam; focused GPU | Address roots require local Var or first-block admitted parameters of internal helpers; calls prove origin and access before type equality. Readonly borrows retain native storage and ordinary loads. No native array/pointer results, exported references, wrapper fields, multidimensional arrays, BF3/BF4 elements or external storage. [Transport](../../tests/cuda/nvvm-local-substandard-record-arrays.slang), [snapshots](../../tests/cuda/nvvm-local-substandard-record-array-copies.slang), [value parameters](../../tests/cuda/nvvm-substandard-record-array-parameter.slang), [references](../../tests/cuda/nvvm-local-substandard-record-array-references.slang) |
| Local-record Generic pointer helper result             | Excluded                                                                                                                                                                                           | Exact synthetic Generic-result boundary is source-reviewed; public UserPointer negative tests are a different shape.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |

BF16 record-array feasibility experiments with raw LLVM, physical BF3/BF4 lane arrays and successful
CUDA metadata queries do not by themselves establish runtime record-array support. Similarly, all
65,536 scalar patterns in a two-element array do not establish a 65,536-element allocation or scalable
compile time. Nested dynamic tests cover correlated field combinations, not a Cartesian product.
[Direct static units](../../tools/slang-static-unit-test/unit-test-nvvm-local-record-arrays.cpp)
qualify each Value/Storage/HelperParameter first-use order for fixed arrays of flat and nested
identity records, including forbidden-role lookups before and after successful caching. Forged
pointer roots cannot acquire the local allocation's checked address permission. These tests do not
qualify arbitrary nested dynamic records or array reference/result/export signatures. The value
parameter fixture checks caller saved/live arrays, callee-local mutation and a returned selected
record with integer oracles. All 65,536 original bit patterns are covered, with four slot/lane
combinations occurring 16,384 times each; this is correlated coverage, not a Cartesian product.

The architecture preserves the numeric details needed to extend these regions: CUDA BF sizes and
alignments, separate Natural/CUDA packing, BF16 NaN/cast policy, source-ordered dot and FP8 overflow.
These details must accompany any widening of a role predicate.

## Waves, synchronization and observations

| Region                                            | Current contract / evidence                                                                  | Limits / regression                                                                                                                                                                                                                          |
| ------------------------------------------------- | -------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Lane reads, ballots, votes, rotation and prefixes | Named primitives and core payload, reduction/prefix, quad and rotation composition           | Exact type widths and mask participation govern legality. [Shuffle widths](../../tests/cuda/nvvm-wave-shuffle-widths.slang), [rotation](../../tests/cuda/nvvm-wave-rotation.slang)                                                           |
| Hardware active mask                              | Side-effecting convergent snapshot; implicit aggregate helpers compose the required ballot   | Distinct from logical active-mask synthesis; source branch membership need not imply reconvergence. [Hardware mask](../../tests/cuda/nvvm-hardware-active-mask.slang), [aggregate mask](../../tests/cuda/nvvm-implicit-aggregate-mask.slang) |
| Masked integer MIN/MAX                            | Narrow and 64-bit signed/unsigned identities in core prefix/reduction folds                  | Identity bits are width-specific; no invalid shift by 64. [Narrow](../../tests/cuda/nvvm-narrow-masked-minmax.slang), [64-bit](../../tests/cuda/nvvm-i64-masked-minmax.slang)                                                                |
| Masked Float/Double MIN/MAX                       | Source-ordered reduction and prefix rules, signed-zero/NaN/singleton behavior; GPU           | Ordinary min/max calls are not interchangeable with ordered wave selection. [Float](../../tests/cuda/nvvm-fp32-minmax-order.slang), [Double prefix](../../tests/cuda/nvvm-fp64-prefix-minmax-order.slang)                                    |
| Masked Half MIN/MAX                               | Separately qualified raw-half transport and ordered selection; finite exclusive seeds ±65504 | Infinity/NaN/signed-zero behavior follows exact seed and source order. Matrix prefixes remain separate. [Half](../../tests/cuda/nvvm-fp16-masked-minmax.slang)                                                                               |
| QuadAny/QuadAll                                   | Core composition with four unconditional source-lane reads; GPU                              | Complete source quads and matching shuffle sequences required; standalone requirement markers, partial quads and SPIR-V active-only semantics are not admitted. [Quad votes](../../tests/cuda/nvvm-quad-votes.slang)                         |
| Clock/clock64                                     | Live side-effecting per-SM cycle observations; relational GPU oracle                         | Wrapping counters, not wall time, cross-SM ordering or memory fence. Exact timestamp comparison is invalid. [Clock observations](../../tests/cuda/nvvm-clock-observations.slang)                                                             |

Public wave primitives now select eight named registry calls; raw payload decomposition and
vector/matrix composition live in core bodies. Nine semantic tags, six numeric IDs
(16/19/20/21/22/23), and their obsolete compound text routes are removed. The existing shuffle-width
and Float64 aggregate tests cover payload transport and shared mask reuse. The
[core wave edges](../../tests/cuda/nvvm-core-waves.slang) cover exited lanes named in the original
mask, identical/different NaN payloads, signed zero, 64-bit high-word differences and aggregate
equality. Matching/equality naturally extends to narrow and wide raw payloads; this does not change
CUDA's pre-existing match-all implementation of `WaveMaskMatch`, which differs from NVVM match-any
for unequal payloads.

Hardware snapshots use effectful canonical `WaveGetConvergedMask` (stable IR 909), retaining the
convergent side-effecting operation 79. Logical active-mask synthesis remains separate. The compact
edge fixture checks that snapshots exclude exited lanes and contain the observing lane; it does
not equate hardware participation with a source-level branch predicate. All-equal compares raw
match intersections with a participating ballot and evaluates every component collective.
Masked scan/reduction, quad and rotation algorithms now live in core compositions with their
existing ordering contracts. One generic fold shares control flow across seven algebras and three
modes. Sum/product retain Int32/UInt32/Float32/Float64 admission; bitwise retains Int32/UInt32;
min/max retains integer 8/16/32/64 and Half/Float32/Float64. Double singleton payloads and negative-zero
sum seeds, Half finite exclusive seeds and source-ordered floating min/max remain unchanged.
Numeric 15/17/43/44/45/48 are retired; only canonical ballot/match/hardware-mask 18/71/79 remain.
Scalar all/any now uses canonical Boolean conversion, retaining NaN=true and either zero=false.
Standalone quad requirement markers and the removed complete CUDA helper bodies still reject.

Core synchronization helpers directly name `llvm.nvvm.barrier0`, `llvm.nvvm.membar.cta` and
`llvm.nvvm.membar.gl` in their NVVM branches. Provider and compiler units cover canonical void
results, malformed signature rejection before module creation, retained effects and LLVM attributes.
The [synchronization fixture](../../tests/cuda/nvvm-synchronization-effects.slang) exchanges shared
values across two warps through non-inlined helpers, with two runtime loop iterations and an
independent count/error/checksum oracle of 128/0/4544. Its PTX checks retain both fence scopes and the
barrier. The [memory-only fence fixture](../../tests/language-feature/execution-model/group-memory-barrier-no-sync-emission.slang)
requires `membar.cta` without `bar.sync`. This preserves existing barrier scopes, including the
subgroup and WithGroupSync mappings. It does not qualify arbitrary transitive helper convergence:
LLVM convergence metadata is retained on the intrinsic but is not propagated to helper declarations.

## CUDA entry parameter transport

Compute entries accept Bool, signed/unsigned 8/16/32/64-bit integers, Half, Float and Double,
each scalar or vector2/3/4. Shared CUDA layout owns size/alignment; entry-only byval scalar arrays
carry vectors, Bool uses byte transport, and Half uses integer bit transport. Half3's fourth storage
lane is padding. Ordinary value, helper and storage caches keep their own roles. The sibling RHI
`numeric-entry-abi` fixture independently checks all 48 shapes, exact bits and neighboring words at
O0/O3. Fixed arrays and numeric records now use recursive checked CUDA launch plans and canonical
value reconstruction. The sibling `aggregate-entry-abi` fixture covers all twelve scalar kinds,
vector widths 2/3/4, nested arrays/records, whole-record helper forwarding, mixed Half/non-Half
records and neighboring argument/output guards at O0/O3. Half leaves use recursive integer helper
transport. Zero/oversized arrays and explicit strides inconsistent with CUDA packing reject.
BF16/FP8 and unsupported pointer/resource roles remain excluded.

Explicit row-major non-square matrices are qualified after physical array lowering. Column-major
entry matrices remain an open shared CUDA ABI issue: reflection honors column-major packing,
while the CUDA Matrix representation uses rows. The retained mixed float2x3/float3x2 fixture fails
11 output lanes per optimization on both NVVM and NVRTC. This is not a resource-buffer matrix
layout change; do not infer column-major entry qualification from the row-major fixture.

Byte/structured buffers retain pointer/count transport, with counts in bytes/elements respectively.
Equivalent structured views divide byte extents by the checked element stride (2/4/8), including
UInt32 views; pointer reinterpretation does not preserve the count units. Division support is
checked before provider mutation. Seven supported readonly and combined texture geometries,
five writable geometries and regular samplers retain single 64-bit slots. Combined handles are
not split. The sibling `resource-parameter-abi` fixture checks mixed entry layout, resource contents,
helper transport, dimensions and guards at O0/O3. Static tests cover role/cache order and rejection
boundaries; focused evidence retains all 24 resolved original RHI failures and three new fixtures.
No new main-corpus admission or full checkpoint is implied.

## Texture, surface and descriptor contracts

| Region                                  | Current contract / evidence                                                                                                        | Limits / regression                                                                                                                                                                                                                                                                                           |
| --------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Sampled textures                        | Typed Sample/Level/Fetch/Gather operations with unchanged provider contracts; focused compile/runtime evidence                     | Full texture API and every shape/offset/format not implied. [Sample boundary](../../tests/cuda/nvvm-sampled-texture-unsupported.slang), [fetch boundary](../../tests/cuda/nvvm-texture-fetch-unsupported.slang)                                                                                               |
| Surfaces                                | Typed physical accesses; explicit static Half conversion; masks preserve untouched bits                                            | Byte-addressed access and element-count geometry differ. [Float surface](../../tests/cuda/nvvm-native-float-surface.slang), [formatted provenance](../../tests/cuda/nvvm-formatted-surface-provenance.slang), [integer boundary](../../tests/cuda/nvvm-native-integer-surface-unsupported.slang)              |
| Read-only texture descriptor conversion | Resource↔descriptor↔uint64 identity and UInt2 low/high word transport for accepted read-only texture families                      | Buffer descriptors contain pointer/count and do not inherit integer-handle conversion. [Texture descriptors](../../tests/cuda/nvvm-texture-descriptor-conversion.slang), [buffer negative](../../tests/cuda/nvvm-texture-descriptor-buffer-unsupported.slang)                                                 |
| Selected non-mip dimensions             | Typed UInt32 spatial queries for 1D/2D/3D/Cube and separate view-relative layer counts for 1DArray/2DArray; int/uint/float outputs | Other array counts, requested mip and total/view level counts remain excluded. [Dimensions](../../tests/cuda/nvvm-texture-dimensions.slang), [array counts](../../tests/cuda/nvvm-texture-array-layer-counts.slang), [query negatives](../../tests/cuda/nvvm-texture-query-unsupported.slang)                 |
| Undefined ordinary sampler              | Qualified CUDA placeholder semantics                                                                                               | Comparison samplers and arbitrary undefined resources remain excluded. [Sampler](../../tests/cuda/nvvm-undefined-sampler.slang), [comparison negative](../../tests/cuda/nvvm-undefined-comparison-sampler-unsupported.slang), [resource negative](../../tests/cuda/nvvm-undefined-resource-unsupported.slang) |

Core producers now own texture operation identity and output composition. Existing Sample and
logical ImageLoad/ImageStore are joined by stable operations for explicit level,
integer fetch, ordinary gather, spatial size and scalar array layer count. Full resource types reach
preflight; sampler validation remains required, while the provider still uses CUDA texture-object
state. Gather offsets remain ignored. Non-mip Texture1DArray/Texture2DArray layer counts use a
distinct scalar UInt32 query mapped to `txq.height`/`txq.depth` with the real array descriptor
preserved. Spatial rank remains one/two. Integer and Float32 outputs use ordinary numeric casts; other array-count bodies retain the previous zero
fallback. Mip and multisample admission remain unchanged. Writable dimensions are qualified
separately below. Public direct
RWTexture2DArray.Store gains the same native32 path already admitted by canonical image stores.
Float3 sampling, extra access modes and query-level admission remain excluded.

Writable dimension queries cover 1D/2D/3D/1DArray/2DArray and use surface queries, preserving
existing scalar kinds and widths1/2/4. Global fields, entry formals or exact descriptor conversions
own writable format; required helper inlining preserves those binding selectors. Unproven runtime
format selections and mismatched memory accesses remain rejected. The sibling `surface-dimensions` fixture checks native Float shapes and a formatted
Half surface at O0/O3. Direct CUDA array probes qualify height as 1DArray layer count and depth as
2DArray layer count on the tested driver/toolkit. This is an empirical CUDA representation contract,
not a universal PTX rule: isolated `suq.array_size` fails module loading with CUDA801. CUDA C++ gives
zero for the two array counts at O0/O3; the allocation-based expected counts remain 3 and 4.
Integer 1DArray fetch covers Float/Int/UInt32 widths1/2/4 and explicitly reorders `(x, layer)` to
PTX `(layer, x)`; provider assertions and real bindless RHI execution qualify that ordering.
Lane3 texture values, MS/comparison/shadow resources, RWCube and 3D arrays remain excluded.

All active semantic tags and resource-text recognizers are retired. Deprecated AST/IR serialized
slots remain inert under module 43; ABI 46/container 2 are unchanged. Exact removed-tag/text tests
reject before provider creation, and existing named explicit-operand tests keep that syntax live.

Texture research distinguishes base geometry, selected mip geometry, layer/cube count and total
allocated/view levels. The CUDA source producer currently loses some of those semantics. Agreement
with NVRTC can therefore preserve the same error. A texture object and a surface handle are not
interchangeable, and opaque descriptor bytes cannot be inspected as an undocumented metadata API.
For correctly bound non-mip 1D/2D arrays, layer count follows the bound view, consistent with
[Direct3D resinfo](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/resinfo--sm4---asm-)
and [Vulkan image queries](https://docs.vulkan.org/spec/latest/chapters/images.html#spirvenv-image-query).
CUDA C++ currently returns zero for this count, an intentional comparison discrepancy rather than
the NVVM oracle. The render-test input producer currently treats `arrayLength=1` as non-array;
this separate host limitation is not repaired with a shader fallback. CubeArray counts remain
outside the qualification. NVVM O0/O3 runtime covers full arrays with 2/3/5 layers and all three
output types. A raw-handle Slang O3 kernel additionally returns 11×7×5 for the full view and
11×7×3 for a restricted view of the same allocation on CUDA12.9/SM80. Explicit view descriptors and
untouched guards were independently checked. Null-view descriptor getter garbage is retained as an
API anomaly and is never used as query metadata. The same output families now also cover 1D arrays
with 2/3/5 layers; a raw Slang O3 kernel returns width/count 11/5 and 11/3 for independently validated
full/restricted views of one layered allocation.

A separate four-case probe retained two CubeArray mismatches: explicit views of an 8×8×30-face
allocation with requested layer ranges 0..29 and 6..23 returned query depth 30/18, against intended
public counts of 5/3 cubes. All APIs, echoed descriptors and guards passed. Echoed range fields do
not establish their semantic units. Earlier null-view cube queries returned cube counts; these
observations do not justify a universal division by six. A subsequent content probe filled each
physical face with `100*cube+face`. Null-full and explicit 0..4 views returned count 5 and sampled
all five source cubes correctly. Explicit 1..3 returned count 3, but +X/+Z samples were
`1,5,101,105,201,205` instead of `100,104,200,204,300,304`. Guards and unused output slots survived.
Thus matching query size does not establish correct cube selection. A subsequent face-aligned
6..23 content probe samples the intended three source cubes correctly (`100,104,200,204,300,304`),
but still reports depth 18 instead of 3. Direct `cuTexObjectCreate` and runtime creation produce
identical descriptor, query, content and guard results, including passing null/full controls.
This rules out a discrepancy confined to runtime descriptor translation; it does not establish a
portable restricted-cube count correction. Both earlier failed hypotheses remain recorded.

A real 8×4 four-level mip allocation returns widths 8, 4, 2, 1 through `txq.level.width`; a checked view
of mips 1..2 returns relative-LOD widths 4, 2. PTX JIT and preassembled cubin both execute all guarded
width checks. Isolated `txq.array_size` and `txq.num_mipmap_levels` assemble successfully but fail
with driver error 500 at kernel lookup under default lazy loading and at module load under eager loading,
for both artifact paths and both views. Cubins contain the global kernel symbol; the failing ones
also contain weak undefined `.nv.unified.texrefDescSize`, a diagnostic lead without an established
repair. No reserved metadata value is fabricated. This is qualified-stack evidence, not a claim
that the documented instructions are universally unavailable. The [PTX query contract](https://docs.nvidia.com/cuda/archive/12.9.1/parallel-thread-execution/index.html#texture-instructions-txq)
and [CUDA view contract](https://docs.nvidia.com/cuda/archive/12.9.1/cuda-runtime-api/group__CUDART__TEXTURE__OBJECT.html)
remain the external references. No full mip `GetDimensions` repair is implemented; a correct level
count is still required.

NVVM surface legalization supports the existing native 32-bit signed/unsigned/Float32 scalar, two-
and four-channel transfers in 1D/2D/3D and 1DArray/2DArray shapes. Native Half and annotated Half
storage with Float32 shader values support the same 1D/2D/3D and 1DArray/2DArray shapes. Matching Float32 uses Float32
payloads directly. Static component masks and in-range dynamic scalar indices update physical channels without
re-encoding untouched NaN payloads. Dynamic indices use the existing non-atomic whole-texel merge;
there is no new out-of-range lane guarantee. Byte-X scaling and conversion are explicit IR; no runtime format discovery is added.

Explicit `r/rg/rgba` signed/unsigned 8/16-bit formats additionally use matching `int/uint`32
shader values in 1D/2D/3D and 1DArray/2DArray surfaces. Loads sign- or zero-extend; stores clamp in the logical
32-bit type before narrowing. This follows the [D3D narrower-integer conversion contract,
§3.2.3.13](https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm).
[Vulkan integer image reads](https://docs.vulkan.org/spec/latest/chapters/images.html#images-reads)
agree on load interpretation; out-of-range Vulkan storage-image encoding equivalence has not been
established. Mixed integer signedness remains excluded; normalized floating conversions are qualified below.
The static annotation owns physical interpretation; opaque handles do not infer runtime formats.

Native signed/unsigned 8/16-bit logical values support these same shapes and widths 1/2/4.
Canonical format inference supplies matching integer formats; explicit annotations must match native
width, signedness and channels. Native stores preserve bits without the logical32 clamp.
[Native narrow fixtures](../../tests/cuda/nvvm-surface-physical-native-narrow.slang) independently
observe original loads through 32-bit outputs before writing native marker bits.

The [physical-storage harness](../../extras/validate-nvvm-surfaces.py) independently checks 121 cases,
including exhaustive Half loads, conversion boundaries, direct literals, written NaN classification,
nonzero guards and zero-boundary accesses. Native signed/unsigned32 cases exercise 1D/2D scalar,
two- and four-channel loads, whole stores and component stores with exact integer bits. Four mixed
cases bind eight separate native Float32/r16f/rg16f/rgba16f source/result resources, checking both copy
directions and untouched channels. The last full checkpoint passed 81 cases in each NVVM mode and retained two dynamic-index
failures per mode. Focused dynamic-component qualification now covers those two cases plus two
static controls at O0/O3, with unchanged physical oracles and guards. The dynamic cases exercise
all four lanes in 2D Float32/Half storage and exact untouched NaN payloads; signed/two-lane shapes
have compiler-unit coverage, not fresh physical runtime coverage. NVRTC's full checkpoint retains
52 passes, 20 compile failures and 11 rounding mismatches.
Two additional [layered cases](../../tests/cuda/nvvm-surface-physical-layered.slang) group all nine
native32 kind/width combinations in 1D arrays and isolate 2D-array coordinate ordering. Distinct
resource/layer/coordinate/lane bits, independent stores and complete host readback prevent a matching
wrong load/store address from hiding corruption. The original 83 case/oracle identities remain
unchanged. Explicit array role selects layered allocation and copy depth, even for a singleton;
CPU checks cover that role without claiming singleton GPU qualification.
Four [Half-array cases](../../tests/cuda/nvvm-surface-physical-half-layered.slang) cover both array
ranks, native Half and formatted Float32 values, widths 1/2/4 and whole/static-component updates.
Fourteen independently read-back resources distinguish raw copies, narrowing, widening and source
marker writes. NVVM O0/O3 passes all eight cells; four CUDA cells retain missing layered conversion
helpers and invalid component-subscript compilation. Converted NaNs require class; native copies,
untouched channels and layer guards retain exact bits. No new dynamic-array runtime claim is made.
Two [Half-volume cases](../../tests/cuda/nvvm-surface-physical-half-volume.slang) cover the same
representations and widths with ordinary XYZ coordinates. All four new NVVM cells pass; volume
allocation/copy depth is separate from the layered flag. CPU and report checks reject incorrect
depth, height and array roles. CUDA fails both entries because their shared helper emits invalid
component subscripts; this does not establish failure of CUDA whole-volume operations generally.
Six [integer-format cases](../../tests/cuda/nvvm-surface-physical-integer-formats.slang) cover all twelve
explicit signed/unsigned 8/16-bit formats with whole/static/dynamic writes at both non-array ranks.
All twelve NVVM O0/O3 cells pass. Loads are recorded in twelve separate native32 arrays before
independently generated store edges overwrite the source arrays. Representative signedness patterns,
each selected lane's sixteen store edges, complete guards and unchanged component bits are exact.
These tests are bounded patterns, not exhaustive integer decode. All six CUDA comparison cells fail
compilation because integer surface-conversion helpers are missing; component generation also has
existing subscript limitations. Their failure identities are unchanged from the before run.
Nine [integer spatial cases](../../tests/cuda/nvvm-surface-physical-integer-spatial.slang) extend the
same twelve formats and three write styles to 1DArray/2DArray/non-array3D. All eighteen new NVVM
cells pass. Explicit array flags and volume depth, asymmetric Y/Z extents, guarded outer planes,
coordinate-varying input bits and independent Z-phased stores exercise the geometry separately
from conversion. CPU checks reject a bounded matching load/store plane permutation. All nine CUDA
comparison cells retain missing integer read-conversion helpers; the numerical policy is unchanged.
The recurring checkpoint replays physical readbacks and compares its complete inventory against the
[current baseline](../../issue-nvvm-backend/accepted-baseline.json), preserving known failures.
That full baseline still owns its original 249 cells; the sixty-nine added array/volume/integer cells require explicit
reviewed adoption at the next full checkpoint.
[Ordinary Half conversion](../../tests/cuda/nvvm-half-narrow-conversion.slang)
has a separate three-mode regression. Half stores use RN-even; this differs from the existing NVRTC
formatted store's observed truncation. NVRTC component writes (including dynamic indexing), three-channel transfers and additional packed
formats remain outside this qualification. Required helper legalization is qualified separately below;
component stores do not add atomicity or concurrent-alias guarantees.

The complete `r/rg/rgba`8/16-bit UNORM/SNORM family supports Half/Float logical values, whole
loads/stores and static/dynamic component stores across1D/2D/3D and1DArray/2DArray surfaces.
[The normalized fixture](../../tests/cuda/nvvm-surface-physical-normalized.slang) groups twelve formats
per compile:30geometry/operation/type rows pass atNVVM O0/O3 (60cells). Independent raw initialization,
Float32 observations, guarded bytes and64boundary inputs separate load and store correctness.
Focused enhanced minimum-code cases also check that partial writes retain SNORM's raw minimum code.
Stores map NaN to0, clamp, scale inFloat32 and round nearest/ties-away; Half inputs widen first.
Loads divide inFloat32, clamp SNORM minimum to-1, then narrow for Half. This fits the
[Direct3D normalized conversion contract, §§3.2.3.3–6](https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm)
and [Vulkan fixed-point conversions](https://github.khronos.org/Vulkan-Site/spec/latest/chapters/fundamentals.html#fundamentals-fixedconv),
which does not mandate a tie direction. sRGB transfer is not repeated by storage conversion.
Four selected NVRTC comparisons fail compilation at missing surface read-conversion helpers and
component generation; they provide no numerical qualification. Exact failures remain in focused evidence.

[Wrapper provenance](../../tests/cuda/nvvm-surface-wrapper-provenance.slang) qualifies declared fields
and fixed arrays rooted in entry value bindings or collected globals. Required helper inlining
preserves these bindings through value inputs/results, out/inout/readonly references and nested
record/array transport. Twelve O0/O3 GPU cells cover normalized whole/static/dynamic operations,
native Float32 and formatted Half callers, actual local handle overwrites and observable side effects.
The same helper can access distinct formats; runtime choices lacking a single proven format still
reject before provider mutation. Raw-pointer provenance and recursion remain rejected. The shared
load-deferral proof preserves snapshots across mutation. Required inlining can increase code size.

The484-node application selection passes completely:96helper-format failures resolve and388prior
passes remain. The earlier50normalized-format resolutions are preserved. This is focused evidence,
not an updated full surface/application checkpoint.

The unannotated surface contract requires a matching physical channel width, count and scalar
interpretation. A focused host-readback experiment binds the same `RWTexture1D<int4>` kernels to
signed8×4 and signed32×4 CUDA arrays. Whole stores fail the packed physical oracle in NVRTC O3 and
NVVM O0/O3; component stores fail in both NVVM modes, while NVRTC component source does not compile.
All five matching signed32 controls pass. Every physical byte and six neighboring texels are checked;
these are bounded 1D observations, not qualification of arbitrary formats or array shapes.
The original `compute/texture-subscript.slang` binds RGBA8Sint but checks results through the same
shader access mapping. Its historical NVVM passes do not establish packed-format correctness.
The CUDA component-source gap and packed-format limitation remain separate open obligations in
[focused evidence](../../issue-nvvm-backend/focused-evidence.json); no source fix or format conversion
is claimed by this investigation.

## Language composition and application evidence

| Region                                                                            | Evidence currently available                                                                                                             | Limit                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| --------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Dynamic dispatch                                                                  | Permanent simple and nested AnyValue records with live runtime selection/mutation                                                        | Concrete record roles and payload capacity remain bounded. [Original substandard case](../../tests/compute/dynamic-dispatch-substandard-float.slang), [nested regression](../../tests/cuda/nvvm-nested-substandard-dynamic.slang)                                                                                                                                                                                                                                                                                                                                      |
| Errors and generic throwing calls                                                 | Runtime-dependent scalar/aggregate propagation, catches, generic-specialized witness path; permanent three-mode GPU tests                | Generic specialization is not an existential throwing-witness ABI. [Runtime errors](../../tests/cuda/nvvm-runtime-errors.slang), [throwing witness](../../tests/cuda/nvvm-runtime-throwing-witness.slang)                                                                                                                                                                                                                                                                                                                                                              |
| Inheritance and initialization                                                    | Focused unchanged-source three-mode qualification of struct inheritance, derived initializer lists, 16-bit defaults and scoped constants | Some expressions/calls/arrays can fold; no IR survival, arbitrary inheritance depth or nonintegral Half claim. [Inheritance](../../tests/language-feature/inheritance/struct-inheritance.slang), [derived initialization](../../tests/language-feature/inheritance/derived-struct-init-list.slang), [16-bit defaults](../../tests/language-feature/initializer-lists/default-init-16bit-types.slang), [constants](../../tests/language-feature/constants/static-const-in-struct.slang)                                                                                 |
| Accessors, generics, variadics, constrained extensions, lambdas, tuples and defer | Focused unchanged-source experiments plus main-corpus selection                                                                          | Passing isolated examples does not establish all feature intersections. Selected membership and semantic coverage are different measurements.                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Material shaders                                                                  | Both registered entries × three modes compile/assemble and execute with two live synthetic textures and independent scalar oracles       | 65 active records/entry/mode and 63 untouched guards. Eval checks 260 components; sample checks 455 floats/65 flags, selected-layer throughput, coherent arithmetic candidates and exact rejection zeros. Fixed synthetic inputs only; no original assets, live LUTs or sampling-distribution claim. Device-event evidence has its own scoped row below. [Runtime validator](../../extras/validate-nvvm-material-runtime.py). [Material manifest](../../issue-nvvm-backend/complex-corpus.manifest.json)                                                               |
| Synthetic material device events                                                  | Current candidate and requalified pre-fix NVVM O3 cubins; each arm passes 18 qualification cells, 216 samples and 72 warmups             | At 1,048,577 records, eval 2.741 → 0.313 ms and sample 2.381 → 0.391 ms (8.75×/6.09× faster), with the same helper/oracle/inputs/GPU/driver. Existing shared unpack helpers restore scalar promotion while preserving snapshots and CUDA layout. Synthetic hot textures and periodic inputs only. Current small-count NVVM O3 intervals fall below 0.1 ms and have no ratios. Historical controls retain their identities. [Runner](../../extras/measure-nvvm-material-runtime.py), [protocol](../../issue-nvvm-backend/RESULTS.md#material-device-event-measurement). |

The material validator also qualifies a separately named `linear-filtering` input profile in six
GPU cells. Four dyadic UV locations exercise horizontal, vertical and bilinear blends and a footprint
across both wrap seams; encoded color is interpolated before sRGB decoding. Each cell checks 65 active
records and 63 guards. Both entries retain independent oracles, fixed tolerances, and sample-wide
arithmetic hypotheses. Six fresh default-profile cells preserve exact input/oracle/output bytes.
Integer-offset equivalence is observed at the first location; arbitrary UV quantization, assets,
LUTs and timing remain outside this qualification. The original device-event protocol uses centers.

A separate [imported mini-LUT fixture](../../tests/cuda/nvvm-material-mini-lut.slang) executes the
unchanged material library's 16×16 mini-microfacet interpolation and reflection compensation in all
three modes. Twelve runtime queries cover corners, cosine saturation, asymmetric interiors and both
sides of the compensation denominator floor. Each mode checks 48 numeric components against an
independent rational oracle, exact zero-knot bits, completion and a guard. The table is synthetic and
Fresnel is fixed; original assets, larger LUT families and LUT execution in the registered analytic
graph remain unqualified. This fixture uses the standard renderer, with no material-driver change.

The [imported dielectric fixture](../../tests/cuda/nvvm-material-dielectric.slang) separately qualifies
runtime reflection/transmission evaluation and PDFs with the existing no-compensation policy.
Twelve queries cover normal combined/single/disabled modes and off-axis anisotropic x/y directions.
Independent geometric equations check 48 float components per mode; raw IEEE readback and permanent
finite/error masks, exact disabled zeros, completion and guard pass all three modes. The fixed budget
is `1e-6 + 1e-5*abs(expected)`. This does not qualify sampling, TIR, backside handling, absorption,
retroreflection, compensation, arbitrary inputs or performance. The registered graph remains
reflection-only; its code and assets are unchanged.

Material-derived aggregate/indexing coverage also runs through Slang in all three modes. The
[bounded and literal controls](../../tests/cuda/nvvm-aggregate-index-material.slang) retain initialized
unused fields, a receiver snapshot and a nested stack copy. A separate
[copy-then-mutate witness](../../tests/cuda/nvvm-aggregate-index-snapshot.slang) checks saved and live
slots independently, including all vector lanes. Six GPU cells check 96 exact integer components;
these two focused sources remain outside the main corpus denominator. This qualifies behavior, not
identical generated code or performance relative to the CUDA reduction.

The eval diagnostic also has a source-derived CUDA reduction with independent static address proof.
Eight bounded-index variants retain 27 never-read store instructions and a 584-byte stack; eight
constant-index variants have no local memory. The other three source factors do not change PTX.
Three full-material force-inline controls reproduce the original PTX exactly. All 19 candidates
compile and assemble; none has a reduced-kernel GPU correctness or timing qualification.

The [current focused evidence](../../issue-nvvm-backend/focused-evidence.json) retains the inheritance/
initialization and mixed-record-array boundary outcomes separately from the full baseline.

The historical suite inventory counted 4,639 active files and 10,834 executable directives, including
1,852 compute-oriented sources and 795 explicit-CUDA sources. The current main selection has 576
source files and 580 cases/1,740 mode cells. Its 31.1% compute-source and 65.0% explicit-CUDA-source
selection figures (576/1,852 and 517/795) are historical inventory ratios, not percentages of language
semantics covered.
Later focused cases remain outside that main denominator unless explicitly enrolled.

Field/index address recipes are shared by pointer validation, ordinary memory planning and
provider emission. Nested borrowed-vector coverage checks dynamic lane access across a mutation,
scalar alignment and the absence of invariant-load metadata for Generic Read borrows. Compact
parameter-group loads retain their independent representation, alignment and invariant metadata.
These checks preserve existing support boundaries. Parent proofs and raw-view access are retained
directly; pointer spelling alone cannot grant a child writable resource access.

## Current compilation and execution observations

The October 3 full refresh, preceding the material snapshot fix below, uses the optimized compiler on an AMD EPYC 7R13 host with eight visible logical
CPUs, one compiler process at a time, CUDA 12.9.2 and an L4 with driver 595.71.05. All 1,512 standalone
variants compile and assemble; 9,072 measured fresh-process compilations and 3,024 warmups preserve
qualified PTX. The cohort retains 504 cases/500 sources and 76 explicit exclusions.

| Cohort    | Cases | NVRTC O3 / NVVM O3 | NVRTC O3 / NVVM O0 |
| --------- | ----: | -----------------: | -----------------: |
| Frozen    |   406 |              1.95× |              1.99× |
| Discovery |    98 |              1.82× |              1.92× |

These are geometric means of per-case median compilation-time ratios, six samples per cell with
warm caches and equal `-g0`. NVVM O3 has lower medians in 400/406 frozen and 93/98 discovery cases;
NVVM O0 in 404/406 and 94/98. Timed CLI PTX is compile/assembly-qualified, not separately GPU-qualified.
Historical NVVM compilation times are roughly unchanged on matched primary sources while NVRTC
is slower; CPU count, physical GPU, driver and dependencies changed. The relative advantage does not
establish a longitudinal NVVM compiler speedup. Material compilation uses 18 samples/cell: eval medians
are 1.478/1.570/1.441 seconds and sample 1.504/1.697/1.498 seconds for NVRTC O3/NVVM O3/NVVM O0.
Assembly is measured separately. Broad-corpus gains do not transfer uniformly to larger entries.

The current material snapshot fix reuses shared storage unpack helpers at the original ordinary
aggregate read, eliminating an intervening whole-storage copy. At 1,048,577 records, requalified
before/after NVVM O3 cubins take **2.741 → 0.313 ms** for eval and **2.381 → 0.391 ms** for sample
(8.75×/6.09× faster), using the same current helper, oracle, inputs, GPU and driver. Current NVRTC O3
controls take 2.815/2.488 ms. Each arm independently qualifies 18 cells, then checks all active and 63 guard
records on 216 measured launches and 72 warmups. The earlier regression against historical 0.312/0.390 ms
cubins remains preserved as resolved history. These periodic 65-record/hot 2×2-texture workloads are
not renderer frames or arbitrary-material evidence.

Both entry frames shrink 592/624 → 0 bytes, without spills. Eval has zero static local stores; sample
retains 8 library stores. Snapshot semantics and compact CUDA pointer layout are unchanged; attributed
loads retain their physical memory operation. Inlining and numeric-matrix select controls did not
resolve the frame. A late load-deferral experiment was discarded; the retained decision belongs to
the shared storage conversion producer. The new
[static regression](../../tools/slang-static-unit-test/unit-test-nvvm-storage-loads.cpp) fails before
the fix and passes afterward, including aligned/scoped controls. Focused 35/35, RHI 4/4, the AD guard,
569 units, 16 smoke and 1,726 working configurations pass. Full RHI/SlangPy acceptance retains its earlier
identity. Module 43 / ABI 46 / container 2 are unchanged.

A bounded paired compilation check uses preserved before/after libraries: material eval 1.593 → 1.599 s,
sample 1.711 → 1.757 s, and 8 numeric variants range −6.3% to +1.1%, with three measured samples per arm/case.
It does not replace the preceding full benchmark. Matched compact-pointer RHI wall times are
113.77/116.55 s; the older 82 s comparison was not specific to this change.

Original-input dispatch attempts all 452 frozen and 128 discovery cases in three modes and two reversed
rounds, with three warmups/nine samples per mode/round and reset resources before each launch.
It accepts 3,424 mode-round cells and 30,816 samples; 56 cells remain excluded. All 3,402 prior measured
cells remain measured, 22 previously excluded cells now measure, and no measured cell regresses to
excluded. The initialized copyable-context fixture passes all six cells; its original repeatability
failure remains recorded. No general bare-static zero initialization guarantee is inferred.

Of 567 complete three-mode cases, 558 fall below the 0.1 ms median ratio policy cutoff. Eight of the
nine longer cases exercise wave/min/max and take 2.07–3.03 times NVRTC O3 intervals; FP8 scalar
transport is near parity at 1.04 times. Thirteen cases remain incomplete. These are correctness
fixtures including shader-side checks. CUDA events include RHI parameter upload and host enqueue
gaps; compilation, resource reset and readback are excluded. Clocks are not locked. Short intervals
remain without ratios, and no aggregate shader-throughput claim follows.

Fresh code capture/analysis covers all 27 variants of those nine longer cases. Offline CUDA 12.9 SM89
assembly reports zero spills for all 27 entries. O3 stack sizes match across backends: 112 bytes for
narrow min/max and zero for the other eight. Integer prefix folds miss eligible-mask tree fast paths;
floating folds retain trees but carry sparse/tree selection and first-set-bit work inside their loop.
Vector/matrix folds repeat scalar traversal. Static instruction/register counts alone do not explain
runtime; NVVM is often smaller. Preserve NaN, signed-zero, tie/second-operand and masked-lane semantics
when optimizing. Offline SASS is not recorded driver-JIT code.

The September 28 full 580-case code-quality analysis remains historical: 1,704 qualified captures,
36 gaps, 567 complete O3 pairs, 194 identical executable-section binaries, 46 similar profiles and 327
different profiles. It was not rerun on the current compiler. The older 12-source static subset and
material local-store diagnostic also retain their original identities; neither is fresh timing evidence.

See [measurement protocols](../../issue-nvvm-backend/RESULTS.md), the maintained
[capture tool](../../extras/capture-nvvm-corpus-code.py) and
[analyzer](../../extras/analyze-nvvm-corpus-code.py). The `compilation-performance`,
`corpus-dispatch-performance` and `tiled-brass-synthetic-device-events` records in
[current evidence](../../issue-nvvm-backend/focused-evidence.json) pin each measured identity,
independent reviews, exact transitions and preserved failure histories. After the material repair,
consolidating one storage/ABI conversion family is the recommended next cleanup; wave fast paths
remain a separate opportunity. Neither recommendation starts further development.

## Known gaps and evidence boundaries

- The current main baseline retains 36 unresolved cells: three column-major host-packing mismatches
  and 33 infrastructure/preflight outcomes. Keep original bytes, identities and expected outputs.
- Canonical multi-lane vector updates now emit legal component assignments in CUDA/C++/WGSL.
  [Snapshot coverage](../../tests/compute/swizzle-set-snapshot.slang) checks overlapping Half lanes,
  noncontiguous updates, untouched lanes and one effectful Float32 helper call in three GPU modes.
  [Source coverage](../../tests/compute/swizzle-set-source.slang) checks all five source targets;
  HLSL/GLSL retain multi-lane syntax. WGSL has source checks only. The original Half-vector corpus
  test is correct in all three modes with unchanged input and oracle.
- The separate Half-vector noinline helper failure survived the source-emission repair: NVVM O3
  omitted argument/result transfers while O0 and NVRTC were correct. Parameter-only, result-only,
  combined and exported probes isolated the boundary with identical O0/O3 provider LLVM inputs.
  Physical integer lane transport now repairs this boundary; the original wrong-output observations
  remain preserved in focused evidence. Ordinary arithmetic evidence alone does not qualify an ABI.
- The qualified column-major `float3x2` uses CUDA column stride 12 and size 24. The permanent
  [compact-column test](../../tests/cuda/nvvm-column-major-compact.slang) checks all six elements
  plus multiplication in three GPU modes (24 exact Float32 components), with fresh 24-byte layout
  reflection. Graphics-packed eight words produce `11,1`; the CUDA-packed six words produce `11,22`.
  The original three discovery mismatches remain unchanged. This does not establish that arbitrary
  graphics host layouts can be reused for CUDA or expand the main corpus denominator.
- Three focused integer nested-array cases still produce wrong output under NVRTC O3 while the
  corrected NVVM modes pass. They are outside the main-corpus outcome count.
- Local fixed mixed-record arrays now have permanent transport/snapshot GPU coverage. Exactly two
  historical mixed-local NVVM cells transition from preflight refusal to correct output. Internal
  value-array parameters pass a separate two-cell NVVM qualification. A bounded three-mode reference
  fixture checks mutable and readonly forwarding, source array returns via OutParam, snapshots,
  both slots and BF2 lanes using four independent bit-pattern seeds. Readonly observations before
  and after caller mutation keep read permission separate from immutable memory. Native array
  results, external roles and guarded wrappers remain excluded. Prior integer controls, corruption rejection and NVRTC
  failure history retain their original identities. Neither qualification establishes large-array
  scalability.
- The result classifier previously mistook `nvrtc` in a source path for an NVRTC compilation
  diagnostic. The parser now requires a downstream compiler identity prefix and accepts the actual
  `error :` producer form. The original failed attempt/adjudication remains in focused evidence;
  its output mismatch never becomes a passing shader.
- Toolkit/architecture compile-and-assembly matrices are separate from physical-device execution.
  Historical CUDA 12.9 SM70/80/90 and CUDA 13 SM80/90 results are not fresh qualification of every
  later feature. The manually defined container workflow was not dispatched in that evidence.
- OptiX beyond the raygen/triangle qualification above, source-level debug support, relocatable device code,
  device LTO, dynamic parallelism and
  device syscalls remain separate tracks. No general cooperative or autodiff support claim follows.

Further feature work should select a coherent role/operation combination and preserve neighboring
negative contracts. A diagnostic advancing to another unsupported instruction is research progress,
not implemented support. New evidence updates this matrix and permanent tests where appropriate;
chronological plans, reports and repeated raw outputs belong in working artifacts or Git history.
