// unit-test-translation-unit-import.cpp

#include "core/slang-io.h"
#include "core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
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
        "import slang.raytracing.portable;",
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

// Read entry-point names and signatures through the public reflection API. These tests live in
// a separate shared library, so they cannot depend on private compiler classes or their RTTI.
static slang::EntryPointReflection* _getEntryPointReflection(
    slang::IComponentType* program,
    SlangUInt index = 0)
{
    auto layout = program->getLayout(0);
    SLANG_CHECK_ABORT(layout != nullptr);
    auto entryPoint = layout->getEntryPointByIndex(index);
    SLANG_CHECK_ABORT(entryPoint != nullptr);
    return entryPoint;
}

// Returns a type reflection's fully-qualified name as a String, for comparing the concrete
// contract types of two entry-point selections.
static String _getTypeFullName(slang::TypeReflection* type)
{
    ComPtr<ISlangBlob> blob;
    type->getFullName(blob.writeRef());
    return String((const char*)blob->getBufferPointer());
}

// Renders a diagnostics blob as a String, empty for a null blob.
static String _blobToString(slang::IBlob* blob)
{
    if (!blob)
        return String();
    return String(UnownedStringSlice((const char*)blob->getBufferPointer(), blob->getBufferSize()));
}

// Supplying a structural stage's generic arguments separately through specialize() must select
// the same specialization as binding the arguments in the entry name: the same target name and
// the same concrete contract types. Two specializations of one stage stay independent, and a
// wrong argument count keeps the ordinary specialization diagnostic.
SLANG_UNIT_TEST(structuralRayTracingEntryPointSeparateSpecialization)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalFeature = {};
    experimentalFeature.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalFeature.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalFeature.value.intValue0 = 1;
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalFeature;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    const char* sourcePath = "tests/ray-tracing-2/frontend/entry-point/struct-entry-generic.slang";
    String source;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(sourcePath, source)));
    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "SeparateSpecializationModule",
        sourcePath,
        source.getBuffer(),
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    auto pType = module->getLayout()->findTypeByName("P");
    auto rType = module->getLayout()->findTypeByName("R");

    // Direct specialized-name selection provides the reference target names and input types.
    ComPtr<slang::IEntryPoint> directPEntry;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "M<P>",
        SLANG_STAGE_MISS,
        directPEntry.writeRef(),
        diagnostics.writeRef())));
    auto directP = _getEntryPointReflection(directPEntry);

    // The bare stage name selects the generic stage like a bare generic function name does.
    ComPtr<slang::IEntryPoint> bareMEntry;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "M",
        SLANG_STAGE_MISS,
        bareMEntry.writeRef(),
        diagnostics.writeRef())));

    slang::SpecializationArg pArgs[] = {slang::SpecializationArg::fromType(pType)};
    ComPtr<slang::IComponentType> specializedP;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(bareMEntry->specialize(pArgs, 1, specializedP.writeRef(), nullptr)));
    SLANG_CHECK_ABORT(specializedP != nullptr);
    auto specializedPReflection = _getEntryPointReflection(specializedP);
    // The public name stays the bare selection name even though the target name below becomes
    // the specialized one; the two-name split is part of the tested contract.
    SLANG_CHECK(String(specializedPReflection->getName()) == "M");
    SLANG_CHECK(
        String(specializedPReflection->getNameOverride()) == String(directP->getNameOverride()));

    slang::SpecializationArg rArgs[] = {slang::SpecializationArg::fromType(rType)};
    ComPtr<slang::IComponentType> specializedR;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(bareMEntry->specialize(rArgs, 1, specializedR.writeRef(), nullptr)));
    SLANG_CHECK_ABORT(specializedR != nullptr);
    auto specializedRReflection = _getEntryPointReflection(specializedR);
    SLANG_CHECK(
        String(specializedRReflection->getNameOverride()) !=
        String(specializedPReflection->getNameOverride()));

    // The specialized invoke receives the same concrete input type as direct-name selection,
    // and a different specialization receives a different one.
    auto directPInputType =
        _getTypeFullName(directP->getFunction()->getParameterByIndex(0)->getType());
    auto specializedPInputType =
        _getTypeFullName(specializedPReflection->getFunction()->getParameterByIndex(0)->getType());
    auto specializedRInputType =
        _getTypeFullName(specializedRReflection->getFunction()->getParameterByIndex(0)->getType());
    SLANG_CHECK(specializedPInputType == directPInputType);
    SLANG_CHECK(specializedRInputType != directPInputType);

    // A wrong argument count keeps the ordinary specialization diagnostic.
    slang::SpecializationArg twoArgs[] = {
        slang::SpecializationArg::fromType(pType),
        slang::SpecializationArg::fromType(rType)};
    ComPtr<slang::IComponentType> overApplied;
    auto result =
        bareMEntry->specialize(twoArgs, 2, overApplied.writeRef(), diagnostics.writeRef());
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(overApplied == nullptr);
    SLANG_CHECK(_blobToString(diagnostics).indexOf(toSlice("specialization arguments")) != -1);

    // An explicit rename takes precedence over the specialization's derived target name, in
    // either order: renaming the bare entry before specializing, and renaming the specialized
    // component afterwards.
    ComPtr<slang::IComponentType> renamedBare;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(bareMEntry->renameEntryPoint("customMissP", renamedBare.writeRef())));
    ComPtr<slang::IComponentType> specializedRenamedBare;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        renamedBare->specialize(pArgs, 1, specializedRenamedBare.writeRef(), nullptr)));
    SLANG_CHECK_ABORT(specializedRenamedBare != nullptr);
    SLANG_CHECK(
        String(_getEntryPointReflection(specializedRenamedBare)->getNameOverride()) ==
        "customMissP");

    ComPtr<slang::IComponentType> renamedSpecialized;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        specializedP->renameEntryPoint("customMissQ", renamedSpecialized.writeRef())));
    SLANG_CHECK_ABORT(renamedSpecialized != nullptr);
    SLANG_CHECK(
        String(_getEntryPointReflection(renamedSpecialized)->getNameOverride()) == "customMissQ");

    // Both specializations of one stage coexist in one composed program, each carrying its own
    // target name; that distinctness is what lets one linked program hold them both.
    slang::IComponentType* specializations[] = {module, specializedP.get(), specializedR.get()};
    ComPtr<slang::IComponentType> composedProgram;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        specializations,
        SLANG_COUNT_OF(specializations),
        composedProgram.writeRef(),
        diagnostics.writeRef())));
    SLANG_CHECK_ABORT(composedProgram != nullptr);
    auto composedLayout = composedProgram->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(composedLayout != nullptr);
    SLANG_CHECK(composedLayout->getEntryPointCount() == 2);
    SLANG_CHECK(
        String(_getEntryPointReflection(composedProgram, 0)->getNameOverride()) ==
        String(specializedPReflection->getNameOverride()));
    SLANG_CHECK(
        String(_getEntryPointReflection(composedProgram, 1)->getNameOverride()) ==
        String(specializedRReflection->getNameOverride()));

    // An unspecialized structural entry in a composed program must survive layout generation:
    // IR-for-layout skips it like an ordinary unspecialized generic rather than lowering its
    // free-parameter contract types. This pins the classification the layout skip relies on.
    SLANG_CHECK(bareMEntry->getSpecializationParamCount() == 1);
    slang::IComponentType* unspecializedComponents[] = {module, bareMEntry.get()};
    ComPtr<slang::IComponentType> unspecializedProgram;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        unspecializedComponents,
        SLANG_COUNT_OF(unspecializedComponents),
        unspecializedProgram.writeRef(),
        diagnostics.writeRef())));
    SLANG_CHECK_ABORT(unspecializedProgram != nullptr);
    auto unspecializedLayout = unspecializedProgram->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(unspecializedLayout != nullptr);
    SLANG_CHECK(unspecializedLayout->getEntryPointCount() == 1);
}


// Selecting two stages implemented by the same struct must give them distinct target names.
// Qualified names should remain readable, with dots replaced by underscores. Generic types use
// ordinary mangling to retain their arguments. Reflection exposes the overrides used by IR linking.
SLANG_UNIT_TEST(structuralRayTracingEntryPointNamesIncludeStage)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalFeature = {};
    experimentalFeature.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalFeature.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalFeature.value.intValue0 = 1;
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
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
    SLANG_CHECK(String(_getEntryPointReflection(closestHit)->getName()) == "MultiStage");
    SLANG_CHECK(String(_getEntryPointReflection(anyHit)->getName()) == "MultiStage");

    slang::IComponentType* components[] = {module, closestHit.get(), anyHit.get()};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        program.writeRef(),
        diagnostics.writeRef())));
    auto programLayout = program->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(programLayout != nullptr);
    SLANG_CHECK(programLayout->getEntryPointCount() == 2);
    SLANG_CHECK(
        String(_getEntryPointReflection(program, 0)->getNameOverride()) == "MultiStage_closesthit");
    SLANG_CHECK(
        String(_getEntryPointReflection(program, 1)->getNameOverride()) == "MultiStage_anyhit");

    // Selecting the overloaded method uses the stage conformance to choose the same function.
    slang::IEntryPoint* stageEntries[] = {closestHit.get(), anyHit.get()};
    SlangStage stages[] = {SLANG_STAGE_CLOSEST_HIT, SLANG_STAGE_ANY_HIT};
    for (Index i = 0; i < SLANG_COUNT_OF(stageEntries); ++i)
    {
        ComPtr<slang::IEntryPoint> methodEntry;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
            "MultiStage.invoke",
            stages[i],
            methodEntry.writeRef(),
            diagnostics.writeRef())));
        auto methodReflection = _getEntryPointReflection(methodEntry);
        SLANG_CHECK(methodReflection->getStage() == stages[i]);
        SLANG_CHECK(String(methodReflection->getName()) == "MultiStage.invoke");
        SLANG_CHECK(methodReflection->getFunction() == stageEntries[i]->getFunctionReflection());
        SLANG_CHECK(
            String(methodReflection->getNameOverride()) ==
            _getEntryPointReflection(program, SlangUInt(i))->getNameOverride());
    }

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
    auto qualifiedReflection = _getEntryPointReflection(qualifiedEntry);
    SLANG_CHECK(String(qualifiedReflection->getName()) == "Stages.main");
    SLANG_CHECK(String(qualifiedReflection->getNameOverride()) == "Stages_main_closesthit");

    ComPtr<slang::IEntryPoint> qualifiedMethodEntry;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(qualifiedModule->findAndCheckEntryPoint(
        "Stages.main.invoke",
        SLANG_STAGE_CLOSEST_HIT,
        qualifiedMethodEntry.writeRef(),
        diagnostics.writeRef())));
    auto qualifiedMethodReflection = _getEntryPointReflection(qualifiedMethodEntry);
    SLANG_CHECK(qualifiedMethodReflection->getStage() == SLANG_STAGE_CLOSEST_HIT);
    SLANG_CHECK(String(qualifiedMethodReflection->getName()) == "Stages.main.invoke");
    SLANG_CHECK(qualifiedMethodReflection->getFunction() == qualifiedReflection->getFunction());
    SLANG_CHECK(String(qualifiedMethodReflection->getNameOverride()) == "Stages_main_closesthit");

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
        auto genericReflection = _getEntryPointReflection(genericEntry);
        SLANG_CHECK(String(genericReflection->getName()) == genericEntryNames[i]);
        SLANG_CHECK(String(genericReflection->getNameOverride()) == genericTargetNames[i]);

        auto methodName = String(genericEntryNames[i]) + ".invoke";
        ComPtr<slang::IEntryPoint> genericMethodEntry;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(genericModule->findAndCheckEntryPoint(
            methodName.getBuffer(),
            SLANG_STAGE_MISS,
            genericMethodEntry.writeRef(),
            diagnostics.writeRef())));
        auto genericMethodReflection = _getEntryPointReflection(genericMethodEntry);
        SLANG_CHECK(genericMethodReflection->getStage() == SLANG_STAGE_MISS);
        SLANG_CHECK(String(genericMethodReflection->getName()) == methodName);
        SLANG_CHECK(genericMethodReflection->getFunction() == genericReflection->getFunction());
        SLANG_CHECK(String(genericMethodReflection->getNameOverride()) == genericTargetNames[i]);
    }
}

// Selecting Stage.invoke is another spelling of the structural entry selected by Stage. Both
// spellings allow the logical input and select the same checked function and target name.
SLANG_UNIT_TEST(structuralRayTracingEntryPointAcceptsMethodSelection)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalFeature = {};
    experimentalFeature.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalFeature.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalFeature.value.intValue0 = 1;
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalFeature;

    const char* sourcePath =
        "tests/ray-tracing-2/compatibility/mixed-api/native-entry-input-signature.slang";
    String source;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(sourcePath, source)));

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "StructuralMethodSelection",
        sourcePath,
        source.getBuffer(),
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> typeEntry;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "Stage",
        SLANG_STAGE_CLOSEST_HIT,
        typeEntry.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IEntryPoint> methodEntry;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "Stage.invoke",
        SLANG_STAGE_CLOSEST_HIT,
        methodEntry.writeRef(),
        diagnostics.writeRef())));

    auto typeReflection = _getEntryPointReflection(typeEntry);
    auto methodReflection = _getEntryPointReflection(methodEntry);
    SLANG_CHECK(typeReflection->getStage() == SLANG_STAGE_CLOSEST_HIT);
    SLANG_CHECK(methodReflection->getStage() == SLANG_STAGE_CLOSEST_HIT);
    SLANG_CHECK(typeReflection->getFunction() == methodReflection->getFunction());
    SLANG_CHECK(String(typeReflection->getName()) == "Stage");
    SLANG_CHECK(String(methodReflection->getName()) == "Stage.invoke");
    SLANG_CHECK(String(typeReflection->getNameOverride()) == "Stage_closesthit");
    SLANG_CHECK(String(methodReflection->getNameOverride()) == "Stage_closesthit");

    // An explicit rename overrides the generated name. An empty rename retains the existing
    // API behavior of falling back to the source name, not to the generated stage name.
    const char* renamedNames[] = {"customClosestHit", ""};
    const char* expectedNames[] = {"customClosestHit", "Stage"};
    for (Index i = 0; i < SLANG_COUNT_OF(renamedNames); ++i)
    {
        ComPtr<slang::IComponentType> renamedEntry;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(typeEntry->renameEntryPoint(renamedNames[i], renamedEntry.writeRef())));
        auto renamedReflection = _getEntryPointReflection(renamedEntry);
        SLANG_CHECK(String(renamedReflection->getName()) == "Stage");
        SLANG_CHECK(String(renamedReflection->getNameOverride()) == expectedNames[i]);
        SLANG_CHECK(renamedReflection->getFunction() == typeReflection->getFunction());
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
