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
    void invoke(rt::ClosestHitInput<TestHitContext> input) {}
}

[require(closesthit)]
void callStageDirectly(
    TestClosestHit stage,
    rt::ClosestHitInput<TestHitContext> input)
{
    stage.invoke(input);
//        ^^^^^^ a structural ray-tracing stage method may only be invoked by compiler-generated dispatch
}

[require(closesthit)]
void callImportedStageDirectly(
    rt::NoClosestHit<TestHitContext> stage,
    rt::ClosestHitInput<TestHitContext> input)
{
    stage.invoke(input);
//        ^^^^^^ a structural ray-tracing stage method may only be invoked by compiler-generated dispatch
}
