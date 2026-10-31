// DXR 1.3 declares `template<attr_t> void dx::HitObject::GetAttributes(out attr_t Attributes)`;
// NVAPI uses `NvGetAttributesFromHitObject(hitObject, attributes)`. The DXR row checks the
// out-parameter member call, the DXIL row checks that DXC accepts it, and the NVAPI row checks
// that NVAPI still uses its free function.

//TEST:SIMPLE(filecheck=DXR): -target hlsl -entry rayGenerationMain -stage raygeneration -profile sm_6_9 -capability ser_dxr
//TEST:SIMPLE(filecheck=DXIL): -target dxil-asm -entry rayGenerationMain -stage raygeneration -profile sm_6_9 -capability ser_dxr
//TEST:SIMPLE(filecheck=NVAPI): -target hlsl -entry rayGenerationMain -stage raygeneration -profile sm_6_5 -capability hlsl_nvapi

uniform RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> outputBuffer;

struct MyAttributes { uint value; };
struct MyPayload { float4 color; };

[shader("raygeneration")]
void rayGenerationMain()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    MyPayload payload = { float4(0) };
    HitObject hit = HitObject::TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    // DXR: {{[a-zA-Z0-9_]+}}.GetAttributes({{[a-zA-Z0-9_]+}});
    // NVAPI: NvGetAttributesFromHitObject(({{.*}}), ({{.*}}));
    // DXIL: call void @dx.op.hitObject_Attributes
    MyAttributes attributes = hit.GetAttributes<MyAttributes>();
    outputBuffer[0] = attributes.value;
}
