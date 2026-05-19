// Companion to descriptor-heap-acceleration-structure.slang.
// Exercises the RayTracingKHR branch of requireSPIRVAnyCapability when
// loading a RaytracingAccelerationStructure from a descriptor heap in a
// raygeneration shader (which already requires SpvCapabilityRayTracingKHR).
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage raygeneration -entry rayGenMain -profile sm_6_6 -capability spvDescriptorHeapEXT -spirv-resource-heap-stride 32

struct PushConstants
{
    DescriptorHandle<RaytracingAccelerationStructure> accelerationStructure;
    DescriptorHandle<RWTexture2D<float4>> outputTexture;
}

struct RayPayload
{
    float t;
}

[shader("raygeneration")]
void rayGenMain(uniform PushConstants pushConstants)
{
    // CHECK-DAG: %[[U64:[A-Za-z0-9_]+]] = OpTypeInt 64 0
    // CHECK-DAG: %[[AS_TYPE:[A-Za-z0-9_]+]] = OpTypeAccelerationStructureKHR
    // CHECK-DAG: %[[AS_HEAP_ARRAY:[A-Za-z0-9_]+]] = OpTypeRuntimeArray %[[AS_TYPE]]
    // CHECK-DAG: OpDecorate %[[AS_HEAP_ARRAY]] ArrayStride 32
    // CHECK: OpUntypedAccessChainKHR %_ptr_UniformConstant %[[AS_HEAP_ARRAY]] %slang_resourceHeap
    // CHECK-NEXT: %[[AS_ADDR:[A-Za-z0-9_]+]] = OpLoad %[[U64]]
    // CHECK-NEXT: OpConvertUToAccelerationStructureKHR %[[AS_TYPE]] %[[AS_ADDR]]
    // CHECK-DAG: OpCapability RayTracingKHR
    // CHECK-NOT: OpCapability RayQueryKHR
    RaytracingAccelerationStructure scene =
        RaytracingAccelerationStructure(pushConstants.accelerationStructure);

    RayPayload payload = { 0.0 };
    RayDesc ray;
    ray.Origin = float3(0.1, 0.1, 0.0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMax = 1e4f;
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload);

    RWTexture2D<float4> outputTexture = pushConstants.outputTexture;
    outputTexture[uint2(0, 0)] = float4(payload.t, 0, 0, 1);
}
