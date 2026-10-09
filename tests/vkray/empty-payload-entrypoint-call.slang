// An entry point can also be called as an ordinary function. `legalizeRayTracingPayloads` first
// gives the empty native payload physical storage. The pipeline then resolves varying references
// before `fixEntryPointCallsites` clones the already-resolved function for ordinary callers. Later
// entry-point legalization can therefore adapt the entry-point and callable roles independently
// without making the clone refer to the entry point's obsolete payload representation.
// Other helper parameters remain logical: the `Empty` parameter of `recordEvent` disappears
// normally. The replacement argument needs storage, but its dummy field does not need an initial
// value.

//TEST:SIMPLE(filecheck=HLSL): -entry caller -entry callee -target hlsl -profile sm_6_6 -whole-program -O0 -validate-ir-detailed
//TEST:SIMPLE(filecheck=HLSL): -entry caller -entry callee -target hlsl -profile sm_6_6 -whole-program -O3 -validate-ir-detailed
//TEST:SIMPLE(filecheck=DXIL): -entry caller -entry callee -target dxil -profile sm_6_6 -whole-program -O0 -validate-ir-detailed
//TEST:SIMPLE(filecheck=DXIL): -entry caller -entry callee -target dxil -profile sm_6_6 -whole-program -O3 -validate-ir-detailed

struct Empty
{
}

RWStructuredBuffer<uint> events;

[noinline]
void recordEvent(Empty value, uint index)
{
    InterlockedAdd(events[index], 1);
}

[shader("miss")]
[noinline]
void callee(inout Empty payload)
{
    recordEvent(payload, 0);
}

[shader("miss")]
void caller(inout Empty payload)
{
    recordEvent(payload, 1);
    callee(payload);
    recordEvent(payload, 2);
}

// HLSL: void [[record:recordEvent[A-Za-z0-9_]*]](uint {{[A-Za-z0-9_]+}})
// HLSL: InterlockedAdd
// HLSL: struct [[payloadType:DummyRayPayload[A-Za-z0-9_]*]]
// HLSL: uint _slang_dummy{{.*}};
// HLSL: void [[clone:callee_[A-Za-z0-9_]+]](inout [[payloadType]] {{[A-Za-z0-9_]+}})
// HLSL: [[record]](0U);
// HLSL: void callee(inout [[payloadType]] {{[A-Za-z0-9_]+}})
// HLSL: [[record]](0U);
// HLSL: void caller(inout [[payloadType]] {{[A-Za-z0-9_]+}})
// HLSL: [[record]](1U);
// HLSL: [[payloadType]] [[physical:[A-Za-z0-9_]+]];
// HLSL-NOT: [[physical]]._slang_dummy
// HLSL-NOT: [[physical]] =
// HLSL: [[clone]]([[physical]]);
// HLSL: [[record]](2U);

// DXIL: [[payloadType:%struct.DummyRayPayload[A-Za-z0-9_]*]] = type { i32 }
// DXIL: define internal fastcc void @[[record:"[^"]*recordEvent[^"]*"]](i32 %{{[A-Za-z0-9_]+}})
// DXIL: define internal fastcc void @[[clone:"[^"]*callee_[^"]*"]]([[payloadType]]* {{.*}})
// DXIL: call fastcc void @[[record]](i32 0)
// DXIL: define void {{.*}}callee{{.*}}([[payloadType]]* {{.*}})
// DXIL: define void {{.*}}caller{{.*}}([[payloadType]]* {{.*}})
// DXIL: [[physical:%[A-Za-z0-9_]+]] = alloca [[payloadType]]
// DXIL: call fastcc void @[[record]](i32 1)
// DXIL-NOT: store
// DXIL: call fastcc void @[[clone]]([[payloadType]]* {{.*}}[[physical]])
// DXIL: call fastcc void @[[record]](i32 2)
