//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry ClosestHit -stage closesthit -target hlsl

// CHECK: outputBuffer_0[int(0)] = 7U;
// CHECK: [shader("closesthit")]void ClosestHit()
// CHECK: ClosestHit_invoke_0();

import slang.raytracing;

RWStructuredBuffer<uint> outputBuffer;

struct Payload {}
struct Record {}

struct TraceContext : rt::ITraceContext
{
    typealias Payload = Payload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = Record;
}

struct ClosestHit {}

extension ClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        outputBuffer[0] = 7;
    }
}
