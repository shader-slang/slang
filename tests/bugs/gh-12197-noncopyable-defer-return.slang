// A ray query returned by value from a function with a `defer` must be built in a callee-local
// query, not aliased to the caller's return destination: a deferred statement runs after the
// return value has been chosen, so if the returned local aliased the destination a later mutation
// could corrupt the returned value. `makeQuery` must therefore keep its own local ray query
// (initialized in place) instead of initializing the return-destination parameter directly.
// Compiled at -O0 so the callee is not inlined and its own local is observable. (`RayQuery` cannot
// be executed on the CPU, so this pins the SPIR-V lowering shape.)

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -stage compute -entry main -O0

RaytracingAccelerationStructure g_tlas;
RWStructuredBuffer<uint> g_out;

RayQuery<RAY_FLAG_NONE> makeQuery(RayDesc ray)
{
    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(g_tlas, RAY_FLAG_NONE, 0xff, ray);
    defer
    {
        g_out[1] = 7;
    }
    return q;
}

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    RayDesc ray;
    ray.Origin = 0;
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 1;
    RayQuery<RAY_FLAG_NONE> q = makeQuery(ray);
    g_out[0] = q.Proceed() ? 1u : 0u;
}

// CHECK: %makeQuery = OpFunction
// CHECK: [[LOCAL:%[A-Za-z0-9_]+]] = OpVariable %{{[A-Za-z0-9_]+}} Function
// CHECK: OpRayQueryInitializeKHR [[LOCAL]]
