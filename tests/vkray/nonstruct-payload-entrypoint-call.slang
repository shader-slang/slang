// An entry point can also be called as an ordinary function. On D3D its non-struct payload
// parameter becomes a wrapper struct before fixEntryPointCallsites clones the entry point for
// ordinary callers, so each call passes a wrapper temporary: the argument's value is copied in
// before the call and copied back afterward, where caller's result write observes it.

//TEST:SIMPLE(filecheck=HLSL): -entry caller -entry callee -target hlsl -profile sm_6_6 -whole-program -O0 -validate-ir-detailed
//TEST:SIMPLE(filecheck=HLSL): -entry caller -entry callee -target hlsl -profile sm_6_6 -whole-program -O3 -validate-ir-detailed
//TEST:SIMPLE(filecheck=DXIL): -entry caller -entry callee -target dxil -profile sm_6_6 -whole-program -O0
//TEST:SIMPLE(filecheck=DXIL): -entry caller -entry callee -target dxil -profile sm_6_6 -whole-program -O3

RWStructuredBuffer<float4> result;

[shader("miss")]
[noinline]
void callee(inout float4 payload)
{
    payload.x += 1.0;
}

[shader("miss")]
void caller(inout float4 payload)
{
    payload.y = result[0].y;
    callee(payload);
    result[1] = payload;
}

// HLSL: struct [[payloadType:RayPayload_t[A-Za-z0-9_]*]]
// HLSL-NEXT: {
// HLSL-NEXT: float4 [[field:[A-Za-z0-9_]+]];
// HLSL: void [[clone:callee_[A-Za-z0-9_]+]](inout [[payloadType]] [[cloneParam:[A-Za-z0-9_]+]])
// HLSL: [[cloneParam]].[[field]][int(0)] = [[cloneParam]].[[field]][int(0)] + 1.0f;
// HLSL: void callee(inout [[payloadType]] [[calleeParam:[A-Za-z0-9_]+]])
// HLSL: [[calleeParam]].[[field]][int(0)] = [[calleeParam]].[[field]][int(0)] + 1.0f;
// HLSL: void caller(inout [[payloadType]] [[callerParam:[A-Za-z0-9_]+]])
// HLSL: [[callerParam]].[[field]][int(1)] =
// HLSL: [[payloadType]] [[temporary:[A-Za-z0-9_]+]];
// HLSL: [[temporary]].[[field]] = [[callerParam]].[[field]];
// HLSL: [[clone]]([[temporary]]);
// HLSL: [[callerParam]].[[field]] = [[temporary]].[[field]];
// HLSL: [[temporary]].[[field]];

// DXIL: [[payloadType:%struct.RayPayload_t[A-Za-z0-9_]*]] = type { <4 x float> }
// DXIL: define internal fastcc void @[[clone:"[^"]*callee_[^"]*"]]([[payloadType]]*
// DXIL: define void @{{"[^"]*}}callee{{[^"]*"}}([[payloadType]]*
// DXIL: define void @{{"[^"]*}}caller{{[^"]*"}}([[payloadType]]*
// DXIL: [[temporary:%[A-Za-z0-9_]+]] = alloca [[payloadType]]
// DXIL: call fastcc void @[[clone]]([[payloadType]]* {{.*}}[[temporary]])
