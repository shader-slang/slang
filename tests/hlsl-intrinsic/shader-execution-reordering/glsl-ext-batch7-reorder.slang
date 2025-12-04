//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage raygeneration -entry main -capability GL_EXT_shader_invocation_reorder

// BATCH 7: Reorder Functions - EXT Version
// Tests ReorderThread overloads

//CHECK: OpCapability ShaderInvocationReorderEXT
//CHECK: OpExtension "SPV_EXT_shader_invocation_reorder"

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
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

    // =================================================================
    // Function 31: reorderThreadEXT(uint, uint) → OpReorderThreadWithHintEXT
    // =================================================================
    // CHECK: OpReorderThreadWithHintEXT
    uint hint = 42;
    uint bits = 16;
    ReorderThread(hint, bits);

    // =================================================================
    // Function 32: reorderThreadEXT(hitObjectEXT) → OpReorderThreadWithHitObjectEXT
    // =================================================================
    // CHECK: OpReorderThreadWithHitObjectEXT
    ReorderThread(hitObj);

    // =================================================================
    // Function 33: reorderThreadEXT(hitObjectEXT, uint, uint) → OpReorderThreadWithHitObjectEXT + hint
    // =================================================================
    // CHECK: OpReorderThreadWithHitObjectEXT
    ReorderThread(hitObj, hint, bits);

    // =================================================================
    // Function 34: OpTypeHitObjectEXT (Type declaration - verified by compilation)
    // =================================================================
    // Type is used throughout, verified by successful compilation

    // =================================================================
    // Function 35: OpHitObjectRecordHitWithIndexEXT (Skipped - MakeHit needs EXT support)
    // =================================================================
    // Note: MakeHit overloads currently only support spirv_nv, not spirv (EXT)
    // TODO: Add EXT support to MakeHit in future work

    outputBuffer[idx] = 1;
}
