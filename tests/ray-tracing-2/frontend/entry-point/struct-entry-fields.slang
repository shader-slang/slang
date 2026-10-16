// A field-bearing stage is a valid entry point. Dispatch calls its static function with input.
//TEST:SIMPLE: -experimental-feature -entry ClosestHit -stage closesthit -no-codegen

import slang.raytracing;

struct TestPayload {}
struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    uint state;

    static void invoke(rt::ClosestHitInput<HitContext> input) {}
}
