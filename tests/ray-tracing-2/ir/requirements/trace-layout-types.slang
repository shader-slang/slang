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

struct TestAttributes
{
    float2 normal;
}

struct TestProceduralHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<TestAttributes>;
    typealias Record = TestRecord;
}

struct TestProceduralClosestHit : rt::IClosestHitShader<TestProceduralHitContext>
{
    void invoke(rt::ClosestHitInput<TestProceduralHitContext> input) {}
}

struct TestAnyHit : rt::IAnyHitShader<TestProceduralHitContext>
{
    void invoke(rt::AnyHitInput<TestProceduralHitContext> input) {}
}

struct TestIntersection : rt::IIntersectionShader<TestProceduralHitContext>
{
    void invoke(rt::IntersectionInput<TestProceduralHitContext> input) {}
}

struct TestProceduralHitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<3>;
    typealias Context = TestProceduralHitContext;
    typealias ClosestHit = TestProceduralClosestHit;
    typealias AnyHit = TestAnyHit;
    typealias Intersection = TestIntersection;
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
    typealias HitGroups =
        rt::HitGroupList<TestTraceContext, TestHitGroup, TestProceduralHitGroup>;
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

// IR: [nameHint("TestCallable.invoke")]
// IR: func %TestCallablex5Finvoke
// IR: [nameHint("TestMiss.invoke")]
// IR: func %TestMissx5Finvoke
// IR: [nameHint("TestProceduralClosestHit.invoke")]
// IR: func %TestProceduralClosestHitx5Finvoke
// IR: [nameHint("TestAnyHit.invoke")]
// IR: func %TestAnyHitx5Finvoke
// IR: [nameHint("TestIntersection.invoke")]
// IR: func %TestIntersectionx5Finvoke
// IR: [nameHint("TestClosestHit.invoke")]
// IR: func %TestClosestHitx5Finvoke
// IR: [structuralRayTracingCallableGroupInfo(%TestCallableGroup,
// IR-SAME: %TestCallableContext, %TestCallable, %TestCallablex5Finvoke)]
// IR: [structuralRayTracingMissGroupInfo(%TestMissGroup,
// IR-SAME: %TestMissContext, %TestMiss, %TestMissx5Finvoke)]
// IR: [structuralRayTracingHitGroupInfo(%TestProceduralHitGroup,
// IR-SAME: %TestProceduralHitContext, %TestProceduralClosestHit,
// IR-SAME: %TestProceduralClosestHitx5Finvoke, %TestAnyHit, %TestAnyHitx5Finvoke,
// IR-SAME: %TestIntersection, %TestIntersectionx5Finvoke)]
// IR: [structuralRayTracingHitGroupInfo(%TestHitGroup,
// IR-SAME: %TestHitContext, %TestClosestHit, %TestClosestHitx5Finvoke,
// IR-SAME: Void, void_constant, Void, void_constant)]
// IR: structuralRayTracingTrace({{.*}}, %TestProgramLayout, %TestTraceContext,
// IR-SAME: %rtx5FHitGroupList, TypePack(%TestHitGroup, %TestProceduralHitGroup),
// IR-SAME: %rtx5FMissGroupList, TypePack(%TestMissGroup),
// IR-SAME: %rtx5FCallableGroupList, TypePack(%TestCallableGroup),
