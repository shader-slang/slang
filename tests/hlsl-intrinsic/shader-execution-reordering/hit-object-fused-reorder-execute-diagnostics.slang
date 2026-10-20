// #13774: the SER fused functions are ray-generation-only, `ReorderExecute` (both overloads) has no
// NVAPI lowering because NVAPI `HitObject::Invoke` requires an AccelerationStructure, and
// `TraceMotionReorderExecute` has no D3D12 lowering because D3D12 has no `TraceMotionRay`.

//DIAGNOSTIC_TEST:SIMPLE(diag=STAGE,non-exhaustive): -target spirv -entry closestHitMain -stage closesthit
//DIAGNOSTIC_TEST:SIMPLE(diag=STAGEREORDER,non-exhaustive): -target spirv -entry closestHitReorder -stage closesthit
//DIAGNOSTIC_TEST:SIMPLE(diag=NVAPI,non-exhaustive): -target hlsl -entry rayGenerationMain -stage raygeneration -profile sm_6_5 -capability ser_nvapi
//DIAGNOSTIC_TEST:SIMPLE(diag=NVAPIHINT,non-exhaustive): -target hlsl -entry rayGenerationHint -stage raygeneration -profile sm_6_5 -capability ser_nvapi
//DIAGNOSTIC_TEST:SIMPLE(diag=DXRMOTION,non-exhaustive): -target hlsl -entry rayGenerationMotion -stage raygeneration -profile sm_6_9 -capability ser_dxr

uniform RaytracingAccelerationStructure scene;

struct MyPayload { float4 color; };

[shader("closesthit")]
void closestHitMain(inout MyPayload payload, BuiltInTriangleIntersectionAttributes attr)
{
    RayDesc ray = { float3(0), 0.01f, float3(0, 1, 0), 1e4f };
    //STAGE: E36107
    TraceReorderExecute(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);
}

[shader("closesthit")]
void closestHitReorder(inout MyPayload payload, BuiltInTriangleIntersectionAttributes attr)
{
    HitObject hit = HitObject::MakeNop();
    //STAGEREORDER: E36107
    ReorderExecute(hit, payload);
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

[shader("raygeneration")]
void rayGenerationHint()
{
    MyPayload payload = { float4(0) };
    HitObject hit = HitObject::MakeNop();
    //NVAPIHINT: 41400
    //NVAPIHINT: ReorderExecute has no NVAPI form
    ReorderExecute(hit, 7, 2, payload);
}

[shader("raygeneration")]
void rayGenerationMotion()
{
    MyPayload payload = { float4(0) };
    RayDesc ray = { float3(0), 0.01f, float3(0, 1, 0), 1e4f };
    //DXRMOTION: E36107
    TraceMotionReorderExecute(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, 0.5, 4, 2, payload);
}
