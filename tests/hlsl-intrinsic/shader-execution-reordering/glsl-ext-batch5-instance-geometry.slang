//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage raygeneration -entry main -capability GL_EXT_shader_invocation_reorder

// BATCH 5: Instance and Geometry Getters - EXT Version
// Tests 5 instance and geometry property functions

//CHECK: OpCapability ShaderInvocationReorderEXT
//CHECK: OpExtension "SPV_EXT_shader_invocation_reorder"

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

struct [raypayload] RayPayload
{
    float3 color : read(caller, closesthit, miss) : write(caller, closesthit, miss);
};

[shader("raygeneration")]
void main()
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

    HitObject hitObj = HitObject::TraceRay(
        scene,
        RAY_FLAG_NONE,
        0xFF,
        0, 1, 0,
        ray,
        payload);

    uint result = 0;

    // =================================================================
    // Function 21: hitObjectGetGeometryIndexEXT → OpHitObjectGetGeometryIndexEXT
    // =================================================================
    // CHECK: OpHitObjectGetGeometryIndexEXT
    if (hitObj.IsHit())
    {
        uint geomIdx = hitObj.GetGeometryIndex();
        result += geomIdx;
    }

    // =================================================================
    // Function 22: hitObjectGetPrimitiveIndexEXT → OpHitObjectGetPrimitiveIndexEXT
    // =================================================================
    // CHECK: OpHitObjectGetPrimitiveIndexEXT
    if (hitObj.IsHit())
    {
        uint primIdx = hitObj.GetPrimitiveIndex();
        result += primIdx;
    }

    // =================================================================
    // Function 23: hitObjectGetHitKindEXT → OpHitObjectGetHitKindEXT
    // =================================================================
    // CHECK: OpHitObjectGetHitKindEXT
    if (hitObj.IsHit())
    {
        uint hitKind = hitObj.GetHitKind();
        result += hitKind;
    }

    // =================================================================
    // Function 24: hitObjectGetInstanceIdEXT → OpHitObjectGetInstanceIdEXT
    // =================================================================
    // CHECK: OpHitObjectGetInstanceIdEXT
    if (hitObj.IsHit())
    {
        uint instanceId = hitObj.GetInstanceID();
        result += instanceId;
    }

    // =================================================================
    // Function 25: hitObjectGetInstanceCustomIndexEXT → OpHitObjectGetInstanceCustomIndexEXT
    // =================================================================
    // CHECK: OpHitObjectGetInstanceCustomIndexEXT
    if (hitObj.IsHit())
    {
        uint customIdx = hitObj.GetInstanceIndex();
        result += customIdx;
    }

    outputBuffer[idx] = result;
}
