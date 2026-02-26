// Test HitObject API emitting correct SPIRV opcodes for both NV and EXT targets.
// Verifies that NV targets emit NV opcodes and EXT targets emit EXT opcodes,
// with no cross-contamination between extensions.
//
// Note: GetRayFlags() and SetShaderTableIndex() are EXT-only (no NV SPIRV opcodes exist)
// and are excluded from this shared test. They are guarded by static_assert in hlsl.meta.slang.

//TEST:SIMPLE(filecheck=NV):  -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=EXT): -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderEXT

// NV-DAG: OpExtension "SPV_NV_shader_invocation_reorder"
// NV-DAG: OpCapability ShaderInvocationReorderNV
// NV-NOT: OpExtension "SPV_EXT_shader_invocation_reorder"
// NV-NOT: OpCapability ShaderInvocationReorderEXT

// EXT-DAG: OpExtension "SPV_EXT_shader_invocation_reorder"
// EXT-DAG: OpCapability ShaderInvocationReorderEXT

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

struct MyAttributes { float2 barycentrics; };
struct MyPayload { float4 color; };

[shader("raygeneration")]
void rayGenerationMain()
{
    int idx = DispatchRaysIndex().x;

    RayDesc ray;
    ray.Origin = float3(idx, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    MyPayload payload = { float4(0) };
    uint r = 0;

    // NV: OpHitObjectTraceRayNV
    // EXT: OpHitObjectTraceRayEXT
    HitObject hitFromTrace = HitObject::TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    // NV: OpHitObjectRecordMissNV
    // EXT: OpHitObjectRecordMissEXT
    HitObject hitFromMiss = HitObject::MakeMiss(0, ray);

    // NV: OpHitObjectRecordEmptyNV
    // EXT: OpHitObjectRecordEmptyEXT
    HitObject hitFromNop = HitObject::MakeNop();

    // NV: OpHitObjectExecuteShaderNV
    // EXT: OpHitObjectExecuteShaderEXT
    HitObject::Invoke(scene, hitFromTrace, payload);

    // NV: OpHitObjectIsHitNV
    // EXT: OpHitObjectIsHitEXT
    if (hitFromTrace.IsHit()) { r += 1; }

    // NV: OpHitObjectIsMissNV
    // EXT: OpHitObjectIsMissEXT
    if (hitFromMiss.IsMiss()) { r += 2; }

    // NV: OpHitObjectIsEmptyNV
    // EXT: OpHitObjectIsEmptyEXT
    if (hitFromNop.IsNop()) { r += 4; }

    // GetRayDesc — returns composite ray info via individual ops
    // NV: OpHitObjectGetWorldRayOriginNV
    // NV: OpHitObjectGetRayTMinNV
    // NV: OpHitObjectGetWorldRayDirectionNV
    // NV: OpHitObjectGetRayTMaxNV
    // EXT: OpHitObjectGetWorldRayOriginEXT
    // EXT: OpHitObjectGetRayTMinEXT
    // EXT: OpHitObjectGetWorldRayDirectionEXT
    // EXT: OpHitObjectGetRayTMaxEXT
    RayDesc retrievedRay = hitFromTrace.GetRayDesc();
    r += uint(retrievedRay.TMin);

    // NV: OpHitObjectGetObjectRayOriginNV
    // EXT: OpHitObjectGetObjectRayOriginEXT
    float3 objOrigin = hitFromTrace.GetObjectRayOrigin();
    r += uint(objOrigin.x);

    // NV: OpHitObjectGetObjectRayDirectionNV
    // EXT: OpHitObjectGetObjectRayDirectionEXT
    float3 objDir = hitFromTrace.GetObjectRayDirection();
    r += uint(objDir.y);

    // NV: OpHitObjectGetWorldToObjectNV
    // EXT: OpHitObjectGetWorldToObjectEXT
    float4x3 w2o = hitFromTrace.GetWorldToObject();
    r += uint(w2o[0][0]);

    // NV: OpHitObjectGetObjectToWorldNV
    // EXT: OpHitObjectGetObjectToWorldEXT
    float4x3 o2w = hitFromTrace.GetObjectToWorld();
    r += uint(o2w[0][0]);

    // NV: OpHitObjectGetInstanceIdNV
    // EXT: OpHitObjectGetInstanceIdEXT
    r += hitFromTrace.GetInstanceIndex();

    // NV: OpHitObjectGetInstanceCustomIndexNV
    // EXT: OpHitObjectGetInstanceCustomIndexEXT
    r += hitFromTrace.GetInstanceID();

    // NV: OpHitObjectGetGeometryIndexNV
    // EXT: OpHitObjectGetGeometryIndexEXT
    r += hitFromTrace.GetGeometryIndex();

    // NV: OpHitObjectGetPrimitiveIndexNV
    // EXT: OpHitObjectGetPrimitiveIndexEXT
    r += hitFromTrace.GetPrimitiveIndex();

    // NV: OpHitObjectGetHitKindNV
    // EXT: OpHitObjectGetHitKindEXT
    r += hitFromTrace.GetHitKind();

    // NV: OpHitObjectGetShaderBindingTableRecordIndexNV
    // EXT: OpHitObjectGetShaderBindingTableRecordIndexEXT
    r += hitFromTrace.GetShaderTableIndex();

    // NV: OpHitObjectGetAttributesNV
    // EXT: OpHitObjectGetAttributesEXT
    MyAttributes retrievedAttr = hitFromTrace.GetAttributes<MyAttributes>();
    r += uint(retrievedAttr.barycentrics.x);

    // NV: OpHitObjectGetCurrentTimeNV
    // EXT: OpHitObjectGetCurrentTimeEXT
    float currentTime = hitFromTrace.GetCurrentTime();
    r += uint(currentTime);

    // NV: OpReorderThreadWithHintNV
    // EXT: OpReorderThreadWithHintEXT
    ReorderThread(1u, 4u);

    // NV: OpReorderThreadWithHitObjectNV
    // EXT: OpReorderThreadWithHitObjectEXT
    ReorderThread(hitFromTrace);

    // NV: OpReorderThreadWithHitObjectNV
    // EXT: OpReorderThreadWithHitObjectEXT
    ReorderThread(hitFromTrace, 2u, 8u);

    // Individual accessors (these have both NV and EXT opcodes)
    // NV: OpHitObjectGetRayTMinNV
    // EXT: OpHitObjectGetRayTMinEXT
    float tmin = hitFromTrace.GetRayTMin();
    r += uint(tmin);

    // NV: OpHitObjectGetWorldRayOriginNV
    // EXT: OpHitObjectGetWorldRayOriginEXT
    float3 worldOrigin = hitFromTrace.GetWorldRayOrigin();
    r += uint(worldOrigin.x);

    // NV: OpHitObjectGetWorldRayDirectionNV
    // EXT: OpHitObjectGetWorldRayDirectionEXT
    float3 worldDir = hitFromTrace.GetWorldRayDirection();
    r += uint(worldDir.x);

    // Note: GetRayFlags() and SetShaderTableIndex() are EXT-only —
    // SPV_NV_shader_invocation_reorder has no opcodes for these.
    // They are guarded by static_assert for NV targets in hlsl.meta.slang.

    outputBuffer[idx] = r;
}
