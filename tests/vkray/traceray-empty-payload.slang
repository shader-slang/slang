// An unannotated TraceRay payload is recognized through the D3D-only
// ForceVarIntoRayPayloadStructTemporarily marker. Ray-tracing legalization must provide a physical
// carrier at that boundary before type legalization erases the logical payload. Otherwise the
// emitted TraceRay call has only seven arguments. The original EmptyPayload type stays empty.

//TEST:SIMPLE(filecheck=HLSL): -stage raygeneration -entry main -target hlsl -profile sm_6_7
//TEST:SIMPLE(filecheck=DXIL): -stage raygeneration -entry main -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=SPIRV): -stage raygeneration -entry main -target spirv
//TEST:SIMPLE(filecheck=GLSL): -stage raygeneration -entry main -target glsl

RaytracingAccelerationStructure scene;

struct EmptyPayload
{
}

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1.0;

    EmptyPayload payload;
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, payload);
}

// HLSL-DAG: struct {{.*}}EmptyPayload
// HLSL-DAG: int _slang_dummy{{.*}};
// HLSL: EmptyPayload{{[A-Za-z0-9_]*}} [[payload:[A-Za-z0-9_]+]];
// HLSL: TraceRay({{.*}}, [[payload]]);

// A successful DXIL compile is the end-to-end check that DXC accepts the eight-argument call.
// DXIL: define void @

// Khronos ray tracing also carries the payload through a decorated module-scope object. It must
// remain physical for both direct SPIR-V and GLSL emission.
// SPIRV: OpMemberName %{{[0-9A-Za-z_]+}} {{[0-9]+}} "_slang_dummy"
// SPIRV: %[[payload:[0-9A-Za-z_]+]] = OpVariable %{{[0-9A-Za-z_]+}} RayPayloadKHR
// SPIRV: OpTraceRayKHR {{.*}} %[[payload]]
// GLSL-DAG: _slang_dummy
// GLSL-DAG: rayPayloadEXT
// GLSL: traceRayEXT(
