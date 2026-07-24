// The `-fvk-use-direct-resource-params` opt-in lets a plain read-only texture pass as a
// resource-typed SPIR-V function parameter. Default (flag off) specializes it into a bindless
// `uint` index, so the image descriptor is loaded inside the callee; the opt-in keeps it a resource
// parameter, so the descriptor is loaded once on the caller side and reused across calls (where
// caller-side load coalescing collapses the N loads to one). Only plain read-only textures are
// affected: an RW texture and a read-only texture *array* stay specialized even with the flag on.

//TEST:SIMPLE(filecheck=DEFAULT): -target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=DIRECT): -target spirv-asm -stage compute -entry computeMain -fvk-use-direct-resource-params
//TEST:SIMPLE(filecheck=RWSTAYS): -target spirv-asm -stage compute -entry rwMain -fvk-use-direct-resource-params
//TEST:SIMPLE(filecheck=ARRSTAYS): -target spirv-asm -stage compute -entry arrMain -fvk-use-direct-resource-params

RWStructuredBuffer<float> outputBuffer;

[noinline]
float sample(Texture2D image, SamplerState samplerState, float2 uv)
{
    return image.SampleLevel(samplerState, uv, 0.0).x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Texture2D.Handle texH = {uint2(tid.x, 0)};
    SamplerState.Handle sampH = {uint2(tid.y, 0)};
    Texture2D tex = texH;
    SamplerState s = sampH;
    float acc = 0.0;
    [ForceUnroll]
    for (int i = 0; i < 3; ++i)
        acc += sample(tex, s, float2(i * 0.1, 0.0));
    outputBuffer[0] = acc;
}

// The image-load count is 1 in every mode; the discriminator between DEFAULT and DIRECT is the
// `sample` parameter type (`%uint` index vs the image type) and where that single load sits (callee
// body vs caller side). The `-COUNT-1`/`-NOT` pair pins the static representation, not a per-call
// dynamic count.

// DEFAULT (index-passing workaround): `sample`'s first parameter is a `uint` descriptor index and
// the image descriptor is loaded inside the callee body, so it is fetched once per call across the
// three `[ForceUnroll]` calls. The `%uint` callee type does not reference the image type, so the two
// type declarations are captured independently with `-DAG`.
// DEFAULT-DAG: OpTypeFunction %float %uint
// DEFAULT-DAG: [[IMG:%[A-Za-z0-9_]+]] = OpTypeImage
// DEFAULT-COUNT-3: OpFunctionCall
// DEFAULT-COUNT-1: OpLoad [[IMG]] %
// DEFAULT-NOT: OpLoad [[IMG]] %

// DIRECT (flag on): `sample`'s first parameter is the image type itself, so the descriptor loads on
// the caller side. The callee function type references the image type, so `OpTypeImage` necessarily
// precedes `OpTypeFunction`. Caller-side coalescing collapses the load to a single image `OpLoad`
// reused across all three calls (the `-NOT` fences any second/third).
// DIRECT: [[IMG:%[A-Za-z0-9_]+]] = OpTypeImage
// DIRECT: OpTypeFunction %float [[IMG]]
// DIRECT-COUNT-1: OpLoad [[IMG]] %
// DIRECT-NOT: OpLoad [[IMG]] %
// DIRECT-COUNT-3: OpFunctionCall

[noinline]
void store(RWTexture2D<float> image, uint2 p, float v)
{
    image[p] = v;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void rwMain(uint3 tid: SV_DispatchThreadID)
{
    RWTexture2D<float>.Handle imgH = {uint2(tid.x, 0)};
    RWTexture2D<float> img = imgH;
    store(img, tid.xy, 1.0);
    outputBuffer[0] = 0.0;
}

// RWSTAYS (flag on, RW texture): an RW texture is illegal as a direct SPIR-V parameter regardless of
// the flag, so `store` is still specialized — its image parameter becomes a `uint` descriptor index
// and no image type appears in the callee function type.
// RWSTAYS: OpTypeFunction %void %uint

Texture2D gTextures[2];
SamplerState gSampler;

[noinline]
float sampleArr(Texture2D images[2], SamplerState samplerState, float2 uv)
{
    return images[0].SampleLevel(samplerState, uv, 0.0).x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void arrMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = sampleArr(gTextures, gSampler, float2(0.0, 0.0));
}

// ARRSTAYS (flag on, texture array): the opt-in only relaxes a plain non-array read-only texture, so
// the array-of-texture parameter stays specialized away. In the emitted `sampleArr` signature the
// texture array is gone entirely — only the sampler and `float2 uv` parameters remain, and the
// array type is declared (globally) but is never a parameter of any function type.
// ARRSTAYS: [[SAMP:%[A-Za-z0-9_]+]] = OpTypeSampler
// ARRSTAYS: OpTypeFunction %float [[SAMP]] %v2float
// ARRSTAYS: [[ARR:%[A-Za-z0-9_]+]] = OpTypeArray
// ARRSTAYS-NOT: OpTypeFunction %{{.*}} [[ARR]]
