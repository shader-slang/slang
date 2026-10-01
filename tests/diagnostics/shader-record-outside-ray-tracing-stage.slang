// On CUDA/OptiX a shader record is only available to ray tracing programs, so a compute kernel
// that reads one is rejected instead of calling `optixGetSbtDataPointer()` from a plain kernel.
// See shader-slang/slang#12628.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry csMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -entry csMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry csMain -stage compute -DVIA_GENERIC
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry chsMain -stage closesthit -entry csMain -stage compute -DVIA_STATIC_INIT

struct SbtData
{
    float4 color;
    uint id;
};

layout(shaderRecordEXT) ConstantBuffer<SbtData> gSbtRecordData;
//CHECK:                                        ^^^^^^^^^^^^^^ error E39033
//CHECK:                                        ^^^^^^^^^^^^^^ this shader record is used by entry point 'csMain', which is not a ray tracing stage

RWStructuredBuffer<float4> gOut;

#if defined(VIA_GENERIC)

// The kernel reaches the record only through an interface method called from a generic.
interface IColorSource
{
    float4 get();
}

struct SbtColor : IColorSource
{
    float4 get() { return gSbtRecordData.color; }
}

float4 readVia<T : IColorSource>(T source)
{
    return source.get();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void csMain()
{
    SbtColor source;
    gOut[0] = readVia(source);
}

#elif defined(VIA_STATIC_INIT)

// Only the closest-hit program reads `gCachedId`, but CUDA runs every `static` initializer at the
// start of every entry point, so the compute kernel would read the record too.
static uint gCachedId = gSbtRecordData.id;

[shader("closesthit")]
void chsMain(inout float4 payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload = float4(gCachedId);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void csMain()
{
    gOut[0] = float4(1);
}

#else

// The read sits in a helper, so the kernel reaches the record only through a call.
float4 readColor()
{
    return gSbtRecordData.color;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void csMain()
{
    gOut[0] = readColor();
}

#endif
