//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature

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
        this.invoke(input);
//CHECK:     ^^^^^^ direct invocation of a structural ray-tracing stage
//CHECK:     ^^^^^^ a structural ray-tracing stage method may only be invoked by compiler-generated dispatch
    }
}
