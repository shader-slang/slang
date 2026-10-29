//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-via-glsl

uniform uint64_t accelStructAddr;
RWStructuredBuffer<int> output;

// This shader reaches the acceleration structure only through RayQuery (inline ray
// tracing), never a ray-tracing pipeline. The emitted SPIR-V must therefore declare
// only the ray-query capability/extension; leaking SPV_KHR_ray_tracing without a
// RayTracingKHR capability is the invalid module Vulkan rejects with
// VUID-VkShaderModuleCreateInfo-pCode-08742 (issue #12694).

// The emitted module must declare the ray-query capability/extension and must not
// leak the ray-tracing ones. Each CHECK-NOT has a bounded scan region: the ray-tracing
// capability must not appear before RayQueryKHR, and the ray-tracing extension must
// not appear between the ray-query extension and the conversion instruction (where
// the leaked OpExtension was emitted).
// CHECK-NOT: OpCapability RayTracingKHR
// CHECK: OpCapability RayQueryKHR
// CHECK: OpExtension "SPV_KHR_ray_query"
// CHECK-NOT: OpExtension "SPV_KHR_ray_tracing"
// CHECK: %[[REG:[A-Za-z0-9_]+]] = OpConvertUToAccelerationStructureKHR
// CHECK: OpRayQueryInitializeKHR %rayQuery{{.*}} %[[REG]]

[numthreads(1,1,1)]
void main()
{
    let accelStruct = RaytracingAccelerationStructure(accelStructAddr);
    RayQuery rayQuery;
    RayDesc ray;
    ray.Direction = float3(0, 0, 1);
    ray.Origin = float3(0, 0, 0);
    ray.TMax = 1000;
    ray.TMin = 0;
    let rs = rayQuery.TraceRayInline(accelStruct, 0, 0, ray);
    output[0] = rayQuery.CandidateGeometryIndex();
}
