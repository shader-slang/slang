// Check that a resource choice that also reaches an access through a never-written texture or
// sampler compiles to SPIR-V. Resource selection must eliminate the chosen sampler, texture or
// combined texture-sampler without an opaque-typed local variable, and the access through the
// never-written handle must not leave an `OpUndef` of the handle's type.

//TEST:SIMPLE(filecheck=SAMPLER):-target spirv -entry selectedSampler -stage compute -Wno-41016 -validate-ir
//TEST:SIMPLE(filecheck=SAMPLER):-target spirv -entry selectedSampler -stage compute -Wno-41016 -validate-ir -O0
//TEST:SIMPLE(filecheck=SAMPLER):-target spirv -entry selectedSampler -stage compute -Wno-41016 -validate-ir -g
//TEST:SIMPLE(filecheck=NO-UNDEF):-target spirv -entry selectedSampler -stage compute -Wno-41016
//TEST:SIMPLE(filecheck=TEXTURE):-target spirv -entry selectedTexture -stage compute -Wno-41016 -validate-ir
//TEST:SIMPLE(filecheck=TEXTURE):-target spirv -entry selectedTexture -stage compute -Wno-41016 -validate-ir -O0
//TEST:SIMPLE(filecheck=NO-UNDEF):-target spirv -entry selectedTexture -stage compute -Wno-41016
//TEST:SIMPLE(filecheck=PAIR):-target spirv -entry selectedPairInBranch -stage compute -Wno-41016 -validate-ir
//TEST:SIMPLE(filecheck=PAIR):-target spirv -entry selectedPairInBranch -stage compute -Wno-41016 -validate-ir -O0
//TEST:SIMPLE(filecheck=NO-UNDEF):-target spirv -entry selectedPairInBranch -stage compute -Wno-41016

// NO-UNDEF-DAG: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 1
// NO-UNDEF-DAG: %[[SMP:[A-Za-z0-9_]+]] = OpTypeSampler
// NO-UNDEF-NOT: OpUndef %[[IMG]]
// NO-UNDEF-NOT: OpUndef %[[SMP]]

Texture2D<float4> textureA;
Texture2D<float4> textureB;
SamplerState samplerA;
SamplerState samplerB;
RWStructuredBuffer<float4> outputBuffer;

// The sampler is chosen and used for a sample through `textureA` and a sample through a texture
// that was never written. Only the sample through `textureA` remains, executed with each sampler on
// its own path.
//
// SAMPLER: OpFunction
// SAMPLER-DAG: OpLoad %{{[A-Za-z0-9_]+}} %samplerA
// SAMPLER-DAG: OpLoad %{{[A-Za-z0-9_]+}} %samplerB
// SAMPLER-DAG: OpImageSampleExplicitLod
// SAMPLER-DAG: OpImageSampleExplicitLod
// SAMPLER: OpFunctionEnd
[numthreads(1, 1, 1)]
void selectedSampler(uint3 id: SV_DispatchThreadID)
{
    float2 uv = float2(id.xy) * 0.5;
    Texture2D<float4> missingTexture;
    SamplerState s = id.x != 0 ? samplerA : samplerB;
    float4 good = textureA.SampleLevel(s, uv, 0);
    float4 bad = missingTexture.SampleLevel(s, uv, 0);
    outputBuffer[0] = good + bad;
}

// The texture is chosen and sampled once with `samplerA` and once with a sampler that was never
// written. Only the sample with `samplerA` remains, executed with each texture on its own path.
//
// TEXTURE: OpFunction
// TEXTURE-DAG: OpLoad %{{[A-Za-z0-9_]+}} %textureA
// TEXTURE-DAG: OpLoad %{{[A-Za-z0-9_]+}} %textureB
// TEXTURE-DAG: OpImageSampleExplicitLod
// TEXTURE-DAG: OpImageSampleExplicitLod
// TEXTURE: OpFunctionEnd
[numthreads(1, 1, 1)]
void selectedTexture(uint3 id: SV_DispatchThreadID)
{
    float2 uv = float2(id.xy) * 0.5;
    SamplerState missingSampler;
    Texture2D<float4> t = id.x != 0 ? textureA : textureB;
    float4 good = t.SampleLevel(samplerA, uv, 0);
    float4 bad = t.SampleLevel(missingSampler, uv, 0);
    outputBuffer[0] = good + bad;
}

// The never-written texture is read inside one branch, which does not dominate the sample after
// the merge. Only the path that combines `textureB` samples.
//
// PAIR: OpFunction
// PAIR: OpLoad %{{[A-Za-z0-9_]+}} %textureB
// PAIR: OpImageSampleExplicitLod
// PAIR-NOT: OpImageSampleExplicitLod
// PAIR: OpFunctionEnd
[numthreads(1, 1, 1)]
void selectedPairInBranch(uint3 id: SV_DispatchThreadID)
{
    Sampler2D<float4> pair;
    if (id.x != 0)
    {
        Texture2D<float4> missingTexture;
        pair = __makeCombinedTextureSampler(missingTexture, samplerA);
    }
    else
    {
        pair = __makeCombinedTextureSampler(textureB, samplerB);
    }
    outputBuffer[0] = pair.SampleLevel(float2(0.5), 0);
}
