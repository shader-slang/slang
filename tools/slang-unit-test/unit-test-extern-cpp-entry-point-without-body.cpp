// unit-test-extern-cpp-entry-point-without-body.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Test that a body-less `__extern_cpp` function selected as an entry point through
// `IModule::findAndCheckEntryPoint` is reported once as an unresolved symbol rather than crashing
// the compiler. The function has neither `[shader]` nor `[numthreads]`, so entry-point discovery
// does not pick it up and the module is lowered before anything marks it as an entry point; the
// check in `specializeIRForEntryPoint` has to recognize it from the program being linked.
// See https://github.com/shader-slang/slang/issues/14200

static const char* kExternCppSource = R"slang(
__extern_cpp void NoBody(uniform float* p, uint3 tid : SV_DispatchThreadID);
)slang";

// A generic entry point is found by the mangled name of its specialization, while cloning checks
// the generic itself, so only `findSymbolCheckedByCloning` lets the check in
// `specializeIRForEntryPoint` report this function, at the generic's declaration.
static const char* kIntrinsicGenericSource = R"slang(
__target_intrinsic(hlsl, "noBody()")
__target_intrinsic(cuda, "noBody()")
__target_intrinsic(cpp, "noBody()")
void NoBodyGeneric<T>(uniform T* p, uint3 tid : SV_DispatchThreadID);
)slang";

static void checkBodylessEntryPointIsUnresolved(
    UnitTestContext* unitTestContext,
    SlangCompileTarget format,
    const char* source,
    const char* entryPointName)
{
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
        entryPointName,
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
    Index firstError = diagnostics.indexOf(toSlice("error[E45001]"));
    SLANG_CHECK(firstError >= 0);
    SLANG_CHECK(diagnostics.tail(firstError + 1).indexOf(toSlice("error[E45001]")) < 0);
    // E99997-E99999 report an internal compiler error.
    SLANG_CHECK(diagnostics.indexOf(toSlice("E9999")) < 0);
    // The report is at the declaration in `m.slang`, not at a location-less specialization.
    UnownedStringSlice afterError = diagnostics.tail(firstError);
    Index lineEnd = afterError.indexOf('\n');
    SLANG_CHECK(lineEnd >= 0 && afterError.tail(lineEnd + 1).startsWith(toSlice(" --> m.slang")));
}

SLANG_UNIT_TEST(externCppEntryPointWithoutBodyCuda)
{
    checkBodylessEntryPointIsUnresolved(
        unitTestContext,
        SLANG_CUDA_SOURCE,
        kExternCppSource,
        "NoBody");
}

SLANG_UNIT_TEST(externCppEntryPointWithoutBodyCpp)
{
    checkBodylessEntryPointIsUnresolved(
        unitTestContext,
        SLANG_CPP_SOURCE,
        kExternCppSource,
        "NoBody");
}

SLANG_UNIT_TEST(externCppEntryPointWithoutBodySpirv)
{
    checkBodylessEntryPointIsUnresolved(unitTestContext, SLANG_SPIRV, kExternCppSource, "NoBody");
}

SLANG_UNIT_TEST(externCppEntryPointWithoutBodyHlsl)
{
    checkBodylessEntryPointIsUnresolved(unitTestContext, SLANG_HLSL, kExternCppSource, "NoBody");
}

SLANG_UNIT_TEST(intrinsicGenericEntryPointWithoutBodyCuda)
{
    checkBodylessEntryPointIsUnresolved(
        unitTestContext,
        SLANG_CUDA_SOURCE,
        kIntrinsicGenericSource,
        "NoBodyGeneric<float>");
}

SLANG_UNIT_TEST(intrinsicGenericEntryPointWithoutBodySpirv)
{
    checkBodylessEntryPointIsUnresolved(
        unitTestContext,
        SLANG_SPIRV,
        kIntrinsicGenericSource,
        "NoBodyGeneric<float>");
}
