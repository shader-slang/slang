// unit-test-spirv-interface-default-init-validation.cpp
//
// Repro for: Invalid SPIR-V generated for default-initialized interface variables.
// Expected behavior: successful compilation (possibly with a diagnostic; see issue #9191).
// Actual behavior (with SLANG_RUN_SPIRV_VALIDATION=1): SPIR-V validation fails with
// "Validation of generated SPIR-V failed." (internal error 99999).
//
// NOTE: This test is written as a *regression test* (it expects success). It will fail until the
// underlying bug is fixed.

#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdlib.h>

using namespace Slang;

static int _writeEnvironmentVariable(const char* key, const char* val)
{
#ifdef _WIN32
    String var = String(key) + "=" + val;
    return _putenv(var.getBuffer());
#else
    return setenv(key, val, 1);
#endif
}

static int _unsetEnvironmentVariable(const char* key)
{
#ifdef _WIN32
    // On Windows, setting KEY= clears it for the process.
    String var = String(key) + "=";
    return _putenv(var.getBuffer());
#else
    return unsetenv(key);
#endif
}

struct ScopedEnvVar
{
    const char* key;
    bool hadOldValue = false;
    String oldValue;

    ScopedEnvVar(const char* inKey, const char* inVal)
        : key(inKey)
    {
#ifdef _WIN32
        char* v = nullptr;
        size_t len = 0;
        if (_dupenv_s(&v, &len, key) == 0 && v)
        {
            hadOldValue = true;
            oldValue = v;
            free(v);
        }
#else
        if (const char* v = getenv(key))
        {
            hadOldValue = true;
            oldValue = v;
        }
#endif
        _writeEnvironmentVariable(key, inVal);
    }

    ~ScopedEnvVar()
    {
        if (hadOldValue)
        {
            _writeEnvironmentVariable(key, oldValue.getBuffer());
        }
        else
        {
            _unsetEnvironmentVariable(key);
        }
    }
};

SLANG_UNIT_TEST(spirvInterfaceDefaultInitValidation)
{
    // Enable the built-in SPIR-V validation path in `slang-emit.cpp`.
    ScopedEnvVar validateSpirv("SLANG_RUN_SPIRV_VALIDATION", "1");

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::CompilerOptionEntry compilerOptionEntries[1] = {};
    compilerOptionEntries[0].name = slang::CompilerOptionName::EmitSpirvDirectly;
    compilerOptionEntries[0].value.kind = slang::CompilerOptionValueKind::Int;
    compilerOptionEntries[0].value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = compilerOptionEntries;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    const char* source = R"SLANG(
        interface IHelper
        {
            int getVal();
        }

        struct HelperImpl1 : IHelper
        {
            int storedVal;
            int getVal() { return storedVal; }
        }

        IHelper test(int val)
        {
            IHelper ret = { };

            switch (val % 4)
            {
            case 0:
                ret = { HelperImpl1(val) };
                break;
            default:
                break;
            }

            return ret;
        }

        RWStructuredBuffer<int> gOutputBuffer;

        [numthreads(4, 1, 1)]
        void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            uint tid = dispatchThreadID.x;
            IHelper helper = test(int(tid));
            int outputVal = helper.getVal();
            gOutputBuffer[tid] = outputVal;
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
    SlangResult linkRes = composedProgram->link(linkedProgram.writeRef(), diagnostics.writeRef());
    SLANG_CHECK_ABORT(linkRes == SLANG_OK);
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    ComPtr<slang::IBlob> code;
    SlangResult codeRes =
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());

    // Expected behavior: compilation should succeed and validation should not fail.
    // Current buggy behavior: `codeRes` fails and diagnostics contain
    // "Validation of generated SPIR-V failed."
    SLANG_CHECK(codeRes == SLANG_OK);
    SLANG_CHECK(code != nullptr);
    if (code)
    {
        SLANG_CHECK(code->getBufferSize() != 0);
    }
}
