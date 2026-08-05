// unit-test-lazy-autodiff-module.cpp

// slang-compiler-api.h transitively includes slang-global-session.h (which declares
// getLoadedBuiltinModuleCountForUnitTest) along with the full definitions it depends on.
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

    // Observe lazy builtin-module loading through the libslang accessor rather than dereferencing
    // an internal Session* here: that dereference would make this object file reference `typeinfo
    // for Slang::Session` under -fsanitize=vptr, which is not exported to this separately linked
    // tool and fails to link on ELF.
    const Index baseCoreModuleCount = getLoadedBuiltinModuleCountForUnitTest(globalSession.get());

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
    SLANG_CHECK(getLoadedBuiltinModuleCountForUnitTest(globalSession.get()) == baseCoreModuleCount);

    // The base-surface autodiff symbols (diffPair, the update helpers, detach, and the tensor-view
    // types) live in the eager `autodiff-base` segment folded into the core module, so ordinary
    // code may use them without pulling in the lazy supplement. A `//TEST:SIMPLE:` shader cannot
    // observe load state, so assert here that referencing this whole cluster leaves the supplement
    // unloaded. This is the guard against a base-surface declaration silently regressing to require
    // the supplement.
    _loadModule(
        firstSession,
        "baseSurfaceModule",
        "float useBase(float v) {"
        "  var p = diffPair(v, 1.0); updateDiff(p, 2.0);"
        "  DiffTensorView<float> dv; return detach(p.p); }");
    SLANG_CHECK(getLoadedBuiltinModuleCountForUnitTest(globalSession.get()) == baseCoreModuleCount);

    // A `[PrimalSubstituteOf]` attribute is not a differentiability header modifier, so it does not
    // load the supplement through `checkDifferentiableCallableCommon`. With no `[Differentiable]`
    // header and no `fwd_diff`/`bwd_diff` in the module, the only thing that can drive the load is
    // the `PrimalSubstituteExpr` synthesized while checking the attribute (the
    // `as<PrimalSubstituteExpr>` disjunct in `_checkHigherOrderInvokeExpr`). Observing the count
    // increment here is what keeps that branch from becoming dead: without it the supplement would
    // never load and this check would fail.
    _loadModule(
        firstSession,
        "primalSubstituteModule",
        "float original(float x) { return x * x; }\n"
        "[PrimalSubstituteOf(original)]\n"
        "float primalSubst(float x) { return 2.0f * x * x; }\n");
    SLANG_CHECK(
        getLoadedBuiltinModuleCountForUnitTest(globalSession.get()) == baseCoreModuleCount + 1);

    _loadModule(
        firstSession,
        "differentiableModule",
        "[ForwardDifferentiable] float f(float value) { return sin(value); }");
    SLANG_CHECK(
        getLoadedBuiltinModuleCountForUnitTest(globalSession.get()) == baseCoreModuleCount + 1);

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
    SLANG_CHECK(
        getLoadedBuiltinModuleCountForUnitTest(globalSession.get()) == baseCoreModuleCount + 1);
}
