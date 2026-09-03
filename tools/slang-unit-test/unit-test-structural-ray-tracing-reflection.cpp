#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>

using namespace Slang;

SLANG_UNIT_TEST(structuralRayTracingReflection)
{
    const char* source = R"(
        import slang.raytracing;

        struct Payload { float value; }
        struct HitRecord { float value; }
        struct MissRecord { float value; }
        struct CallableRecord { float value; }
        struct CallableData { float value; }

        typealias TracePayload = Payload;
        typealias HitRecordType = HitRecord;
        typealias MissRecordType = MissRecord;
        typealias CallableRecordType = CallableRecord;
        typealias CallableDataType = CallableData;

        struct TraceContext : rt::ITraceContext
        {
            typealias Payload = TracePayload;
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        typealias TraceContextType = TraceContext;

        struct HitContext : rt::IHitContext
        {
            typealias TraceContext = TraceContextType;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = HitRecordType;
        }

        struct MissContext : rt::IMissGroupContext
        {
            typealias TraceContext = TraceContextType;
            typealias Record = MissRecordType;
        }

        struct CallableContext : rt::ICallableGroupContext
        {
            typealias TraceContext = TraceContextType;
            typealias CallableData = CallableDataType;
            typealias Record = CallableRecordType;
        }

        struct ClosestHit : rt::IClosestHitShader<HitContext>
        {
            void invoke(rt::ClosestHitInput<HitContext> input) {}
        }

        struct AnyHit : rt::IAnyHitShader<HitContext>
        {
            void invoke(rt::AnyHitInput<HitContext> input) {}
        }

        struct Miss : rt::IMissShader<MissContext>
        {
            void invoke(rt::MissInput<MissContext> input) {}
        }

        struct Callable : rt::ICallableShader<CallableContext>
        {
            void invoke(rt::CallableInput<CallableContext> input) {}
        }

        typealias ClosestHitStage = ClosestHit;
        typealias AnyHitStage = AnyHit;
        typealias MissStage = Miss;
        typealias CallableStage = Callable;

        struct ReflectedHitGroup : rt::IHitGroup
        {
            typealias Slot = rt::HitGroupSlot<4>;
            typealias Context = HitContext;
            typealias ClosestHit = ClosestHitStage;
            typealias AnyHit = AnyHitStage;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct ReflectedMissGroup : rt::IMissGroup
        {
            typealias Slot = rt::MissSlot<2>;
            typealias Context = MissContext;
            typealias Miss = MissStage;
        }

        struct ReflectedCallableGroup : rt::ICallableGroup
        {
            typealias Slot = rt::CallableSlot<7>;
            typealias Context = CallableContext;
            typealias Callable = CallableStage;
        }

        struct ReflectedLayout : rt::ITraceProgramLayout
        {
            typealias TraceContext = TraceContextType;
            typealias HitGroups = rt::HitGroupList<TraceContextType, ReflectedHitGroup>;
            typealias MissGroups = rt::MissGroupList<TraceContextType, ReflectedMissGroup>;
            typealias CallableGroups =
                rt::CallableGroupList<TraceContextType, ReflectedCallableGroup>;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_SPIRV;
    target.profile = globalSession->findProfile("spirv_1_5");
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
    auto layout = program->findTraceProgramLayout("ReflectedLayout");
    SLANG_CHECK(layout != nullptr);
    SLANG_CHECK(UnownedStringSlice(layout->getType()->getName()) == "ReflectedLayout");
    SLANG_CHECK(UnownedStringSlice(layout->getTraceContextType()->getName()) == "TraceContext");

    SLANG_CHECK(layout->getHitGroupCount() == 1);
    auto hitGroup = layout->getHitGroup(0);
    SLANG_CHECK(hitGroup != nullptr);
    SLANG_CHECK(hitGroup->getSlot() == 4);
    SLANG_CHECK(UnownedStringSlice(hitGroup->getRecordType()->getName()) == "HitRecord");
    SLANG_CHECK(UnownedStringSlice(hitGroup->getPrimitiveType()->getName()) == "TrianglePrimitive");
    SLANG_CHECK(
        UnownedStringSlice(hitGroup->getIntersectionAttributesType()->getName()) == "TriangleData");
    SLANG_CHECK(hitGroup->getClosestHit()->getStage() == SLANG_STAGE_CLOSEST_HIT);
    SLANG_CHECK(UnownedStringSlice(hitGroup->getClosestHit()->getEntryPointName()) == "ClosestHit");
    SLANG_CHECK(hitGroup->getAnyHit()->getStage() == SLANG_STAGE_ANY_HIT);
    SLANG_CHECK(hitGroup->getIntersection() == nullptr);

    SLANG_CHECK(layout->getMissGroupCount() == 1);
    auto missGroup = layout->getMissGroup(0);
    SLANG_CHECK(missGroup != nullptr);
    SLANG_CHECK(missGroup->getSlot() == 2);
    SLANG_CHECK(UnownedStringSlice(missGroup->getRecordType()->getName()) == "MissRecord");
    SLANG_CHECK(missGroup->getMiss()->getStage() == SLANG_STAGE_MISS);
    SLANG_CHECK(UnownedStringSlice(missGroup->getMiss()->getEntryPointName()) == "Miss");

    SLANG_CHECK(layout->getCallableGroupCount() == 1);
    auto callableGroup = layout->getCallableGroup(0);
    SLANG_CHECK(callableGroup != nullptr);
    SLANG_CHECK(callableGroup->getSlot() == 7);
    SLANG_CHECK(UnownedStringSlice(callableGroup->getRecordType()->getName()) == "CallableRecord");
    SLANG_CHECK(UnownedStringSlice(callableGroup->getDataType()->getName()) == "CallableData");
    SLANG_CHECK(callableGroup->getCallable()->getStage() == SLANG_STAGE_CALLABLE);
    SLANG_CHECK(
        UnownedStringSlice(callableGroup->getCallable()->getEntryPointName()) == "Callable");

    SLANG_CHECK(program->findTraceProgramLayout("HitContext") == nullptr);
}

SLANG_UNIT_TEST(structuralRayTracingEntryPointRename)
{
    const char* expectedClosestHitName =
        "__slang_structural_rt_"
        "53746167654e616d6573706163652e54657374436c6f73657374486974";
    const char* expectedMissName =
        "__slang_structural_rt_53746167654e616d6573706163652e546573744d697373";
    const char* expectedGenericMissName = "__slang_structural_rt_"
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
                typealias Payload = StageNamespace.Payload;
                typealias AccelerationStructure = rt::AccelerationStructure;
                typealias Motion = rt::NoMotion;
            }

            struct MissContext : rt::IMissGroupContext
            {
                typealias TraceContext = StageNamespace.TraceContext;
                typealias Record = StageNamespace.Record;
            }

            struct HitContext : rt::IHitContext
            {
                typealias TraceContext = StageNamespace::TraceContext;
                typealias Primitive = rt::TrianglePrimitive;
                typealias Record = StageNamespace::Record;
            }

            struct TestClosestHit : rt::IClosestHitShader<HitContext>
            {
                void invoke(rt::ClosestHitInput<HitContext> input)
                {
                    input.payload.value = input.triangle.barycentricCoord.x;
                }
            }

            struct HitGroup : rt::IHitGroup
            {
                typealias Slot = rt::HitGroupSlot<0>;
                typealias Context = HitContext;
                typealias ClosestHit = TestClosestHit;
                typealias AnyHit = rt::NoAnyHit<HitContext>;
                typealias Intersection = rt::NoIntersection<HitContext>;
            }

            struct TestMiss : rt::IMissShader<MissContext>
            {
                void invoke(rt::MissInput<MissContext> input)
                {
                    input.payload.value = 1.0f;
                }
            }

            struct MissGroup : rt::IMissGroup
            {
                typealias Slot = rt::MissSlot<0>;
                typealias Context = MissContext;
                typealias Miss = TestMiss;
            }

            struct ProgramLayout : rt::ITraceProgramLayout
            {
                typealias TraceContext = StageNamespace::TraceContext;
                typealias HitGroups =
                    rt::HitGroupList<StageNamespace::TraceContext, StageNamespace::HitGroup>;
                typealias MissGroups =
                    rt::MissGroupList<StageNamespace::TraceContext, StageNamespace::MissGroup>;
                typealias CallableGroups = rt::NoCallableGroups<StageNamespace::TraceContext>;
            }

            struct GenericMiss<T> : rt::IMissShader<MissContext>
            {
                void invoke(rt::MissInput<MissContext> input)
                {
                    input.payload.value = 4.0f;
                }
            }

            struct GenericMissGroup<T> : rt::IMissGroup
            {
                typealias Slot = rt::MissSlot<0>;
                typealias Context = MissContext;
                typealias Miss = GenericMiss<T>;
            }

            struct GenericProgramLayout<T> : rt::ITraceProgramLayout
            {
                typealias TraceContext = StageNamespace::TraceContext;
                typealias HitGroups = rt::NoHitGroups<StageNamespace::TraceContext>;
                typealias MissGroups =
                    rt::MissGroupList<StageNamespace::TraceContext, GenericMissGroup<T>>;
                typealias CallableGroups = rt::NoCallableGroups<StageNamespace::TraceContext>;
            }
        }

        struct main : rt::IMissShader<StageNamespace::MissContext>
        {
            void invoke(rt::MissInput<StageNamespace::MissContext> input)
            {
                input.payload.value = 2.0f;
            }
        }

        struct MainMissGroup : rt::IMissGroup
        {
            typealias Slot = rt::MissSlot<0>;
            typealias Context = StageNamespace::MissContext;
            typealias Miss = main;
        }

        struct MainProgramLayout : rt::ITraceProgramLayout
        {
            typealias TraceContext = StageNamespace::TraceContext;
            typealias HitGroups =
                rt::HitGroupList<StageNamespace::TraceContext, StageNamespace::HitGroup>;
            typealias MissGroups = rt::MissGroupList<StageNamespace::TraceContext, MainMissGroup>;
            typealias CallableGroups = rt::NoCallableGroups<StageNamespace::TraceContext>;
        }

        // This legal source identifier is exactly the encoded spelling of
        // `StageNamespace.TestMiss`. It must itself be encoded to keep the mapping injective.
        struct __slang_structural_rt_53746167654e616d6573706163652e546573744d697373
            : rt::IMissShader<StageNamespace::MissContext>
        {
            void invoke(rt::MissInput<StageNamespace::MissContext> input)
            {
                input.payload.value = 3.0f;
            }
        }

        struct ReservedPrefixMissGroup : rt::IMissGroup
        {
            typealias Slot = rt::MissSlot<0>;
            typealias Context = StageNamespace::MissContext;
            typealias Miss =
                __slang_structural_rt_53746167654e616d6573706163652e546573744d697373;
        }

        struct ReservedPrefixProgramLayout : rt::ITraceProgramLayout
        {
            typealias TraceContext = StageNamespace::TraceContext;
            typealias HitGroups =
                rt::HitGroupList<StageNamespace::TraceContext, StageNamespace::HitGroup>;
            typealias MissGroups =
                rt::MissGroupList<StageNamespace::TraceContext, ReservedPrefixMissGroup>;
            typealias CallableGroups = rt::NoCallableGroups<StageNamespace::TraceContext>;
        }

        rt::AccelerationStructure scene;
        rt::TraceProgramDescriptor<StageNamespace::ProgramLayout> traceProgram;
        rt::TraceProgramDescriptor<StageNamespace::GenericProgramLayout<uint>> genericTraceProgram;

        void traceHelper(inout StageNamespace::Payload payload)
        {
            rt::RayTraversalDesc desc = {};
            desc.ray.direction = float3(0.0f, 0.0f, 1.0f);
            desc.ray.tMax = 1.0f;
            desc.instanceMask = 0xff;
            rt::RayTracer<StageNamespace::ProgramLayout> tracer;
            tracer.trace(desc, scene, traceProgram, payload);
        }

        void traceGenericHelper(inout StageNamespace::Payload payload)
        {
            rt::RayTraversalDesc desc = {};
            desc.ray.direction = float3(0.0f, 0.0f, 1.0f);
            desc.ray.tMax = 1.0f;
            desc.instanceMask = 0xff;
            rt::RayTracer<StageNamespace::GenericProgramLayout<uint>> tracer;
            tracer.trace(desc, scene, genericTraceProgram, payload);
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
    auto reflectedLayout = reflectedProgram->findTraceProgramLayout("StageNamespace.ProgramLayout");
    SLANG_CHECK_ABORT(reflectedLayout != nullptr);
    auto reflectedClosestHit = reflectedLayout->getHitGroup(0)->getClosestHit();
    auto reflectedMiss = reflectedLayout->getMissGroup(0)->getMiss();
    SLANG_CHECK_ABORT(reflectedClosestHit != nullptr);
    SLANG_CHECK_ABORT(reflectedMiss != nullptr);
    String closestHitEntryPointName(reflectedClosestHit->getEntryPointName());
    String missEntryPointName(reflectedMiss->getEntryPointName());
    SLANG_CHECK(closestHitEntryPointName == expectedClosestHitName);
    SLANG_CHECK(missEntryPointName == expectedMissName);
    String expectedMissSymbol = String("__miss__") + missEntryPointName;
    String expectedClosestHitSymbol = String("__closesthit__") + closestHitEntryPointName;

    auto reflectedMainLayout = reflectedProgram->findTraceProgramLayout("MainProgramLayout");
    SLANG_CHECK_ABORT(reflectedMainLayout != nullptr);
    auto reflectedMain = reflectedMainLayout->getMissGroup(0)->getMiss();
    SLANG_CHECK_ABORT(reflectedMain != nullptr);
    String mainEntryPointName(reflectedMain->getEntryPointName());
    SLANG_CHECK(mainEntryPointName == expectedMainName);

    auto reflectedReservedLayout =
        reflectedProgram->findTraceProgramLayout("ReservedPrefixProgramLayout");
    SLANG_CHECK_ABORT(reflectedReservedLayout != nullptr);
    auto reflectedReserved = reflectedReservedLayout->getMissGroup(0)->getMiss();
    SLANG_CHECK_ABORT(reflectedReserved != nullptr);
    String reservedEntryPointName(reflectedReserved->getEntryPointName());
    SLANG_CHECK(reservedEntryPointName != expectedMissName);
    SLANG_CHECK(reservedEntryPointName.getUnownedSlice().startsWith("__slang_structural_rt_"));

    auto reflectedGenericLayout =
        reflectedProgram->findTraceProgramLayout("StageNamespace.GenericProgramLayout<uint>");
    SLANG_CHECK_ABORT(reflectedGenericLayout != nullptr);
    auto reflectedGenericMiss = reflectedGenericLayout->getMissGroup(0)->getMiss();
    SLANG_CHECK_ABORT(reflectedGenericMiss != nullptr);
    String genericMissEntryPointName(reflectedGenericMiss->getEntryPointName());
    SLANG_CHECK(genericMissEntryPointName == expectedGenericMissName);

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
    // must synthesize the same names that the program-layout reflection above advertised.
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
    SLANG_CHECK(genericGeneratedCode.indexOf(expectedGenericMissSymbol.getUnownedSlice()) != -1);
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
}
