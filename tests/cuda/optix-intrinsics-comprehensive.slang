//TEST:SIMPLE(filecheck=CHECK): -target cuda
//TEST:SIMPLE(filecheck=CHECK_PTX): -target ptx

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

// Ray query and geometry intrinsics - order independent
//CHECK-DAG: optixGetHitKind()
//CHECK-DAG: optixGetInstanceIndex()
//CHECK-DAG: optixGetInstanceId()
//CHECK-DAG: optixGetObjectRayDirection()
//CHECK-DAG: optixGetObjectRayOrigin()
//CHECK-DAG: optixGetPrimitiveIndex()
//CHECK-DAG: optixGetRayFlags()
//CHECK-DAG: optixGetRayTmin()
//CHECK-DAG: optixGetWorldRayDirection()
//CHECK-DAG: optixGetWorldRayOrigin()

// HitObject intrinsics - uses generic templated call, not individual _N functions
//CHECK-DAG: optixHitObjectGetAttribute<CustomAttributes_0>
//CHECK-DAG: slangOptixHitObjectIsMiss
//CHECK-DAG: optixHitObjectGetHitKind()
//CHECK-DAG: optixGetObjectRayOrigin()
//CHECK-DAG: optixGetObjectRayDirection()

// PTX checks for HitObject methods in raygen
//CHECK_PTX-DAG: _optix_get_object_ray_origin_x
//CHECK_PTX-DAG: _optix_get_object_ray_direction_x

// Control flow intrinsics
//CHECK-DAG: optixTerminateRay
//CHECK-DAG: optixTraverse
//CHECK-DAG: optixMakeHitObject
//CHECK-DAG: optixIgnoreIntersection

// Transform matrix intrinsics
//CHECK-DAG: slangOptixGetObjectToWorldTransformMatrix
//CHECK-DAG: slangOptixGetWorldToObjectTransformMatrix
//CHECK-DAG: slangOptixGetObjectToWorldTransformMatrix4x3
//CHECK-DAG: slangOptixGetWorldToObjectTransformMatrix4x3

// Traditional SBT data access
//CHECK-DAG: optixGetSbtDataPointer

// PTX intrinsics validation - using actual PTX names (order independent)
//CHECK_PTX-DAG: _optix_get_hit_kind
//CHECK_PTX-DAG: _optix_read_instance_idx
//CHECK_PTX-DAG: _optix_read_instance_id
//CHECK_PTX-DAG: _optix_get_object_ray_direction_x
//CHECK_PTX-DAG: _optix_get_object_ray_origin_x
//CHECK_PTX-DAG: _optix_read_primitive_idx
//CHECK_PTX-DAG: _optix_get_ray_flags
//CHECK_PTX-DAG: _optix_get_ray_tmin
//CHECK_PTX-DAG: _optix_get_world_ray_direction_x
//CHECK_PTX-DAG: _optix_get_world_ray_origin_x
//CHECK_PTX-DAG: _optix_terminate_ray
//CHECK_PTX-DAG: _optix_ignore_intersection
//CHECK_PTX-DAG: _optix_hitobject_get_traverse_data
//CHECK_PTX-DAG: _optix_hitobject_make_with_traverse_data
//CHECK_PTX-DAG: _optix_get_sbt_data_ptr_64
//CHECK_PTX-DAG: _optix_hitobject_get_hitkind

struct CustomAttributes
{
    float attr0;
    uint attr1;
    float attr2;
    uint attr3;
    float attr4;
    uint attr5;
    float attr6;
    uint attr7;
};

struct IntrinsicData
{
    // Ray geometry data
    uint hitKind;
    uint instanceIndex;
    uint instanceID;
    uint primitiveIndex;
    float3 objectRayDirection;
    float3 objectRayOrigin;
    float3 worldRayDirection;
    float3 worldRayOrigin;
    float rayTmin;
    uint rayFlags;

    // Transform matrices
    float3x4 objectToWorld3x4;
    float3x4 worldToObject3x4;
    float4x3 objectToWorld4x3;
    float4x3 worldToObject4x3;

    // HitObject attributes
    float hitObjAttr0;
    uint hitObjAttr1;
    float hitObjAttr2;
    uint hitObjAttr3;
    float hitObjAttr4;
    uint hitObjAttr5;
    float hitObjAttr6;
    uint hitObjAttr7;

    // HitObject state
    bool isMiss;
};

struct RayPayload
{
    IntrinsicData data;
    bool shouldTerminate;
};

// Traditional SBT data structure
struct TraditionalSbtData
{
    float multiplier;
    uint flags;
    float3 color;
};

// Add a separate payload for anyhit shader testing
struct AnyHitPayload
{
    bool terminateRequested;
    uint hitCount;
};

[shader("closesthit")]
void closestHitShader(
    uniform TraditionalSbtData sbtData,  // This triggers traditional optixGetSbtDataPointer
    inout RayPayload payload, 
    in CustomAttributes attr)
{
    // Test basic ray query intrinsics using correct HLSL names
    payload.data.hitKind = HitKind();
    payload.data.instanceIndex = InstanceIndex();
    payload.data.instanceID = InstanceID();
    payload.data.primitiveIndex = PrimitiveIndex();
    payload.data.objectRayDirection = ObjectRayDirection();
    payload.data.objectRayOrigin = ObjectRayOrigin();
    payload.data.worldRayDirection = WorldRayDirection();
    payload.data.worldRayOrigin = WorldRayOrigin();
    payload.data.rayTmin = RayTMin();
    payload.data.rayFlags = RayFlags();

    // Test transform matrix intrinsics (now support CUDA via OptiX)
    payload.data.objectToWorld3x4 = ObjectToWorld3x4();
    payload.data.worldToObject3x4 = WorldToObject3x4();
    payload.data.objectToWorld4x3 = ObjectToWorld4x3();
    payload.data.worldToObject4x3 = WorldToObject4x3();
    
    // Test HitObject operations (using a NOP HitObject for simplicity)
    HitObject hitObj = HitObject::MakeNop();
    
    // Test HitObject attribute access with the correct API
    CustomAttributes hitObjAttrs = hitObj.GetAttributes<CustomAttributes>();
    payload.data.hitObjAttr0 = hitObjAttrs.attr0;
    payload.data.hitObjAttr1 = hitObjAttrs.attr1;
    payload.data.hitObjAttr2 = hitObjAttrs.attr2;
    payload.data.hitObjAttr3 = hitObjAttrs.attr3;
    payload.data.hitObjAttr4 = hitObjAttrs.attr4;
    payload.data.hitObjAttr5 = hitObjAttrs.attr5;
    payload.data.hitObjAttr6 = hitObjAttrs.attr6;
    payload.data.hitObjAttr7 = hitObjAttrs.attr7;
    
    // Test HitObject state queries
    payload.data.isMiss = hitObj.IsMiss();
    
    // Test traditional SBT data access - this should generate optixGetSbtDataPointer
    payload.data.rayTmin *= sbtData.multiplier;
    if (sbtData.flags > 0)
    {
        payload.data.rayTmin += sbtData.color.x + sbtData.color.y + sbtData.color.z;
    }
    
    // Mark test as completed
    payload.shouldTerminate = true;
}

[shader("anyhit")]
void anyHitShader(inout AnyHitPayload payload, in CustomAttributes attr)
{
    // Test anyhit-specific intrinsics
    uint hitKind = HitKind();
    uint instanceID = InstanceID();
    float rayT = RayTCurrent();
    
    // Count hits for testing
    payload.hitCount++;
    
    // Test termination based on some criteria
    if (payload.terminateRequested || payload.hitCount > 3)
    {
        // Test optixTerminateRay - only available in anyhit shaders
        AcceptHitAndEndSearch();  // This maps to optixTerminateRay for CUDA
    }
    else
    {
        // Test ignoring hit to continue traversal
        IgnoreHit();
    }
}

[shader("miss")]
void missShader(inout RayPayload payload)
{
    // Initialize with miss data
    payload.data.hitKind = 0;
    payload.data.instanceIndex = ~0u;
}

[shader("raygeneration")]
void rayGenShader()
{
    uint2 index = DispatchRaysIndex().xy;
    
    RayPayload payload;
    payload.shouldTerminate = (index.x % 2) == 0;
    
    RayDesc ray;
    ray.Origin = float3(index.x, index.y, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.001f;
    ray.TMax = 1000.0f;
    
    // Test optixTrace through HitObject TraceRay
    HitObject hit = HitObject::TraceRay(
        scene,
        RAY_FLAG_NONE,
        0xFF,
        0,
        1, 
        0,
        ray,
        payload.data
    );
    
    // Test optixMakeHitObject and optixHitObjectGetTraverseData
    // Create a custom hit object with specific parameters
    CustomAttributes testAttrs;
    testAttrs.attr0 = 1.0f;
    testAttrs.attr1 = 42;
    
    uint hitGroupRecordIndex = 0;
    uint instanceIndex = index.x;
    uint geometryIndex = 0;
    uint primitiveIndex = index.y;
    uint hitKind = 0;
    
    // Test optixMakeHitObject through HitObject::MakeHit
    HitObject customHit = HitObject::MakeHit(
        hitGroupRecordIndex,
        scene,
        instanceIndex,
        geometryIndex,
        primitiveIndex,
        hitKind,
        ray,
        testAttrs
    );
    
    // Test optixHitObjectGetTraverseData - this should be called internally
    // by the HitObject operations and generate the optix call
    bool isValidHit = customHit.IsHit();
    if (isValidHit)
    {
        // Access traverse data indirectly through HitObject queries
        CustomAttributes retrievedAttrs = customHit.GetAttributes<CustomAttributes>();
        payload.data.hitObjAttr0 = retrievedAttrs.attr0;
        payload.data.hitObjAttr1 = retrievedAttrs.attr1;

        // Test HitObject.GetHitKind() - should generate optixHitObjectGetHitKind()
        uint hitObjKind = customHit.GetHitKind();
        payload.data.hitKind = hitObjKind;

        // Test HitObject.GetObjectRayOrigin() and GetObjectRayDirection()
        float3 objOrigin = customHit.GetObjectRayOrigin();
        float3 objDirection = customHit.GetObjectRayDirection();
        payload.data.objectRayOrigin = objOrigin;
        payload.data.objectRayDirection = objDirection;
    }
}
