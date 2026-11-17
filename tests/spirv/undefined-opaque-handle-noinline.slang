// Check that a never-written opaque handle passed to a function that is not inlined compiles to
// SPIR-V without an `OpUndef` of the handle's type, and that a call to the same function with a
// defined handle still reads that handle.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry passDefinedAndUndefined -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry passDefinedAndUndefined -stage compute -g
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry passOneOfTwo -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry passCombinedOfUndefinedTexture -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry passTextureOfUndefinedCombined -stage compute
//TEST:SIMPLE(filecheck=SAMPLER):-target spirv -entry extractSamplerInCallee -stage compute

// CHECK-DAG: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 1
// CHECK-DAG: %[[SI:[A-Za-z0-9_]+]] = OpTypeSampledImage %[[IMG]]
// CHECK-NOT: OpUndef %[[IMG]]
// CHECK-NOT: OpUndef %[[SI]]

// SAMPLER-DAG: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 1
// SAMPLER-DAG: %[[SMP:[A-Za-z0-9_]+]] = OpTypeSampler
// SAMPLER-NOT: OpUndef %[[IMG]]
// SAMPLER-NOT: OpUndef %[[SMP]]
// SAMPLER: %extractSamplerInCallee = OpFunction
// SAMPLER: OpFunctionCall %v4float %[[CLONE:[A-Za-z0-9_]+]]
// SAMPLER: %[[CLONE]] = OpFunction
// SAMPLER: OpLoad %[[SMP]] %gSampler
// SAMPLER: OpImageSampleExplicitLod
// SAMPLER-NOT: OpUndef %[[IMG]]
// SAMPLER-NOT: OpUndef %[[SMP]]

Texture2D<float4> gTexture;
SamplerState gSampler;
RWStructuredBuffer<float4> outputBuffer;
cbuffer Settings
{
    bool useUndefined;
    uint depth;
};

[noinline]
float4 readTexture(Texture2D<float4> t)
{
    return t.SampleLevel(gSampler, float2(0), 0);
}

[numthreads(1, 1, 1)]
void passDefinedAndUndefined()
{
    Texture2D<float4> t;
    float4 r = readTexture(gTexture);
    if (useUndefined)
        r = readTexture(t);
    outputBuffer[0] = r;
}

[noinline]
float4 readBoth(Texture2D<float4> a, Texture2D<float4> b)
{
    return a.SampleLevel(gSampler, float2(0), 0) + b.Load(int3(0));
}

// Each call is specialized only for the parameter that receives the never-written texture.
[numthreads(1, 1, 1)]
void passOneOfTwo()
{
    Texture2D<float4> t;
    outputBuffer[0] = readBoth(gTexture, t) + readBoth(t, gTexture);
}

[noinline]
float4 readCombined(Sampler2D<float4> c)
{
    return c.SampleLevel(float2(0), 0);
}

// Combining a never-written texture with a sampler yields an undefined combined texture-sampler,
// which is then passed to a call.
[numthreads(1, 1, 1)]
void passCombinedOfUndefinedTexture()
{
    Texture2D<float4> t;
    float4 r = gTexture.SampleLevel(gSampler, float2(0), 0);
    if (useUndefined)
        r = readCombined(__makeCombinedTextureSampler(t, gSampler));
    outputBuffer[0] = r;
}

// The texture of a never-written combined texture-sampler is an undefined texture, which is then
// passed to a call.
[numthreads(1, 1, 1)]
void passTextureOfUndefinedCombined()
{
    Sampler2D<float4> c;
    float4 r = readTexture(gTexture);
    if (useUndefined)
        r = readTexture(c.__getTexture());
    outputBuffer[0] = r;
}

[noinline]
float4 sampleWithItsSampler(Sampler2D<float4> c)
{
    return gTexture.SampleLevel(c.__getSampler(), float2(0), 0);
}

// The sampler extracted from a combined texture-sampler of a never-written texture and a defined
// sampler, in the callee that it is passed to, is the defined sampler.
[numthreads(1, 1, 1)]
void extractSamplerInCallee()
{
    Texture2D<float4> t;
    outputBuffer[0] = sampleWithItsSampler(__makeCombinedTextureSampler(t, gSampler));
}
