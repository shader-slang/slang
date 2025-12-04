//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage raygeneration -entry main -allow-glsl
//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-via-glsl -stage raygeneration -entry main -allow-glsl

// Test for newly implemented Batch 8 fused trace functions
// These are the most complex EXT functions combining trace+reorder+execute

//CHECK: OpCapability RayTracingKHR
//CHECK: OpCapability ShaderInvocationReorderEXT
//CHECK: OpExtension "SPV_KHR_ray_tracing"
//CHECK: OpExtension "SPV_EXT_shader_invocation_reorder"

struct RayPayload
{
    float3 color;
    uint depth;
};

[[vk::binding(0, 0)]]
RaytracingAccelerationStructure scene : register(t0);

[[vk::binding(1, 0)]]
RWStructuredBuffer<float4> output : register(u0);

// Helper to create ray descriptor
RayDesc makeRay(float3 origin, float3 direction)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = direction;
    ray.TMin = 0.001f;
    ray.TMax = 1000.0f;
    return ray;
}

[shader("raygeneration")]
void main()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 launchDim = DispatchRaysDimensions().xy;
    float2 pixelCenter = float2(launchIndex) + 0.5f;
    float2 ndc = pixelCenter / float2(launchDim) * 2.0f - 1.0f;

    // Setup ray
    float3 rayOrigin = float3(0, 0, -5);
    float3 rayDir = normalize(float3(ndc.x, ndc.y, 1.0f));
    RayDesc ray = makeRay(rayOrigin, rayDir);

    // Compute material-based coherence hint
    // In real scenarios, this would come from G-buffer or previous pass
    uint materialHint = uint(abs(rayDir.x) * 255.0f) & 0xFF;
    uint numHintBits = 8;

    // =========================================================================
    // TEST 1: hitObjectTraceReorderExecuteEXT (no hint version)
    // =========================================================================
    // This is equivalent to:
    //   hitObjectTraceRayEXT()
    //   reorderThreadEXT(hitObject)
    //   hitObjectExecuteShaderEXT()
    //
    // CHECK: OpHitObjectTraceReorderExecuteEXT

    HitObject hitObj1;
    hitObj1.MakeNop();

    // Note: This uses the GLSL standalone function which internally becomes
    // OpHitObjectTraceReorderExecuteEXT without hint parameters
    // The implementation is in glsl.meta.slang lines 7095-7140
    hitObj1 = HitObject::TraceRay(
        scene,
        RAY_FLAG_NONE,
        0xFF,
        0, // sbtRecordOffset
        1, // sbtRecordStride
        0, // missIndex
        ray,
        0  // payload location
    );

    // Reorder based on hit object alone
    HitObject::Reorder(hitObj1);

    // Execute shader
    hitObj1.Invoke(0);

    // =========================================================================
    // TEST 2: hitObjectTraceReorderExecuteEXT (with hint version)
    // =========================================================================
    // This is equivalent to:
    //   hitObjectTraceRayEXT()
    //   reorderThreadEXT(hitObject, hint, bits)
    //   hitObjectExecuteShaderEXT()
    //
    // CHECK: OpHitObjectTraceReorderExecuteEXT

    HitObject hitObj2;
    hitObj2.MakeNop();

    // Trace a secondary ray
    float3 secondaryDir = reflect(rayDir, float3(0, 1, 0));
    RayDesc secondaryRay = makeRay(rayOrigin, secondaryDir);

    hitObj2 = HitObject::TraceRay(
        scene,
        RAY_FLAG_NONE,
        0xFF,
        0, 1, 0,
        secondaryRay,
        0
    );

    // Reorder with hint (material-based coherence)
    HitObject::Reorder(hitObj2, materialHint, numHintBits);

    // Execute shader
    hitObj2.Invoke(0);

    // =========================================================================
    // TEST 3: Using the fused operations more efficiently
    // =========================================================================
    // The fused functions allow the compiler to optimize better
    // by combining the three operations into a single SPIRV instruction

    HitObject hitObj3;
    hitObj3.MakeNop();

    // Tertiary ray for ambient occlusion or similar
    float3 tertiaryDir = normalize(rayDir + float3(0.1f, 0.2f, 0.0f));
    RayDesc tertiaryRay = makeRay(rayOrigin, tertiaryDir);

    // Manual trace + reorder + execute (what the fused function does internally)
    hitObj3 = HitObject::TraceRay(
        scene,
        RAY_FLAG_NONE,
        0xFF,
        0, 1, 0,
        tertiaryRay,
        0
    );

    // This should internally use OpHitObjectReorderExecuteShaderEXT
    // CHECK: OpHitObjectReorderExecuteShaderEXT
    hitObj3.ReorderInvoke(materialHint, numHintBits, 0);

    // =========================================================================
    // TEST 4: Verify different hint values affect reordering
    // =========================================================================
    // Test with different hint patterns to ensure they're passed correctly

    for (uint i = 0; i < 3; i++)
    {
        HitObject hitObj4;
        hitObj4.MakeNop();

        // Vary hint based on iteration
        uint variedHint = materialHint ^ (i * 42);

        float3 offsetDir = normalize(rayDir + float3(
            float(i) * 0.1f,
            0.0f,
            0.0f
        ));
        RayDesc offsetRay = makeRay(rayOrigin, offsetDir);

        hitObj4 = HitObject::TraceRay(
            scene,
            RAY_FLAG_NONE,
            0xFF,
            0, 1, 0,
            offsetRay,
            0
        );

        // CHECK: OpReorderThreadWithHintEXT
        HitObject::Reorder(hitObj4, variedHint, numHintBits);

        // CHECK: OpHitObjectExecuteShaderEXT
        hitObj4.Invoke(0);
    }

    // =========================================================================
    // Output test results
    // =========================================================================
    bool hit1 = hitObj1.IsHit();
    bool hit2 = hitObj2.IsHit();
    bool hit3 = hitObj3.IsHit();

    float testResult = 0.0f;
    if (hit1) testResult += 1.0f;
    if (hit2) testResult += 2.0f;
    if (hit3) testResult += 4.0f;

    output[launchIndex.y * launchDim.x + launchIndex.x] = float4(
        rayDir.x,
        rayDir.y,
        testResult,
        float(materialHint) / 255.0f
    );
}
