// Regression test for:
// https://github.com/shader-slang/slang/issues/10267

//TEST:SIMPLE(filecheck=HLSL68): -target hlsl -profile sm_6_8 -stage raygeneration -entry rgen_main
//TEST:SIMPLE(filecheck=HLSL66): -target hlsl -profile sm_6_6 -stage raygeneration -entry rgen_main
//TEST:SIMPLE(filecheck=DXIL68): -target dxil -profile sm_6_8 -stage raygeneration -entry rgen_main
//TEST:SIMPLE(filecheck=DXIL66): -target dxil -profile sm_6_6 -stage raygeneration -entry rgen_main

// Implicit ray payload (no [raypayload] attribute): Slang should add default access
// qualifiers on SM 6.7+. Fields that already carry explicit qualifiers must be left alone.

// HLSL68: struct [raypayload] RayPayload_0
// HLSL68: float3 color_0 : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);
// HLSL68: float3 normal_0 : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);
// HLSL66-NOT: [raypayload]
// HLSL66: struct RayPayload_0
// HLSL66: float3 color_0;
// HLSL66: float3 normal_0;
// HLSL66-NOT: : read(
// DXIL68: define void @
// DXIL68: !dx.dxrPayloadAnnotations
// DXIL66: define void @
// DXIL66-NOT: !dx.dxrPayloadAnnotations

RWTexture2D<float4> outputImage;
RaytracingAccelerationStructure topLevelAS;

struct RayPayload
{
    float3 color;
    float3 normal : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);
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
    payload.normal = float3(0, 0, 1);

    TraceRay(topLevelAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    outputImage[DispatchRaysIndex().xy] = float4(payload.color, 1.0);
}
