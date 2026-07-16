// unit-test-lazy-autodiff-module.cpp

#include "slang/slang-compiler-api.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

/// Loads a source module and aborts the current test if semantic checking fails.
static ComPtr<slang::IModule> _loadModule(
    slang::ISession* session,
    const char* moduleName,
    const char* source)
{
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module;
    module =
        session->loadModuleFromSourceString(moduleName, moduleName, source, diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);
    return module;
}

SLANG_UNIT_TEST(lazyAutodiffModuleLoading)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    auto internalGlobalSession = asInternal(globalSession.get());
    const Index baseCoreModuleCount = internalGlobalSession->coreModules.getCount();

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("sm_5_0");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> firstSession;
    SLANG_CHECK_ABORT(
        globalSession->createSession(sessionDesc, firstSession.writeRef()) == SLANG_OK);

    _loadModule(firstSession, "plainModule", "float identity(float value) { return value; }");
    SLANG_CHECK(internalGlobalSession->coreModules.getCount() == baseCoreModuleCount);

    _loadModule(
        firstSession,
        "differentiableModule",
        "[ForwardDifferentiable] float f(float value) { return sin(value); }");
    SLANG_CHECK(internalGlobalSession->coreModules.getCount() == baseCoreModuleCount + 1);

    // A new linkage reuses the same global session after its supplement has been loaded. Its first
    // cache construction therefore includes the supplement through Session::coreModules. A later
    // differentiability trigger must not append those extensions or associations a second time.
    ComPtr<slang::ISession> reusedSession;
    SLANG_CHECK_ABORT(
        globalSession->createSession(sessionDesc, reusedSession.writeRef()) == SLANG_OK);
    _loadModule(
        reusedSession,
        "reusedSessionModule",
        R"(
            struct Plain
            {
                float value;
            }

            float readPlain(Plain value)
            {
                return value.value;
            }

            struct Aggregate : IDifferentiable
            {
                Optional<float> value;
            }

            [ForwardDifferentiable]
            float differentiated(float value)
            {
                return sin(value);
            }

            float useBuiltinDerivative(float value)
            {
                return fwd_diff(sin)(diffPair(value, 1.0)).d;
            }
        )");
    SLANG_CHECK(internalGlobalSession->coreModules.getCount() == baseCoreModuleCount + 1);
}
