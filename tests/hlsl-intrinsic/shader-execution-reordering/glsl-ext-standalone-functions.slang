//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage raygeneration -entry main -allow-glsl
//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-via-glsl -stage raygeneration -entry main -allow-glsl

// Test for GLSL standalone EXT functions (glsl.meta.slang implementation)
// These are the public GLSL functions that map to SPIRV EXT instructions

//CHECK: OpCapability RayTracingKHR
//CHECK: OpCapability ShaderInvocationReorderEXT
//CHECK: OpExtension "SPV_KHR_ray_tracing"
//CHECK: OpExtension "SPV_EXT_shader_invocation_reorder"

struct RayPayload
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

    // Initialize hit object to empty
    // CHECK: OpHitObjectRecordEmptyEXT
    HitObject hitObj;
    hitObj.MakeNop();

    // Trace ray into hit object
    // CHECK: OpHitObjectTraceRayEXT
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = normalize(float3(
        float(launchIndex.x),
        float(launchIndex.y),
        1.0f
    ));
    ray.TMin = 0.001f;
    ray.TMax = 1000.0f;

    hitObj = HitObject::TraceRay(
        scene,
        RAY_FLAG_NONE,
        0xFF,
        0, 0, 0,
        ray,
        0
    );

    // Test all query functions
    // CHECK: OpHitObjectIsEmptyEXT
    // CHECK: OpHitObjectIsHitEXT
    // CHECK: OpHitObjectIsMissEXT
    bool isEmpty = hitObj.IsNop();
    bool isHit = hitObj.IsHit();
    bool isMiss = hitObj.IsMiss();

    float3 result = float3(0, 0, 0);

    // Test ray property queries
    if (!isEmpty)
    {
        // CHECK: OpHitObjectGetRayTMinEXT
        // CHECK: OpHitObjectGetRayTMaxEXT
        // CHECK: OpHitObjectGetRayFlagsEXT
        float tMin = hitObj.GetRayTMin();
        float tMax = hitObj.GetRayTMax();
        uint flags = hitObj.GetRayFlags();

        // CHECK: OpHitObjectGetWorldRayOriginEXT
        // CHECK: OpHitObjectGetWorldRayDirectionEXT
        float3 worldOrigin = hitObj.GetWorldRayOrigin();
        float3 worldDir = hitObj.GetWorldRayDirection();

        result += worldOrigin * 0.1f + worldDir * 0.1f;
        result += float3(tMin, tMax, float(flags)) * 0.001f;
    }

    // Test hit-specific queries
    if (isHit)
    {
        // CHECK: OpHitObjectGetObjectRayOriginEXT
        // CHECK: OpHitObjectGetObjectRayDirectionEXT
        float3 objOrigin = hitObj.GetObjectRayOrigin();
        float3 objDir = hitObj.GetObjectRayDirection();

        // CHECK: OpHitObjectGetObjectToWorldEXT
        // CHECK: OpHitObjectGetWorldToObjectEXT
        float4x3 o2w = hitObj.GetObjectToWorld();
        float4x3 w2o = hitObj.GetWorldToObject();

        // CHECK: OpHitObjectGetInstanceIdEXT
        // CHECK: OpHitObjectGetInstanceCustomIndexEXT
        int instId = hitObj.GetInstanceID();
        int custIdx = hitObj.GetInstanceIndex();

        // CHECK: OpHitObjectGetGeometryIndexEXT
        // CHECK: OpHitObjectGetPrimitiveIndexEXT
        int geomIdx = hitObj.GetGeometryIndex();
        int primIdx = hitObj.GetPrimitiveIndex();

        // CHECK: OpHitObjectGetHitKindEXT
        uint hitKind = hitObj.GetHitKind();

        result += objOrigin * 0.01f + objDir * 0.01f;
        result += float3(instId, custIdx, geomIdx) * 0.0001f;
    }

    // Test SBT functions
    // CHECK: OpHitObjectGetShaderBindingTableRecordIndexEXT
    uint sbtIdx = hitObj.GetShaderTableIndex();

    // CHECK: OpHitObjectSetShaderBindingTableRecordIndexEXT
    hitObj.SetShaderTableIndex(sbtIdx + 1);

    // CHECK: OpHitObjectGetShaderRecordBufferHandleEXT
    uint64_t sbtHandle = hitObj.GetShaderRecordBufferHandle();

    // Test reorder functions - all 3 overloads
    uint coherenceHint = uint(result.x * 255.0f) & 0xFF;

    // CHECK: OpReorderThreadWithHintEXT
    HitObject::Reorder(coherenceHint, 8);

    // CHECK: OpReorderThreadWithHitObjectEXT
    HitObject::Reorder(hitObj);

    // CHECK: OpReorderThreadWithHintEXT
    HitObject::Reorder(hitObj, coherenceHint, 8);

    // Test execute
    // CHECK: OpHitObjectExecuteShaderEXT
    hitObj.Invoke(0);

    // Test fused reorder+execute - both overloads
    // CHECK: OpHitObjectReorderExecuteShaderEXT
    hitObj.ReorderInvoke(0);

    // CHECK: OpHitObjectReorderExecuteShaderEXT
    hitObj.ReorderInvoke(coherenceHint, 8, 0);

    // Test record functions
    // CHECK: OpHitObjectRecordMissEXT
    HitObject missObj = HitObject::MakeMiss(
        RAY_FLAG_NONE,
        0,
        ray
    );

    // CHECK: OpHitObjectRecordEmptyEXT
    HitObject nopObj = HitObject::MakeNop();

    // Output results
    output[launchIndex.y * DispatchRaysDimensions().x + launchIndex.x] =
        float4(result, isHit ? 1.0f : 0.0f);
}
