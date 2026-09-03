# Slice 192: Preserve canonical implicit texture-sample semantics

## Motivation

Consider the frozen-v1 workload:

```slang
Texture2D<float> texture;
SamplerState sampler;
outputBuffer[tid] = texture.Sample(sampler, uv);
```

Its native CUDA reference compiled and ran correctly, but both direct modes stopped at the first
finalized helper:

```text
GenericAsm assembly=tex2D<$T0>($0, ($2).x, ($2).y),
signature=float(Texture2D, SamplerState, vector<float,2>)
```

The provider already had a descriptor-based texture interface and explicit-level sampling. The
missing invariant was producer-owned identity for ordinary implicit sampling; recovering it from
seven possible CUDA spellings would have extended the text-based bring-up architecture that Slice
182 began removing.

## Proposed solution

Tag every ordinary no-offset CUDA `Texture.Sample(sampler, coordinate)` producer with one internal
`TextureSample` semantic. Preserve existing generic value-operation IDs unchanged and allocate
rich-family semantics after that fixed range. NVVM legalization then emits an `IRNVVMIntrinsic`
terminator and discards the CUDA spelling.

Preflight accepts only the exact three-parameter ordinary helper. It checks the selected texture
and sampler types, derives shape, arrayness, Float32 element lanes, and coordinate width from the
texture type, and records one typed texture requirement. Provider ABI revision 35 appends
`SLANG_NVVM_TEXTURE_OP_SAMPLE` to the existing texture descriptor and callback. The LLVM 14
provider selects the corresponding implicit unified texture intrinsic and returns the requested
one, two, or four lanes.

## Change summary

- Added a shared internal semantic namespace for rich NVVM intrinsic families and tagged all seven
  ordinary implicit-sample producer branches in `hlsl.meta.slang`.
- Added exact tagged-helper classification and stored its resolved texture/coordinate parameters in
  the preflight requirement consumed by emission.
- Advanced the forward-only provider ABI to revision 35 and added typed implicit sampling to the
  facade, fake provider, and isolated LLVM 14 provider.
- Added real-provider query/emission coverage and compiler/fake-provider coverage proving semantic,
  not textual, selection.
- Promoted `cuda-texture.slang` to permanent O0/O3 differential coverage and corrected the stale
  neighboring negative test to the genuinely unsupported Float3 result shape.
- Regenerated separate frozen/discovery census and Pareto artifacts and added a representative
  measurement manifest.

## Concepts and vocabulary

**Implicit sample** selects a mip level using implicit derivatives rather than an explicit level.
**Producer semantic** is the compiler-internal operation identity attached where the standard
module creates target intrinsic assembly. **Selected texture type** is the finalized canonical IR
resource type after target specialization; it owns shape, arrayness, element type, and coordinate
width. **Unified texture intrinsic** is LLVM/NVVM's texture-object form, which takes one CUDA
texture handle rather than separate texture and sampler objects.

## Process report

`StmtLoweringVisitor::visitIntrinsicAsmStmt` already turns a tagged intrinsic-assembly statement
into `IRGenericAsm` plus `IRNVVMSemanticDecoration`. `_legalizeNVVMSemanticIntrinsics` consumes that
decoration and creates `IRNVVMIntrinsic(semantic-id)` without copying the string operand. Slice 192
extends the internal ID namespace instead of pretending texture sampling is a provider
`SlangNVVMValueOperation`: ordinary value IDs remain numerically identical, while
`TextureSample` starts at `SLANG_NVVM_VALUE_OPERATION_COUNT` and routes through the texture
interface.

The exact shape reaching `_resolveNVVMTaggedTextureSample` is the complete, one-block helper
produced by the ordinary `_Texture.Sample(SamplerState, location)` CUDA target case. It has result
`T` and parameters `(Texture, SamplerState, coordinate)`. This is canonical and intentionally
allowed. The selected `NVVMReadOnlyTextureType` is already the semantic source of truth, so the
resolver does not rebuild syntax or inspect assembly. It verifies that the helper result equals the
texture element, the sampler is selected, Float32 lane count is one, two, or four, and coordinate
lanes equal the selected texture contract. Removing the producer tag restores the exact frozen
GenericAsm failure recorded by Slice 191; the new compiler unit succeeds only because the tagged IR
routes to the texture requirement.

CUDA's texture object already includes sampling state. Consequently emission lowers the recorded
texture and coordinate parameters while deliberately omitting the checked sampler. The provider's
existing coordinate packing orders an array layer before ordinary floating coordinates, matching
the LLVM intrinsic signatures. `SAMPLE` differs from `SAMPLE_LEVEL` only in operand count and
intrinsic selection; no provider callback or duplicate resource representation was introduced.
The real-provider test serializes both implicit and explicit-level calls and checks their distinct
intrinsic names.

The first implementation probe also tagged the combined `Sampler2D.Sample(location)` producer.
That shape fails earlier at `helper function parameter: Sampler2D`; direct type lowering does not
yet select the combined resource. It was removed from this slice rather than adding an unproved
resource widening. The retained seven tags are the complete shape switch for the ordinary
texture/sampler producer, and provider query coverage checks 1D, 2D, 3D, cube, and every valid
array form.

The previous negative fixture used `Texture2D<float2>.SampleLevel` while claiming every vector
result was unsupported. Existing compiler and provider contracts already accept two and four
lanes, so that test produced no diagnostic. It now uses `float3`, the exact neighboring shape the
LLVM four-result intrinsic cannot project without inventing or truncating a lane, and checks the
complete producer/signature diagnostic.

The self-review inventory contains the internal semantic constant, seven producer tags, exact
tagged resolver, three stored parameter pointers, provider operation/ABI revision, implicit
intrinsic selector, and fake-provider/test updates. All survive. The internal ID prevents collision
with generic operations; the tags are the source-side invariant; the resolver proves the complete
canonical ABI; the stored pointers prevent emission from reclassifying it; the provider operation
is required because generic builder operations cannot express an NVVM texture intrinsic. No
fixture-name check, text fallback, malformed-IR patch, syntax reconstruction, or compatibility path
was added.

Frozen corpus v1 retains exactly 452 identities and its 427 healthy-MVP denominator. It advances
from 420 to 421 correct at O0, O3, and both, with `cuda/cuda-texture.slang#cuda-1` as the only gain
and zero old-correct regressions. All-row direct totals are 435 correct, 16 preflight failures, and
one infrastructure failure per mode. The frozen generic-asm-texture cluster disappears. Its six
remaining healthy gaps are three substandard helper-ABI types, two identical Half2 atomic-reduce
helpers, and one live `RequirePrelude` marker.

The separate discovery corpus retains exactly 82 identities and 72 healthy references. It remains
72/72/72; classifications stay 72 correct, seven infrastructure, one runtime mismatch, and two
preflight per direct mode. The selected unit prefix passes 438/438, and permanent NVVM coverage
passes 98/98.

The measurement gate compiled and assembled native NVRTC, direct O0 SM70, and direct O3
SM70/SM80/SM90 outputs. Median standalone compilation was 353.5 ms native, 239.1 ms direct O0
SM70, and 239.0 ms direct O3 SM70. PTX sizes were 8,810, 4,648, and 871 bytes respectively; direct
O3 remained 871 bytes at SM80 and SM90. These are exploratory measurements rather than controlled
benchmarks.
