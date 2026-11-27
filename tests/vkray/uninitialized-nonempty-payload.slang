// An undefined value is not an empty type. Even when the incoming payload has no initialized
// contents, its real fields are part of the shader ABI and must not be replaced by a dummy.
// Reading the value after dispatch also keeps the callee's possible write observable.

//TEST:SIMPLE(filecheck=HLSL): -entry callMain -stage raygeneration -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=HLSL): -entry traceMain -stage raygeneration -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -entry callMain -stage raygeneration -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -entry traceMain -stage raygeneration -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -entry callMain -stage raygeneration -target glsl -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -entry traceMain -stage raygeneration -target glsl -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry callMain -stage raygeneration -target spirv -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry traceMain -stage raygeneration -target spirv -validate-ir

struct Payload
{
    uint value;
}

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> result;

[shader("raygeneration")]
void callMain()
{
    Payload payload;
    CallShader(0, payload);
    result[0] = payload.value;
}

[shader("raygeneration")]
void traceMain()
{
    Payload payload;
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1.0;
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, payload);
    result[0] = payload.value;
}

// HLSL-NOT: _slang_dummy
// HLSL: struct {{(\[raypayload\] )?}}[[payloadType:Payload[A-Za-z0-9_]*]]
// HLSL-NEXT: {
// HLSL-NEXT: uint value{{.*}};
// HLSL-NEXT: };
// HLSL: {{CallShader|TraceRay}}({{.*}}, {{[A-Za-z0-9_]+}});
// HLSL-NOT: _slang_dummy
// DXIL: %struct.Payload{{[A-Za-z0-9_]*}} = type { i32 }
// GLSL-NOT: _slang_dummy
// GLSL: struct [[payloadType:Payload[A-Za-z0-9_]*]]
// GLSL-NEXT: {
// GLSL-NEXT: uint value{{[A-Za-z0-9_]*}};
// GLSL-NEXT: };
// GLSL: {{callableDataEXT|rayPayloadEXT}}
// GLSL-NEXT: [[payloadType]] {{[A-Za-z0-9_]+}};
// GLSL: {{executeCallableEXT|traceRayEXT}}(
// GLSL-NOT: _slang_dummy
// SPIRV-NOT: _slang_dummy
// SPIRV: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// SPIRV: %[[payloadType:[A-Za-z0-9_]+]] = OpTypeStruct %[[uint]]{{$}}
// SPIRV: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer {{RayPayloadKHR|CallableDataKHR}} %[[payloadType]]
// SPIRV: %[[payload:[A-Za-z0-9_]+]] = OpVariable %[[pointer]] {{RayPayloadKHR|CallableDataKHR}}
// SPIRV: Op{{TraceRayKHR|ExecuteCallableKHR}} {{.*}} %[[payload]]
// SPIRV-NOT: _slang_dummy
