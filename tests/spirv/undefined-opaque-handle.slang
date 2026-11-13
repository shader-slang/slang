// Check that reading or writing through a never-written texture, sampler, storage texture, subpass
// input or acceleration structure compiles to SPIR-V without an `OpUndef` of the handle's type,
// which Vulkan forbids (VUID-StandaloneSpirv-OpUndef-12464). Each entry point also
// uses a real resource of the same type, so that the type's id exists for the checks to name.

//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry sampleUndefinedTexture -stage compute
//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry sampleUndefinedTexture -stage compute -g
//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry sampleUndefinedTextureTwice -stage compute
//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry queryUndefinedTexture -stage compute
//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry queryUndefinedTextureLevels -stage compute
//TEST:SIMPLE(filecheck=SAMPLER):-target spirv -entry sampleWithUndefinedSampler -stage compute
//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry sampleUndefinedTextureWithUndefinedSampler -stage compute
//TEST:SIMPLE(filecheck=STORAGE):-target spirv -entry writeUndefinedStorageTexture -stage compute
//TEST:SIMPLE(filecheck=ACCEL):-target spirv -entry traceUndefinedAccelerationStructure -stage compute
//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry passUndefinedTextureToCalleeWithItsOwn -stage compute
//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry gatherFromUndefinedTexture -stage compute
//TEST:SIMPLE(filecheck=SAMPLED):-target spirv -entry queryUndefinedCombinedSampler -stage compute
//TEST:SIMPLE(filecheck=COUNTER):-target spirv -entry addToUndefinedStorageTexture -stage compute
//TEST:SIMPLE(filecheck=SAMPLE):-target spirv -entry passTwoUndefinedTextures -stage compute
//TEST:SIMPLE(filecheck=STORAGE):-target spirv -entry writeComponentInCallee -stage compute
//TEST:SIMPLE(filecheck=EXTRACTION):-target spirv -entry extractTextureFromCombined -stage compute
//TEST:SIMPLE(filecheck=SUBPASS):-target spirv -entry loadFromUndefinedSubpassInput -stage fragment

// SAMPLE: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 1
// SAMPLE-NOT: OpUndef %[[IMG]]

// SAMPLED: %[[SI:[A-Za-z0-9_]+]] = OpTypeSampledImage
// SAMPLED-NOT: OpUndef %[[SI]]

// EXTRACTION-DAG: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 1
// EXTRACTION-DAG: %[[SMP:[A-Za-z0-9_]+]] = OpTypeSampler
// EXTRACTION-NOT: OpUndef %[[IMG]]
// EXTRACTION-NOT: OpUndef %[[SMP]]
// EXTRACTION: %[[TEX:[A-Za-z0-9_]+]] = OpLoad %[[IMG]] %gTexture
// EXTRACTION-NEXT: OpImageFetch %v4float %[[TEX]]
// EXTRACTION-NOT: OpUndef %[[IMG]]
// EXTRACTION-NOT: OpUndef %[[SMP]]

// SAMPLER: %[[SMP:[A-Za-z0-9_]+]] = OpTypeSampler
// SAMPLER-NOT: OpUndef %[[SMP]]

// STORAGE: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 2
// STORAGE-NOT: OpUndef %[[IMG]]

// COUNTER: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %uint 2D 2 0 0 2
// COUNTER-NOT: OpUndef %[[IMG]]

// ACCEL: %[[AS:[A-Za-z0-9_]+]] = OpTypeAccelerationStructureKHR
// ACCEL-NOT: OpUndef %[[AS]]

// SUBPASS: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float SubpassData
// SUBPASS-NOT: OpUndef %[[IMG]]

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

// A query of both the size and the number of levels names the texture twice in one `SPIRVAsm`
// block, which is removed once.
[numthreads(1, 1, 1)]
void queryUndefinedTextureLevels()
{
    Texture2D<float4> t;
    uint width = 0;
    uint height = 0;
    uint levels = 0;
    if (useUndefined)
        t.GetDimensions(0, width, height, levels);
    outputBuffer[0] = float4(width, height, levels, 0) + gTexture.Load(int3(0));
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

// A never-written texture and a never-written sampler form one combined texture-sampler, which is
// reached from each of them and rewritten once.
[numthreads(1, 1, 1)]
void sampleUndefinedTextureWithUndefinedSampler()
{
    Texture2D<float4> t;
    SamplerState s;
    float4 r = gTexture.SampleLevel(gSampler, float2(0), 0);
    if (useUndefined)
        r = t.SampleLevel(s, float2(0), 0);
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

// `sampleBoth` has a never-written texture of its own, and is also called with one.
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

// One call receives two never-written textures.
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

// A callee writes one component through a never-written storage texture.
[numthreads(1, 1, 1)]
void writeComponentInCallee()
{
    RWTexture2D<float4> t;
    setX(gStorageTexture);
    if (useUndefined)
        setX(t);
}

// The texture extracted from a combined texture-sampler of a defined texture and a never-written
// sampler is the defined texture.
[numthreads(1, 1, 1)]
void extractTextureFromCombined()
{
    SamplerState s;
    outputBuffer[0] = __makeCombinedTextureSampler(gTexture, s).__getTexture().Load(int3(0)) +
                      gTexture.SampleLevel(gSampler, float2(0), 0);
}

[[vk::input_attachment_index(0)]]
SubpassInput<float4> gSubpassInput;

[shader("fragment")]
float4 loadFromUndefinedSubpassInput() : SV_Target
{
    SubpassInput<float4> s;
    float4 r = gSubpassInput.SubpassLoad();
    if (useUndefined)
        r = s.SubpassLoad();
    return r;
}
