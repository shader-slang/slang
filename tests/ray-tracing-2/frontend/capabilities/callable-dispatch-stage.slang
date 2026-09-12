//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

import slang.raytracing;

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
//CHECK:   ^^^^^^^^^^ callable dispatch is not available from this ray-tracing stage
//CHECK:   ^^^^^^^^^^ 'callShader' cannot be reached from structural any-hit logic
}

struct BadAnyHit : rt::IAnyHitShader
{
    typealias Context = TestHitContext;
    void invoke(rt::AnyHitInput<TestHitContext> input)
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

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestHitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::CallableShaderList<TestCallable>;
}

rt::TraceProgramDescriptor<TestSchema> program;
