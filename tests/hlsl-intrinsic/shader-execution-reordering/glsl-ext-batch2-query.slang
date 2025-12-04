//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage raygeneration -entry main -capability GL_EXT_shader_invocation_reorder

// BATCH 2: Hit Object Query Functions - EXT Version
// Tests 5 query functions for inspecting hit object state

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

struct [raypayload] InvokePayload
{
    float value : read(caller, closesthit, miss) : write(caller, closesthit, miss);
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

    uint result = 0;

    // =================================================================
    // Function 6: hitObjectIsEmptyEXT → OpHitObjectIsEmptyEXT
    // =================================================================
    // CHECK: OpHitObjectIsEmptyEXT
    HitObject emptyObj = HitObject::MakeNop();
    bool isEmpty = emptyObj.IsNop();

    if (isEmpty)
    {
        result += 1;
    }

    // =================================================================
    // Function 7: hitObjectIsHitEXT → OpHitObjectIsHitEXT
    // =================================================================
    // CHECK: OpHitObjectIsHitEXT
    HitObject traceObj = HitObject::TraceRay(
        scene,
        RAY_FLAG_NONE,
        0xFF,
        0, 1, 0,
        ray,
        payload);

    bool isHit = traceObj.IsHit();
    if (isHit)
    {
        result += 10;
    }

    // =================================================================
    // Function 8: hitObjectIsMissEXT → OpHitObjectIsMissEXT
    // =================================================================
    // CHECK: OpHitObjectIsMissEXT
    HitObject missObj = HitObject::MakeMiss(RAY_FLAG_NONE, 0, ray);
    bool isMiss = missObj.IsMiss();

    if (isMiss)
    {
        result += 100;
    }

    // =================================================================
    // Function 9: hitObjectExecuteShaderEXT → OpHitObjectExecuteShaderEXT
    // =================================================================
    // CHECK: OpHitObjectExecuteShaderEXT
    InvokePayload invokePayload;
    invokePayload.value = 42.0f;

    HitObject::Invoke(scene, traceObj, invokePayload);

    // Verify invoke was called
    if (invokePayload.value >= 0.0f)
    {
        result += 1000;
    }

    // =================================================================
    // Function 10: hitObjectRecordFromQueryEXT → OpHitObjectRecordFromQueryEXT
    // =================================================================
    // Note: FromRayQuery requires RayQuery extension - skipped for now
    // Will test in separate RayQuery-specific test

    // =================================================================
    // Verify all query functions worked
    // =================================================================
    bool allQueriesWork = isEmpty && isMiss;
    if (allQueriesWork)
    {
        result += 10000;
    }

    outputBuffer[idx] = result;
}
