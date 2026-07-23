// The `-fvk-use-direct-resource-params` opt-in lets a read-only texture pass as a resource-typed
// SPIR-V function parameter. Default (flag off) specializes it into a bindless `uint` index, so the
// image descriptor is loaded inside the callee and re-executed on every call; the opt-in keeps it a
// resource parameter, so the descriptor is loaded once on the caller side and reused across calls,
// where caller-side load coalescing collapses the N loads to one.

//TEST:SIMPLE(filecheck=DEFAULT): -target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=DIRECT): -target spirv-asm -stage compute -entry computeMain -fvk-use-direct-resource-params

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

// DEFAULT (index-passing workaround): `sample`'s first parameter is a `uint` descriptor index, and
// the image descriptor is loaded inside the callee body. The three `[ForceUnroll]` iterations emit
// three calls, and the single callee-body image `OpLoad` re-executes once per call. (The image type
// is declared before the function type; the callee's load appears after the three call sites.)
// DEFAULT: OpTypeFunction %float %uint
// DEFAULT: [[IMG:%[A-Za-z0-9_]+]] = OpTypeImage
// DEFAULT-COUNT-3: OpFunctionCall
// DEFAULT-COUNT-1: OpLoad [[IMG]]
// DEFAULT-NOT: OpLoad [[IMG]]

// DIRECT (flag on): `sample`'s first parameter is the image type itself, so the descriptor is loaded
// on the caller side. Caller-side coalescing collapses that to a single image `OpLoad` reused across
// all three calls: exactly one image-typed load appears (the `-NOT`s fence any second/third), and
// there are three `OpFunctionCall`s. Without coalescing the three calls would each reload it.
// DIRECT: [[IMG:%[A-Za-z0-9_]+]] = OpTypeImage
// DIRECT: OpTypeFunction %float [[IMG]]
// DIRECT-COUNT-1: OpLoad [[IMG]]
// DIRECT-NOT: OpLoad [[IMG]]
// DIRECT-COUNT-3: OpFunctionCall
// DIRECT-NOT: OpLoad [[IMG]]
