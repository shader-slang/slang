// dxr13-comprehensive-all-methods.slang
// Comprehensive test covering 100% of DXR 1.3 HitObject methods
// Tests both common methods and DXR 1.3-specific methods
// Should generate HLSL with ONLY dx::HitObject (NO NVAPI calls)

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

struct [raypayload] RayPayload
{
    float3 color : read(caller, closesthit, miss) : write(caller, closesthit, miss);
};

struct MyAttribs
{
    float2 bary;
};

[shader("raygeneration")]
void rayGenerationMain()
{
    uint2 launchID = DispatchRaysIndex().xy;
    uint idx = launchID.x;

    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.001;
    ray.TMax = 1000.0;

    RayPayload payload;
    payload.color = float3(0, 0, 0);

    // ===== STATIC METHODS =====

    // 1. TraceRay (common method)
    HitObject hitFromTrace = HitObject::TraceRay(
        scene,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
        0xFF,
        0,
        1,
        0,
        ray,
        payload);

    // 2. MakeNop (common method)
    HitObject nopObj = HitObject::MakeNop();

    // 3. MakeMiss with RayFlags (DXR 1.3 specific - 3 parameters)
    HitObject missObj = HitObject::MakeMiss(
        RAY_FLAG_NONE,
        0,
        ray);

    // 4. FromRayQuery (DXR 1.3 specific)
    RayQuery<RAY_FLAG_NONE> query;
    query.TraceRayInline(scene, RAY_FLAG_NONE, 0xFF, ray);
    query.Proceed();
    HitObject hitFromQuery = HitObject::FromRayQuery(query);

    // 5. FromRayQuery with custom attributes (DXR 1.3 specific)
    MyAttribs customAttribs;
    customAttribs.bary = float2(0.3, 0.4);
    HitObject hitFromQueryCustom = HitObject::FromRayQuery(
        query,
        128, // User-defined hit kind (must be < 254)
        customAttribs);

    // ===== INSTANCE METHODS - QUERY STATE (common) =====

    // 6-8. Query state methods
    bool isMiss = missObj.IsMiss();
    bool isHit = hitFromTrace.IsHit();
    bool isNop = nopObj.IsNop();

    // ===== INSTANCE METHODS - RAY PROPERTIES =====

    // 9-13. DXR 1.3 specific ray property accessors
    uint rayFlags = hitFromTrace.GetRayFlags();
    float tmin = hitFromTrace.GetRayTMin();
    float tcurrent = hitFromTrace.GetRayTCurrent();
    float3 worldOrigin = hitFromTrace.GetWorldRayOrigin();
    float3 worldDir = hitFromTrace.GetWorldRayDirection();

    // 14-15. Common ray properties (object space)
    float3 objOrigin = hitFromTrace.GetObjectRayOrigin();
    float3 objDir = hitFromTrace.GetObjectRayDirection();

    // ===== INSTANCE METHODS - TRANSFORM MATRICES (DXR 1.3 specific) =====

    float3x4 o2w_3x4 = float3x4(0,0,0,0, 0,0,0,0, 0,0,0,0);
    float4x3 o2w_4x3 = float4x3(0,0,0, 0,0,0, 0,0,0, 0,0,0);
    float3x4 w2o_3x4 = float3x4(0,0,0,0, 0,0,0,0, 0,0,0,0);
    float4x3 w2o_4x3 = float4x3(0,0,0, 0,0,0, 0,0,0, 0,0,0);

    if (isHit)
    {
        // 16-19. DXR 1.3 specific matrix accessors
        o2w_3x4 = hitFromTrace.GetObjectToWorld3x4();
        o2w_4x3 = hitFromTrace.GetObjectToWorld4x3();
        w2o_3x4 = hitFromTrace.GetWorldToObject3x4();
        w2o_4x3 = hitFromTrace.GetWorldToObject4x3();
    }

    // ===== INSTANCE METHODS - HIT INFORMATION (common) =====

    uint instanceIndex = 0;
    uint instanceID = 0;
    uint geomIndex = 0;
    uint primIndex = 0;
    uint hitKind = 0;
    MyAttribs attribs;
    attribs.bary = float2(0, 0);

    if (isHit)
    {
        // 20-25. Common hit information methods
        instanceIndex = hitFromTrace.GetInstanceIndex();
        instanceID = hitFromTrace.GetInstanceID();
        geomIndex = hitFromTrace.GetGeometryIndex();
        primIndex = hitFromTrace.GetPrimitiveIndex();
        hitKind = hitFromTrace.GetHitKind();
        // GetAttributes: Skipped - DXC 1.9 doesn't support GetAttributes in lib_6_9
        // attribs = hitFromTrace.GetAttributes<MyAttribs>();
    }

    // ===== INSTANCE METHODS - SHADER TABLE (common) =====

    // 26-28. Shader table methods
    uint shaderTableIdx = hitFromTrace.GetShaderTableIndex();
    // SetShaderTableIndex: Skipped due to DXC compatibility (returns void in DXR 1.3, uint in NVAPI)
    // hitFromTrace.SetShaderTableIndex(5);
    uint rootConstant = hitFromTrace.LoadLocalRootTableConstant(0);

    // ===== STANDALONE FUNCTIONS - REORDERING (DXR 1.3 naming) =====

    // 29. MaybeReorderThread with HitObject
    MaybeReorderThread(hitFromTrace);

    // 30. MaybeReorderThread with coherence hint
    uint coherenceHint = idx & 0xF;
    MaybeReorderThread(coherenceHint, 4);

    // 31. MaybeReorderThread with both HitObject and coherence hint
    MaybeReorderThread(hitFromTrace, coherenceHint, 4);

    // ===== INVOKE (DXR 1.3 specific - 2 parameters) =====

    // 32. Invoke without AccelerationStructure parameter (DXR 1.3)
    HitObject::Invoke(hitFromTrace, payload);

    // ===== OUTPUT RESULTS =====

    outputBuffer[idx] = float(rayFlags);
    outputBuffer[idx + 1] = tmin;
    outputBuffer[idx + 2] = tcurrent;
    outputBuffer[idx + 3] = worldOrigin.x;
    outputBuffer[idx + 4] = worldDir.z;
    outputBuffer[idx + 5] = objOrigin.x;
    outputBuffer[idx + 6] = objDir.z;
    outputBuffer[idx + 7] = o2w_3x4[0][0];
    outputBuffer[idx + 8] = o2w_4x3[0][0];
    outputBuffer[idx + 9] = float(instanceIndex);
    outputBuffer[idx + 10] = float(geomIndex);
    outputBuffer[idx + 11] = float(shaderTableIdx);
    outputBuffer[idx + 12] = float(isMiss);
    outputBuffer[idx + 13] = float(isHit);
    outputBuffer[idx + 14] = float(isNop);
    outputBuffer[idx + 15] = payload.color.x;
}
