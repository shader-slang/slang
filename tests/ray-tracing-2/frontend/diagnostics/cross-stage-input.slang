//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -no-codegen

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

[require(anyhit)]
void invalidAnyHitHelper(
    rt::ClosestHitInput<TestHitContext> input)
//CHECK:^^^^^^^^^^^^^^^ ray-tracing stage input used by the wrong stage
//CHECK:^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<TestHitContext>' requires logical stage 'closesthit', but function 'invalidAnyHitHelper' is not restricted to that stage
{
    IgnoreHit();
}

struct TestClosestHit : rt::IClosestHitShader<TestHitContext>
{
    void invoke(rt::ClosestHitInput<TestHitContext> input)
    {
        invalidAnyHitHelper(input);
    }
}
