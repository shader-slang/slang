// Regression test for:
// https://github.com/shader-slang/slang/issues/10267

//TEST:SIMPLE(filecheck=HLSL66): -target hlsl -profile sm_6_6 -stage raygeneration -entry rgen_main
//TEST:SIMPLE(filecheck=HLSL67): -target hlsl -profile sm_6_7 -stage raygeneration -entry rgen_main
//TEST:SIMPLE(filecheck=DXIL66): -target dxil -profile sm_6_6 -stage raygeneration -entry rgen_main
//TEST:SIMPLE(filecheck=DXIL67): -target dxil -profile sm_6_7 -stage raygeneration -entry rgen_main
//TEST:SIMPLE(filecheck=NONSTRUCT67): -target hlsl -profile sm_6_7 -stage raygeneration -entry rgen_nonstruct

// Implicit ray payload (no [raypayload] attribute): Slang should add default access
// qualifiers on SM 6.7+. Explicit payload qualifiers must be left alone.

// HLSL66-NOT: [raypayload]
// HLSL66-DAG: struct RayPayload
// HLSL66-DAG: float3 color_0;
// HLSL66-DAG: struct ExplicitRayPayload
// HLSL66-DAG: float3 normal_0;
// HLSL66-DAG: struct EmptyRayPayload
// HLSL66-DAG: int _slang_dummy_0;
// HLSL66-NOT: : read(
// HLSL66-NOT: : write(
// HLSL67-DAG: struct [raypayload] RayPayload
// HLSL67-DAG: float3 color_0 : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);
// HLSL67-DAG: struct [raypayload] ExplicitRayPayload
// HLSL67-DAG: float3 normal_0 : read(caller) : write(miss);
// HLSL67-DAG: struct [raypayload] EmptyRayPayload
// HLSL67-DAG: int _slang_dummy_0 : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);
// DXIL66: define void @
// DXIL66-NOT: !dx.dxrPayloadAnnotations
// DXIL67: define void @
// DXIL67: !dx.dxrPayloadAnnotations
// NONSTRUCT67-DAG: struct [raypayload] RayPayload_t
// NONSTRUCT67-DAG: float4 data_0 : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);

RWTexture2D<float4> outputImage;
RaytracingAccelerationStructure topLevelAS;

struct RayPayload
{
    float3 color;
};

[raypayload]
struct ExplicitRayPayload
{
    float3 normal : read(caller) : write(miss);
};

[raypayload]
struct EmptyRayPayload
{
};

[shader("raygeneration")]
void rgen_main()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.001;
    ray.TMax = 1000.0;

    RayPayload payload;
    payload.color = float3(0, 0, 0);
    ExplicitRayPayload explicitPayload;
    explicitPayload.normal = float3(0, 0, 1);
    EmptyRayPayload emptyPayload;

    TraceRay(topLevelAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);
    TraceRay(topLevelAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, explicitPayload);
    TraceRay(topLevelAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, emptyPayload);

    outputImage[DispatchRaysIndex().xy] = float4(payload.color, 1.0);
}

[shader("raygeneration")]
void rgen_nonstruct()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.001;
    ray.TMax = 1000.0;

    float4 payload = float4(0, 0, 0, 0);

    TraceRay(topLevelAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    outputImage[DispatchRaysIndex().xy] = payload;
}
