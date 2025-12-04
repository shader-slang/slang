//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage raygeneration -entry main -capability GL_EXT_shader_invocation_reorder
//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-via-glsl -stage raygeneration -entry main -capability GL_EXT_shader_invocation_reorder

// BATCH 1: Core Hit Object Creation Functions - EXT Version
// Tests 5 fundamental functions using HitObject methods (NOT NV-specific)

//CHECK: OpCapability RayTracingKHR
//CHECK: OpCapability ShaderInvocationReorderEXT
//CHECK: OpExtension "SPV_KHR_ray_tracing"
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

    // =================================================================
    // Function 1: HitObject::MakeNop() → OpHitObjectRecordEmptyEXT
    // =================================================================
    // CHECK: OpHitObjectRecordEmptyEXT
    HitObject emptyObj = HitObject::MakeNop();

    uint result = 0;
    if (emptyObj.IsNop())
    {
        result += 1;
    }

    // =================================================================
    // Function 2: HitObject::MakeMiss() → OpHitObjectRecordMissEXT
    // =================================================================
    // CHECK: OpHitObjectRecordMissEXT
    HitObject missObj = HitObject::MakeMiss(RAY_FLAG_NONE, 0, ray);

    if (missObj.IsMiss())
    {
        result += 10;
    }

    // =================================================================
    // Function 4: HitObject::TraceRay() → OpHitObjectTraceRayEXT
    // =================================================================
    // CHECK: OpHitObjectTraceRayEXT
    HitObject traceObj = HitObject::TraceRay(
        scene,
        RAY_FLAG_NONE,
        0xFF, // cullMask
        0,    // sbtRecordOffset
        1,    // sbtRecordStride
        0,    // missIndex
        ray,
        payload);

    // Verify trace completed
    if (traceObj.IsHit() || traceObj.IsMiss())
    {
        result += 100;
    }

    // =================================================================
    // Verify query functions work on created objects
    // =================================================================
    // CHECK: OpHitObjectIsEmptyEXT
    // CHECK: OpHitObjectIsMissEXT
    // CHECK: OpHitObjectIsHitEXT

    bool isEmpty = emptyObj.IsNop();
    bool isMiss = missObj.IsMiss();
    bool isTraced = (traceObj.IsHit() || traceObj.IsMiss());

    if (isEmpty && isMiss && isTraced)
    {
        result += 1000;
    }

    outputBuffer[idx] = result;
}
