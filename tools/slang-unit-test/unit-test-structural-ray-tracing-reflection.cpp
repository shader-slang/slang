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
