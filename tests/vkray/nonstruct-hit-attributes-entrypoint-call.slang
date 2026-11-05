// An entry point can also be called as an ordinary function. On D3D a closesthit shader's
// non-struct hit-attribute parameter becomes a wrapper struct, so an ordinary call passes a
// wrapper temporary. The callee cannot write its attributes, so the argument is copied in only.
// The MULTI run checks only that the pass wraps each hit-attribute parameter of a signature D3D
// rejects, with more than one, without asserting; DXC diagnoses that signature.

//TEST:SIMPLE(filecheck=HLSL): -entry caller -entry callee -target hlsl -profile lib_6_6 -whole-program -O0 -validate-ir-detailed
//TEST:SIMPLE(filecheck=HLSL): -entry caller -entry callee -target hlsl -profile lib_6_6 -whole-program -O3 -validate-ir-detailed
//TEST:SIMPLE(filecheck=DXIL): -entry caller -entry callee -target dxil-asm -profile lib_6_6 -whole-program -O0
//TEST:SIMPLE(filecheck=DXIL): -entry caller -entry callee -target dxil-asm -profile lib_6_6 -whole-program -O3
//TEST:SIMPLE(filecheck=MULTI): -entry twoAttributes -stage closesthit -target hlsl -profile lib_6_6 -validate-ir-detailed

RWStructuredBuffer<float4> result;

[shader("closesthit")]
[noinline]
void callee(inout float4 payload, in float2 attributes)
{
    payload.xy += attributes;
}

[shader("closesthit")]
void caller(inout float4 payload, in float2 attributes)
{
    float2 local = attributes * 2.0;
    callee(payload, local);
    result[0] = float4(local, payload.zw);
}

[shader("closesthit")]
void twoAttributes(inout float4 payload, in float2 first, in float2 second)
{
    payload = float4(first + second, 0.0, 1.0);
}

// HLSL: struct [[payloadType:RayPayload_t[A-Za-z0-9_]*]]
// HLSL: struct [[attrType:HitAttributes_t[A-Za-z0-9_]*]]
// HLSL-NEXT: {
// HLSL-NEXT: float2 [[field:[A-Za-z0-9_]+]];
// HLSL: void [[clone:callee_[A-Za-z0-9_]+]](inout [[payloadType]] {{[A-Za-z0-9_]+}}, [[attrType]] [[cloneAttr:[A-Za-z0-9_]+]])
// HLSL: + [[cloneAttr]].[[field]];
// HLSL: void callee(inout [[payloadType]] {{[A-Za-z0-9_]+}}, [[attrType]] [[calleeAttr:[A-Za-z0-9_]+]])
// HLSL: + [[calleeAttr]].[[field]];
// HLSL: void caller(inout [[payloadType]] {{[A-Za-z0-9_]+}}, [[attrType]] [[callerAttr:[A-Za-z0-9_]+]])
// HLSL: float2 [[local:[A-Za-z0-9_]+]] = [[callerAttr]].[[field]] * 2.0f;
// HLSL: [[attrType]] [[temporary:[A-Za-z0-9_]+]];
// HLSL: [[temporary]].[[field]] = [[local]];
// HLSL: [[clone]]({{[A-Za-z0-9_]+}}, [[temporary]]);
// HLSL-NOT: = [[temporary]].[[field]]
// HLSL: result{{[A-Za-z0-9_]*}}[int(0)] = float4([[local]],

// DXIL: [[attrType:%struct.HitAttributes_t[A-Za-z0-9_]*]] = type { <2 x float> }
// DXIL: define internal fastcc void @[[clone:"[^"]*callee_[^"]*"]]({{.*}}, [[attrType]]*
// DXIL: define void @{{"[^"]*}}callee{{[^"]*"}}({{.*}}, [[attrType]]*
// DXIL: define void @{{"[^"]*}}caller{{[^"]*"}}({{.*}}, [[attrType]]*
// DXIL: [[temporary:%[A-Za-z0-9_]+]] = alloca [[attrType]]
// DXIL: call fastcc void @[[clone]]({{.*}}, [[attrType]]* {{.*}}[[temporary]])

// MULTI: struct [[attrType:HitAttributes_t[A-Za-z0-9_]*]]
// MULTI: void twoAttributes(inout RayPayload_t{{[A-Za-z0-9_]*}} {{[A-Za-z0-9_]+}}, [[attrType]] {{[A-Za-z0-9_]+}}, [[attrType]] {{[A-Za-z0-9_]+}})
