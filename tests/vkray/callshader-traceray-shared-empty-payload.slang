// One logical empty type can serve both ray and callable payload roles. Each required target
// object must contain exactly one dummy field without adding storage to the original type.

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

// Ray and callable ABI roles may use separate dummy types. Each must have exactly one field.
// HLSL: struct {{.*}}
// HLSL-NEXT: {
// HLSL-NEXT: int _slang_dummy{{.*}};
// HLSL-NEXT: };
// HLSL: struct {{.*}}
// HLSL-NEXT: {
// HLSL-NEXT: int _slang_dummy{{.*}};
// HLSL-NEXT: };
// HLSL: TraceRay({{.*}}, {{[A-Za-z0-9_]+}});
// HLSL: CallShader(0U, {{[A-Za-z0-9_]+}});
