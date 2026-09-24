// Regression test for physical-carrier reuse in `legalizeRayTracingPayloads`. An empty struct
// used as BOTH a TraceRay ray payload and a CallShader callable payload must have a shared
// physical carrier with exactly one dummy field. Neither payload use may add another field.

//TEST:SIMPLE(filecheck=HLSL): -stage raygeneration -entry rgen -target hlsl -profile sm_6_7

struct Shared
{
}

RaytracingAccelerationStructure accel;

[shader("raygeneration")]
void rgen()
{
    Shared p;
    RayDesc ray;
    ray.Origin = float3(0.0, 0.0, 0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1.0;
    TraceRay(accel, 0, 0xff, 0, 0, 0, ray, p);
    CallShader(0, p);
}

// Both boundary uses share one carrier with one dummy field; the logical type stays empty.
// HLSL: struct {{.*}}
// HLSL-NEXT: {
// HLSL-NEXT: int _slang_dummy{{.*}};
// HLSL-NEXT: };
// HLSL-NOT: int _slang_dummy
