//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5 -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -profile glsl_460 -emit-spirv-directly
//TEST:SIMPLE(filecheck=DXIL): -experimental-feature -entry main -stage raygeneration -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=REUSE): -experimental-feature -entry main -stage raygeneration -entry TestClosestHit -stage closesthit -target hlsl -profile sm_6_5

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

struct TestProceduralSlot : rt::IShaderGroupSlot
{
    static const int index = 3;
}

struct TestProceduralHitGroup : rt::IHitGroup
{
    typealias Slot = TestProceduralSlot;
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
// IR-SAME: %rtx5FShaderGroupSlot{{[0-9]*}}, 2 : Int, %TestCallableContext,
// IR-SAME: %TestCallableData, %TestRecord, %TestCallable,
// IR-SAME: %TestCallablex5Finvoke)]
// IR: [structuralRayTracingMissGroupInfo(%TestMissGroup,
// IR-SAME: %rtx5FShaderGroupSlot{{[0-9]*}}, 1 : Int, %TestMissContext,
// IR-SAME: %TestPayload, %TestRecord, %TestMiss, %TestMissx5Finvoke)]
// IR: [structuralRayTracingHitGroupInfo(%TestProceduralHitGroup,
// IR-SAME: %TestProceduralSlot, 3 : Int, %TestProceduralHitContext,
// IR-SAME: %rtx5FBoundingBoxPrimitive, %TestPayload, %TestRecord,
// IR-SAME: %TestAttributes, 3 : Int, %TestProceduralClosestHit,
// IR-SAME: %TestProceduralClosestHitx5Finvoke, %TestAnyHit, %TestAnyHitx5Finvoke,
// IR-SAME: %TestIntersection, %TestIntersectionx5Finvoke)]
// IR: [structuralRayTracingHitGroupInfo(%TestHitGroup,
// IR-SAME: %rtx5FShaderGroupSlot{{[0-9]*}}, 0 : Int, %TestHitContext,
// IR-SAME: %rtx5FTrianglePrimitive, %TestPayload, %TestRecord, %rtx5FTriangleData,
// IR-SAME: 1 : Int, %TestClosestHit, %TestClosestHitx5Finvoke,
// IR-SAME: Void, void_constant, Void, void_constant)]
// IR: structuralRayTracingTrace({{.*}}, %TestProgramLayout, %TestTraceContext,
// IR-SAME: %rtx5FHitGroupList, TypePack(%TestHitGroup, %TestProceduralHitGroup),
// IR-SAME: %rtx5FMissGroupList, TypePack(%TestMissGroup),
// IR-SAME: %rtx5FCallableGroupList, TypePack(%TestCallableGroup),

// HLSL-DAG: [shader("raygeneration")]void main()
// HLSL-DAG: [shader("callable")]void TestCallable(inout TestCallableData_0
// HLSL-DAG: [shader("miss")]void TestMiss(inout TestPayload_0
// HLSL-DAG: [shader("closesthit")]void TestProceduralClosestHit(inout TestPayload_0
// HLSL-DAG: [shader("anyhit")]void TestAnyHit(inout TestPayload_0
// HLSL-DAG: [shader("intersection")]void TestIntersection()
// HLSL-DAG: [shader("closesthit")]void TestClosestHit(inout TestPayload_0
// HLSL-NOT: NoClosestHit
// HLSL-NOT: NoAnyHit
// HLSL-NOT: NoIntersection

// SPIRV-DAG: OpEntryPoint RayGenerationKHR %main "main"
// SPIRV-DAG: OpEntryPoint CallableKHR %TestCallable "TestCallable"
// SPIRV-DAG: OpEntryPoint MissKHR %TestMiss "TestMiss"
// SPIRV-DAG: OpEntryPoint ClosestHitKHR %TestProceduralClosestHit "TestProceduralClosestHit"
// SPIRV-DAG: OpEntryPoint AnyHitKHR %TestAnyHit "TestAnyHit"
// SPIRV-DAG: OpEntryPoint IntersectionKHR %TestIntersection "TestIntersection"
// SPIRV-DAG: OpEntryPoint ClosestHitKHR %TestClosestHit "TestClosestHit"

// DXIL-DAG: TestCallable
// DXIL-DAG: TestMiss
// DXIL-DAG: TestProceduralClosestHit
// DXIL-DAG: TestAnyHit
// DXIL-DAG: TestIntersection
// DXIL-DAG: TestClosestHit

// REUSE-COUNT-1: [shader("closesthit")]void TestClosestHit(
