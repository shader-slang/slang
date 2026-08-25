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

struct TestCallableData
{
    uint value;
}

struct TestCallableContext : rt::ICallableGroupContext
{
    typealias TraceContext = TestTraceContext;
    typealias CallableData = TestCallableData;
    typealias Record = TestRecord;
}

void callCallableFromHelper()
{
    rt::RayTracer<TestProgramLayout> tracer;
    TestCallableData data = {0};
    tracer.callShader<TestCallableContext>(0, program, data);
//CHECK:   ^^^^^^^^^^ callable dispatch is not available from this ray-tracing stage
//CHECK:   ^^^^^^^^^^ 'callShader' cannot be reached from structural any-hit logic
}

struct BadAnyHit : rt::IAnyHitShader<TestHitContext>
{
    void invoke(rt::AnyHitInput<TestHitContext> input)
    {
        callCallableFromHelper();
    }
}

struct TestCallable : rt::ICallableShader<TestCallableContext>
{
    void invoke(rt::CallableInput<TestCallableContext> input) {}
}

struct TestCallableGroup : rt::ICallableGroup
{
    typealias Slot = rt::CallableSlot<0>;
    typealias Context = TestCallableContext;
    typealias Callable = TestCallable;
}

struct TestHitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = TestHitContext;
    typealias ClosestHit = rt::NoClosestHit<TestHitContext>;
    typealias AnyHit = BadAnyHit;
    typealias Intersection = rt::NoIntersection<TestHitContext>;
}

struct TestProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestTraceContext, TestHitGroup>;
    typealias MissGroups = rt::NoMissGroups<TestTraceContext>;
    typealias CallableGroups = rt::CallableGroupList<TestTraceContext, TestCallableGroup>;
}

rt::TraceProgramDescriptor<TestProgramLayout> program;
