// Regression test for https://github.com/shader-slang/slang/issues/12197
//
// A plain (no-interface) function that returns a `RayQuery<...>` by value used to silently
// miscompile on SPIR-V: `OpRayQueryInitializeKHR` targeted the callee's local ray-query, but the
// caller's `Proceed()` loop ran on a different, never-initialized variable (the by-value return was
// lowered as an opaque whole-handle copy, which a non-copyable ray-query cannot honor). The two ops
// must target the SAME ray-query variable.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -stage compute -entry main -O3

RaytracingAccelerationStructure g_tlas;
RWStructuredBuffer<uint> g_out;

RayQuery<RAY_FLAG_NONE> initQuery(RayDesc ray)
{
    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(g_tlas, RAY_FLAG_NONE, 0xFF, ray);
    return q;
}

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    RayDesc ray;
    ray.Origin = float3(0, 0, -2);
    ray.TMin = 0;
    ray.Direction = float3(0, 0, 1);
    ray.TMax = 100;

    RayQuery<RAY_FLAG_NONE> q = initQuery(ray);
    uint n = 0;
    while (q.Proceed())
    {
        q.CommitNonOpaqueTriangleHit();
        n++;
    }
    g_out[0] = n + (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT ? 100u : 0u);
}

// The initialized query and the proceeded query must be the SAME variable. `OpRayQueryProceedKHR`
// yields a bool, so its query operand follows the result type; `OpRayQueryInitializeKHR` is void
// and takes the query first.
// CHECK: OpRayQueryInitializeKHR [[Q:%[A-Za-z0-9_]+]]
// CHECK: OpRayQueryProceedKHR %{{[A-Za-z0-9_]+}} [[Q]]
