// HLSL requires a struct payload, so a scalar is wrapped for TraceRay and unwrapped afterward.
// Copy-out must load the scalar field, not store its address into the original scalar variable.
// Reading the value after dispatch makes the copy-out observable to the caller.

//TEST:SIMPLE(filecheck=HLSL): -entry main -stage raygeneration -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -entry main -stage raygeneration -target dxil -profile sm_6_7 -validate-ir

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> result;

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1.0;

    uint payload = result[0];
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, payload);
    result[1] = payload;
}

// HLSL-NOT: _slang_dummy
// HLSL: struct {{(\[raypayload\] )?}}[[carrier:[A-Za-z0-9_]+]]
// HLSL-NEXT: {
// HLSL-NEXT: uint [[field:[A-Za-z0-9_]+]]{{.*}};
// HLSL-NEXT: };
// HLSL: [[carrier]] [[payload:[A-Za-z0-9_]+]];
// HLSL: [[payload]].[[field]] =
// HLSL: TraceRay({{.*}}, [[payload]]);
// HLSL: {{[A-Za-z0-9_]+}}[{{.*}}1{{.*}}] = [[payload]].[[field]];
// HLSL-NOT: _slang_dummy
// DXIL: define void @
