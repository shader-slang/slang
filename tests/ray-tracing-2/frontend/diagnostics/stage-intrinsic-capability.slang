//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

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
void invalidAnyHitHelper()
{
    IgnoreHit();
}

struct TestClosestHit : rt::IClosestHitShader<TestHitContext>
{
    void invoke(rt::ClosestHitInput<TestHitContext> input)
//CHECK: ^^^^^^ dependencies not compatible on stage
//CHECK: ^^^^^^ 'invoke' requires support for stage 'closesthit', but stage is unsupported.
    {
        invalidAnyHitHelper();
    }
}
