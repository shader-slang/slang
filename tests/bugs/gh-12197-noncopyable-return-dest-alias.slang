// A ray query returned by value from a function that also takes a ray query by reference must be
// built in a callee-local query, not aliased to the caller's return destination. A caller may write
// `q = makeQuery(q, ...)`, making the return destination and the by-reference argument the same
// storage; NRVO would then write that storage incrementally through the body and corrupt the
// aliased argument, whereas a non-NRVO return writes it once at the end. Because an opaque handle is
// always passed by reference, the presence of another ray-query parameter disables NRVO here, so
// `makeQuery` keeps its own local ray query. Compiled at -O0 so the callee's own local is
// observable. (`RayQuery` cannot be executed on the CPU, so this pins the SPIR-V lowering shape.)

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -stage compute -entry main -O0

RaytracingAccelerationStructure g_tlas;
RWStructuredBuffer<uint> g_out;

RayQuery<RAY_FLAG_NONE> makeQuery(inout RayQuery<RAY_FLAG_NONE> other, RayDesc ray)
{
    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(g_tlas, RAY_FLAG_NONE, 0xff, ray);
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
    RayQuery<RAY_FLAG_NONE> q;
    q = makeQuery(q, ray);
    g_out[0] = q.Proceed() ? 1u : 0u;
}

// CHECK: %makeQuery = OpFunction
// CHECK: [[LOCAL:%[A-Za-z0-9_]+]] = OpVariable %{{[A-Za-z0-9_]+}} Function
// CHECK: OpRayQueryInitializeKHR [[LOCAL]]
