// Regression test for:
// https://github.com/shader-slang/slang/issues/10267

//TEST:SIMPLE(filecheck=SM68): -target hlsl -profile sm_6_8 -stage raygeneration -entry rgen_main
//TEST:SIMPLE(filecheck=SM66): -target hlsl -profile sm_6_6 -stage raygeneration -entry rgen_main

// SM68: struct [raypayload] RayPayload_0
// SM68: float3 color_0 : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);
// SM66-NOT: [raypayload]
// SM66: struct RayPayload_0
// SM66: float3 color_0;
// SM66-NOT: : read(

RWTexture2D<float4> outputImage;
RaytracingAccelerationStructure topLevelAS;

struct RayPayload
{
    float3 color;
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

    TraceRay(topLevelAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    outputImage[DispatchRaysIndex().xy] = float4(payload.color, 1.0);
}
