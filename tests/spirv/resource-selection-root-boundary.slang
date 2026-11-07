//TEST:SIMPLE(filecheck=GLOBAL): -target spirv-asm -stage fragment -entry globalRoot -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLOBAL): -target spirv-asm -stage fragment -entry globalRoot -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=PARAM): -target spirv-asm -stage fragment -entry parameterRoot -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=PARAM): -target spirv-asm -stage fragment -entry parameterRoot -validate-ir -validate-ir-detailed

// We sample one texture with a selected sampler and separately copy that texture in assembly.
// The selection pass retains resource roots, so that assembly operation needs no reconstruction
// and must not prevent the sampler selection from being legalized. We also copy a sampler
// parameter of an ordinary function to exercise a retained parameter root.
// GLOBAL: %[[IMAGE:[^ ]+]] = OpTypeImage
// GLOBAL: %[[COPIED:[^ ]+]] = OpCopyObject %[[IMAGE]]{{( |$)}}
// GLOBAL: OpBranchConditional
// GLOBAL: OpImageSampleExplicitLod
// GLOBAL: OpImageSampleExplicitLod
// GLOBAL: OpImageFetch {{[^ ]+}} %[[COPIED]]{{( |$)}}
// PARAM: %[[SAMPLER:[^ ]+]] = OpTypeSampler
// PARAM: %[[INPUT:[^ ]+]] = OpFunctionParameter %[[SAMPLER]]{{( |$)}}
// PARAM: %[[COPIED:[^ ]+]] = OpCopyObject %[[SAMPLER]] %[[INPUT]]{{( |$)}}
// PARAM: OpBranchConditional
// PARAM: OpImageSampleExplicitLod
// PARAM: OpImageSampleExplicitLod
// PARAM: %[[COMBINED:[^ ]+]] = OpSampledImage {{[^ ]+}} {{[^ ]+}} %[[COPIED]]{{( |$)}}
// PARAM: OpImageSampleExplicitLod {{[^ ]+}} %[[COMBINED]]{{( |$)}}

Texture2D<float4> texture;
SamplerState samplerA;
SamplerState samplerB;

// The selected sample and the independent assembly copy use the same global texture binding.
// Only the selected sample needs rewriting; the final fetch keeps the assembly copy live.
float4 globalRoot(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> copied = spirv_asm
    {
        result : $$Texture2D<float4> = OpCopyObject $texture;
    };
    SamplerState selected = select(uv.x > 0.5, samplerA, samplerB);
    return texture.SampleLevel(selected, uv, 0) + copied.Load(int3(0));
}

// The sampler parameters remain available throughout this function. The selected sample and
// the independent assembly copy use `firstSampler`, so only the selected sample needs rewriting.
[noinline]
float4 sampleWithParameterRoots(
    SamplerState firstSampler,
    SamplerState secondSampler,
    float2 uv)
{
    SamplerState copied = spirv_asm
    {
        result : $$SamplerState = OpCopyObject $firstSampler;
    };
    SamplerState selected = select(uv.x > 0.5, firstSampler, secondSampler);
    return texture.SampleLevel(selected, uv, 0) + texture.SampleLevel(copied, uv, 1);
}

float4 parameterRoot(float2 uv : TEXCOORD) : SV_Target
{
    return sampleWithParameterRoots(samplerA, samplerB, uv);
}
