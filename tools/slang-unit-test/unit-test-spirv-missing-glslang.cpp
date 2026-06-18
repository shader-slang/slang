// unit-test-spirv-missing-glslang.cpp
//
// Regression test for shader-slang/slang#11662: a missing optional `spirv-opt`
// (served by the dlopen-ed slang-glslang shared library) must not turn a
// successfully-emitted, valid SPIR-V module into a fatal compile error.
//
// The failure is a runtime `dlopen` absence and cannot be expressed as an ordinary
// `tests/*.slang` case (that harness always installs DefaultSharedLibraryLoader and
// offers no directive to hide a library). Instead we install a custom
// `ISlangSharedLibraryLoader` that fails any load whose path mentions
// "slang-glslang" and delegates everything else to the default loader. Installing
// a loader clears the per-session downstream-compiler cache
// (`Session::_setSharedLibraryLoader`), so this deterministically reproduces the
// missing-library condition even on CI where slang-glslang is present.

#include "../../source/core/slang-basic.h"
#include "../../source/core/slang-shared-library.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

namespace
{
// A shared-library loader that pretends slang-glslang (and therefore spirv-opt) is
// unavailable, while loading every other requested library normally.
class MissingGlslangLoader : public ISlangSharedLibraryLoader
{
public:
    // The test owns the single instance for its whole lifetime, so reference
    // counting is a no-op (mirrors DefaultSharedLibraryLoader's singleton style).
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }

    ISlangUnknown* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() ||
                guid == ISlangSharedLibraryLoader::getTypeGuid())
                   ? static_cast<ISlangSharedLibraryLoader*>(this)
                   : nullptr;
    }
    SLANG_IUNKNOWN_QUERY_INTERFACE

    SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outSharedLibrary) SLANG_OVERRIDE
    {
        if (path && strstr(path, "slang-glslang") != nullptr)
        {
            *outSharedLibrary = nullptr;
            return SLANG_E_NOT_FOUND;
        }
        return DefaultSharedLibraryLoader::getSingleton()->loadSharedLibrary(
            path,
            outSharedLibrary);
    }
};

bool diagnosticsContain(slang::IBlob* diagnostics, const char* needle)
{
    if (!diagnostics || diagnostics->getBufferSize() == 0)
        return false;
    UnownedStringSlice text(
        (const char*)diagnostics->getBufferPointer(),
        (const char*)diagnostics->getBufferPointer() + diagnostics->getBufferSize());
    return text.indexOf(UnownedStringSlice(needle)) >= 0;
}

// Compile a trivial single-entry-point compute shader to SPIR-V with slang-glslang
// made unavailable, optionally requesting separate debug info (which genuinely
// needs spirv-opt). Returns the `getEntryPointCode` result plus the produced code
// and diagnostics.
SlangResult compileWithMissingGlslang(
    bool requestSeparateDebug,
    ComPtr<slang::IBlob>& outCode,
    ComPtr<slang::IBlob>& outDiagnostics)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    static MissingGlslangLoader loader;
    globalSession->setSharedLibraryLoader(&loader);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::CompilerOptionEntry options[2] = {};
    uint32_t optionCount = 0;
    options[optionCount].name = slang::CompilerOptionName::EmitSpirvDirectly;
    options[optionCount].value.kind = slang::CompilerOptionValueKind::Int;
    options[optionCount].value.intValue0 = 1;
    optionCount++;
    if (requestSeparateDebug)
    {
        options[optionCount].name = slang::CompilerOptionName::EmitSeparateDebug;
        options[optionCount].value.kind = slang::CompilerOptionValueKind::Int;
        options[optionCount].value.intValue0 = 1;
        optionCount++;
    }

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntryCount = optionCount;
    sessionDesc.compilerOptionEntries = options;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    const char* source = R"SLANG(
        RWStructuredBuffer<int> gOutputBuffer;
        [numthreads(1, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            gOutputBuffer[tid.x] = int(tid.x);
        }
    )SLANG";

    ComPtr<slang::IBlob> diagnostics;
    auto module =
        session->loadModuleFromSourceString("m", "test.slang", source, diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    slang::IComponentType* componentTypes[2] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> composedProgram;
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(
            componentTypes,
            2,
            composedProgram.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(
        composedProgram->link(linkedProgram.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    outCode.setNull();
    outDiagnostics.setNull();
    return linkedProgram->getEntryPointCode(0, 0, outCode.writeRef(), outDiagnostics.writeRef());
}
} // namespace

// Degrade-success: with slang-glslang unavailable, a plain single-module SPIR-V
// compile must still succeed and emit valid SPIR-V, rather than failing with the
// fatal "failed to load downstream compiler 'spirv-opt'" error (E00100) from #11662.
SLANG_UNIT_TEST(spirvMissingGlslangDegradesGracefully)
{
    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnostics;
    SlangResult res = compileWithMissingGlslang(/*requestSeparateDebug:*/ false, code, diagnostics);

    SLANG_CHECK(res == SLANG_OK);
    SLANG_CHECK(code != nullptr);
    if (code)
    {
        SLANG_CHECK(code->getBufferSize() != 0);
    }
    SLANG_CHECK(!diagnosticsContain(diagnostics, "E00100"));
}

// Required-fail: separate debug-info extraction genuinely needs spirv-opt, so with
// slang-glslang unavailable the compile must fail loudly with E00100 (and must not
// crash dereferencing the null debug artifact in the caller).
SLANG_UNIT_TEST(spirvMissingGlslangRequiredStepFailsLoudly)
{
    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnostics;
    SlangResult res = compileWithMissingGlslang(/*requestSeparateDebug:*/ true, code, diagnostics);

    SLANG_CHECK(SLANG_FAILED(res));
    SLANG_CHECK(diagnosticsContain(diagnostics, "E00100"));
}
