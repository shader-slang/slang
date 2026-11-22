// Using Empty independently as a payload must not give every Empty field in the module storage.
// In particular, Data.value must have the same representation in a caller that also calls with
// Empty, a caller that only calls with Data, and a separately compiled callable receiving Data.
// Exercise both CallShader and TraceRay uses of Empty to cover sharing across payload roles.

//TEST:SIMPLE(filecheck=HLSL): -stage raygeneration -entry caller -target hlsl -profile sm_6_7 -DEMPTY_PAYLOAD_USE=1 -validate-ir
//TEST:SIMPLE(filecheck=HLSL): -stage raygeneration -entry caller -target hlsl -profile sm_6_7 -DEMPTY_PAYLOAD_USE=0 -validate-ir
//TEST:SIMPLE(filecheck=HLSL): -stage raygeneration -entry caller -target hlsl -profile sm_6_7 -DEMPTY_PAYLOAD_USE=2 -validate-ir
//TEST:SIMPLE(filecheck=HLSL): -stage callable -entry callee -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage raygeneration -entry caller -target dxil -profile sm_6_7 -DEMPTY_PAYLOAD_USE=1 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage raygeneration -entry caller -target dxil -profile sm_6_7 -DEMPTY_PAYLOAD_USE=0 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage raygeneration -entry caller -target dxil -profile sm_6_7 -DEMPTY_PAYLOAD_USE=2 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage callable -entry callee -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -stage raygeneration -entry caller -target glsl -DEMPTY_PAYLOAD_USE=1 -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -stage raygeneration -entry caller -target glsl -DEMPTY_PAYLOAD_USE=0 -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -stage raygeneration -entry caller -target glsl -DEMPTY_PAYLOAD_USE=2 -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -stage callable -entry callee -target glsl -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -stage raygeneration -entry caller -target spirv -DEMPTY_PAYLOAD_USE=1 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -stage raygeneration -entry caller -target spirv -DEMPTY_PAYLOAD_USE=0 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -stage raygeneration -entry caller -target spirv -DEMPTY_PAYLOAD_USE=2 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -stage callable -entry callee -target spirv -validate-ir

// 0: no independent Empty payload, 1: callable payload, 2: ray payload.
#ifndef EMPTY_PAYLOAD_USE
#define EMPTY_PAYLOAD_USE 0
#endif

struct Empty
{
}

struct Data
{
    Empty unused;
    uint value;
}

RWStructuredBuffer<uint> result;
RaytracingAccelerationStructure scene;

[shader("raygeneration")]
void caller()
{
    Data data;
    data.value = 37;
#if EMPTY_PAYLOAD_USE
    Empty empty;
#if EMPTY_PAYLOAD_USE == 1
    CallShader(0, empty);
#else
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1.0;
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, empty);
#endif
#endif
    CallShader(1, data);
    result[0] = data.value;
}

[shader("callable")]
void callee(inout Data data)
{
    data.value += 5;
}

// Adjacent field and closing-brace checks reject any leading or trailing padding in Data.
// HLSL: struct Data{{[A-Za-z0-9_]*}}
// HLSL-NEXT: {
// HLSL-NEXT: uint value{{[A-Za-z0-9_]*}};
// HLSL-NEXT: };
// DXIL: %struct.Data{{[A-Za-z0-9_]*}} = type { i32 }
// GLSL: struct Data{{[A-Za-z0-9_]*}}
// GLSL-NEXT: {
// GLSL-NEXT: uint value{{[A-Za-z0-9_]*}};
// GLSL-NEXT: };
// SPIRV: OpName %[[data:[A-Za-z0-9_]+]] "Data"
// SPIRV: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// SPIRV: %[[data]] = OpTypeStruct %[[uint]]{{$}}
