// Verifies that a texture/sampler-family `DescriptorHandle` uses the native `spvBindlessTextureNV`
// conversion (`OpConvertUTo{Image,Sampler,SampledImage}NV`) whenever that capability is enabled —
// including when `spvDescriptorHeapEXT` is also enabled, in which case the non-family kinds (here a
// constant buffer) are still loaded from the EXT descriptor heap. See shader-slang/slang#13070.
//
// The BOTH run covers five NV-encodable types, one per encodable descriptor-kind value:
// `Texture2D`, `Buffer`, `RWBuffer`, `SamplerState`, and the combined `Sampler1DShadow`.
// Encodability is a property of the type, not the descriptor kind: `SubpassInput` and
// `TextureBuffer` share these kinds yet are not encodable. The
// combined texture-sampler is the key case: it has a dedicated competing branch in the EXT
// arm, so a regression that routed it to the heap while leaving Texture/Sampler native would
// otherwise slip through. Texture, the uniform texel buffer, and the storage texel buffer all
// convert via `OpConvertUToImageNV`, so each configuration requires three distinct
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
uniform RWBuffer<float>.Handle storageTexelBuffer;
uniform ConstantBuffer<Params>.Handle params;

[shader("compute")]
[numthreads(8, 8, 1)]
void computeMain()
{
    float sum = texture.SampleLevel(sampler, float2(0.0f, 0.0f), 0.0f);
    sum += combined.Sample(0);
    sum += texelBuffer[0];
    sum += storageTexelBuffer[0];
    outputBuffer[0] = sum * (*params).scale;
}

// spvBindlessTextureNV alone: every NV-encodable type uses its native conversion (three image
// conversions: the texture, the uniform texel buffer, and the storage texel buffer).
//NV-DAG: OpConvertUToImageNV
//NV-DAG: OpConvertUToImageNV
//NV-DAG: OpConvertUToImageNV
//NV-DAG: OpConvertUToSamplerNV
//NV-DAG: OpConvertUToSampledImageNV

// spvDescriptorHeapEXT alone: no native NV conversion is emitted anywhere. The `OpEntryPoint`
// anchor precedes every function body (where any `OpConvertUTo*NV` would appear), and the `-NOT`s
// have no following positive check, so their range runs to end-of-module: no NV conversion appears
// anywhere in the shader, not merely before the first heap access.
//EXT: OpEntryPoint GLCompute
//EXT-NOT: OpConvertUToImageNV
//EXT-NOT: OpConvertUToSamplerNV
//EXT-NOT: OpConvertUToSampledImageNV

// Both enabled (the regression): all five NV-encodable types keep their native conversions — the
// three image conversions (texture + uniform texel buffer + storage texel buffer), the sampler,
// and the combined texture-sampler — while the constant buffer is still loaded from the EXT
// resource heap (its descriptor is fetched by an untyped access chain into the variable decorated
// `BuiltIn ResourceHeapEXT`). The native conversions and the EXT heap access coexist in one module.
//BOTH-DAG: OpConvertUToImageNV
//BOTH-DAG: OpConvertUToImageNV
//BOTH-DAG: OpConvertUToImageNV
//BOTH-DAG: OpConvertUToSamplerNV
//BOTH-DAG: OpConvertUToSampledImageNV
//BOTH-DAG: OpDecorate %slang_resourceHeap BuiltIn ResourceHeapEXT
//BOTH-DAG: OpUntypedAccessChainKHR {{.*}}%slang_resourceHeap
