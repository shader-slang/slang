//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage raygeneration -entry main
//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-via-glsl -stage raygeneration -entry main

// Simple validation test for GL_EXT_shader_invocation_reorder SPIRV emission
// Tests that EXT functions generate correct SPIRV opcodes in both emission paths

//CHECK: OpCapability RayTracingKHR
//CHECK: OpCapability ShaderInvocationReorderEXT
//CHECK: OpExtension "SPV_KHR_ray_tracing"
//CHECK: OpExtension "SPV_EXT_shader_invocation_reorder"

struct Payload
{
    float3 color;
};

[[vk::binding(0, 0)]]
RaytracingAccelerationStructure scene : register(t0);

[[vk::binding(1, 0)]]
RWStructuredBuffer<float4> output : register(u0);

[shader("raygeneration")]
void main()
{
    uint2 launchIndex = DispatchRaysIndex().xy;

    // Test 1: Record empty (MakeNop)
    // CHECK: OpHitObjectRecordEmptyEXT
    HitObject hitObj = HitObject::MakeNop();

    // Test 2: Trace ray
    // CHECK: OpHitObjectTraceRayEXT
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.001f;
    ray.TMax = 1000.0f;

    hitObj = HitObject::TraceRay(
        scene,
        RAY_FLAG_NONE,
        0xFF,
        0, 1, 0,
        ray,
        Payload()
    );

    // Test 3: Query state
    // CHECK: OpHitObjectIsHitEXT
    // CHECK: OpHitObjectIsMissEXT
    // CHECK: OpHitObjectIsEmptyEXT
    bool isHit = hitObj.IsHit();
    bool isMiss = hitObj.IsMiss();
    bool isEmpty = hitObj.IsNop();

    // Test 4: Get ray properties (if not empty)
    float tMin = 0;
    if (!isEmpty)
    {
        // CHECK: OpHitObjectGetRayTMinEXT
        tMin = hitObj.GetRayTMin();
    }

    // Test 5: Get instance properties (if hit)
    int instanceId = 0;
    if (isHit)
    {
        // CHECK: OpHitObjectGetInstanceIdEXT
        instanceId = hitObj.GetInstanceID();
    }

    // Test 6: Record miss
    // CHECK: OpHitObjectRecordMissEXT
    HitObject missObj = HitObject::MakeMiss(RAY_FLAG_NONE, 0, ray);

    // Test 7: Invoke shader with payload
    // CHECK: OpHitObjectExecuteShaderEXT
    Payload p;
    p.color = float3(1, 0, 0);
    HitObject::Invoke(scene, hitObj, p);

    // Output result
    output[launchIndex.y * DispatchRaysDimensions().x + launchIndex.x] =
        float4(p.color, isHit ? 1.0f : 0.0f);
}
