// TraceRay needs a physical payload even when its struct has only empty fields. Compile the
// D3D hit and miss shaders independently to verify their required payload parameters are also
// preserved. The input-only hit attributes remain ordinary structs without payload qualifiers.

//TEST:SIMPLE(filecheck=HLSL_CALLER): -stage raygeneration -entry rgen -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=HLSL_CALLEE): -stage miss -entry missMain -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=HLSL_HIT): -stage closesthit -entry closestHitMain -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=HLSL_HIT): -stage anyhit -entry anyHitMain -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage raygeneration -entry rgen -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage miss -entry missMain -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage closesthit -entry closestHitMain -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage anyhit -entry anyHitMain -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -stage raygeneration -entry rgen -target spirv -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -stage raygeneration -entry rgen -target glsl -validate-ir

RaytracingAccelerationStructure scene;

struct Empty
{
}

struct EmptyInner
{
    Empty value;
}

struct EmptyOuter
{
    EmptyInner inner;
}

struct HitAttributes
{
    float2 barycentrics;
}

[shader("miss")]
void missMain(inout EmptyOuter payload)
{
}

[shader("closesthit")]
void closestHitMain(inout EmptyOuter payload, HitAttributes attributes)
{
}

[shader("anyhit")]
void anyHitMain(inout EmptyOuter payload, HitAttributes attributes)
{
}

[shader("raygeneration")]
void rgen()
{
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1.0;

    EmptyOuter payload;
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, payload);
}

// The unchanged nested logical types disappear. D3D's required payload slots get one-uint objects.
// HLSL_CALLER: struct {{(\[raypayload\] )?}}[[carrier:[A-Za-z0-9_]+]]
// HLSL_CALLER: {
// HLSL_CALLER-NEXT: uint _slang_dummy{{.*}};
// HLSL_CALLER-NEXT: };
// HLSL_CALLER: [[carrier]] [[payload:[A-Za-z0-9_]+]];
// HLSL_CALLER: TraceRay({{.*}}, [[payload]]);
// HLSL_CALLEE: struct {{(\[raypayload\] )?}}[[carrier:[A-Za-z0-9_]+]]
// HLSL_CALLEE: {
// HLSL_CALLEE-NEXT: uint _slang_dummy{{.*}};
// HLSL_CALLEE-NEXT: };
// HLSL_CALLEE: void missMain(inout [[carrier]] {{[A-Za-z0-9_]+}})
// HLSL_HIT: struct {{(\[raypayload\] )?}}[[carrier:[A-Za-z0-9_]+]]
// HLSL_HIT: {
// HLSL_HIT-NEXT: uint _slang_dummy{{.*}};
// HLSL_HIT-NEXT: };
// HLSL_HIT: struct HitAttributes
// HLSL_HIT: {
// HLSL_HIT-NEXT: float2 barycentrics{{[A-Za-z0-9_]*}};
// HLSL_HIT-NEXT: };
// HLSL_HIT: void {{closestHitMain|anyHitMain}}(inout [[carrier]] {{[A-Za-z0-9_]+}}, HitAttributes{{[A-Za-z0-9_]*}} {{[A-Za-z0-9_]+}})
// DXIL: define void @

// SPIRV: %[[payload:[0-9A-Za-z_]+]] = OpVariable %{{[0-9A-Za-z_]+}} RayPayloadKHR
// SPIRV: OpTraceRayKHR {{.*}} %[[payload]]
// GLSL-DAG: rayPayloadEXT
// GLSL: traceRayEXT(
