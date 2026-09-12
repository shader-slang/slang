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
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestRecord
{}

struct TestHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct TestClosestHit : rt::IClosestHitShader
{
    typealias Context = TestHitContext;
    void invoke(rt::ClosestHitInput<TestHitContext> input) {}
}

struct TestHitGroup : rt::IHitGroup
{
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
    typealias Payload = TestPayload;
    typealias Primitive = rt::BoundingBoxPrimitive<TestAttributes>;
    typealias Record = TestRecord;
}

struct TestProceduralClosestHit : rt::IClosestHitShader
{
    typealias Context = TestProceduralHitContext;
    void invoke(rt::ClosestHitInput<TestProceduralHitContext> input) {}
}

struct TestAnyHit : rt::IAnyHitShader
{
    typealias Context = TestProceduralHitContext;
    void invoke(rt::AnyHitInput<TestProceduralHitContext> input) {}
}

struct TestIntersection : rt::IIntersectionShader
{
    typealias Context = TestProceduralHitContext;
    void invoke(rt::IntersectionInput<TestProceduralHitContext> input) {}
}

struct TestProceduralHitGroup : rt::IHitGroup
{
    typealias Context = TestProceduralHitContext;
    typealias ClosestHit = TestProceduralClosestHit;
    typealias AnyHit = TestAnyHit;
    typealias Intersection = TestIntersection;
}

struct TestMissContext : rt::IPayloadContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Record = TestRecord;
}

struct TestMiss : rt::IMissShader
{
    typealias Context = TestMissContext;
    void invoke(rt::MissInput<TestMissContext> input) {}
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

struct TestCallable : rt::ICallableShader
{
    typealias Context = TestCallableContext;
    void invoke(rt::CallableInput<TestCallableContext> input) {}
}

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestHitGroup, TestProceduralHitGroup>;
    typealias MissShaders = rt::MissShaderList<TestMiss>;
    typealias CallableShaders = rt::CallableShaderList<TestCallable>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<TestSchema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<TestSchema> tracer;
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
// IR: [structuralRayTracingCallableShaderInfo(0 : Int, %TestCallableContext,
// IR-SAME: %TestTraceContext, %TestCallableData, %TestRecord, %TestCallable,
// IR-SAME: "TestCallable", {{.*}}, %TestCallablex5Finvoke)]
// IR: [structuralRayTracingMissShaderInfo(0 : Int, %TestMissContext,
// IR-SAME: %TestTraceContext, %TestPayload, {{.*}}, %TestRecord, %TestMiss,
// IR-SAME: "TestMiss", {{.*}}, %TestMissx5Finvoke)]
// IR: [structuralRayTracingHitGroupInfo(%TestProceduralHitGroup,
// IR-SAME: "TestProceduralHitGroup", 1 : Int, %TestProceduralHitContext, %TestTraceContext,
// IR-SAME: %rtx5FBoundingBoxPrimitive, %TestPayload, {{.*}}, %TestRecord,
// IR-SAME: %TestAttributes, 3 : Int, %TestProceduralClosestHit,
// IR-SAME: "TestProceduralClosestHit", {{.*}}, true, %TestProceduralClosestHitx5Finvoke,
// IR-SAME: %TestAnyHit, "TestAnyHit", {{.*}}, true, %TestAnyHitx5Finvoke,
// IR-SAME: %TestIntersection, "TestIntersection", {{.*}}, true, %TestIntersectionx5Finvoke)]
// IR: [structuralRayTracingHitGroupInfo(%TestHitGroup,
// IR-SAME: "TestHitGroup", 0 : Int, %TestHitContext, %TestTraceContext,
// IR-SAME: %rtx5FTrianglePrimitive,
// IR-SAME: %TestPayload, {{.*}}, %TestRecord, %rtx5FTriangleData,
// IR-SAME: 1 : Int, %TestClosestHit, "TestClosestHit", {{.*}}, true,
// IR-SAME: %TestClosestHitx5Finvoke, Void, "", "", false, void_constant,
// IR-SAME: Void, "", "", false, void_constant)]
// IR: structuralRayTracingTrace({{.*}}, %TestSchema, "TestSchema", %TestTraceContext, %TestPayload,
// IR-SAME: %rtx5FHitGroupList, TypePack(%TestHitGroup, %TestProceduralHitGroup),
// IR-SAME: %rtx5FMissShaderList, TypePack(%TestMiss),
// IR-SAME: %rtx5FCallableShaderList, TypePack(%TestCallable),

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

// A target request that asks for both the ray-generation program and the standalone stage emits
// one adapter in each independently compiled entry-point product.
// REUSE-COUNT-2: [shader("closesthit")]void TestClosestHit(
