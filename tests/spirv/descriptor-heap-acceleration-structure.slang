// Regression test for issue #10671. With spvDescriptorHeapEXT, an acceleration
// structure descriptor heap entry is a device address and must be converted to
// OpTypeAccelerationStructureKHR before ray query and ray tracing pipeline use.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry compute_main -stage compute -capability spvDescriptorHeapEXT -capability spvRayQueryKHR -spirv-resource-heap-stride 32
//TEST:SIMPLE(filecheck=TRACE): -target spirv-asm -entry raygen_main -stage raygeneration -capability spvDescriptorHeapEXT -capability spvRayTracingKHR -spirv-resource-heap-stride 32

struct Params
{
    DescriptorHandle<RaytracingAccelerationStructure> accelerationStructure;
    DescriptorHandle<RWStructuredBuffer<uint>> output;
};

struct Payload
{
    uint value;
};

// CHECK: OpExtension "SPV_KHR_ray_tracing"
// CHECK-DAG: %[[UINT64:[A-Za-z0-9_]+]] = OpTypeInt 64 0
// CHECK-DAG: %[[RTAS:[A-Za-z0-9_]+]] = OpTypeAccelerationStructureKHR

// TRACE: OpExtension "SPV_KHR_ray_tracing"
// TRACE-DAG: %[[TRACE_UINT64:[A-Za-z0-9_]+]] = OpTypeInt 64 0
// TRACE-DAG: %[[TRACE_RTAS:[A-Za-z0-9_]+]] = OpTypeAccelerationStructureKHR

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

[shader("raygeneration")]
void raygen_main(uniform Params params)
{
    RayDesc ray;
    ray.Origin = float3(0.0f, 0.0f, 1.0f);
    ray.Direction = float3(0.0f, 0.0f, -1.0f);
    ray.TMin = 0.0f;
    ray.TMax = 100.0f;

    Payload payload;
    payload.value = 0;

    // TRACE: %[[TRACE_ADDRESS:[A-Za-z0-9_]+]] = OpLoad %[[TRACE_UINT64]]
    // TRACE: %[[TRACE_AS:[A-Za-z0-9_]+]] = OpConvertUToAccelerationStructureKHR %[[TRACE_RTAS]] %[[TRACE_ADDRESS]]
    // TRACE: OpTraceRayKHR %[[TRACE_AS]]
    TraceRay(params.accelerationStructure, RAY_FLAG_FORCE_NON_OPAQUE, 0xff, 0, 0, 0, ray, payload);

    params.output[0] = payload.value;
}
