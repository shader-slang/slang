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

// SAMPLE: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 1
// SAMPLE-NOT: OpUndef %[[IMG]]
// SAMPLE: OpFunctionEnd

// SAMPLER: %[[SMP:[A-Za-z0-9_]+]] = OpTypeSampler
// SAMPLER-NOT: OpUndef %[[SMP]]
// SAMPLER: OpFunctionEnd

// STORAGE: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 2
// STORAGE-NOT: OpUndef %[[IMG]]
// STORAGE: OpFunctionEnd

// ACCEL: %[[AS:[A-Za-z0-9_]+]] = OpTypeAccelerationStructureKHR
// ACCEL-NOT: OpUndef %[[AS]]
// ACCEL: OpFunctionEnd

Texture2D<float4> gTexture;
RWTexture2D<float4> gStorageTexture;
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
