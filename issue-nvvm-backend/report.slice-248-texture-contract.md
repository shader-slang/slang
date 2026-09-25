# Slice 248: Qualify the texture-dimensions API gap

Status: independently accepted research. No compiler, library, provider, runner,
registered fixture, input, oracle, or ABI change.

## Motivation

The remaining unqualified NVRTC wrong-output case is
`compute/texture-get-dimensions.slang#discovery-1`. Consider its 2D-array branch:

```slang
//TEST_INPUT: Texture2D(size=4, content=one, arrayLength=3):name t2DArray
Texture2DArray<float> t2DArray;
//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint width, height, elements, mipWidth, mipHeight, levels;
    t2DArray.GetDimensions(width, height, elements);
    t2DArray.GetDimensions(0, mipWidth, mipHeight, elements, levels);
    uint packed = (width << 24) | (height << 16) | (elements << 8) | levels;
    outputBuffer[0] = packed;
}
```

The fixture allocates a full mip chain: 4x4, 2x2, 1x1, each with three layers. Its API expectation
is width4, height4, elements3, levels3. Slang's CUDA helper returns width4, height4, elements0,
levels0. Conversion of the packed integer to float produces `4C808060` versus `4C808000`.
The original test has seven resources and an idle eighth lane; all eight expected bit patterns
were independently reconstructed without editing the original oracle.

## Proposed solution

Complete a bounded research qualification. Preserve the one runtime mismatch and two direct NVVM
preflight failures. Do not admit the mip GenericAsm merely to reproduce the same incomplete CUDA
behavior. Document the actual producer, binding and driver boundaries before choosing an API fix.
Supported nonzero-LOD width queries and several shape-specific layer queries mean a blanket claim
that CUDA cannot supply any of these values would be incorrect. Total mip count still needs an
explicit contract; base geometry cannot distinguish complete and partial chains.

## Change summary

- Completed plan and this report record scope, exact observations, ownership and next gates.
- `semantic-evidence.slice-248.json` records original outcomes, immutable246 histories, identities,
  fresh controls and hash references to local raw evidence.
- `docs/design/nvvm-texture-query-contract.md` records the durable API/resource/query distinctions;
  the backend design links to it. STATUS queues a bounded follow-up without claiming support.
- Research-only scripts, uint-output shaders, CUDA probes, PTX/cubins, official documentation,
  primary source snapshots and failed attempts remain under ignored `build/nvvm-loop/slice-248-*`.

## Concepts and vocabulary

**Texture object** is an opaque CUDA handle containing resource/sampler/view associations. A
**surface object** references one CUDA array, including a selected array from a mip chain; it is
not the handle bound for the original read-only textures. **Mip count** is the allocated/view level
count, distinct from base size, requested LOD and sampler clamp. **Cube count** differs from the
number of physical faces. **Packed float oracle** means the fixture converts the packed uint
numerically to Float32; it does not bitcast it.

## Process report

`InputTextureDesc::mipMapCount` defaults to zero, and `_createTexture` in shader-input-layout.cpp
interprets zero as `floor(log2(size))+1`. `ShaderRendererUtil::createTexture` forwards this count
and chooses array resource kinds only when `arrayLength > 1`. Thus the seven original resources
have counts3,4,2,5,2,3,5; the source's `TextureCubeArray` with arrayLength1 is actually bound to a
nonlayered cube resource. That distinction belongs to the fixture/resource binding contract and
must not be concealed by a future array-count helper.

The active implementation is external/slang-rhi, not the legacy tools/gfx CUDA implementation.
`DeviceImpl::createTexture` constructs the corresponding CUDA arrays/mipmapped arrays. Its cube
array path allocates six faces per cube. `TextureImpl::getTexObject` uses an array or mipmapped-array
resource descriptor and attaches a resource-view descriptor only for a partial view. The original
read-only bindings in cuda-shader-object.cpp obtain a texture handle. By contrast,
`TextureImpl::getSurfObject` obtains `range.mip` with `cuMipmappedArrayGetLevel` and creates a surface
from that single array. These facts are source trace evidence, not runtime instrumentation of RHI.

`TextureTypeInfo::writeGetDimensionFunctions` intentionally generates a mip overload containing
base `txq.width/height/depth` queries, shifts the output argument positions to account for mipLevel,
but never consumes that mip argument. It writes zero for array size and total levels. This valid
standard-library GenericAsm shape is consumed by NVRTC as written. The problem is incomplete API
semantics at that producer/resource contract, not malformed IR or wrong integer arithmetic.
`_resolveNVVMTextureDimensionsGenericAsm` admits exact non-mip forms and deliberately has no mip
form. The first original direct diagnostic is the 1D mip helper with signature
`Void(Texture1D,uint,OutParam<uint>,OutParam<uint>)`. O0 and O3 retain that exact old diagnostic.
Its plain 1D-array form is also independently unsupported; two isolated negative controls retain
that minimal handoff without expanding this slice.

The original eight outputs independently match the checked-in expected file when packing the API
values. Removing only mip counts yields the same Float32 bits for all seven active lanes because
rounding discards those low bits. Removing layer counts changes only lanes4,5,6, exactly the fresh
NVRTC mismatch. Independent uint-output controls make every field visible: mip0 and mip1 both
return base dimensions and zeros for counts. Explicit one-, two-, and four-level 8x8 resources all
return `[8,8,0]`, while their API count expectations differ. No graphics-oracle result is relabeled
a pass.

The standalone CUDA probe creates nine independent objects with these geometries and a surface
load/store allocation flag so it can also test surfaces. Host `cudaGetTextureObjectResourceDesc`,
`cudaGetMipmappedArrayLevel` and `cudaArrayGetInfo` confirm allocated level sizes and layer/face
counts, including a partial two-level8x8 chain. The first absent level returns invalid-value.
These probes validate the external object contract; they do not claim to inspect the live test
harness's objects. All nine `txq.level.width` executions return the actual level1 widths despite
sampler max-mip-clamp0. Base `txq.height` reports two layers for a 1D array; `txq.depth` reports three
layers for a 2D array and two cubes for twelve cubemap-array faces. Nonlayered cube depth is zero.
A level1 surface's width query returns elements, not byte offsets; surface load/store addressing
and query units must not be conflated.

Three isolated PTX kernels use the same `probe` entry, parameters and toolchain. CUDA12.9 ptxas
assembles array_size, num_mipmap_levels and level.width. All cubins contain that global function
symbol, and module loading succeeds. On this driver, function lookup for the first two returns
CUDA_ERROR_NOT_FOUND(500), whereas level.width lookup/launch succeeds. Both failed cubins also
contain undefined weak `.nv.unified.texrefDescSize`, absent from the passing cubin. This is an
observed runtime availability boundary with correlated symbol evidence, not proof that these
instructions are universally unsupported or a claim establishing the driver's internal cause.
The PTX specification's presence of an instruction alone does not prove CUDA-runtime availability.

No production helper/fallback/special case is added. Research controls survive only as local
evidence. A new equivalence, guessed count from width, zero-value fallback, opaque-handle pointer
walk, or direct-only recognition that claims full GetDimensions support would be unprincipled.
The semantic source of truth for allocated/view count is resource/view metadata; existing device
queries can serve the geometry portions. A future producer fix should use explicit semantics
shared by CUDA and direct NVVM, and settle single-cube/array binding plus partial views before
claiming full support. No speculative texture metadata ABI is introduced here.

Validation uses the unchanged246 RelWithDebInfo compiler/library/provider and ABI40 on L4SM89,
SM80 target, CUDA12.9.2/NVRTC12.9.86, driver580.126.09. Source base is
`7d8cd100aaf29d2334876655fc7c8eee0249a691`; all117 source,12 artifact and561 runtime-input hashes
match246 before/after. Compiler/library/provider hashes are retained in the compact evidence.

| Evidence                                 | Fresh result                                                               |
| ---------------------------------------- | -------------------------------------------------------------------------- |
| Original discovery identity, three modes | Exact one mismatch/two preflight outcomes preserved                        |
| Supplemental Slang controls              | 12 executions,246 expected CUDA output values; two expected 1D-array stops |
| Accepted texture-dimensions fixture      | All three modes included in those12 executions                             |
| Trace compiles/assemblies                | Five/five pass                                                             |
| Independent CUDA objects                 | Nine host/geometry/LOD controls;18 unavailable query lookups retained      |
| Isolated PTX query assemblies            | Three pass; symbols verified                                               |
| Small runtime gate                       | Four pass                                                                  |

Full246 remains1695 cells:1654 correct and41 unresolved, with16 resolved histories unchanged.
Only three registered classifier rows are freshly replayed;1692 remain inherited. Supplemental
accepted-fixture executions are not a replacement full census. No full checkpoint, implementation
cadence increment, new registered ID or support unlock. Latest targeted233, full246, cadence0.
Compiler units, semantic regressions, toolkit and material support evidence are inherited246.
The compile-time macro-shadowing failure, combined unavailable-query kernel failure, and corrected
surface-field label are retained. No device loss, driver/system change, reboot or push occurred.

Parent acceptance independently reconstructs the original Float32 oracle and all246 current-helper
uint outputs; verifies nine mip-level tables,27 base-query values, nine LOD values and four surface
values; confirms all original outcomes and41/16 histories unchanged; and checks158 indexed files,
16 source snapshots and171 compact references before its own two audit references. The worker
commentary count of five surface values was corrected to four from the raw records. No raw output
changed. Source/binary/input identities remain exact246. See parent-audit.py/json.
