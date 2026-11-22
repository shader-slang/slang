// Check that a helper that extracts the defined texture or sampler from a combined texture-sampler
// with a never-written sampler or texture reads the defined one, with or without optimization, and that
// SPIR-V has no `OpUndef` of the handle types.

//TEST:SIMPLE(filecheck=TEXTURE):-target spirv -entry textureInHelper -stage compute -O0
//TEST:SIMPLE(filecheck=TEXTURE):-target spirv -entry textureInHelper -stage compute
//TEST:SIMPLE(filecheck=SAMPLER):-target spirv -entry samplerInHelper -stage compute -O0
//TEST:SIMPLE(filecheck=SAMPLER):-target spirv -entry samplerInHelper -stage compute
//TEST:SIMPLE(filecheck=TEXTURE):-target spirv -entry textureThroughTwoHelpers -stage compute -O0
//TEST:SIMPLE(filecheck=TEXTURE):-target spirv -entry textureThroughTwoHelpers -stage compute
//TEST:SIMPLE(filecheck=DIMENSIONS):-target spirv -entry dimensionsInHelper -stage compute -O0
//TEST:SIMPLE(filecheck=DIMENSIONS):-target spirv -entry dimensionsInHelper -stage compute

// TEXTURE: %[[IMAGE:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 1
// TEXTURE-NOT: OpUndef %[[IMAGE]]
// TEXTURE-DAG: %[[TEX:[A-Za-z0-9_]+]] = OpLoad %[[IMAGE]] %gTexture
// TEXTURE-DAG: OpImageFetch %v4float %[[TEX]]
// TEXTURE-NOT: OpUndef %[[IMAGE]]

// SAMPLER-DAG: %[[IMAGE:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 1
// SAMPLER-DAG: %[[SMP:[A-Za-z0-9_]+]] = OpTypeSampler
// SAMPLER-NOT: OpUndef %[[IMAGE]]
// SAMPLER-NOT: OpUndef %[[SMP]]
// SAMPLER-DAG: %[[S:[A-Za-z0-9_]+]] = OpLoad %[[SMP]] %gSampler
// SAMPLER-DAG: OpSampledImage {{.*}} %[[S]]
// SAMPLER-NOT: OpUndef %[[IMAGE]]
// SAMPLER-NOT: OpUndef %[[SMP]]

// DIMENSIONS: %[[IMAGE:[A-Za-z0-9_]+]] = OpTypeImage
// DIMENSIONS-NOT: OpUndef %[[IMAGE]]
// DIMENSIONS-DAG: OpImageQuerySizeLod
// DIMENSIONS-DAG: OpImageQueryLevels
// DIMENSIONS: OpStore

Texture2D<float4> gTexture;
SamplerState gSampler;
RWStructuredBuffer<float4> outputBuffer;

float4 readTexture(Sampler2D<float4> combined)
{
    return combined.__getTexture().Load(int3(0));
}

float4 useSampler(Sampler2D<float4> combined)
{
    return gTexture.SampleLevel(combined.__getSampler(), float2(0), 0);
}

float4 forwardTexture(Sampler2D<float4> combined)
{
    return readTexture(combined);
}

[numthreads(1, 1, 1)]
void textureInHelper()
{
    SamplerState s;
    outputBuffer[0] = readTexture(__makeCombinedTextureSampler(gTexture, s));
}

[numthreads(1, 1, 1)]
void samplerInHelper()
{
    Texture2D<float4> t;
    outputBuffer[0] = useSampler(__makeCombinedTextureSampler(t, gSampler));
}

[numthreads(1, 1, 1)]
void textureThroughTwoHelpers()
{
    SamplerState s;
    outputBuffer[0] = forwardTexture(__makeCombinedTextureSampler(gTexture, s));
}

float4 queryDimensions(Sampler2D<float4> combined)
{
    uint width, height, levels;
    combined.GetDimensions(0, width, height, levels);
    return float4(width, height, levels, 0);
}

[numthreads(1, 1, 1)]
void dimensionsInHelper()
{
    SamplerState s;
    outputBuffer[0] = queryDimensions(__makeCombinedTextureSampler(gTexture, s));
}
