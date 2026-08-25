//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry TestClosestHit -stage closesthit -target hlsl

import slang.raytracing;

struct TestPayload {}
struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct TestClosestHit : rt::IClosestHitShader<TestHitContext>
{
    void invoke(rt::ClosestHitInput<TestHitContext> input)
    {
        float rayTime = input.time;
//CHECK:                      ^^^^ member not found
//CHECK:                      ^^^^ 'time' is not a member of 'rt.ClosestHitInput<TestHitContext>'.
    }
}
