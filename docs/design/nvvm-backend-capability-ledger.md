# NVVM feature and evidence matrix

This matrix describes qualified combinations, not a claim of complete Slang or CUDA support. The
[architecture](nvvm-backend.md) owns representations and invariants. [STATUS](../../issue-nvvm-backend/STATUS.md)
owns the current accepted checkpoint and loaded compiler identity; its referenced manifests own exact
inventories and outcomes. Source test links below identify durable contracts; the accepted evidence
records which compiler and inputs were actually tested. Deeper historical evidence is available through the
[archive guide](../../issue-nvvm-backend/HISTORY.md).

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

## Compute, values and memory

| Region                          | Qualified domain / evidence                                                                                                           | Boundaries and durable anchors                                                                                                                                                                                                                                                                                                                                                                                                           |
| ------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Direct PTX routing              | Opt-in direct route, target-scoped selector, explicit architecture; `case nvvm` refines CUDA for direct PTX                           | NVRTC remains default. [Compiler units](../../tools/slang-unit-test/unit-test-nvvm-compiler.cpp)                                                                                                                                                                                                                                                                                                                                         |
| Compute launch and builtins     | Conventional and raw compute entry contracts; explicit LLVM register reads for thread/block/grid dimensions; group barrier; GPU       | No graphics/OptiX entry claim. [Core execution](../../tests/cuda/nvvm-core-execution.slang), [ordinary entry](../../tests/cuda/nvvm-ordinary-compute-entry.slang)                                                                                                                                                                                                                                                                        |
| Scalar numeric values           | Selected signed/unsigned 8/16/32/64-bit integers, Bool and IEEE Half/Float/Double; typed transport/conversion and admitted operations | Exact operation descriptors govern overloads; type admission alone grants no operation. [Mixed numeric](../../tests/cuda/nvvm-mixed-numeric.slang), [Half](../../tests/cuda/nvvm-half-values.slang), [Double](../../tests/cuda/nvvm-float64-values.slang)                                                                                                                                                                                |
| Vectors                         | Selected widths 2–4; construction, extraction, swizzle updates, typed selection and operation families                                | Boolean lane references need local SSA normalization; arbitrary pointer escape remains rejected. [Typed select](../../tests/cuda/nvvm-typed-select.slang), [Boolean lanes](../../tests/cuda/nvvm-local-boolean-lanes.slang), [negative](../../tests/cuda/nvvm-boolean-lane-reference-unsupported.slang)                                                                                                                                  |
| Matrices                        | Selected logical numeric matrices lowered to aggregate values and physical storage; GPU and layout checks                             | Physical row/column layout and host packing remain explicit. [Float matrices](../../tests/cuda/nvvm-float-matrix-values.slang), column-major qualification below                                                                                                                                                                                                                                                                         |
| Helpers and control flow        | Typed calls/results, phi values, loops/switches, finite copyable records/arrays, noinline functions, selected mutable references      | Internal by-value ABI and exported CUDA ABI are separate. [Helper values](../../tests/cuda/nvvm-helper-copyable-values.slang), [mutable forwarding](../../tests/cuda/nvvm-mutable-parameter-forwarding.slang)                                                                                                                                                                                                                            |
| Pointer-bearing helper values   | Canonical device/UserPointer and qualified recursive helper transport                                                                 | Access/address-space/layout operands and producer provenance are checked. [Pointer forwarding](../../tests/cuda/nvvm-mutable-pointer-payload-forwarding.slang)                                                                                                                                                                                                                                                                           |
| Parameter groups and resources  | Selected conventional globals, uniforms, constant/parameter blocks, structured and byte-address buffers, resource-bearing aggregates  | Separate launch, parameter-group and structured-buffer representation; no universal aggregate ABI. [Multiple resources](../../tests/cuda/nvvm-conventional-global-multi-resource.slang), [compact storage](../../tests/cuda/nvvm-compact-vector-storage.slang)                                                                                                                                                                           |
| Borrowed float3 storage         | Readonly borrowed fields/array elements preserve native storage across mutable use; GPU and provider-memory regression                | Readonly access must not imply compact parameter-group storage. [Borrowed vector storage](../../tests/cuda/nvvm-borrowed-vector-storage.slang)                                                                                                                                                                                                                                                                                           |
| Shared/local memory and atomics | Selected finite shared storage, typed integer atomic families and admitted memory orders; unit/integration/corpus gates               | Every element type, pointer role and atomic overload still needs admission. [Emitter units](../../tools/slang-unit-test/unit-test-nvvm-emitter.cpp), [integration units](../../tools/slang-unit-test/unit-test-nvvm-integration.cpp)                                                                                                                                                                                                     |
| Thread-local context            | Selected per-invocation global/context values and finite copyable aggregates pass ordinary single-dispatch checks                     | Repeated dispatch of bare `static State state;` in the copyable-context fixture fails in all three modes: both routes leave its private aggregate uninitialized. The implicit-initialization language contract needs resolution. Does not imply arbitrary global initialization or host ABI. [Context](../../tests/cuda/nvvm-thread-local-global-context.slang), [copyable context](../../tests/cuda/nvvm-copyable-kernel-context.slang) |
| Receiver snapshots              | Canonical aggregate parameter snapshots preserve values across resource mutation; GPU and material compile evidence                   | Aggregate-memory ordering issue is fixed; material runtime is separately scoped below. [Snapshot](../../tests/cuda/nvvm-aggregate-param-snapshot.slang), [resource snapshot](../../tests/cuda/nvvm-aggregate-param-resource-snapshot.slang)                                                                                                                                                                                              |
| Nested integer aggregate stores | Nested records and root/wrapped/multidimensional arrays; NVVM O0/O3 GPU and 39 provider shape/alignment checks                        | Qualified libNVVM store workaround; focused NVRTC optimized copies still fail. [Root](../../tests/cuda/nvvm-nested-array-root.slang), [wrapper](../../tests/cuda/nvvm-nested-array-wrapped.slang), [multidimensional](../../tests/cuda/nvvm-nested-array-multidimensional.slang)                                                                                                                                                         |

Core execution helpers and varying-parameter legalization use named
`llvm.nvvm.read.ptx.sreg.*` intrinsics for all twelve thread/block/grid coordinates.
The named-intrinsic boundary admits zero-argument scalar i32 register reads and the three void
barrier/fence operations described below; it does not admit arbitrary LLVM assembly. Provider signature queries
remain pure and emission consumes the checked plan. CUDA source and NVRTC retain CUDA target
selection, including when an explicit `nvvm` capability is supplied. Ordinary comma-separated
`__intrinsic_asm` arguments remain supported. [Target/argument fixture](../../tests/cuda/nvvm-target-switch-explicit-args.slang)
and [varying composition](../../tests/cuda/nvvm-execution-register-varyings.slang) define the new contracts.

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
General structured-storage recursion and recursive ancestor admission remain outside this
boundary; no transforming Slang IR storage pass is claimed.

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

| Feature / role                                         | Current state and evidence                                                                                                                              | Explicit limits / regression                                                                                                                                                                                                                                                                                                                                                                                                               |
| ------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| BF16 scalar transport, literals, local mutable storage | Supported as physical i16; GPU includes all 65,536 bit encodings and signed literals                                                                    | Raw transport preserves NaN payloads; casts have separate policy. [Scalar](../../tests/cuda/nvvm-bf16-scalar.slang), [signed literals](../../tests/cuda/nvvm-bf16-signed-literals.slang)                                                                                                                                                                                                                                                   |
| BF16↔Float32                                           | Scalar and equal-width vector conversion; nearest-even narrowing and exact bit expansion on qualified SM80                                              | NaN narrowing classification only; no integer, Half or Double cast admission. [Scalar](../../tests/cuda/nvvm-bf16-scalar.slang), [vector values](../../tests/cuda/nvvm-bf16-vector-values.slang)                                                                                                                                                                                                                                           |
| BF2/BF3/BF4 register and internal helper values        | Supported `<N x i16>` construction/extraction and branch/phi transport; GPU                                                                             | No external CUDA helper ABI claim or inferred explicit vector Select contract. [Vector values](../../tests/cuda/nvvm-bf16-vector-values.slang)                                                                                                                                                                                                                                                                                             |
| BF2/BF3/BF4 local mutable references                   | BF2 native-vector storage; BF3/BF4 component arrays; symmetric raw lane conversion; GPU with both cache orders                                          | Does not grant component-pointer, device/resource, readonly or exported roles. [Local vectors](../../tests/cuda/nvvm-bf16-local-vectors.slang)                                                                                                                                                                                                                                                                                             |
| Flat local BF16 records including BF3/BF4              | Integer and BF16 scalar/vector fields; field load/store through proven local root; layout and exhaustive transport evidence                             | BF3/BF4-containing whole record values, nesting, arrays and external storage not admitted. [Local records](../../tests/cuda/nvvm-bf16-local-records.slang)                                                                                                                                                                                                                                                                                 |
| CUDA BF16 layout queries                               | Canonical scalar/vector/matrix/wrapper layout matches selected CUDA forms; reflection, IR and GPU query tests                                           | Layout metadata, including record-array queries, is not runtime memory admission. [Layout](../../tests/cuda/nvvm-bf16-cuda-layout.slang)                                                                                                                                                                                                                                                                                                   |
| BF16 dot widths 2–4                                    | Dedicated source-ordered separately rounded product/sum recipe; GPU                                                                                     | No unrestricted FP32 accumulation, general BF16 arithmetic/comparisons or width-0/1 contract. [Dot](../../tests/cuda/nvvm-bf16-dot.slang)                                                                                                                                                                                                                                                                                                  |
| FP8 scalar transport and finite literals               | E4M3/E5M2 i8 values, internal helpers, same-size bitcasts and same-format selection; GPU over all 256 encodings per format                              | Transported nonfinite bytes supported; nonfinite literals rejected. [Transport](../../tests/cuda/nvvm-fp8-scalar-transport.slang)                                                                                                                                                                                                                                                                                                          |
| FP8→Float32                                            | Both formats, exact finite values and E5M2 infinity; GPU over every byte                                                                                | NaN payload/sign unspecified; no native FP8 hardware required on SM80. [Widening](../../tests/cuda/nvvm-fp8-widening.slang)                                                                                                                                                                                                                                                                                                                |
| Shared finite FP8 folding                              | Repaired finite normal/subnormal conversion and nearest-even rounding; exhaustive independent unit evidence and source regression                       | Shared overflow policy differs from CUDA SATFINITE. [Finite literals](../../tests/cuda/nvvm-fp8-finite-literals.slang)                                                                                                                                                                                                                                                                                                                     |
| Float32→FP8 runtime narrowing                          | Research only; no backend admission                                                                                                                     | Prototype nearest-even/saturation evidence does not qualify production conversion. Integer/Half/Double conversions, FP8 arithmetic and vectors remain excluded.                                                                                                                                                                                                                                                                            |
| Flat/nested mixed record values                        | Integer leaves with FP8/BF16 scalar/BF2, at least one substandard descendant; local mutable storage and internal value transport supported              | No BF3/BF4 whole values or resource/shared/readonly/exported roles. [Flat](../../tests/cuda/nvvm-substandard-records.slang), [nested](../../tests/cuda/nvvm-nested-substandard-records.slang)                                                                                                                                                                                                                                              |
| BF2 record-field component address                     | Supported from actual field of a qualified local record, preserving exact pointer/index checks                                                          | Does not qualify BF3/BF4 component pointers or bare FP8 pointers. [Flat records](../../tests/cuda/nvvm-substandard-records.slang)                                                                                                                                                                                                                                                                                                          |
| Nested dynamic AnyValue payloads                       | Two runtime-selected conformers, Natural unpack/CUDA local/mutate/repack and saved interface copy; three-mode GPU                                       | Raw bit transport; no new arithmetic or arbitrary interface ABI. [Substandard](../../tests/cuda/nvvm-nested-substandard-dynamic.slang), [integer control](../../tests/cuda/nvvm-nested-integer-dynamic.slang)                                                                                                                                                                                                                              |
| Local mixed record arrays                              | Nonempty fixed arrays of existing integer/FP8/BF16/BF2 identity records; dynamic selection, field mutation and saved whole-array copies; three-mode GPU | Address roots require local Var; internal value parameters preserve snapshots. No reference/result/export ABI, wrapper fields, multidimensional arrays, BF3/BF4 elements or external storage. [Transport](../../tests/cuda/nvvm-local-substandard-record-arrays.slang), [snapshots](../../tests/cuda/nvvm-local-substandard-record-array-copies.slang), [value parameters](../../tests/cuda/nvvm-substandard-record-array-parameter.slang) |
| Local-record Generic pointer helper result             | Excluded                                                                                                                                                | Exact synthetic Generic-result boundary is source-reviewed; public UserPointer negative tests are a different shape.                                                                                                                                                                                                                                                                                                                       |

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
| Lane reads, ballots, votes, rotation and prefixes | Selected typed scalar/vector/aggregate recipes; GPU plus semantic/provider gates             | Exact type widths and mask participation govern legality. [Shuffle widths](../../tests/cuda/nvvm-wave-shuffle-widths.slang), [rotation](../../tests/cuda/nvvm-wave-rotation.slang)                                                           |
| Hardware active mask                              | Side-effecting convergent snapshot; implicit aggregate helpers compose the required ballot   | Distinct from logical active-mask synthesis; source branch membership need not imply reconvergence. [Hardware mask](../../tests/cuda/nvvm-hardware-active-mask.slang), [aggregate mask](../../tests/cuda/nvvm-implicit-aggregate-mask.slang) |
| Masked integer MIN/MAX                            | Narrow and 64-bit signed/unsigned identities and admitted prefix/reduction recipes           | Identity bits are width-specific; no invalid shift by 64. [Narrow](../../tests/cuda/nvvm-narrow-masked-minmax.slang), [64-bit](../../tests/cuda/nvvm-i64-masked-minmax.slang)                                                                |
| Masked Float/Double MIN/MAX                       | Source-ordered reduction and prefix rules, signed-zero/NaN/singleton behavior; GPU           | Ordinary numeric min/max descriptors are not interchangeable with ordered wave selection. [Float](../../tests/cuda/nvvm-fp32-minmax-order.slang), [Double prefix](../../tests/cuda/nvvm-fp64-prefix-minmax-order.slang)                      |
| Masked Half MIN/MAX                               | Separately qualified raw-half transport and ordered selection; finite exclusive seeds ±65504 | Infinity/NaN/signed-zero behavior follows exact seed and source order. Matrix prefixes remain separate. [Half](../../tests/cuda/nvvm-fp16-masked-minmax.slang)                                                                               |
| QuadAny/QuadAll                                   | Complete typed helper with matching signatures/requirements; four source-lane reads; GPU     | Complete source quads and matching shuffle sequences required; standalone requirement markers, partial quads and SPIR-V active-only semantics are not admitted. [Quad votes](../../tests/cuda/nvvm-quad-votes.slang)                         |
| Clock/clock64                                     | Live side-effecting per-SM cycle observations; relational GPU oracle                         | Wrapping counters, not wall time, cross-SM ordering or memory fence. Exact timestamp comparison is invalid. [Clock observations](../../tests/cuda/nvvm-clock-observations.slang)                                                             |

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

## Texture, surface and descriptor contracts

| Region                                  | Current contract / evidence                                                                                              | Limits / regression                                                                                                                                                                                                                                                                                           |
| --------------------------------------- | ------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Sampled textures                        | Selected exact CUDA texture helper families, including level operations and ordinary gather; compile/GPU corpus evidence | Full texture API and every shape/offset/format not implied. [Sample boundary](../../tests/cuda/nvvm-sampled-texture-unsupported.slang), [fetch boundary](../../tests/cuda/nvvm-texture-fetch-unsupported.slang)                                                                                               |
| Surfaces                                | Typed physical accesses; explicit static Half conversion; masks preserve untouched bits                                  | Byte-addressed access and element-count geometry differ. [Float surface](../../tests/cuda/nvvm-native-float-surface.slang), [formatted provenance](../../tests/cuda/nvvm-formatted-surface-provenance.slang), [integer boundary](../../tests/cuda/nvvm-native-integer-surface-unsupported.slang)              |
| Read-only texture descriptor conversion | Resource↔descriptor↔uint64 identity for accepted texture families; GPU handles and 64-bit transport                      | Buffer descriptors contain pointer/count and do not inherit integer-handle conversion. [Texture descriptors](../../tests/cuda/nvvm-texture-descriptor-conversion.slang), [buffer negative](../../tests/cuda/nvvm-texture-descriptor-buffer-unsupported.slang)                                                 |
| Selected non-mip dimensions             | Exact supported geometry helpers                                                                                         | Does not establish requested mip, array count or total/view level count. [Dimensions](../../tests/cuda/nvvm-texture-dimensions.slang), [query negatives](../../tests/cuda/nvvm-texture-query-unsupported.slang)                                                                                               |
| Undefined ordinary sampler              | Qualified CUDA placeholder semantics                                                                                     | Comparison samplers and arbitrary undefined resources remain excluded. [Sampler](../../tests/cuda/nvvm-undefined-sampler.slang), [comparison negative](../../tests/cuda/nvvm-undefined-comparison-sampler-unsupported.slang), [resource negative](../../tests/cuda/nvvm-undefined-resource-unsupported.slang) |

Texture research distinguishes base geometry, selected mip geometry, layer/cube count and total
allocated/view levels. The CUDA source producer currently loses some of those semantics. Agreement
with NVRTC can therefore preserve the same error. A texture object and a surface handle are not
interchangeable, and opaque descriptor bytes cannot be inspected as an undocumented metadata API.
The observed driver lookup failures for `txq.array_size` and `txq.num_mipmap_levels` are specific to
the qualified stack; `txq.level.width` loaded and executed. No full API repair is implemented.

NVVM surface legalization supports the existing native 32-bit signed/unsigned/Float32 scalar, two-
and four-channel transfers in admitted 1D/2D/3D and array shapes. Native Half and annotated Half
storage with Float32 shader values remain limited to non-array 1D/2D. Matching Float32 uses Float32
payloads directly. Static component masks update physical channels without re-encoding untouched
NaN payloads. Byte-X scaling and conversion are explicit IR; no runtime format discovery is added.

The [physical-storage harness](../../extras/validate-nvvm-surfaces.py) independently checks 83 cases,
including exhaustive Half loads, conversion boundaries, direct literals, written NaN classification,
nonzero guards and zero-boundary accesses. Native signed/unsigned32 cases exercise 1D/2D scalar,
two- and four-channel loads, whole stores and component stores with exact integer bits. Four mixed
cases bind eight separate native Float32/r16f/rg16f/rgba16f source/result resources, checking both copy
directions and untouched channels. All 81 supported cases pass in each NVVM mode; two dynamic-index
negatives remain per mode. NVRTC has 52 passes, 20 compile failures and 11 rounding mismatches.
The recurring checkpoint replays physical readbacks and compares all 249 obligations against the
[current baseline](../../issue-nvvm-backend/accepted-baseline.json), preserving known failures.
[Ordinary Half conversion](../../tests/cuda/nvvm-half-narrow-conversion.slang)
has a separate three-mode regression. Half stores use RN-even; this differs from the existing NVRTC
formatted store's observed truncation. NVRTC component source, dynamic component indexing, user
resource-helper format provenance, three-channel transfers and additional packed/normalized formats
remain outside this qualification. The pass does not add layered helper writes or general aliases.

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

| Region                                                                            | Evidence currently available                                                                                                             | Limit                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| --------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Dynamic dispatch                                                                  | Permanent simple and nested AnyValue records with live runtime selection/mutation                                                        | Concrete record roles and payload capacity remain bounded. [Original substandard case](../../tests/compute/dynamic-dispatch-substandard-float.slang), [nested regression](../../tests/cuda/nvvm-nested-substandard-dynamic.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| Errors and generic throwing calls                                                 | Runtime-dependent scalar/aggregate propagation, catches, generic-specialized witness path; permanent three-mode GPU tests                | Generic specialization is not an existential throwing-witness ABI. [Runtime errors](../../tests/cuda/nvvm-runtime-errors.slang), [throwing witness](../../tests/cuda/nvvm-runtime-throwing-witness.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| Inheritance and initialization                                                    | Focused unchanged-source three-mode qualification of struct inheritance, derived initializer lists, 16-bit defaults and scoped constants | Some expressions/calls/arrays can fold; no IR survival, arbitrary inheritance depth or nonintegral Half claim. [Inheritance](../../tests/language-feature/inheritance/struct-inheritance.slang), [derived initialization](../../tests/language-feature/inheritance/derived-struct-init-list.slang), [16-bit defaults](../../tests/language-feature/initializer-lists/default-init-16bit-types.slang), [constants](../../tests/language-feature/constants/static-const-in-struct.slang)                                                                                                                                                                                                                                                                                                                                       |
| Accessors, generics, variadics, constrained extensions, lambdas, tuples and defer | Focused unchanged-source experiments plus main-corpus selection                                                                          | Passing isolated examples does not establish all feature intersections. Selected membership and semantic coverage are different measurements.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| Material shaders                                                                  | Both registered entries × three modes compile/assemble and execute with two live synthetic textures and independent scalar oracles       | 65 active records/entry/mode and 63 untouched guards. Eval checks 260 components; sample checks 455 floats/65 flags, selected-layer throughput, coherent arithmetic candidates and exact rejection zeros. Fixed synthetic inputs only; no original assets, live LUTs or sampling-distribution claim. Device-event evidence has its own scoped row below. [Runtime validator](../../extras/validate-nvvm-material-runtime.py). [Material manifest](../../issue-nvvm-backend/complex-corpus.manifest.json)                                                                                                                                                                                                                                                                                                                     |
| Synthetic material device events                                                  | Both entries × two counts × three modes × two reversed rounds; 216 measured launches and 72 warmups pass complete output checks          | At N1,048,577, fresh NVRTC O3/NVVM O3 time ratios are 8.93–8.97 eval and 6.26–6.30 sample across the two rounds on the qualified L4 (pooled medians: 8.95×/6.29×). Tiny hot textures, periodic inputs and inter-launch correctness transfers only; no application performance or causal claim. Small NVVM O3 cells fail the 0.1 ms throughput gate. An earlier eval-only diagnostic, not rerun in this refresh, removes 65 proven never-read PTX stores: reassembly eliminates its stack and reduces NVRTC time about 8.15× with exact outputs. This is not a production pass or a sampling result. [Runner](../../extras/measure-nvvm-material-runtime.py), [CPU contracts](../../issue-nvvm-backend/test-nvvm-material-measurement.py), [protocol](../../issue-nvvm-backend/RESULTS.md#material-device-event-measurement). |

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
These checks preserve existing support boundaries; initial recursive ancestor admission remains.

## Current compilation observations

The current results refresh uses the accepted optimized compiler on an AMD EPYC 7R13 host with
four visible logical CPUs; one compiler process runs at a time. The standalone release cohort has
504 cases/500 sources and 76 explicit exclusions. All 1,512 variants compile and assemble; all
9,072 measured compilations and 3,024 warmups preserve the qualified PTX.

| Cohort    | Cases | NVRTC O3 / NVVM O3 | NVRTC O3 / NVVM O0 |
| --------- | ----: | -----------------: | -----------------: |
| Frozen    |   406 |              1.47× |              1.51× |
| Discovery |    98 |              1.41× |              1.49× |

These are geometric means of per-case median fresh-process time ratios, using six samples per cell
and warm caches. NVVM O3 has lower observed medians in 400/406 frozen and 94/98 discovery cases;
NVVM O0 does so in all selected cases. Equal `-g0` policy avoids asymmetric renderer debug options.
Original runtime contracts remain authoritative; timed release PTX is compile/assembly-qualified,
not separately GPU-qualified. No general application compilation or statistical-significance claim
follows. See [protocol](../../issue-nvvm-backend/RESULTS.md#standalone-corpus-compilation) and the
`compilation-performance` feature in [current evidence](../../issue-nvvm-backend/focused-evidence.json).

Material compilation uses 18 samples per cell. Eval medians are 1.381/1.376/1.292 seconds and
sample medians 1.401/1.473/1.336 seconds for NVRTC O3/NVVM O3/NVVM O0. Thus the broad-corpus gains
do not transfer uniformly to these larger entries. Assembly and inclusive Slang phases remain
separate measurements. The 12-shader quality subset supplies fresh static resources for 36 cells;
its single compile observations and code sizes are not GPU-performance evidence.

Original-input GPU dispatch measurements now attempt all 452 frozen and 128 discovery cases in
NVRTC O3, NVVM O3 and NVVM O0 with explicit `-g0`: two reversed rounds, three warmups and nine
samples per mode/round, reset resources before every launch, and original comparison oracles.
Complete three-mode comparisons cover 449 frozen and 117 discovery cases; 30,618 accepted samples
span 3,402 measured mode-round cells. The 78 exclusions retain 72 instances of the existing 36 gaps
and six new repeatability failures in the copyable-context fixture. The latter returns typed uint32
failure sentinels, not aggregate padding; both backends leave its bare static aggregate uninitialized.
The ordinary full checkpoint still has all 1,740 original outcomes unchanged.

Of 566 complete cases, 557 are below the prespecified 0.1 ms ratio cutoff in at least one mode.
Eight of the nine longer cases exercise wave/min/max and show 1.76–2.42× higher NVVM O3 intervals
than NVRTC O3; FP8 scalar transport is near parity at 1.03×. These are original correctness workloads,
including shader-side checks. CUDA events include RHI parameter upload and host enqueue gaps;
reset, readback, compilation and encoding are outside. Clocks are not locked. Short observations
are retained without ratios; the cutoff is policy, not calibrated resolution. No aggregate shader
throughput or application-performance claim follows. See the
[runner](../../extras/measure-nvvm-corpus-runtime.py),
[CPU contracts](../../issue-nvvm-backend/test-nvvm-corpus-runtime.py),
[harness contracts](../../extras/test-cuda-dispatch-profile.py),
[protocol](../../issue-nvvm-backend/RESULTS.md#original-input-corpus-dispatch-timing) and
`corpus-dispatch-performance` in [current evidence](../../issue-nvvm-backend/focused-evidence.json).

Fresh original-input code capture covers all 580 cases and three modes: 1,704 qualified cells,
36 retained gaps, and 567 complete O3/O3 comparisons. Offline SM89 assembly finds 194 cases
with identical bytes in every named executable section, 0 additional normalized-PTX matches,
46 similar static profiles, 327 different profiles, and 13 incomplete comparisons.
Similarity is a triage heuristic, not performance proof. NVVM uses fewer/equal/more hardware
registers in 103/402/62 paired cases.
The narrow masked min/max slowdown coexists with smaller PTX and fewer offline registers:
eligible-mask fast paths and aggregate traversal differ. Floating min/max already uses trees,
but retains mode selection within loops and repeated component traversal. Interface-return dispatch
shows a separate tag/control-flow simplification opportunity (33→94 PTX, 40→104 SASS instructions);
its short fixture does not support a runtime regression claim. Static metrics use fresh captures
and offline ptxas, not historical timed PTX or recorded driver-JIT machine code. No new timings,
compiler changes or broader feature qualification are implied.

See the [capture tool](../../extras/capture-nvvm-corpus-code.py),
[analyzer](../../extras/analyze-nvvm-corpus-code.py),
[protocol](../../issue-nvvm-backend/RESULTS.md#original-input-corpus-code-quality) and
`corpus-code-quality` in [current evidence](../../issue-nvvm-backend/focused-evidence.json).
The current presentation includes every case, resource/instruction tables and six source/PTX/SASS
case studies. Existing semantic qualifications and failure histories remain unchanged.

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
  value-array parameters now pass a separate two-cell NVVM qualification. Source array returns become
  OutParam and remain unsupported, as do reference roles and guarded wrappers. Prior integer controls, corruption rejection
  and failure history retain their original identities. Two-element fixtures over 65,536 bit patterns
  do not establish large-array scalability.
- The result classifier previously mistook `nvrtc` in a source path for an NVRTC compilation
  diagnostic. The parser now requires a downstream compiler identity prefix and accepts the actual
  `error :` producer form. The original failed attempt/adjudication remains in focused evidence;
  its output mismatch never becomes a passing shader.
- Toolkit/architecture compile-and-assembly matrices are separate from physical-device execution.
  Historical CUDA 12.9 SM70/80/90 and CUDA 13 SM80/90 results are not fresh qualification of every
  later feature. The manually defined container workflow was not dispatched in that evidence.
- OptiX, source-level debug support, relocatable device code, device LTO, dynamic parallelism and
  device syscalls remain separate tracks. No general cooperative or autodiff support claim follows.

Further feature work should select a coherent role/operation combination and preserve neighboring
negative contracts. A diagnostic advancing to another unsupported instruction is research progress,
not implemented support. New evidence updates this matrix and permanent tests where appropriate;
chronological plans, reports and repeated raw outputs belong in working artifacts or Git history.
