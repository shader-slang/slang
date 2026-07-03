//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderEXT -capability spvRayTracingPositionFetchKHR
//DIAGNOSTIC_TEST:SIMPLE(filecheck=NV_ERR): -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV -capability spvRayTracingPositionFetchKHR

// HitObject.GetTriangleVertexPositions is Vulkan-EXT only and lowers to the
// result-producing OpHitObjectGetIntersectionTriangleVertexPositionsEXT (opcode 5307),
// which requires the ray tracing position fetch extension.
//
// Under the NV capability a HitObject is emitted as OpTypeHitObjectNV, which the EXT
// opcode (requiring OpTypeHitObjectEXT) rejects. There is no NV opcode for this query,
// so the NV path is rejected at compile time rather than emitting invalid SPIR-V.

// CHECK-DAG: OpExtension "SPV_KHR_ray_tracing_position_fetch"
// CHECK-DAG: OpCapability RayTracingPositionFetchKHR
// CHECK-DAG: OpExtension "SPV_EXT_shader_invocation_reorder"
// CHECK-DAG: OpCapability ShaderInvocationReorderEXT
// CHECK: OpHitObjectGetIntersectionTriangleVertexPositionsEXT
// NV_ERR: static assertion failed
// NV_ERR: GetTriangleVertexPositions() has no SPIRV NV equivalent

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

struct MyPayload { float4 color; };

[shader("raygeneration")]
void rayGenerationMain()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    MyPayload payload = { float4(0) };
    HitObject hit = HitObject::TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    // Returned by value (float3[3]), matching RayQuery.CommittedGetIntersectionTriangleVertexPositions.
    float3 positions[3] = hit.GetTriangleVertexPositions();

    outputBuffer[0] = positions[0].x + positions[1].y + positions[2].z;
}
