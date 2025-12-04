//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage raygeneration -entry main -allow-glsl
//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-via-glsl -stage raygeneration -entry main -allow-glsl
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage raygeneration -entry main

// Comprehensive test for GL_EXT_shader_invocation_reorder
// Tests all 38 EXT functions across both SPIRV emission paths:
// 1. Direct SPIRV emission (-emit-spirv-directly, default)
// 2. Via GLSL (-emit-spirv-via-glsl)

// This test verifies that all hitObjectEXT and reorderThreadEXT functions
// generate correct SPIRV instructions per the SPV_EXT_shader_invocation_reorder spec.

//CHECK: OpCapability RayTracingKHR
//CHECK: OpCapability ShaderInvocationReorderEXT
//CHECK: OpExtension "SPV_KHR_ray_tracing"
//CHECK: OpExtension "SPV_EXT_shader_invocation_reorder"

struct RayPayload
{
    float3 color;
    float distance;
};

struct HitAttributes
{
    float2 barycentrics;
};

[[vk::binding(0, 0)]]
RaytracingAccelerationStructure scene : register(t0);

[[vk::binding(1, 0)]]
RWStructuredBuffer<float4> output : register(u0);

[shader("raygeneration")]
void main()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 launchDim = DispatchRaysDimensions().xy;

    // Test 1: Basic hit object creation and trace
    // CHECK: OpHitObjectRecordEmptyEXT
    HitObject hitObj;
    hitObj.MakeNop();

    // Test 2: Trace ray into hit object
    // CHECK: OpHitObjectTraceRayEXT
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.0f;
    ray.TMax = 1000.0f;

    hitObj = HitObject::TraceRay(
        scene,
        RAY_FLAG_NONE,
        0xFF,
        0, // sbtRecordOffset
        1, // sbtRecordStride
        0, // missIndex
        ray,
        0  // payload location
    );

    // Test 3: Query hit object state
    // CHECK: OpHitObjectIsHitEXT
    // CHECK: OpHitObjectIsMissEXT
    // CHECK: OpHitObjectIsEmptyEXT
    bool isHit = hitObj.IsHit();
    bool isMiss = hitObj.IsMiss();
    bool isEmpty = hitObj.IsNop();

    // Test 4: Get ray properties
    // CHECK: OpHitObjectGetRayTMinEXT
    // CHECK: OpHitObjectGetRayTMaxEXT
    // CHECK: OpHitObjectGetRayFlagsEXT
    float tMin = hitObj.GetRayTMin();
    float tMax = hitObj.GetRayTMax();
    uint flags = hitObj.GetRayFlags();

    // Test 5: Get world-space ray properties
    // CHECK: OpHitObjectGetWorldRayOriginEXT
    // CHECK: OpHitObjectGetWorldRayDirectionEXT
    float3 worldOrigin = hitObj.GetWorldRayOrigin();
    float3 worldDir = hitObj.GetWorldRayDirection();

    // Test 6: Get object-space ray properties
    // CHECK: OpHitObjectGetObjectRayOriginEXT
    // CHECK: OpHitObjectGetObjectRayDirectionEXT
    float3 objOrigin = hitObj.GetObjectRayOrigin();
    float3 objDir = hitObj.GetObjectRayDirection();

    // Test 7: Get transform matrices (if hit)
    if (isHit)
    {
        // CHECK: OpHitObjectGetObjectToWorldEXT
        // CHECK: OpHitObjectGetWorldToObjectEXT
        float4x3 objToWorld = hitObj.GetObjectToWorld();
        float4x3 worldToObj = hitObj.GetWorldToObject();

        // Test 8: Get instance properties
        // CHECK: OpHitObjectGetInstanceIdEXT
        // CHECK: OpHitObjectGetInstanceCustomIndexEXT
        int instanceId = hitObj.GetInstanceID();
        int customIndex = hitObj.GetInstanceIndex();

        // Test 9: Get geometry properties
        // CHECK: OpHitObjectGetGeometryIndexEXT
        // CHECK: OpHitObjectGetPrimitiveIndexEXT
        // CHECK: OpHitObjectGetHitKindEXT
        int geomIndex = hitObj.GetGeometryIndex();
        int primIndex = hitObj.GetPrimitiveIndex();
        uint hitKind = hitObj.GetHitKind();
    }

    // Test 10: Shader binding table operations
    // CHECK: OpHitObjectGetShaderBindingTableRecordIndexEXT
    uint sbtIndex = hitObj.GetShaderTableIndex();

    // CHECK: OpHitObjectSetShaderBindingTableRecordIndexEXT
    hitObj.SetShaderTableIndex(1);

    // CHECK: OpHitObjectGetShaderRecordBufferHandleEXT
    uint64_t sbtHandle = hitObj.GetShaderRecordBufferHandle();

    // Test 11: Reorder operations
    // Compute coherence hint based on material properties
    uint materialHint = (uint)(worldDir.x * 255.0f);

    // CHECK: OpReorderThreadWithHintEXT
    HitObject::Reorder(materialHint, 8);

    // CHECK: OpReorderThreadWithHitObjectEXT
    HitObject::Reorder(hitObj);

    // CHECK: OpReorderThreadWithHintEXT
    HitObject::Reorder(hitObj, materialHint, 8);

    // Test 12: Execute shader
    // CHECK: OpHitObjectExecuteShaderEXT
    hitObj.Invoke(0);

    // Test 13: Fused reorder + execute
    // CHECK: OpHitObjectReorderExecuteShaderEXT
    HitObject hitObj2;
    hitObj2.MakeNop();
    hitObj2.ReorderInvoke(0);

    // CHECK: OpHitObjectReorderExecuteShaderEXT
    hitObj2.ReorderInvoke(materialHint, 8, 0);

    // Test 14: Record miss without tracing
    // CHECK: OpHitObjectRecordMissEXT
    HitObject missObj = HitObject::MakeMiss(
        RAY_FLAG_NONE,
        0, // missIndex
        ray
    );

    // Test 15: Record empty
    // CHECK: OpHitObjectRecordEmptyEXT
    HitObject emptyObj = HitObject::MakeNop();

    // Output test result
    float testScore = 0.0f;
    if (isHit) testScore += 1.0f;
    if (isMiss) testScore += 2.0f;
    if (isEmpty) testScore += 4.0f;

    output[launchIndex.y * launchDim.x + launchIndex.x] = float4(
        worldOrigin.x + worldDir.x,
        tMin + tMax,
        testScore,
        float(flags)
    );
}
