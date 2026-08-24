//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5 -dump-ir-after specializeModule

import slang.raytracing;

struct TestPayload
{}

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

struct TestClosestHit : rt::IClosestHitShader<TestHitContext>
{
    void invoke(rt::ClosestHitInput<TestHitContext> input) {}
}

struct TestHitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = TestHitContext;
    typealias ClosestHit = TestClosestHit;
    typealias AnyHit = rt::NoAnyHit<TestHitContext>;
    typealias Intersection = rt::NoIntersection<TestHitContext>;
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

struct TestMissGroup : rt::IMissGroup
{
    typealias Slot = rt::MissSlot<1>;
    typealias Context = TestMissContext;
    typealias Miss = TestMiss;
}

struct TestCallableData
{}

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

struct TestCallableGroup : rt::ICallableGroup
{
    typealias Slot = rt::CallableSlot<2>;
    typealias Context = TestCallableContext;
    typealias Callable = TestCallable;
}

struct TestProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestTraceContext, TestHitGroup>;
    typealias MissGroups = rt::MissGroupList<TestTraceContext, TestMissGroup>;
    typealias CallableGroups = rt::CallableGroupList<TestTraceContext, TestCallableGroup>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<TestProgramLayout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<TestProgramLayout> tracer;
    rt::RayTraversalDesc desc;
    TestPayload payload;
    tracer.trace(desc, scene, program, payload);
}

// IR: structuralRayTracingTrace({{.*}}, %TestProgramLayout, %TestTraceContext,
// IR-SAME: %rtx5FHitGroupList, %rtx5FMissGroupList, %rtx5FCallableGroupList,
