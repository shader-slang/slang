//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry rayGenerationMain -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry rayGenerationMain -stage raygeneration -emit-spirv-via-glsl

// Test GL_EXT_shader_invocation_reorder SPIRV emission
// Verifies EXT functions generate correct SPIRV opcodes in both emission paths:
// 1. -emit-spirv-directly (default)
// 2. -emit-spirv-via-glsl

//CHECK: OpCapability RayTracingKHR
//CHECK: OpCapability ShaderInvocationReorderEXT
//CHECK: OpExtension "SPV_KHR_ray_tracing"
//CHECK: OpExtension "SPV_EXT_shader_invocation_reorder"

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

struct Attributes
{
    float2 barycentrics;
};

uint calcValue(HitObject hit)
{
    // CHECK-DAG: OpHitObjectIsHitEXT
    // CHECK-DAG: OpHitObjectIsMissEXT
    // CHECK-DAG: OpHitObjectIsEmptyEXT
    uint r = 0;

    if (hit.IsHit())
    {
        // CHECK-DAG: OpHitObjectGetInstanceIdEXT
        // CHECK-DAG: OpHitObjectGetInstanceCustomIndexEXT
        // CHECK-DAG: OpHitObjectGetGeometryIndexEXT
        // CHECK-DAG: OpHitObjectGetPrimitiveIndexEXT
        // CHECK-DAG: OpHitObjectGetHitKindEXT
        uint instanceId = hit.GetInstanceID();
        uint customIndex = hit.GetInstanceIndex();
        uint geomIndex = hit.GetGeometryIndex();
        uint primIndex = hit.GetPrimitiveIndex();
        uint hitKind = hit.GetHitKind();

        r += instanceId + customIndex + geomIndex + primIndex + hitKind;

        RayDesc rayDesc = hit.GetRayDesc();
        r += uint(rayDesc.TMin > 0);
    }
    else if (hit.IsMiss())
    {
        r += 100;
    }
    else if (hit.IsNop())
    {
        r += 200;
    }

    return r;
}

void rayGenerationMain()
{
    int2 launchID = int2(DispatchRaysIndex().xy);
    int idx = launchID.x;

    // Test 1: Create empty hit object
    // CHECK: OpHitObjectRecordEmptyEXT
    HitObject hitObj = HitObject::MakeNop();

    uint result = calcValue(hitObj);

    // Test 2: Create miss hit object
    // CHECK: OpHitObjectRecordMissEXT
    RayDesc ray;
    ray.Origin = float3(idx, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    HitObject missObj = HitObject::MakeMiss(RAY_FLAG_NONE, 0, ray);

    result += calcValue(missObj);

    // Test 3: Trace ray into hit object
    // CHECK: OpHitObjectTraceRayEXT
    Attributes attr;
    attr.barycentrics = float2(0.5, 0.5);

    hitObj = HitObject::TraceRay(
        scene,
        RAY_FLAG_NONE,
        0xFF,
        0, // sbtRecordOffset
        1, // sbtRecordStride
        0, // missIndex
        ray,
        attr
    );

    result += calcValue(hitObj);

    // Test 4: Get SBT index
    // CHECK: OpHitObjectGetShaderBindingTableRecordIndexEXT
    uint sbtIndex = hitObj.GetShaderTableIndex();
    result += sbtIndex;

    // Test 5: Set SBT index
    // CHECK: OpHitObjectSetShaderBindingTableRecordIndexEXT
    hitObj.SetShaderTableIndex(sbtIndex + 1);

    // Test 6: Create hit object with MakeHit
    // CHECK: OpHitObjectRecordHitWithIndexEXT
    HitObject createdHit = HitObject::MakeHit(
        0, // sbtRecordIndex
        scene,
        idx,      // instanceIndex
        idx * 2,  // primitiveIndex
        idx * 3,  // geometryIndex
        HIT_KIND_TRIANGLE_FRONT_FACE,
        ray,
        attr
    );

    result += calcValue(createdHit);

    outputBuffer[idx] = result;
}
