//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderEXT

// CHECK-DAG: OpExtension "SPV_KHR_ray_tracing_position_fetch"
// CHECK-DAG: OpCapability RayTracingPositionFetchKHR
// CHECK-DAG: OpExtension "SPV_EXT_shader_invocation_reorder"
// CHECK-DAG: OpCapability ShaderInvocationReorderEXT
// CHECK: OpHitObjectGetIntersectionTriangleVertexPositionsEXT

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

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

    float3 positions[3];
    hit.GetTriangleVertexPositions(positions);

    outputBuffer[0] = uint(positions[0].x + positions[1].y + positions[2].z);
}
