// unit-test-translation-unit-import.cpp

#include "core/slang-io.h"
#include "core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "slang/slang-entry-point.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test that the IModule::findAndCheckEntryPoint API supports discovering
// entrypoints without a [shader] attribute.

SLANG_UNIT_TEST(findAndCheckEntryPoint)
{
    // Source for a module that contains an undecorated entrypoint.
    const char* userSourceBody = R"(
        float4 fragMain(float4 pos:SV_Position) : SV_Target
        {
            return pos;
        }
        )";

    auto moduleName = "moduleG" + String(Process::getId());
    String userSource = "import " + moduleName + ";\n" + userSourceBody;
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "fragMain",
        SLANG_STAGE_FRAGMENT,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(linkedProgram != nullptr);

    ComPtr<slang::IBlob> code;
    linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(code != nullptr);
    SLANG_CHECK(code->getBufferSize() != 0);
}

// Loading the structural ray-tracing module elsewhere in the session must not make an unrelated
// ordinary struct an entry point. Lookup finds Plain in the requested module, but it must still
// reject that struct because Plain has no executable stage conformance.
SLANG_UNIT_TEST(structuralRayTracingEntryPointLookupIsModuleLocal)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalFeature = {};
    experimentalFeature.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalFeature.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalFeature.value.intValue0 = 1;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalFeature;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto importingModule = session->loadModuleFromSourceString(
        "StructuralImporter",
        "StructuralImporter.slang",
        "import slang.raytracing;",
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(importingModule != nullptr);

    diagnostics.setNull();
    auto unrelatedModule = session->loadModuleFromSourceString(
        "UnrelatedModule",
        "UnrelatedModule.slang",
        "struct Plain {}",
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(unrelatedModule != nullptr);

    diagnostics.setNull();
    ComPtr<slang::IEntryPoint> entryPoint;
    auto result = unrelatedModule->findAndCheckEntryPoint(
        "Plain",
        SLANG_STAGE_MISS,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(entryPoint == nullptr);
    SLANG_CHECK_ABORT(diagnostics != nullptr);

    auto diagnosticText = UnownedStringSlice(
        (const char*)diagnostics->getBufferPointer(),
        diagnostics->getBufferSize());
    SLANG_CHECK(
        diagnosticText.indexOf(toSlice("struct 'Plain' does not implement an executable structural "
                                       "ray-tracing stage interface")) != -1);
}

// Selecting two stages implemented by the same struct must give them distinct target names.
// Qualified names should remain readable, with dots replaced by underscores. Generic types use
// ordinary mangling to retain their arguments. Check the overrides supplied to IR linking.
SLANG_UNIT_TEST(structuralRayTracingEntryPointNamesIncludeStage)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalFeature = {};
    experimentalFeature.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalFeature.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalFeature.value.intValue0 = 1;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalFeature;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    const char* sourcePath =
        "tests/ray-tracing-2/frontend/entry-point/struct-entry-ambiguous.slang";
    String source;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(sourcePath, source)));
    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "MultiStageModule",
        sourcePath,
        source.getBuffer(),
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> closestHit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "MultiStage",
        SLANG_STAGE_CLOSEST_HIT,
        closestHit.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IEntryPoint> anyHit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "MultiStage",
        SLANG_STAGE_ANY_HIT,
        anyHit.writeRef(),
        diagnostics.writeRef())));

    // The public selection name stays the same; only the target names distinguish the stages.
    SLANG_CHECK(static_cast<EntryPoint*>(closestHit.get())->getName()->text == "MultiStage");
    SLANG_CHECK(static_cast<EntryPoint*>(anyHit.get())->getName()->text == "MultiStage");

    slang::IComponentType* components[] = {module, closestHit.get(), anyHit.get()};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        program.writeRef(),
        diagnostics.writeRef())));
    auto internalProgram = static_cast<ComponentType*>(program.get());
    SLANG_CHECK(internalProgram->getEntryPointCount() == 2);
    SLANG_CHECK(internalProgram->getEntryPointNameOverride(0) == "MultiStage_closesthit");
    SLANG_CHECK(internalProgram->getEntryPointNameOverride(1) == "MultiStage_anyhit");

    const char* qualifiedSourcePath = "tests/ray-tracing-2/frontend/entry-point/struct-entry.slang";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(qualifiedSourcePath, source)));
    auto qualifiedModule = session->loadModuleFromSourceString(
        "QualifiedStageModule",
        qualifiedSourcePath,
        source.getBuffer(),
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(qualifiedModule != nullptr);
    ComPtr<slang::IEntryPoint> qualifiedEntry;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(qualifiedModule->findAndCheckEntryPoint(
        "Stages.main",
        SLANG_STAGE_CLOSEST_HIT,
        qualifiedEntry.writeRef(),
        diagnostics.writeRef())));
    auto internalEntry = static_cast<EntryPoint*>(qualifiedEntry.get());
    SLANG_CHECK(internalEntry->getName()->text == "Stages.main");
    ComponentType* qualifiedComponent = internalEntry;
    SLANG_CHECK(qualifiedComponent->getEntryPointNameOverride(0) == "Stages_main_closesthit");

    const char* genericSourcePath =
        "tests/ray-tracing-2/frontend/entry-point/struct-entry-generic.slang";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(genericSourcePath, source)));
    auto genericModule = session->loadModuleFromSourceString(
        "GenericStageModule",
        genericSourcePath,
        source.getBuffer(),
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(genericModule != nullptr);

    // The two specializations share M's declaration but retain P or R in its mangled name.
    const char* genericEntryNames[] = {"M<P>", "M<R>"};
    const char* genericTargetNames[] = {
        "_ST18GenericStageModule1MG118GenericStageModule1P_miss",
        "_ST18GenericStageModule1MG118GenericStageModule1R_miss"};
    for (Index i = 0; i < SLANG_COUNT_OF(genericEntryNames); ++i)
    {
        ComPtr<slang::IEntryPoint> genericEntry;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(genericModule->findAndCheckEntryPoint(
            genericEntryNames[i],
            SLANG_STAGE_MISS,
            genericEntry.writeRef(),
            diagnostics.writeRef())));
        auto internalGenericEntry = static_cast<EntryPoint*>(genericEntry.get());
        SLANG_CHECK(internalGenericEntry->getName()->text == genericEntryNames[i]);
        ComponentType* genericComponent = internalGenericEntry;
        SLANG_CHECK(genericComponent->getEntryPointNameOverride(0) == genericTargetNames[i]);
    }
}

// Structural and native selections can refer to the same function declaration. The input rule
// belongs to the selected EntryPoint, so a previous lookup must not change the next lookup's
// result.
SLANG_UNIT_TEST(structuralRayTracingInputValidationDependsOnEntrySelection)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalFeature = {};
    experimentalFeature.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalFeature.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalFeature.value.intValue0 = 1;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalFeature;

    const char* sourcePath =
        "tests/ray-tracing-2/compatibility/mixed-api/native-entry-input-signature.slang";
    String source;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(sourcePath, source)));

    const char* moduleNames[] = {"StructuralSelectionFirst", "NativeSelectionFirst"};
    for (Index order = 0; order < SLANG_COUNT_OF(moduleNames); ++order)
    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<slang::IBlob> diagnostics;
        auto module = session->loadModuleFromSourceString(
            moduleNames[order],
            sourcePath,
            source.getBuffer(),
            diagnostics.writeRef());
        SLANG_CHECK_ABORT(module != nullptr);

        for (Index selection = 0; selection < 2; ++selection)
        {
            const bool structural = selection == order;
            diagnostics.setNull();
            ComPtr<slang::IEntryPoint> entryPoint;
            auto result = module->findAndCheckEntryPoint(
                structural ? "Stage" : "Stage.invoke",
                SLANG_STAGE_CLOSEST_HIT,
                entryPoint.writeRef(),
                diagnostics.writeRef());
            if (structural)
            {
                SLANG_CHECK(SLANG_SUCCEEDED(result));
                SLANG_CHECK(entryPoint != nullptr);
            }
            else
            {
                SLANG_CHECK(SLANG_FAILED(result));
                SLANG_CHECK(entryPoint == nullptr);
                SLANG_CHECK_ABORT(diagnostics != nullptr);
                auto diagnosticText = UnownedStringSlice(
                    (const char*)diagnostics->getBufferPointer(),
                    diagnostics->getBufferSize());
                SLANG_CHECK(
                    diagnosticText.indexOf(
                        toSlice("cannot be used in a native entry point signature")) != -1);
            }
        }
    }
}

// This test reproduces issue #6507, where it was noticed that compilation of
// tests/compute/simple.slang for PTX target generates invalid code.
// TODO: Remove this when issue #4760 is resolved, because at that point
// tests/compute/simple.slang should cover the same issue.
SLANG_UNIT_TEST(cudaCodeGenBug)
{
    // We need the CUDA backend for this test
    if (!SLANG_SUCCEEDED(
            unitTestContext->slangGlobalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        SLANG_IGNORE_TEST;
    }

    // Source for a module that contains an undecorated entrypoint.
    const char* userSourceBody = R"(
        RWStructuredBuffer<float> outputBuffer;

        [numthreads(4, 1, 1)]
        void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            outputBuffer[dispatchThreadID.x] = float(dispatchThreadID.x);
        }
        )";

    auto moduleName = "moduleG" + String(Process::getId());
    String userSource = "import " + moduleName + ";\n" + userSourceBody;
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_PTX;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(linkedProgram != nullptr);

    ComPtr<slang::IBlob> code;
    auto res = linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(res == SLANG_OK);
    SLANG_CHECK(code != nullptr && code->getBufferSize() != 0);
}
