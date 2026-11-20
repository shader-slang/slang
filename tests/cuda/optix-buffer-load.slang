//TEST:SIMPLE(filecheck=OPTIX): -target cuda -entry rayGen
//TEST:SIMPLE(filecheck=OPTIX): -target cuda -entry closestHit
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain
//TEST:SIMPLE(filecheck=OPTIX): -target cuda -entry computeMain -entry rayGen
//TEST:SIMPLE(filecheck=PTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/" -entry rayGen

// The host can update this allocation between OptiX launches. The same read
// in a compute-only module can still use the CUDA read-only cache.
StructuredBuffer<uint> values;
RWStructuredBuffer<uint> result;

[noinline]
uint readValue(uint index)
{
    return values[index];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    result[id.x] = readValue(id.x);
}

[shader("raygeneration")]
void rayGen()
{
    result[DispatchRaysIndex().x] = readValue(4);
}

struct Payload
{
    uint value;
};

[shader("closesthit")]
void closestHit(inout Payload payload, BuiltInTriangleIntersectionAttributes attributes)
{
    payload.value = readValue(4);
}

// OPTIX-NOT: __ldg
// OPTIX: {{__(raygen|closesthit)__}}
// OPTIX-NOT: __ldg
// CUDA: __ldg
// PTX-NOT: ld.global.nc
// PTX: ld.global{{(\.ca)?}}.u32
// PTX-NOT: ld.global.nc
