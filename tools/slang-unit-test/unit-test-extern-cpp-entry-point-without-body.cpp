// unit-test-extern-cpp-entry-point-without-body.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Test that a body-less `__extern_cpp` function selected as an entry point through
// `IModule::findAndCheckEntryPoint` is reported as an unresolved symbol rather than crashing the
// compiler. The function has no `[shader]` attribute, so the module is lowered before anything
// marks it as an entry point; the link step has to recognize it from the program being linked.
// See https://github.com/shader-slang/slang/issues/14200

static void checkBodylessEntryPointIsUnresolved(
    UnitTestContext* unitTestContext,
    SlangCompileTarget format)
{
    const char* source = R"(
        [numthreads(1, 1, 1)]
        __extern_cpp void NoBody(uniform float* p, uint3 tid : SV_DispatchThreadID);
        )";

    auto globalSession = unitTestContext->slangGlobalSession;
    SLANG_CHECK_ABORT(globalSession != nullptr);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = format;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module =
        session->loadModuleFromSourceString("m", "m.slang", source, diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "NoBody",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> codeDiagnostics;
    SlangResult result =
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), codeDiagnostics.writeRef());
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK_ABORT(codeDiagnostics != nullptr);

    auto diagnostics = UnownedStringSlice(
        (const char*)codeDiagnostics->getBufferPointer(),
        codeDiagnostics->getBufferSize());
    SLANG_CHECK(diagnostics.indexOf(toSlice("error[E45001]")) >= 0);
    SLANG_CHECK(diagnostics.indexOf(toSlice("E99997")) < 0);
}

SLANG_UNIT_TEST(externCppEntryPointWithoutBodyCuda)
{
    checkBodylessEntryPointIsUnresolved(unitTestContext, SLANG_CUDA_SOURCE);
}

SLANG_UNIT_TEST(externCppEntryPointWithoutBodyCpp)
{
    checkBodylessEntryPointIsUnresolved(unitTestContext, SLANG_CPP_SOURCE);
}
