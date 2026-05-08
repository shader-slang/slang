// Reproducer-based regression test for:
// https://github.com/shader-slang/slang/issues/10774
//
// This variant uses the same concrete non-copyable `RayQuery` return type in both
// the interface requirement and the satisfying method. It should compile, but
// currently fails during interface conformance checking.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -stage compute -entry computeMain

// CHECK: OpCapability RayQueryKHR
// CHECK: OpRayQueryInitializeKHR
// CHECK: OpRayQueryProceedKHR

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
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    TestAccelerationStructure accelerationStructure;
    accelerationStructure.accelerationStructure = scene;
    outputBuffer[dispatchThreadID.x] = trace(accelerationStructure);
}
