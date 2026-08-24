//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestRecord
{}

struct TestHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct HitGroupA : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = TestHitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct HitGroupB : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = TestHitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct TestMissContext : rt::IMissGroupContext
{
    typealias TraceContext = TestTraceContext;
    typealias Record = TestRecord;
}

struct TestMiss : rt::IMissShader<TestMissContext>
{
    void invoke(rt::MissInput<TestMissContext> input) {}
}

struct MissGroup : rt::IMissGroup
{
    typealias Slot = rt::MissSlot<-1>;
    typealias Context = TestMissContext;
    typealias Miss = TestMiss;
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

struct TestCallable : rt::ICallableShader<TestCallableContext>
{
    void invoke(rt::CallableInput<TestCallableContext> input) {}
}

struct CallableGroupA : rt::ICallableGroup
{
    typealias Slot = rt::CallableSlot<2>;
    typealias Context = TestCallableContext;
    typealias Callable = TestCallable;
}

struct CallableGroupB : rt::ICallableGroup
{
    typealias Slot = rt::CallableSlot<2>;
    typealias Context = TestCallableContext;
    typealias Callable = TestCallable;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestTraceContext, HitGroupA, HitGroupB>;
    typealias MissGroups = rt::MissGroupList<TestTraceContext, MissGroup>;
    typealias CallableGroups =
        rt::CallableGroupList<TestTraceContext, CallableGroupA, CallableGroupB>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<ProgramLayout> tracer;
    rt::RayTraversalDesc desc = {};
    TestPayload payload = {};
    tracer.trace(desc, scene, program, payload);
//CHECK:        ^ duplicate structural ray-tracing group slot
//CHECK:        ^ callable group slot 2 is declared more than once in the trace program
//CHECK:        ^ invalid structural ray-tracing group slot
//CHECK:        ^ miss group slot -1 must be non-negative
//CHECK:        ^ duplicate structural ray-tracing group slot
//CHECK:        ^ hit group slot 0 is declared more than once in the trace program
}
