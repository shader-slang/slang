// Test HitObject API emitting SPIRV-EXT opcodes (SPV_EXT_shader_invocation_reorder)

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderEXT

// SPIRV-DAG: OpExtension "SPV_EXT_shader_invocation_reorder"
// SPIRV-DAG: OpCapability ShaderInvocationReorderEXT

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

    // SPIRV: OpHitObjectTraceRayEXT
    HitObject hitFromTrace = HitObject::TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    // SPIRV: OpHitObjectRecordMissEXT
    HitObject hitFromMiss = HitObject::MakeMiss(0, ray);

    // SPIRV: OpHitObjectRecordEmptyEXT
    HitObject hitFromNop = HitObject::MakeNop();

    // SPIRV: OpHitObjectExecuteShaderEXT
    HitObject::Invoke(scene, hitFromTrace, payload);

    // SPIRV: OpHitObjectIsHitEXT
    if (hitFromTrace.IsHit()) { r += 1; }

    // SPIRV: OpHitObjectIsMissEXT
    if (hitFromMiss.IsMiss()) { r += 2; }

    // SPIRV: OpHitObjectIsEmptyEXT
    if (hitFromNop.IsNop()) { r += 4; }

    // SPIRV: OpHitObjectGetWorldRayOriginEXT
    // SPIRV: OpHitObjectGetRayTMinEXT
    // SPIRV: OpHitObjectGetWorldRayDirectionEXT
    // SPIRV: OpHitObjectGetRayTMaxEXT
    RayDesc retrievedRay = hitFromTrace.GetRayDesc();
    r += uint(retrievedRay.TMin);

    // SPIRV: OpHitObjectGetObjectRayOriginEXT
    float3 objOrigin = hitFromTrace.GetObjectRayOrigin();
    r += uint(objOrigin.x);

    // SPIRV: OpHitObjectGetObjectRayDirectionEXT
    float3 objDir = hitFromTrace.GetObjectRayDirection();
    r += uint(objDir.y);

    // SPIRV: OpHitObjectGetWorldToObjectEXT
    float4x3 w2o = hitFromTrace.GetWorldToObject();
    r += uint(w2o[0][0]);

    // SPIRV: OpHitObjectGetObjectToWorldEXT
    float4x3 o2w = hitFromTrace.GetObjectToWorld();
    r += uint(o2w[0][0]);

    // SPIRV: OpHitObjectGetInstanceIdEXT
    r += hitFromTrace.GetInstanceIndex();

    // SPIRV: OpHitObjectGetInstanceCustomIndexEXT
    r += hitFromTrace.GetInstanceID();

    // SPIRV: OpHitObjectGetGeometryIndexEXT
    r += hitFromTrace.GetGeometryIndex();

    // SPIRV: OpHitObjectGetPrimitiveIndexEXT
    r += hitFromTrace.GetPrimitiveIndex();

    // SPIRV: OpHitObjectGetHitKindEXT
    r += hitFromTrace.GetHitKind();

    // SPIRV: OpHitObjectGetShaderBindingTableRecordIndexEXT
    r += hitFromTrace.GetShaderTableIndex();

    // SPIRV: OpHitObjectGetAttributesEXT
    MyAttributes retrievedAttr = hitFromTrace.GetAttributes<MyAttributes>();
    r += uint(retrievedAttr.barycentrics.x);

    // SPIRV: OpHitObjectGetCurrentTimeEXT
    float currentTime = hitFromTrace.GetCurrentTime();
    r += uint(currentTime);

    // SPIRV: OpReorderThreadWithHintEXT
    ReorderThread(1u, 4u);

    // SPIRV: OpReorderThreadWithHitObjectEXT
    ReorderThread(hitFromTrace);

    // SPIRV: OpReorderThreadWithHitObjectEXT
    ReorderThread(hitFromTrace, 2u, 8u);

    outputBuffer[idx] = r;
}
