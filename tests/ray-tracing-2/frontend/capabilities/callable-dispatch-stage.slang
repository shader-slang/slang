//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

import slang.raytracing;

// Callable dispatch is forbidden in any-hit and intersection stages, including through ordinary
// helpers. Both stages use the same inferred capability check as other stage-restricted operations.

struct TestPayload {}
struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct TestProceduralContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = TestRecord;
}

struct TestCallableData
{
    uint value;
}

struct TestCallableContext : rt::ICallableContext
{
    typealias TraceContext = TestTraceContext;
    typealias CallableData = TestCallableData;
    typealias Record = TestRecord;
}

void callCallableFromHelper()
{
    rt::RayTracer<TestSchema> tracer;
    TestCallableData data = {0};
    tracer.callShader<TestCallableContext>(0, program, data);
}

struct BadAnyHit : rt::IAnyHitShader
{
    typealias Context = TestHitContext;
    void invoke(rt::AnyHitInput<TestHitContext> input)
//CHECK: ^^^^^^ dependencies not compatible on stage
//CHECK: ^^^^^^ 'invoke' requires support for stage 'anyhit', but stage is unsupported.
    {
        callCallableFromHelper();
    }
}

struct BadIntersection : rt::IIntersectionShader
{
    typealias Context = TestProceduralContext;
    void invoke(rt::IntersectionInput<TestProceduralContext> input)
//CHECK: ^^^^^^ dependencies not compatible on stage
//CHECK: ^^^^^^ 'invoke' requires support for stage 'intersection', but stage is unsupported.
    {
        callCallableFromHelper();
    }
}

struct TestCallable : rt::ICallableShader
{
    typealias Context = TestCallableContext;
    void invoke(rt::CallableInput<TestCallableContext> input) {}
}

struct TestHitGroup : rt::IHitGroup
{
    typealias Context = TestHitContext;
    typealias ClosestHit = rt::NoClosestHit<TestHitContext>;
    typealias AnyHit = BadAnyHit;
    typealias Intersection = rt::NoIntersection<TestHitContext>;
}

struct TestProceduralHitGroup : rt::IHitGroup
{
    typealias Context = TestProceduralContext;
    typealias ClosestHit = rt::NoClosestHit<TestProceduralContext>;
    typealias AnyHit = rt::NoAnyHit<TestProceduralContext>;
    typealias Intersection = BadIntersection;
}

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestHitGroup, TestProceduralHitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::CallableShaderList<TestCallable>;
}

rt::TraceProgramDescriptor<TestSchema> program;
