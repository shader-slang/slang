// Regression test for issue #10671. With spvDescriptorHeapEXT, an acceleration
// structure descriptor heap entry is a device address and must be converted to
// OpTypeAccelerationStructureKHR before ray query use.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry compute_main -stage compute -capability spvDescriptorHeapEXT -capability spvRayQueryKHR -spirv-resource-heap-stride 32

struct Params
{
    DescriptorHandle<RaytracingAccelerationStructure> accelerationStructure;
    DescriptorHandle<RWStructuredBuffer<uint>> output;
};

// CHECK: OpExtension "SPV_KHR_ray_tracing"
// CHECK-DAG: %[[UINT64:[A-Za-z0-9_]+]] = OpTypeInt 64 0
// CHECK-DAG: %[[RTAS:[A-Za-z0-9_]+]] = OpTypeAccelerationStructureKHR

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(uniform Params params)
{
    RayDesc ray;
    ray.Origin = float3(0.0f, 0.0f, 1.0f);
    ray.Direction = float3(0.0f, 0.0f, -1.0f);
    ray.TMin = 0.0f;
    ray.TMax = 100.0f;

    RayQuery<RAY_FLAG_FORCE_NON_OPAQUE> query;

    // CHECK: %[[ADDRESS:[A-Za-z0-9_]+]] = OpLoad %[[UINT64]]
    // CHECK: %[[AS:[A-Za-z0-9_]+]] = OpConvertUToAccelerationStructureKHR %[[RTAS]] %[[ADDRESS]]
    // CHECK: OpRayQueryInitializeKHR {{.*}} %[[AS]]
    query.TraceRayInline(params.accelerationStructure, RAY_FLAG_FORCE_NON_OPAQUE, 0xff, ray);
    bool proceed = query.Proceed();

    params.output[0] = uint(query.CommittedStatus()) + uint(proceed);
}
