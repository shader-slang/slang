// Regression test for GitHub issues #10671 and #11231.
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry computeMain -profile sm_6_6 -capability spvDescriptorHeapEXT -capability spvRayQueryKHR
//TEST:SIMPLE(filecheck=OVERRIDE): -target spirv-asm -stage compute -entry computeMain -profile sm_6_6 -capability spvDescriptorHeapEXT -capability spvRayQueryKHR -spirv-resource-heap-stride 32

struct PushConstants
{
    DescriptorHandle<RaytracingAccelerationStructure> accelerationStructure;
    DescriptorHandle<RWTexture2D<float4>> outputTexture;
}

bool traceRayClosestHit(
    RaytracingAccelerationStructure scene,
    float3 rayOrigin,
    float3 rayDirection,
    out float t)
{
    RayDesc ray;
    ray.Origin = rayOrigin;
    ray.TMin = 0.01f;
    ray.Direction = rayDirection;
    ray.TMax = 1e4f;

    RayQuery<RAY_FLAG_NONE> rayQuery;
    rayQuery.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
    rayQuery.Proceed();

    if (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        t = rayQuery.CommittedRayT();
        return true;
    }

    unused(t);
    return false;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID, uniform PushConstants pushConstants)
{
    // CHECK-DAG: %[[U64:[A-Za-z0-9_]+]] = OpTypeInt 64 0
    // CHECK-DAG: %[[AS_TYPE:[A-Za-z0-9_]+]] = OpTypeAccelerationStructureKHR
    // CHECK-DAG: %[[AS_HEAP_ARRAY:[A-Za-z0-9_]+]] = OpTypeRuntimeArray %[[U64]]
    // CHECK-DAG: OpDecorate %[[AS_HEAP_ARRAY]] ArrayStride 8
    // OVERRIDE-DAG: %[[OVERRIDE_U64:[A-Za-z0-9_]+]] = OpTypeInt 64 0
    // OVERRIDE-DAG: %[[OVERRIDE_AS_HEAP_ARRAY:[A-Za-z0-9_]+]] = OpTypeRuntimeArray %[[OVERRIDE_U64]]
    // OVERRIDE-DAG: OpDecorate %[[OVERRIDE_AS_HEAP_ARRAY]] ArrayStride 32
    // CHECK-DAG: OpCapability RayQueryKHR
    // CHECK: OpUntypedAccessChainKHR %_ptr_UniformConstant %[[AS_HEAP_ARRAY]] %slang_resourceHeap
    // CHECK-NEXT: %[[AS_ADDR:[A-Za-z0-9_]+]] = OpLoad %[[U64]]
    // CHECK-NEXT: OpConvertUToAccelerationStructureKHR %[[AS_TYPE]] %[[AS_ADDR]]
    // CHECK: OpRayQueryInitializeKHR
    float t = 0.0;
    traceRayClosestHit(
        RaytracingAccelerationStructure(pushConstants.accelerationStructure),
        float3(0.1, 0.1, 0.0),
        float3(0.0, 0.0, 1.0),
        t);

    RWTexture2D<float4> outputTexture = pushConstants.outputTexture;
    outputTexture[dispatchThreadID.xy] = t;
}
