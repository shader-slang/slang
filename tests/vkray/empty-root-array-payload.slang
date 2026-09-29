// An array is a valid Khronos ray-tracing payload even when it is not wrapped in a source struct.
// If all its elements disappear, the required outgoing payload must still have a physical carrier.
// Non-empty array controls verify that legalization does not replace ordinary payload contents.

//TEST:SIMPLE(filecheck=EMPTY_GLSL): -stage raygeneration -entry callEmpty -target glsl -validate-ir
//TEST:SIMPLE(filecheck=EMPTY_GLSL): -stage raygeneration -entry traceEmpty -target glsl -validate-ir
//TEST:SIMPLE(filecheck=EMPTY_SPIRV): -stage raygeneration -entry callEmpty -target spirv -validate-ir
//TEST:SIMPLE(filecheck=EMPTY_SPIRV): -stage raygeneration -entry traceEmpty -target spirv -validate-ir
//TEST:SIMPLE(filecheck=NONEMPTY_GLSL): -stage raygeneration -entry callNonEmpty -target glsl -validate-ir
//TEST:SIMPLE(filecheck=NONEMPTY_GLSL): -stage raygeneration -entry traceNonEmpty -target glsl -validate-ir
//TEST:SIMPLE(filecheck=NONEMPTY_SPIRV): -stage raygeneration -entry callNonEmpty -target spirv -validate-ir
//TEST:SIMPLE(filecheck=NONEMPTY_SPIRV): -stage raygeneration -entry traceNonEmpty -target spirv -validate-ir
//TEST:SIMPLE(filecheck=EMPTY_HLSL): -stage raygeneration -entry traceEmpty -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage raygeneration -entry traceEmpty -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV_VIA_GLSL): -stage raygeneration -entry callEmpty -target spirv -emit-spirv-via-glsl -validate-ir

struct Empty
{
}

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> result;

RayDesc makeRay()
{
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1.0;
    return ray;
}

[shader("raygeneration")]
void callEmpty()
{
    Empty payload[2];
    CallShader(0, payload);
}

[shader("raygeneration")]
void traceEmpty()
{
    Empty payload[2];
    TraceRay(scene, 0, 0xff, 0, 0, 0, makeRay(), payload);
}

[shader("raygeneration")]
void callNonEmpty()
{
    uint payload[2] = { 11, 23 };
    CallShader(0, payload);
    result[0] = payload[0] + payload[1];
}

[shader("raygeneration")]
void traceNonEmpty()
{
    uint payload[2] = { 11, 23 };
    TraceRay(scene, 0, 0xff, 0, 0, 0, makeRay(), payload);
    result[0] = payload[0] + payload[1];
}

// The logical array disappears, without padding its elements. Each backend uses the same
// one-uint physical payload type at the dispatch boundary.
// EMPTY_GLSL: struct [[carrier:[A-Za-z0-9_]+]]
// EMPTY_GLSL-NEXT: {
// EMPTY_GLSL-NEXT: uint _slang_dummy{{[A-Za-z0-9_]*}};
// EMPTY_GLSL-NEXT: };
// EMPTY_GLSL: {{callableDataEXT|rayPayloadEXT}}
// EMPTY_GLSL-NEXT: [[carrier]] {{[A-Za-z0-9_]+}};
// EMPTY_GLSL: {{executeCallableEXT|traceRayEXT}}(
// EMPTY_SPIRV: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// EMPTY_SPIRV: %[[carrier:[A-Za-z0-9_]+]] = OpTypeStruct %[[uint]]{{$}}
// EMPTY_SPIRV: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer {{CallableDataKHR|RayPayloadKHR}} %[[carrier]]
// EMPTY_SPIRV: %[[payload:[A-Za-z0-9_]+]] = OpVariable %[[pointer]] {{CallableDataKHR|RayPayloadKHR}}
// EMPTY_SPIRV: Op{{ExecuteCallableKHR|TraceRayKHR}} {{.*}} %[[payload]]
// EMPTY_HLSL: struct {{.*}}
// EMPTY_HLSL-NEXT: {
// EMPTY_HLSL-NEXT: uint _slang_dummy{{.*}};
// EMPTY_HLSL-NEXT: };
// EMPTY_HLSL: TraceRay({{.*}}, {{[A-Za-z0-9_]+}});
// DXIL: define void @

// The via-GLSL route must agree with direct SPIR-V on the one-uint physical payload type.
// SPIRV_VIA_GLSL: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// SPIRV_VIA_GLSL: %[[carrier:[A-Za-z0-9_]+]] = OpTypeStruct %[[uint]]{{$}}
// SPIRV_VIA_GLSL: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer CallableDataKHR %[[carrier]]
// SPIRV_VIA_GLSL: %[[payload:[A-Za-z0-9_]+]] = OpVariable %[[pointer]] CallableDataKHR
// SPIRV_VIA_GLSL: OpExecuteCallableKHR %{{[A-Za-z0-9_]+}} %[[payload]]

// The same two-element array remains the actual interface type when its elements carry data.
// NONEMPTY_GLSL-NOT: _slang_dummy
// NONEMPTY_GLSL: {{callableDataEXT|rayPayloadEXT}}
// NONEMPTY_GLSL-NEXT: uint {{[A-Za-z0-9_]+}}[2];
// NONEMPTY_GLSL: {{executeCallableEXT|traceRayEXT}}(
// NONEMPTY_GLSL-NOT: _slang_dummy
// NONEMPTY_SPIRV-NOT: _slang_dummy
// NONEMPTY_SPIRV-DAG: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// NONEMPTY_SPIRV-DAG: %[[two:[A-Za-z0-9_]+]] = OpConstant %{{[A-Za-z0-9_]+}} 2{{$}}
// NONEMPTY_SPIRV: %[[array:[A-Za-z0-9_]+]] = OpTypeArray %[[uint]] %[[two]]
// NONEMPTY_SPIRV: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer {{CallableDataKHR|RayPayloadKHR}} %[[array]]
// NONEMPTY_SPIRV: %[[payload:[A-Za-z0-9_]+]] = OpVariable %[[pointer]] {{CallableDataKHR|RayPayloadKHR}}
// NONEMPTY_SPIRV: Op{{ExecuteCallableKHR|TraceRayKHR}} {{.*}} %[[payload]]
// NONEMPTY_SPIRV-NOT: _slang_dummy
