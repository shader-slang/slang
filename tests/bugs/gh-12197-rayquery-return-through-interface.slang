// Regression test for https://github.com/shader-slang/slang/issues/12197
//
// The same by-value `RayQuery` return miscompile, but reached through an interface requirement
// (the shape from #10774): the satisfying method returns a named `RayQuery` local by value. The
// initialized query and the proceeded query must be the SAME variable. `tests/bugs/
// gh-10774-concrete-return.slang` exercises this path but only checks that the two ops exist, so it
// could not catch the disconnected-variable miscompile; this test pins them to the same variable.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -O3

interface IRayQueryProvider
{
    RayQuery<RAY_FLAG_FORCE_NON_OPAQUE> traceRay(RayDesc ray);
}

struct TestAccelerationStructure : IRayQueryProvider
{
    RaytracingAccelerationStructure accelerationStructure;

    RayQuery<RAY_FLAG_FORCE_NON_OPAQUE> traceRay(RayDesc ray)
    {
        RayQuery<RAY_FLAG_FORCE_NON_OPAQUE> rq;
        rq.TraceRayInline(accelerationStructure, RAY_FLAG_FORCE_NON_OPAQUE, 0xFF, ray);
        return rq;
    }
}

uint trace<T : IRayQueryProvider>(T accelerationStructure)
{
    RayDesc ray;
    ray.Origin = float3(0.0, 0.0, 0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1e30;

    RayQuery<RAY_FLAG_FORCE_NON_OPAQUE> rq = accelerationStructure.traceRay(ray);
    while (rq.Proceed())
    {
    }
    return uint(rq.CommittedStatus());
}

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    TestAccelerationStructure accelerationStructure;
    accelerationStructure.accelerationStructure = scene;
    outputBuffer[dispatchThreadID.x] = trace(accelerationStructure);
}

// CHECK: OpRayQueryInitializeKHR [[Q:%[A-Za-z0-9_]+]]
// CHECK: OpRayQueryProceedKHR %{{[A-Za-z0-9_]+}} [[Q]]
