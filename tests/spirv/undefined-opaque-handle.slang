// Regression test for https://github.com/shader-slang/slang/issues/14076
//
// A resource that is never written is a `LoadFromUninitializedMemory` of a handle type. SPIR-V for
// Vulkan has no undefined handle: spirv-val rejects `OpUndef` of an image, sampler or acceleration
// structure type (VUID-StandaloneSpirv-OpUndef-12464). Each entry point below reads or writes
// through such a handle on a path that also uses a real resource of the same type, so the type's
// id exists and the checks can assert that no `OpUndef` of it is emitted.

//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry sampleUndefinedTexture -stage compute
//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry sampleUndefinedTexture -stage compute -g
//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry sampleUndefinedTextureTwice -stage compute
//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry queryUndefinedTexture -stage compute
//TEST:SIMPLE(filecheck=SAMPLER):-target spirv -entry sampleWithUndefinedSampler -stage compute
//TEST:SIMPLE(filecheck=STORAGE):-target spirv -entry writeUndefinedStorageTexture -stage compute
//TEST:SIMPLE(filecheck=ACCEL):-target spirv -entry traceUndefinedAccelerationStructure -stage compute
//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry passUndefinedTextureToCalleeWithItsOwn -stage compute
//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry gatherFromUndefinedTexture -stage compute
//TEST:SIMPLE(filecheck=SAMPLED):-target spirv -entry queryUndefinedCombinedSampler -stage compute
//TEST:SIMPLE(filecheck=COUNTER):-target spirv -entry addToUndefinedStorageTexture -stage compute
//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry passTwoUndefinedTextures -stage compute
//TEST:SIMPLE(filecheck=STORAGE):-target spirv -entry writeComponentInCallee -stage compute
//TEST:SIMPLE(filecheck=PROJECTION):-target spirv -entry projectTextureFromCombined -stage compute

// SAMPLE: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 1
// SAMPLE-NOT: OpUndef %[[IMG]]
// SAMPLE: OpFunctionEnd

// SAMPLED: %[[SI:[A-Za-z0-9_]+]] = OpTypeSampledImage
// SAMPLED-NOT: OpUndef %[[SI]]
// SAMPLED: OpFunctionEnd

// PROJECTION-DAG: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 1
// PROJECTION-DAG: %[[SMP:[A-Za-z0-9_]+]] = OpTypeSampler
// PROJECTION-NOT: OpUndef %[[IMG]]
// PROJECTION-NOT: OpUndef %[[SMP]]
// PROJECTION: %[[TEX:[A-Za-z0-9_]+]] = OpLoad %[[IMG]] %gTexture
// PROJECTION-NEXT: OpImageFetch %v4float %[[TEX]]
// PROJECTION-NOT: OpUndef %[[IMG]]
// PROJECTION-NOT: OpUndef %[[SMP]]
// PROJECTION: OpFunctionEnd

// SAMPLER: %[[SMP:[A-Za-z0-9_]+]] = OpTypeSampler
// SAMPLER-NOT: OpUndef %[[SMP]]
// SAMPLER: OpFunctionEnd

// STORAGE: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 2
// STORAGE-NOT: OpUndef %[[IMG]]
// STORAGE: OpFunctionEnd

// COUNTER: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %uint 2D 2 0 0 2
// COUNTER-NOT: OpUndef %[[IMG]]
// COUNTER: OpFunctionEnd

// ACCEL: %[[AS:[A-Za-z0-9_]+]] = OpTypeAccelerationStructureKHR
// ACCEL-NOT: OpUndef %[[AS]]
// ACCEL: OpFunctionEnd

Texture2D<float4> gTexture;
RWTexture2D<float4> gStorageTexture;
RWTexture2D<uint> gCounter;
RaytracingAccelerationStructure gScene;
SamplerState gSampler;
RWStructuredBuffer<float4> outputBuffer;
cbuffer Settings
{
    bool useUndefined;
};

[numthreads(1, 1, 1)]
void sampleUndefinedTexture()
{
    Texture2D<float4> t;
    float4 r = gTexture.SampleLevel(gSampler, float2(0), 0);
    if (useUndefined)
        r = t.SampleLevel(gSampler, float2(0), 0);
    outputBuffer[0] = r;
}

[numthreads(1, 1, 1)]
void sampleUndefinedTextureTwice()
{
    Texture2D<float4> t;
    float4 r = gTexture.SampleLevel(gSampler, float2(0), 0);
    if (useUndefined)
        r = t.SampleLevel(gSampler, float2(0), 0) - t.SampleLevel(gSampler, float2(0), 0);
    outputBuffer[0] = r;
}

[numthreads(1, 1, 1)]
void queryUndefinedTexture()
{
    Texture2D<float4> t;
    uint width = 0;
    uint height = 0;
    if (useUndefined)
        t.GetDimensions(width, height);
    outputBuffer[0] = float4(width, height, 0, 0) + gTexture.Load(int3(0));
}

[numthreads(1, 1, 1)]
void sampleWithUndefinedSampler()
{
    SamplerState s;
    float4 r = gTexture.SampleLevel(gSampler, float2(0), 0);
    if (useUndefined)
        r = gTexture.SampleLevel(s, float2(0), 0);
    outputBuffer[0] = r;
}

[numthreads(1, 1, 1)]
void writeUndefinedStorageTexture()
{
    RWTexture2D<float4> t;
    if (useUndefined)
        t[uint2(0, 0)] = float4(1);
    gStorageTexture[uint2(0, 0)] = float4(2);
}

bool hasHit(RaytracingAccelerationStructure scene)
{
    RayQuery<RAY_FLAG_NONE> query;
    RayDesc ray;
    ray.Origin = float3(0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 1;
    query.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
    query.Proceed();
    return query.CommittedStatus() != COMMITTED_NOTHING;
}

[numthreads(1, 1, 1)]
void traceUndefinedAccelerationStructure()
{
    RaytracingAccelerationStructure scene;
    bool hit = hasHit(gScene);
    if (useUndefined)
        hit = hasHit(scene);
    outputBuffer[0] = hit ? float4(1) : float4(0);
}

// Inlining `sampleBoth` for the undefined argument also copies its own never-written `u` into the
// caller, so that copy is rewritten too.
float4 sampleBoth(Texture2D<float4> t)
{
    Texture2D<float4> u;
    float4 r = 0;
    if (useUndefined)
        r += u.SampleLevel(gSampler, float2(0), 0) + t.SampleLevel(gSampler, float2(0), 0);
    return r;
}

[numthreads(1, 1, 1)]
void passUndefinedTextureToCalleeWithItsOwn()
{
    Texture2D<float4> t;
    outputBuffer[0] = sampleBoth(gTexture) + sampleBoth(t);
}

[numthreads(1, 1, 1)]
void gatherFromUndefinedTexture()
{
    Texture2D<float4> t;
    float4 r = gTexture.SampleLevel(gSampler, float2(0), 0);
    if (useUndefined)
        r = t.GatherRed(gSampler, float2(0), int2(1, 0));
    outputBuffer[0] = r;
}

[numthreads(1, 1, 1)]
void queryUndefinedCombinedSampler()
{
    Sampler2D<float4> s;
    uint width = 0;
    uint height = 0;
    float4 r = gTexture.SampleLevel(gSampler, float2(0), 0);
    if (useUndefined)
    {
        s.GetDimensions(width, height);
        r = s.Load(int3(0));
    }
    outputBuffer[0] = r + float4(width, height, 0, 0);
}

uint addOne(RWTexture2D<uint> counter)
{
    uint original;
    InterlockedAdd(counter[uint2(0, 0)], 1, original);
    return original;
}

[numthreads(1, 1, 1)]
void addToUndefinedStorageTexture()
{
    RWTexture2D<uint> t;
    uint r = addOne(gCounter);
    if (useUndefined)
        r += addOne(t);
    outputBuffer[0] = float4(r);
}

float4 sampleEither(Texture2D<float4> a, Texture2D<float4> b)
{
    return useUndefined ? a.SampleLevel(gSampler, float2(0), 0) : b.SampleLevel(gSampler, float2(0), 0);
}

// One call that receives two never-written textures is inlined once.
[numthreads(1, 1, 1)]
void passTwoUndefinedTextures()
{
    Texture2D<float4> t;
    Texture2D<float4> u;
    outputBuffer[0] = sampleEither(gTexture, gTexture) + sampleEither(t, u);
}

void setX(RWTexture2D<float4> t)
{
    t[uint2(0, 0)].x = 1;
}

// A write through a never-written storage texture in an inlined callee is removed.
[numthreads(1, 1, 1)]
void writeComponentInCallee()
{
    RWTexture2D<float4> t;
    setX(gStorageTexture);
    if (useUndefined)
        setX(t);
}

// Replacing the combined value of a defined texture and a never-written sampler keeps the texture
// that is projected back out of it.
[numthreads(1, 1, 1)]
void projectTextureFromCombined()
{
    SamplerState s;
    outputBuffer[0] = __makeCombinedTextureSampler(gTexture, s).__getTexture().Load(int3(0)) +
                      gTexture.SampleLevel(gSampler, float2(0), 0);
}
