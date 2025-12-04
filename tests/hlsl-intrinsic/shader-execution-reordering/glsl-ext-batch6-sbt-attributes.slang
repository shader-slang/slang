//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage raygeneration -entry main -capability GL_EXT_shader_invocation_reorder

// BATCH 6: SBT and Attributes Functions - EXT Version
// Tests SBT record index, shader record buffer handle, and attributes functions

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

struct MyAttributes
{
    float2 bary;
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
    // Function 26: hitObjectGetAttributesEXT → OpHitObjectGetAttributesEXT
    // =================================================================
    // CHECK: OpHitObjectGetAttributesEXT
    if (hitObj.IsHit())
    {
        MyAttributes attr = hitObj.GetAttributes<MyAttributes>();
        result += uint(attr.bary.x * 100.0);
    }

    // =================================================================
    // Function 27: hitObjectGetCurrentTimeEXT (skipped - needs motion blur)
    // =================================================================
    // Note: Requires motion blur extension - will test separately

    // =================================================================
    // Function 28: hitObjectGetShaderBindingTableRecordIndexEXT → OpHitObjectGetShaderBindingTableRecordIndexEXT
    // =================================================================
    // CHECK: OpHitObjectGetShaderBindingTableRecordIndexEXT
    uint sbtIdx = hitObj.GetShaderTableIndex();
    result += sbtIdx;

    // =================================================================
    // Function 29: hitObjectSetShaderBindingTableRecordIndexEXT → OpHitObjectSetShaderBindingTableRecordIndexEXT
    // =================================================================
    // CHECK: OpHitObjectSetShaderBindingTableRecordIndexEXT
    hitObj.SetShaderTableIndex(sbtIdx + 1);

    // =================================================================
    // Function 30: hitObjectGetShaderRecordBufferHandleEXT → OpHitObjectGetShaderRecordBufferHandleEXT
    // =================================================================
    // CHECK: OpHitObjectGetShaderRecordBufferHandleEXT
    uint2 handle = hitObj.GetShaderRecordBufferHandle();
    result += handle.x;

    outputBuffer[idx] = result;
}
