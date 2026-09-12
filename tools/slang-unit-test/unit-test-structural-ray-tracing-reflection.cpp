#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>

using namespace Slang;

SLANG_UNIT_TEST(structuralRayTracingReflection)
{
    const char* source = R"(
        import slang.raytracing;

        struct PayloadA { float value; }
        struct PayloadB { uint value; }
        struct HitRecordA { float value; }
        struct HitRecordB { uint4 value; uint tail; }
        struct MissRecordA { float value; }
        struct MissRecordB { uint value; }
        struct CallableRecord { float4 value; float tail; }
        struct CallableData { float value; }

        struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        typealias CommonTraceContext = TraceContext;
        typealias CommonCallableData = CallableData;

        struct HitContextA : rt::IHitContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = PayloadA;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = HitRecordA;
        }

        struct HitContextB : rt::IHitContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = PayloadB;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = HitRecordB;
        }

        struct MissContextA : rt::IPayloadContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = PayloadA;
            typealias Record = MissRecordA;
        }

        struct MissContextB : rt::IPayloadContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = PayloadB;
            typealias Record = MissRecordB;
        }

        struct CallableContext : rt::ICallableContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias CallableData = CommonCallableData;
            typealias Record = CallableRecord;
        }

        struct ClosestHitA0 : rt::IClosestHitShader
        {
            typealias Context = HitContextA;
            void invoke(rt::ClosestHitInput<HitContextA> input) {}
        }

        struct ClosestHitB : rt::IClosestHitShader
        {
            typealias Context = HitContextB;
            void invoke(rt::ClosestHitInput<HitContextB> input) {}
        }

        struct ClosestHitA1 : rt::IClosestHitShader
        {
            typealias Context = HitContextA;
            void invoke(rt::ClosestHitInput<HitContextA> input) {}
        }

        struct AnyHitA0 : rt::IAnyHitShader
        {
            typealias Context = HitContextA;
            void invoke(rt::AnyHitInput<HitContextA> input) {}
        }

        struct HitGroupA0 : rt::IHitGroup
        {
            typealias Context = HitContextA;
            typealias ClosestHit = ClosestHitA0;
            typealias AnyHit = AnyHitA0;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct HitGroupB : rt::IHitGroup
        {
            typealias Context = HitContextB;
            typealias ClosestHit = ClosestHitB;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct HitGroupA1 : rt::IHitGroup
        {
            typealias Context = HitContextA;
            typealias ClosestHit = ClosestHitA1;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct MissA0 : rt::IMissShader
        {
            typealias Context = MissContextA;
            void invoke(rt::MissInput<MissContextA> input) {}
        }

        struct MissB : rt::IMissShader
        {
            typealias Context = MissContextB;
            void invoke(rt::MissInput<MissContextB> input) {}
        }

        struct MissA1 : rt::IMissShader
        {
            typealias Context = MissContextA;
            void invoke(rt::MissInput<MissContextA> input) {}
        }

        struct Callable0 : rt::ICallableShader
        {
            typealias Context = CallableContext;
            void invoke(rt::CallableInput<CallableContext> input) {}
        }

        struct Callable1 : rt::ICallableShader
        {
            typealias Context = CallableContext;
            void invoke(rt::CallableInput<CallableContext> input) {}
        }

        struct ReflectedSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = CommonTraceContext;
            typealias HitGroups = rt::HitGroupList<HitGroupA0, HitGroupB, HitGroupA1>;
            typealias MissShaders = rt::MissShaderList<MissA0, MissB, MissA1>;
            typealias CallableShaders = rt::CallableShaderList<Callable0, Callable1>;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_METAL;
    target.profile = globalSession->findProfile("metal_3_1");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "structuralReflection",
        "structural-reflection.slang",
        source,
        diagnostics.writeRef());
    if (!module && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK(module != nullptr);

    auto program = module->getLayout();
    auto schema = program->findTraceProgramSchema("ReflectedSchema");
    SLANG_CHECK(schema != nullptr);
    SLANG_CHECK(UnownedStringSlice(schema->getName()) == "ReflectedSchema");
    SLANG_CHECK(UnownedStringSlice(schema->getType()->getName()) == "ReflectedSchema");
    SLANG_CHECK(UnownedStringSlice(schema->getTraceContextType()->getName()) == "TraceContext");

    SLANG_CHECK(schema->getPayloadCount() == 2);
    auto payloadA = schema->getPayload(0);
    SLANG_CHECK(payloadA != nullptr);
    SLANG_CHECK(UnownedStringSlice(payloadA->getType()->getName()) == "PayloadA");
    SLANG_CHECK(payloadA->getHitGroupCount() == 2);
    auto hitGroupA0 = payloadA->getHitGroup(0);
    auto hitGroupA1 = payloadA->getHitGroup(1);
    SLANG_CHECK(hitGroupA0 != nullptr);
    SLANG_CHECK(hitGroupA1 != nullptr);
    SLANG_CHECK(hitGroupA0->getFunctionIndex() == 0);
    SLANG_CHECK(hitGroupA1->getFunctionIndex() == 1);
    SLANG_CHECK(UnownedStringSlice(hitGroupA0->getType()->getName()) == "HitGroupA0");
    SLANG_CHECK(UnownedStringSlice(hitGroupA0->getRecordType()->getName()) == "HitRecordA");
    SLANG_CHECK(
        UnownedStringSlice(hitGroupA0->getPrimitiveType()->getName()) == "TrianglePrimitive");
    SLANG_CHECK(
        UnownedStringSlice(hitGroupA0->getIntersectionAttributesType()->getName()) ==
        "TriangleData");
    SLANG_CHECK(hitGroupA0->getClosestHit()->getStage() == SLANG_STAGE_CLOSEST_HIT);
    SLANG_CHECK(hitGroupA0->getClosestHit()->getEntryPointName() != nullptr);
    SLANG_CHECK(
        UnownedStringSlice(hitGroupA0->getClosestHit()->getEntryPointName()) != "ClosestHitA0");
    SLANG_CHECK(hitGroupA0->getAnyHit()->getStage() == SLANG_STAGE_ANY_HIT);
    SLANG_CHECK(hitGroupA0->getAnyHit()->getEntryPointName() == nullptr);
    SLANG_CHECK(hitGroupA0->getIntersection() == nullptr);

    SLANG_CHECK(payloadA->getIntersectionFunctionTableSize() == 1);
    SLANG_CHECK(payloadA->getIntersectionFunctionCount() == 1);
    auto triangleIntersection = payloadA->getIntersectionFunction(0);
    SLANG_CHECK(triangleIntersection != nullptr);
    SLANG_CHECK(triangleIntersection->getIntersectionFunctionTableIndex() == 0);
    SLANG_CHECK(
        triangleIntersection->getGeometryKind() == SLANG_STRUCTURAL_RAY_TRACING_GEOMETRY_TRIANGLE);
    SLANG_CHECK(
        triangleIntersection->getImplementationKind() ==
        SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_EXPORTED_FUNCTION);
    SLANG_CHECK(triangleIntersection->getEntryPointName() != nullptr);

    SLANG_CHECK(payloadA->getMissShaderCount() == 2);
    auto missA0 = payloadA->getMissShader(0);
    auto missA1 = payloadA->getMissShader(1);
    SLANG_CHECK(missA0 != nullptr);
    SLANG_CHECK(missA1 != nullptr);
    SLANG_CHECK(missA0->getFunctionIndex() == 0);
    SLANG_CHECK(missA1->getFunctionIndex() == 1);
    SLANG_CHECK(UnownedStringSlice(missA0->getType()->getName()) == "MissA0");
    SLANG_CHECK(UnownedStringSlice(missA0->getRecordType()->getName()) == "MissRecordA");
    SLANG_CHECK(missA0->getMiss()->getStage() == SLANG_STAGE_MISS);
    SLANG_CHECK(missA0->getMiss()->getEntryPointName() != nullptr);
    SLANG_CHECK(UnownedStringSlice(missA0->getMiss()->getEntryPointName()) != "MissA0");

    auto payloadB = schema->getPayload(1);
    SLANG_CHECK(payloadB != nullptr);
    SLANG_CHECK(UnownedStringSlice(payloadB->getType()->getName()) == "PayloadB");
    SLANG_CHECK(payloadB->getHitGroupCount() == 1);
    SLANG_CHECK(payloadB->getHitGroup(0)->getFunctionIndex() == 0);
    SLANG_CHECK(payloadB->getMissShaderCount() == 1);
    SLANG_CHECK(payloadB->getMissShader(0)->getFunctionIndex() == 0);
    SLANG_CHECK(payloadB->getIntersectionFunctionTableSize() == 0);
    SLANG_CHECK(payloadB->getIntersectionFunctionCount() == 0);

    SLANG_CHECK(schema->getCallableShaderCount() == 2);
    auto callable0 = schema->getCallableShader(0);
    auto callable1 = schema->getCallableShader(1);
    SLANG_CHECK(callable0 != nullptr);
    SLANG_CHECK(callable1 != nullptr);
    SLANG_CHECK(callable0->getFunctionIndex() == 0);
    SLANG_CHECK(callable1->getFunctionIndex() == 1);
    SLANG_CHECK(UnownedStringSlice(callable0->getType()->getName()) == "Callable0");
    SLANG_CHECK(UnownedStringSlice(callable0->getRecordType()->getName()) == "CallableRecord");
    SLANG_CHECK(UnownedStringSlice(callable0->getDataType()->getName()) == "CallableData");
    SLANG_CHECK(callable0->getCallable()->getStage() == SLANG_STAGE_CALLABLE);
    SLANG_CHECK(callable0->getCallable()->getEntryPointName() != nullptr);
    SLANG_CHECK(UnownedStringSlice(callable0->getCallable()->getEntryPointName()) != "Callable0");
    // The physical record sections are shared by both payload partitions. HitRecordB therefore
    // raises the one schema-wide hit stride instead of giving payload B a private interpretation
    // of the same record index.
    SLANG_CHECK(schema->getHitRecordStride() == 48);
    SLANG_CHECK(schema->getMissRecordStride() == 32);
    SLANG_CHECK(schema->getCallableRecordStride() == 48);

    SLANG_CHECK(schema->getDescriptorResourceCount() == 8);
    SLANG_CHECK(
        schema->getDescriptorResourceKind(0) ==
        SLANG_STRUCTURAL_RAY_TRACING_DESCRIPTOR_INTERSECTION_FUNCTION_TABLE);
    SLANG_CHECK(schema->getDescriptorResourcePayloadIndex(0) == 0);
    SLANG_CHECK(
        UnownedStringSlice(schema->getDescriptorResourceName(0)) == "intersectionFunctions0");
    SLANG_CHECK(
        schema->getDescriptorResourceKind(3) ==
        SLANG_STRUCTURAL_RAY_TRACING_DESCRIPTOR_INTERSECTION_FUNCTION_TABLE);
    SLANG_CHECK(schema->getDescriptorResourcePayloadIndex(3) == 1);
    SLANG_CHECK(
        UnownedStringSlice(schema->getDescriptorResourceName(3)) == "intersectionFunctions1");
    SLANG_CHECK(
        schema->getDescriptorResourceKind(6) ==
        SLANG_STRUCTURAL_RAY_TRACING_DESCRIPTOR_CALLABLE_VISIBLE_FUNCTION_TABLE);
    SLANG_CHECK(schema->getDescriptorResourcePayloadIndex(6) == -1);
    SLANG_CHECK(UnownedStringSlice(schema->getDescriptorResourceName(6)) == "callableFunctions");
    SLANG_CHECK(
        schema->getDescriptorResourceKind(7) == SLANG_STRUCTURAL_RAY_TRACING_DESCRIPTOR_RECORDS);
    SLANG_CHECK(schema->getDescriptorResourcePayloadIndex(7) == -1);
    SLANG_CHECK(UnownedStringSlice(schema->getDescriptorResourceName(7)) == "records");

    SLANG_CHECK(program->findTraceProgramSchema("HitContextA") == nullptr);
}

SLANG_UNIT_TEST(structuralRayTracingEntryPointRename)
{
    const char* expectedClosestHitName =
        "__slang_structural_rt_"
        "53746167654e616d6573706163652e54657374436c6f73657374486974";
    const char* expectedMissName =
        "__slang_structural_rt_53746167654e616d6573706163652e546573744d697373";
    const char* unspecializedGenericMissName =
        "__slang_structural_rt_"
        "53746167654e616d6573706163652e47656e657269634d697373";
    const char* expectedMainName = "__slang_structural_rt_6d61696e";

    const char* source = R"(
        import slang.raytracing;

        namespace StageNamespace
        {
            struct Payload { float value; }
            struct Record {}

            struct TraceContext : rt::ITraceContext
            {
                typealias AccelerationStructure = rt::AccelerationStructure;
                typealias Motion = rt::NoMotion;
            }

            struct MissContext : rt::IPayloadContext
            {
                typealias TraceContext = StageNamespace.TraceContext;
                typealias Payload = StageNamespace.Payload;
                typealias Record = StageNamespace.Record;
            }

            struct HitContext : rt::IHitContext
            {
                typealias TraceContext = StageNamespace::TraceContext;
                typealias Payload = StageNamespace::Payload;
                typealias Primitive = rt::TrianglePrimitive;
                typealias Record = StageNamespace::Record;
            }

            struct TestClosestHit : rt::IClosestHitShader
            {
                typealias Context = HitContext;
                void invoke(rt::ClosestHitInput<HitContext> input)
                {
                    input.payload.value = input.triangle.barycentricCoord.x;
                }
            }

            struct HitGroup : rt::IHitGroup
            {
                typealias Context = HitContext;
                typealias ClosestHit = TestClosestHit;
                typealias AnyHit = rt::NoAnyHit<HitContext>;
                typealias Intersection = rt::NoIntersection<HitContext>;
            }

            struct TestMiss : rt::IMissShader
            {
                typealias Context = MissContext;
                void invoke(rt::MissInput<MissContext> input)
                {
                    input.payload.value = 1.0f;
                }
            }

            struct Schema : rt::ITraceProgramSchema
            {
                typealias TraceContext = StageNamespace::TraceContext;
                typealias HitGroups = rt::HitGroupList<StageNamespace::HitGroup>;
                typealias MissShaders = rt::MissShaderList<StageNamespace::TestMiss>;
                typealias CallableShaders = rt::NoCallableShaders;
            }

            struct GenericMiss<T> : rt::IMissShader
            {
                typealias Context = MissContext;
                void invoke(rt::MissInput<MissContext> input)
                {
                    input.payload.value = 4.0f;
                }
            }

            struct GenericSchema<T> : rt::ITraceProgramSchema
            {
                typealias TraceContext = StageNamespace::TraceContext;
                typealias HitGroups = rt::NoHitGroups;
                typealias MissShaders = rt::MissShaderList<GenericMiss<T>>;
                typealias CallableShaders = rt::NoCallableShaders;
            }
        }

        struct main : rt::IMissShader
        {
            typealias Context = StageNamespace::MissContext;
            void invoke(rt::MissInput<StageNamespace::MissContext> input)
            {
                input.payload.value = 2.0f;
            }
        }

        struct MainSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = StageNamespace::TraceContext;
            typealias HitGroups = rt::HitGroupList<StageNamespace::HitGroup>;
            typealias MissShaders = rt::MissShaderList<main>;
            typealias CallableShaders = rt::NoCallableShaders;
        }

        // This legal source identifier is exactly the encoded spelling of
        // `StageNamespace.TestMiss`. It must itself be encoded to keep the mapping injective.
        struct __slang_structural_rt_53746167654e616d6573706163652e546573744d697373
            : rt::IMissShader
        {
            typealias Context = StageNamespace::MissContext;
            void invoke(rt::MissInput<StageNamespace::MissContext> input)
            {
                input.payload.value = 3.0f;
            }
        }

        struct ReservedPrefixSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = StageNamespace::TraceContext;
            typealias HitGroups = rt::HitGroupList<StageNamespace::HitGroup>;
            typealias MissShaders = rt::MissShaderList<
                __slang_structural_rt_53746167654e616d6573706163652e546573744d697373>;
            typealias CallableShaders = rt::NoCallableShaders;
        }

        rt::AccelerationStructure scene;
        rt::TraceProgramDescriptor<StageNamespace::Schema> traceProgram;
        rt::TraceProgramDescriptor<StageNamespace::GenericSchema<uint>> genericTraceProgram;
        rt::TraceProgramDescriptor<StageNamespace::GenericSchema<float>> floatGenericTraceProgram;

        void traceHelper(inout StageNamespace::Payload payload)
        {
            rt::RayTraversalDesc desc = {};
            desc.ray.direction = float3(0.0f, 0.0f, 1.0f);
            desc.ray.tMax = 1.0f;
            desc.instanceMask = 0xff;
            rt::RayTracer<StageNamespace::Schema> tracer;
            tracer.trace(desc, scene, traceProgram, payload);
        }

        void traceGenericHelper(inout StageNamespace::Payload payload)
        {
            rt::RayTraversalDesc desc = {};
            desc.ray.direction = float3(0.0f, 0.0f, 1.0f);
            desc.ray.tMax = 1.0f;
            desc.instanceMask = 0xff;
            rt::RayTracer<StageNamespace::GenericSchema<uint>> tracer;
            tracer.trace(desc, scene, genericTraceProgram, payload);
        }

        void traceFloatGenericHelper(inout StageNamespace::Payload payload)
        {
            rt::RayTraversalDesc desc = {};
            desc.ray.direction = float3(0.0f, 0.0f, 1.0f);
            desc.ray.tMax = 1.0f;
            desc.instanceMask = 0xff;
            rt::RayTracer<StageNamespace::GenericSchema<float>> tracer;
            tracer.trace(desc, scene, floatGenericTraceProgram, payload);
        }
    )";

    const char* raygenSource = R"(
        import structuralRename;

        [shader("raygeneration")]
        void raygenMain()
        {
            StageNamespace::Payload payload = {};
            traceHelper(payload);
        }

        [shader("raygeneration")]
        void genericRaygenMain()
        {
            StageNamespace::Payload payload = {};
            traceGenericHelper(payload);
            traceFloatGenericHelper(payload);
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_CUDA_SOURCE;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "structuralRename",
        "structural-rename.slang",
        source,
        diagnostics.writeRef()));
    if (!module && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(module != nullptr);

    auto reflectedProgram = module->getLayout();
    auto reflectedSchema = reflectedProgram->findTraceProgramSchema("StageNamespace.Schema");
    SLANG_CHECK_ABORT(reflectedSchema != nullptr);
    auto reflectedPayload = reflectedSchema->getPayload(0);
    SLANG_CHECK_ABORT(reflectedPayload != nullptr);
    auto reflectedClosestHit = reflectedPayload->getHitGroup(0)->getClosestHit();
    auto reflectedMiss = reflectedPayload->getMissShader(0)->getMiss();
    SLANG_CHECK_ABORT(reflectedClosestHit != nullptr);
    SLANG_CHECK_ABORT(reflectedMiss != nullptr);
    String closestHitEntryPointName(reflectedClosestHit->getEntryPointName());
    String missEntryPointName(reflectedMiss->getEntryPointName());
    SLANG_CHECK(closestHitEntryPointName == expectedClosestHitName);
    SLANG_CHECK(missEntryPointName == expectedMissName);
    String expectedMissSymbol = String("__miss__") + missEntryPointName;
    String expectedClosestHitSymbol = String("__closesthit__") + closestHitEntryPointName;

    auto reflectedMainSchema = reflectedProgram->findTraceProgramSchema("MainSchema");
    SLANG_CHECK_ABORT(reflectedMainSchema != nullptr);
    auto reflectedMain = reflectedMainSchema->getPayload(0)->getMissShader(0)->getMiss();
    SLANG_CHECK_ABORT(reflectedMain != nullptr);
    String mainEntryPointName(reflectedMain->getEntryPointName());
    SLANG_CHECK(mainEntryPointName == expectedMainName);

    auto reflectedReservedSchema = reflectedProgram->findTraceProgramSchema("ReservedPrefixSchema");
    SLANG_CHECK_ABORT(reflectedReservedSchema != nullptr);
    auto reflectedReserved = reflectedReservedSchema->getPayload(0)->getMissShader(0)->getMiss();
    SLANG_CHECK_ABORT(reflectedReserved != nullptr);
    String reservedEntryPointName(reflectedReserved->getEntryPointName());
    SLANG_CHECK(reservedEntryPointName != expectedMissName);
    SLANG_CHECK(reservedEntryPointName.getUnownedSlice().startsWith("__slang_structural_rt_"));

    auto reflectedGenericSchema =
        reflectedProgram->findTraceProgramSchema("StageNamespace.GenericSchema<uint>");
    SLANG_CHECK_ABORT(reflectedGenericSchema != nullptr);
    SLANG_CHECK(
        UnownedStringSlice(reflectedGenericSchema->getName()) !=
        "StageNamespace.GenericSchema<uint>");
    auto reflectedGenericMiss = reflectedGenericSchema->getPayload(0)->getMissShader(0)->getMiss();
    SLANG_CHECK_ABORT(reflectedGenericMiss != nullptr);
    String genericMissEntryPointName(reflectedGenericMiss->getEntryPointName());
    SLANG_CHECK(genericMissEntryPointName != unspecializedGenericMissName);

    // Both types instantiate the same source declaration, but they are different semantic stage
    // types and must therefore advertise different physical entry-point names.
    auto reflectedFloatGenericSchema =
        reflectedProgram->findTraceProgramSchema("StageNamespace.GenericSchema<float>");
    SLANG_CHECK_ABORT(reflectedFloatGenericSchema != nullptr);
    auto reflectedFloatGenericMiss =
        reflectedFloatGenericSchema->getPayload(0)->getMissShader(0)->getMiss();
    SLANG_CHECK_ABORT(reflectedFloatGenericMiss != nullptr);
    String floatGenericMissEntryPointName(reflectedFloatGenericMiss->getEntryPointName());
    SLANG_CHECK(floatGenericMissEntryPointName != unspecializedGenericMissName);
    SLANG_CHECK(floatGenericMissEntryPointName != genericMissEntryPointName);

    ComPtr<slang::IModule> raygenModule(session->loadModuleFromSourceString(
        "structuralRenameRaygen",
        "structural-rename-raygen.slang",
        raygenSource,
        diagnostics.writeRef()));
    if (!raygenModule && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(raygenModule != nullptr);

    ComPtr<slang::IEntryPoint> sourceEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "StageNamespace.TestMiss",
        SLANG_STAGE_MISS,
        sourceEntryPoint.writeRef(),
        diagnostics.writeRef())));

    // Materializing the selected structural stage without a client rename must use the same
    // target-safe default that reflection advertises.
    ComPtr<slang::IComponentType> linkedUnrenamedEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        sourceEntryPoint->link(linkedUnrenamedEntryPoint.writeRef(), diagnostics.writeRef())));
    ComPtr<slang::IBlob> unrenamedCode;
    auto unrenamedResult = linkedUnrenamedEntryPoint->getEntryPointCode(
        0,
        0,
        unrenamedCode.writeRef(),
        diagnostics.writeRef());
    if (SLANG_FAILED(unrenamedResult) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(unrenamedResult));
    SLANG_CHECK_ABORT(unrenamedCode != nullptr);
    UnownedStringSlice unrenamedGeneratedCode(
        (const char*)unrenamedCode->getBufferPointer(),
        (const char*)unrenamedCode->getBufferPointer() + unrenamedCode->getBufferSize());
    SLANG_CHECK(unrenamedGeneratedCode.indexOf(expectedMissSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(unrenamedGeneratedCode.indexOf(toSlice("__miss__StageNamespace.TestMiss")) == -1);

    ComPtr<slang::IEntryPoint> mainEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "main",
        SLANG_STAGE_MISS,
        mainEntryPoint.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> linkedMainEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        mainEntryPoint->link(linkedMainEntryPoint.writeRef(), diagnostics.writeRef())));
    ComPtr<slang::IBlob> mainCode;
    auto mainResult =
        linkedMainEntryPoint->getEntryPointCode(0, 0, mainCode.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(mainResult) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(mainResult));
    SLANG_CHECK_ABORT(mainCode != nullptr);
    UnownedStringSlice mainGeneratedCode(
        (const char*)mainCode->getBufferPointer(),
        (const char*)mainCode->getBufferPointer() + mainCode->getBufferSize());
    String expectedMainSymbol = String("__miss__") + mainEntryPointName;
    SLANG_CHECK(mainGeneratedCode.indexOf(expectedMainSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(mainGeneratedCode.indexOf(toSlice("__miss__main_")) == -1);

    ComPtr<slang::IComponentType> renamedEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        sourceEntryPoint->renameEntryPoint("renamedStructuralMiss", renamedEntryPoint.writeRef())));

    ComPtr<slang::IEntryPoint> sourceClosestHit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "StageNamespace.TestClosestHit",
        SLANG_STAGE_CLOSEST_HIT,
        sourceClosestHit.writeRef(),
        diagnostics.writeRef())));

    ComPtr<slang::IComponentType> renamedClosestHit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(sourceClosestHit->renameEntryPoint(
        "renamedStructuralClosestHit",
        renamedClosestHit.writeRef())));

    ComPtr<slang::IEntryPoint> raygenEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        raygenModule->findEntryPointByName("raygenMain", raygenEntryPoint.writeRef())));

    // This composite contains no selected structural stage components. The raygen trace operation
    // must synthesize the same names that the program-schema reflection above advertised.
    slang::IComponentType* autoComponents[] = {raygenModule, module, raygenEntryPoint};
    ComPtr<slang::IComponentType> autoProgram;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        autoComponents,
        SLANG_COUNT_OF(autoComponents),
        autoProgram.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> linkedAutoProgram;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(autoProgram->link(linkedAutoProgram.writeRef(), diagnostics.writeRef())));
    ComPtr<slang::IBlob> autoCode;
    auto autoResult =
        linkedAutoProgram->getEntryPointCode(0, 0, autoCode.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(autoResult) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(autoResult));
    SLANG_CHECK_ABORT(autoCode != nullptr);
    UnownedStringSlice autoGeneratedCode(
        (const char*)autoCode->getBufferPointer(),
        (const char*)autoCode->getBufferPointer() + autoCode->getBufferSize());
    SLANG_CHECK(autoGeneratedCode.indexOf(expectedMissSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(autoGeneratedCode.indexOf(expectedClosestHitSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(autoGeneratedCode.indexOf(toSlice("__miss__StageNamespace.TestMiss")) == -1);
    SLANG_CHECK(
        autoGeneratedCode.indexOf(toSlice("__closesthit__StageNamespace.TestClosestHit")) == -1);

    ComPtr<slang::IEntryPoint> genericRaygenEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(raygenModule->findEntryPointByName(
        "genericRaygenMain",
        genericRaygenEntryPoint.writeRef())));
    slang::IComponentType* genericComponents[] = {
        raygenModule,
        module,
        genericRaygenEntryPoint,
    };
    ComPtr<slang::IComponentType> genericProgram;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        genericComponents,
        SLANG_COUNT_OF(genericComponents),
        genericProgram.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> linkedGenericProgram;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        genericProgram->link(linkedGenericProgram.writeRef(), diagnostics.writeRef())));
    ComPtr<slang::IBlob> genericCode;
    auto genericResult = linkedGenericProgram->getEntryPointCode(
        0,
        0,
        genericCode.writeRef(),
        diagnostics.writeRef());
    if (SLANG_FAILED(genericResult) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(genericResult));
    SLANG_CHECK_ABORT(genericCode != nullptr);
    UnownedStringSlice genericGeneratedCode(
        (const char*)genericCode->getBufferPointer(),
        (const char*)genericCode->getBufferPointer() + genericCode->getBufferSize());
    String expectedGenericMissSymbol = String("__miss__") + genericMissEntryPointName;
    String expectedFloatGenericMissSymbol = String("__miss__") + floatGenericMissEntryPointName;
    SLANG_CHECK(genericGeneratedCode.indexOf(expectedGenericMissSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(
        genericGeneratedCode.indexOf(expectedFloatGenericMissSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(genericGeneratedCode.indexOf(toSlice("GenericMiss<uint>")) == -1);

    slang::IComponentType* components[] = {
        raygenModule,
        module,
        raygenEntryPoint,
        renamedClosestHit,
        renamedEntryPoint,
    };
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        program.writeRef(),
        diagnostics.writeRef())));

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(program->link(linkedProgram.writeRef(), diagnostics.writeRef())));

    ComPtr<slang::IBlob> code;
    auto result = linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    SLANG_CHECK_ABORT(code != nullptr);

    UnownedStringSlice generatedCode(
        (const char*)code->getBufferPointer(),
        (const char*)code->getBufferPointer() + code->getBufferSize());
    SLANG_CHECK(generatedCode.indexOf(expectedMissSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(generatedCode.indexOf(expectedClosestHitSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(generatedCode.indexOf(toSlice("__miss__renamedStructuralMiss")) == -1);
    SLANG_CHECK(generatedCode.indexOf(toSlice("__closesthit__renamedStructuralClosestHit")) == -1);
    SLANG_CHECK(generatedCode.indexOf(toSlice("__miss__StageNamespace.TestMiss")) == -1);
    SLANG_CHECK(
        generatedCode.indexOf(toSlice("__closesthit__StageNamespace.TestClosestHit")) == -1);

    code.setNull();
    result = linkedProgram->getEntryPointCode(1, 0, code.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    SLANG_CHECK_ABORT(code != nullptr);
    generatedCode = UnownedStringSlice(
        (const char*)code->getBufferPointer(),
        (const char*)code->getBufferPointer() + code->getBufferSize());
    SLANG_CHECK(generatedCode.indexOf(toSlice("__closesthit__renamedStructuralClosestHit")) != -1);

    code.setNull();
    result = linkedProgram->getEntryPointCode(2, 0, code.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    SLANG_CHECK_ABORT(code != nullptr);
    generatedCode = UnownedStringSlice(
        (const char*)code->getBufferPointer(),
        (const char*)code->getBufferPointer() + code->getBufferSize());
    SLANG_CHECK(generatedCode.indexOf(toSlice("__miss__renamedStructuralMiss")) != -1);

    // The castable interface is available on every compiled target, but target-only Metal ABI
    // records must not leak into CUDA (or any other portable target).
    ComPtr<slang::IMetadata> targetMetadata;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        linkedProgram->getTargetMetadata(0, targetMetadata.writeRef(), diagnostics.writeRef())));
    auto structuralMetadata = static_cast<slang::IStructuralRayTracingMetadata*>(
        targetMetadata->castAs(slang::IStructuralRayTracingMetadata::getTypeGuid()));
    SLANG_CHECK_ABORT(structuralMetadata != nullptr);
    SLANG_CHECK(structuralMetadata->getMetalPayloadInfoCount() == 0);
}

SLANG_UNIT_TEST(structuralRayTracingMetalTargetMetadata)
{
    const char* source = R"(
        import slang.raytracing;

        struct Payload { float value; }
        struct CustomAttributes { float value; }

        struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        struct TriangleContext : rt::IHitContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = ::Payload;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = void;
        }

        struct BoundingBoxContext : rt::IHitContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = ::Payload;
            typealias Primitive = rt::BoundingBoxPrimitive<CustomAttributes>;
            typealias Record = void;
        }

        struct CurveContext : rt::IHitContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = ::Payload;
            typealias Primitive = rt::CurvePrimitive;
            typealias Record = void;
        }

        struct BoundingBoxClosestHit : rt::IClosestHitShader
        {
            typealias Context = BoundingBoxContext;
            void invoke(rt::ClosestHitInput<Context> input)
            {
                input.payload.value = input.hitAttributes.value;
            }
        }

        struct BoundingBoxIntersection : rt::IIntersectionShader
        {
            typealias Context = BoundingBoxContext;
            void invoke(rt::IntersectionInput<Context> input)
            {
                CustomAttributes attributes = { input.worldSpaceOrigin.x };
                input.reportHit(1.0, attributes);
            }
        }

        struct TriangleGroup : rt::IHitGroup
        {
            typealias Context = TriangleContext;
            typealias ClosestHit = rt::NoClosestHit<Context>;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct BoundingBoxGroup : rt::IHitGroup
        {
            typealias Context = BoundingBoxContext;
            typealias ClosestHit = BoundingBoxClosestHit;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = BoundingBoxIntersection;
        }

        struct CurveGroup : rt::IHitGroup
        {
            typealias Context = CurveContext;
            typealias ClosestHit = rt::NoClosestHit<Context>;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct MetadataSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = ::TraceContext;
            typealias HitGroups =
                rt::HitGroupList<TriangleGroup, BoundingBoxGroup, CurveGroup>;
            typealias MissShaders = rt::NoMissShaders;
            typealias CallableShaders = rt::NoCallableShaders;
        }

        rt::AccelerationStructure scene;
        rt::TraceProgramDescriptor<MetadataSchema> program;
        RWStructuredBuffer<float> output;

        [shader("raygeneration")]
        void main()
        {
            rt::RayTraversalDesc desc = {};
            Payload payload = {};
            rt::RayTracer<MetadataSchema> tracer;
            tracer.trace(desc, scene, program, payload);
            output[0] = payload.value;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_METAL;
    target.profile = globalSession->findProfile("metal_3_1");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "structuralMetalMetadata",
        "structural-metal-metadata.slang",
        source,
        diagnostics.writeRef()));
    if (!module && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findEntryPointByName("main", entryPoint.writeRef())));
    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> composite;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        composite.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(composite->link(linkedProgram.writeRef(), diagnostics.writeRef())));

    ComPtr<slang::IBlob> generatedCode;
    auto codeResult =
        linkedProgram->getEntryPointCode(0, 0, generatedCode.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(codeResult) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(codeResult));

    auto schema = linkedProgram->getLayout()->findTraceProgramSchema("MetadataSchema");
    SLANG_CHECK_ABORT(schema != nullptr);
    SLANG_CHECK_ABORT(schema->getName() != nullptr);
    auto payload = schema->getPayload(0);
    SLANG_CHECK_ABORT(payload != nullptr);

    // Activating the bounding-box dispatcher makes the Metal IFT sparse but complete: the plain
    // triangle and curve slots are represented by opaque built-ins at their fixed target indices.
    SLANG_CHECK(payload->getIntersectionFunctionTableSize() == 3);
    SLANG_CHECK(payload->getIntersectionFunctionCount() == 3);
    auto triangleFunction = payload->getIntersectionFunction(0);
    auto boundingBoxFunction = payload->getIntersectionFunction(1);
    auto curveFunction = payload->getIntersectionFunction(2);
    SLANG_CHECK_ABORT(triangleFunction && boundingBoxFunction && curveFunction);
    SLANG_CHECK(triangleFunction->getIntersectionFunctionTableIndex() == 0);
    SLANG_CHECK(
        triangleFunction->getImplementationKind() ==
        SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_OPAQUE_TRIANGLE);
    SLANG_CHECK(triangleFunction->getEntryPointName() == nullptr);
    SLANG_CHECK(boundingBoxFunction->getIntersectionFunctionTableIndex() == 1);
    SLANG_CHECK(
        boundingBoxFunction->getImplementationKind() ==
        SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_EXPORTED_FUNCTION);
    SLANG_CHECK(boundingBoxFunction->getEntryPointName() != nullptr);
    SLANG_CHECK(curveFunction->getIntersectionFunctionTableIndex() == 2);
    SLANG_CHECK(
        curveFunction->getImplementationKind() ==
        SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_OPAQUE_CURVE);
    SLANG_CHECK(curveFunction->getEntryPointName() == nullptr);

    // Source reflection keeps `NoClosestHit` absent. The group-level physical name nevertheless
    // exposes the signature-compatible no-op that must occupy each placeholder slot in the dense
    // Metal closest-hit VFT.
    auto triangleGroup = payload->getHitGroup(0);
    auto boundingBoxGroup = payload->getHitGroup(1);
    auto curveGroup = payload->getHitGroup(2);
    SLANG_CHECK_ABORT(triangleGroup && boundingBoxGroup && curveGroup);
    SLANG_CHECK(triangleGroup->getClosestHit() == nullptr);
    SLANG_CHECK(curveGroup->getClosestHit() == nullptr);
    SLANG_CHECK(triangleGroup->getClosestHitEntryPointName() != nullptr);
    SLANG_CHECK(boundingBoxGroup->getClosestHitEntryPointName() != nullptr);
    SLANG_CHECK(curveGroup->getClosestHitEntryPointName() != nullptr);
    UnownedStringSlice code(
        (const char*)generatedCode->getBufferPointer(),
        (const char*)generatedCode->getBufferPointer() + generatedCode->getBufferSize());
    SLANG_CHECK(
        code.indexOf(UnownedStringSlice(triangleGroup->getClosestHitEntryPointName())) != -1);
    SLANG_CHECK(code.indexOf(UnownedStringSlice(curveGroup->getClosestHitEntryPointName())) != -1);

    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        linkedProgram->getTargetMetadata(0, metadata.writeRef(), diagnostics.writeRef())));
    auto structuralMetadata = static_cast<slang::IStructuralRayTracingMetadata*>(
        metadata->castAs(slang::IStructuralRayTracingMetadata::getTypeGuid()));
    SLANG_CHECK_ABORT(structuralMetadata != nullptr);
    SLANG_CHECK(structuralMetadata->getMetalPayloadInfoCount() == 1);
    slang::StructuralRayTracingMetalPayloadInfo info = {};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(structuralMetadata->getMetalPayloadInfo(0, &info)));
    SLANG_CHECK(UnownedStringSlice(info.schemaName) == schema->getName());
    SLANG_CHECK(info.payloadIndex == 0);
    const auto signature = uint32_t(info.intersectionFunctionSignature);
    SLANG_CHECK((signature & uint32_t(slang::MetalIntersectionFunctionSignature::Instancing)) != 0);
    SLANG_CHECK(
        (signature & uint32_t(slang::MetalIntersectionFunctionSignature::WorldSpaceData)) != 0);
}

SLANG_UNIT_TEST(structuralRayTracingMetalTargetMetadataOperationOrder)
{
    const char* source = R"(
        import slang.raytracing;

        struct PayloadA { uint value; }
        struct PayloadB { float value; }

        struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        struct HitContextA : rt::IHitContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = PayloadA;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = void;
        }

        struct MissContextB : rt::IPayloadContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = PayloadB;
            typealias Record = void;
        }

        struct AnyHitA : rt::IAnyHitShader
        {
            typealias Context = HitContextA;
            void invoke(rt::AnyHitInput<Context> input)
            {
                input.payload.value = input.triangle.frontFacing ? 1 : 0;
            }
        }

        struct HitGroupA : rt::IHitGroup
        {
            typealias Context = HitContextA;
            typealias ClosestHit = rt::NoClosestHit<Context>;
            typealias AnyHit = AnyHitA;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct MissB : rt::IMissShader
        {
            typealias Context = MissContextB;
            void invoke(rt::MissInput<Context> input) { input.payload.value = 2.0; }
        }

        struct Schema : rt::ITraceProgramSchema
        {
            typealias TraceContext = ::TraceContext;
            typealias HitGroups = rt::HitGroupList<HitGroupA>;
            typealias MissShaders = rt::MissShaderList<MissB>;
            typealias CallableShaders = rt::NoCallableShaders;
        }

        rt::AccelerationStructure scene;
        rt::TraceProgramDescriptor<Schema> program;
        RWStructuredBuffer<float2> output;

        [shader("raygeneration")]
        void main()
        {
            rt::RayTraversalDesc desc = {};
            rt::RayTracer<Schema> tracer;
            PayloadA a = {};
            PayloadB b = {};
        #if REVERSE
            tracer.trace(desc, scene, program, b);
            tracer.trace(desc, scene, program, a);
        #else
            tracer.trace(desc, scene, program, a);
            tracer.trace(desc, scene, program, b);
        #endif
            output[0] = float2(a.value, b.value);
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    auto compileSignatures = [&](bool reverse, uint32_t(&outSignatures)[2])
    {
        slang::CompilerOptionEntry options[2] = {};
        options[0].name = slang::CompilerOptionName::ExperimentalFeature;
        options[0].value.kind = slang::CompilerOptionValueKind::Int;
        options[0].value.intValue0 = 1;
        options[1].name = slang::CompilerOptionName::MacroDefine;
        options[1].value.kind = slang::CompilerOptionValueKind::String;
        options[1].value.stringValue0 = "REVERSE";
        options[1].value.stringValue1 = reverse ? "1" : "0";

        slang::TargetDesc target = {};
        target.format = SLANG_METAL;
        target.profile = globalSession->findProfile("metal_3_1");
        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &target;
        sessionDesc.compilerOptionEntryCount = SLANG_COUNT_OF(options);
        sessionDesc.compilerOptionEntries = options;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> diagnostics;
        ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
            reverse ? "structuralMetadataReverse" : "structuralMetadataForward",
            reverse ? "structural-metadata-reverse.slang" : "structural-metadata-forward.slang",
            source,
            diagnostics.writeRef()));
        if (!module && diagnostics)
            fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
        SLANG_CHECK_ABORT(module != nullptr);

        ComPtr<slang::IEntryPoint> entryPoint;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(module->findEntryPointByName("main", entryPoint.writeRef())));
        slang::IComponentType* components[] = {module, entryPoint};
        ComPtr<slang::IComponentType> composite;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
            components,
            SLANG_COUNT_OF(components),
            composite.writeRef(),
            diagnostics.writeRef())));
        ComPtr<slang::IComponentType> linkedProgram;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(composite->link(linkedProgram.writeRef(), diagnostics.writeRef())));

        ComPtr<slang::IBlob> generatedCode;
        auto codeResult = linkedProgram->getEntryPointCode(
            0,
            0,
            generatedCode.writeRef(),
            diagnostics.writeRef());
        if (SLANG_FAILED(codeResult) && diagnostics)
            fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(codeResult));

        auto schema = linkedProgram->getLayout()->findTraceProgramSchema("Schema");
        SLANG_CHECK_ABORT(schema != nullptr);
        SLANG_CHECK(schema->getPayloadCount() == 2);

        ComPtr<slang::IMetadata> metadata;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            linkedProgram->getTargetMetadata(0, metadata.writeRef(), diagnostics.writeRef())));
        auto structuralMetadata = static_cast<slang::IStructuralRayTracingMetadata*>(
            metadata->castAs(slang::IStructuralRayTracingMetadata::getTypeGuid()));
        SLANG_CHECK_ABORT(structuralMetadata != nullptr);
        SLANG_CHECK_ABORT(structuralMetadata->getMetalPayloadInfoCount() == 2);
        for (uint32_t i = 0; i < 2; ++i)
        {
            slang::StructuralRayTracingMetalPayloadInfo info = {};
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(structuralMetadata->getMetalPayloadInfo(i, &info)));
            SLANG_CHECK_ABORT(UnownedStringSlice(info.schemaName) == schema->getName());
            SLANG_CHECK_ABORT(info.payloadIndex < 2);
            outSignatures[info.payloadIndex] = uint32_t(info.intersectionFunctionSignature);
        }
    };

    uint32_t forwardSignatures[2] = {};
    uint32_t reverseSignatures[2] = {};
    compileSignatures(false, forwardSignatures);
    compileSignatures(true, reverseSignatures);
    SLANG_CHECK(forwardSignatures[0] == reverseSignatures[0]);
    SLANG_CHECK(forwardSignatures[1] == reverseSignatures[1]);
    SLANG_CHECK(
        (forwardSignatures[0] &
         uint32_t(slang::MetalIntersectionFunctionSignature::TriangleData)) != 0);
    SLANG_CHECK(
        (forwardSignatures[1] &
         uint32_t(slang::MetalIntersectionFunctionSignature::TriangleData)) == 0);
}
