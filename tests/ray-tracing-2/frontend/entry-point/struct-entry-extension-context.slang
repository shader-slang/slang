//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry ClosestHit -stage closesthit -target hlsl

import slang.raytracing;

struct Payload
{
    uint value;
}

struct Record
{
    uint value;
}

struct OtherRecord
{
    float value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext
{}

// Keep the conformance in the primary module but outside the type declaration. Associated-type
// recovery must follow this already-checked witness instead of assuming that all context
// conformances are declared directly on the context struct.
extension HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = ::Record;
}

// This more-direct conformance is the canonical `IStageContext` route for `HitContext`. The
// structural entry-point signature and the source-level `input.record` property must resolve the
// same checked route even though entry-point discovery starts with an `IHitContext` witness.
extension HitContext : rt::IStageContext
{
    typealias TraceContext = ::TraceContext;
    typealias Record = ::OtherRecord;
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;

    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        Payload payload = input.payload;
        OtherRecord record = input.record;
    }
}

// CHECK: struct OtherRecord_0
// CHECK: cbuffer record_0
// CHECK: [shader("closesthit")]void ClosestHit
