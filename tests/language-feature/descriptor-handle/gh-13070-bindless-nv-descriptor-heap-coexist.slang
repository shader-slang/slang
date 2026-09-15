// Verifies that a texture/sampler-family `DescriptorHandle` uses the native `spvBindlessTextureNV`
// conversion (`OpConvertUTo{Image,Sampler,SampledImage}NV`) whenever that capability is enabled —
// including when `spvDescriptorHeapEXT` is also enabled, in which case the non-family kinds (here a
// constant buffer) are still loaded from the EXT descriptor heap. See shader-slang/slang#13070.
//
// The BOTH run covers four representative NV-encodable kinds. CombinedTextureSampler (Sampler1DShadow)
// is the key case: it has a dedicated competing branch in the EXT arm, so a regression that routed it
// to the heap while leaving Texture/Sampler native would otherwise slip through. The texture and the
// texel buffer both convert via `OpConvertUToImageNV`, so each configuration requires two distinct
// occurrences — otherwise one of them could regress to the heap unnoticed.

//TEST:SIMPLE(filecheck=NV):   -target spirv-asm -capability spvBindlessTextureNV -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=EXT):  -target spirv-asm -capability spvDescriptorHeapEXT -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=BOTH): -target spirv-asm -capability spvBindlessTextureNV+spvDescriptorHeapEXT -stage compute -entry computeMain

struct Params { float scale; }

[[vk::binding(0)]] RWStructuredBuffer<float> outputBuffer;

uniform Texture2D<float>.Handle texture;
uniform SamplerState.Handle sampler;
uniform Sampler1DShadow.Handle combined;
uniform Buffer<float>.Handle texelBuffer;
uniform ConstantBuffer<Params>.Handle params;

[shader("compute")]
[numthreads(8, 8, 1)]
void computeMain()
{
    float sum = texture.SampleLevel(sampler, float2(0.0f, 0.0f), 0.0f);
    sum += combined.Sample(0);
    sum += texelBuffer[0];
    outputBuffer[0] = sum * (*params).scale;
}

// spvBindlessTextureNV alone: every NV-encodable kind uses its native conversion (two image
// conversions: the texture and the texel buffer).
//NV-DAG: OpConvertUToImageNV
//NV-DAG: OpConvertUToImageNV
//NV-DAG: OpConvertUToSamplerNV
//NV-DAG: OpConvertUToSampledImageNV

// spvDescriptorHeapEXT alone: every kind comes from the EXT descriptor heap; no native NV conversion is
// emitted. The `-NOT`s precede the positive check on the *last* descriptor load (the constant buffer's
// access into `%Params_std140`), so their range spans the whole shader body rather than only the region
// before the first heap access.
//EXT-NOT: OpConvertUToImageNV
//EXT-NOT: OpConvertUToSamplerNV
//EXT-NOT: OpConvertUToSampledImageNV
//EXT: OpUntypedAccessChainKHR {{.*}}%Params_std140

// Both enabled (the regression): the whole NV-encodable family keeps its native conversions — the two
// image conversions (texture + texel buffer), the sampler, and the combined texture-sampler — while the
// constant buffer is still loaded from the EXT resource heap (its descriptor is fetched by an untyped
// access chain into the variable decorated `BuiltIn ResourceHeapEXT`). The native conversions and the
// EXT heap access coexist in one module.
//BOTH-DAG: OpConvertUToImageNV
//BOTH-DAG: OpConvertUToImageNV
//BOTH-DAG: OpConvertUToSamplerNV
//BOTH-DAG: OpConvertUToSampledImageNV
//BOTH-DAG: OpDecorate %slang_resourceHeap BuiltIn ResourceHeapEXT
//BOTH-DAG: OpUntypedAccessChainKHR {{.*}}%slang_resourceHeap
