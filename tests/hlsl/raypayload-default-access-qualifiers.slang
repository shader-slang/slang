// Regression test for #10267: when a struct without an explicit `[raypayload]`
// attribute is used as a TraceRay payload, Slang implicitly tags it with
// `IRRayPayloadDecoration`. At SM 6.7+ the HLSL emitter then writes
// `[raypayload]` on the struct; without this pass DXC rejects the result
// because the fields carry no payload access qualifiers.

//TEST:SIMPLE(filecheck=CHECK_67): -target hlsl -profile lib_6_7 -stage raygeneration -entry rgen_main
//TEST:SIMPLE(filecheck=DXIL_67): -target dxil -profile lib_6_7 -stage raygeneration -entry rgen_main
//TEST:SIMPLE(filecheck=CHECK_66): -target hlsl -profile lib_6_6 -stage raygeneration -entry rgen_main

// SM 6.7+: `[raypayload]` is emitted, and each field carries default
// `read/write(caller, anyhit, closesthit, miss)` qualifiers covering all stages
// — preserving the implicit pre-SM 6.7 behavior where any stage could touch
// any field.
// CHECK_67: struct [raypayload] RayPayload_0
// CHECK_67: float3 color_0 : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);

// And DXC accepts the resulting HLSL.
// DXIL_67: define void @

// SM 6.6: no `[raypayload]` attribute and no payload access qualifiers (the
// SM 6.7+ feature should not leak into older targets).
// CHECK_66: struct RayPayload_0
// CHECK_66-NOT: [raypayload]
// CHECK_66: float3 color_0;
// CHECK_66-NOT: read(caller
// CHECK_66-NOT: write(caller

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
