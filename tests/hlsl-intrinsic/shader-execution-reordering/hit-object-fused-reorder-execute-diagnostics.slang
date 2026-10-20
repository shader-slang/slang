// #13774: the SER fused functions are ray-generation-only, and the two-argument `ReorderExecute`
// has no NVAPI lowering because NVAPI `HitObject::Invoke` requires an AccelerationStructure.

//DIAGNOSTIC_TEST:SIMPLE(diag=STAGE,non-exhaustive): -target spirv -entry closestHitMain -stage closesthit
//DIAGNOSTIC_TEST:SIMPLE(diag=NVAPI,non-exhaustive): -target hlsl -entry rayGenerationMain -stage raygeneration -profile sm_6_5 -capability ser_nvapi

uniform RaytracingAccelerationStructure scene;

struct MyPayload { float4 color; };

[shader("closesthit")]
void closestHitMain(inout MyPayload payload, BuiltInTriangleIntersectionAttributes attr)
{
    RayDesc ray = { float3(0), 0.01f, float3(0, 1, 0), 1e4f };
    //STAGE: E36107
    TraceReorderExecute(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);
}

[shader("raygeneration")]
void rayGenerationMain()
{
    MyPayload payload = { float4(0) };
    HitObject hit = HitObject::MakeNop();
    //NVAPI: 41400
    //NVAPI: ReorderExecute has no NVAPI form
    ReorderExecute(hit, payload);
}
