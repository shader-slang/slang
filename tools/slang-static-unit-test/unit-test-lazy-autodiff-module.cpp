// unit-test-lazy-autodiff-module.cpp
//
// Tests that the autodiff builtin supplement loads on demand: that ordinary code
// leaves it unloaded, that the semantic nodes which need derivative
// implementations load it, and that it loads at most once per global session.
//
// The observable being asserted on is `Session::coreModules`, which is internal
// compiler state. That is what places these tests here rather than in
// `slang-unit-test`: a plugin test can reach only exported symbols, so observing
// the count from there previously required adding a `SLANG_API` accessor to
// libslang that existed for no other reason. Linking statically reads the field
// directly and keeps the test-only surface out of the shipped library.

// `slang-global-session.h` returns `RefPtr<ASTBuilder>` by value, so it needs that type
// complete; include its definition rather than relying on a transitive include.
#include "slang/slang-ast-builder.h"
#include "slang/slang-check-impl.h"
#include "slang/slang-global-session.h"
#include "slang/slang-module.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

/// Returns how many builtin/core modules `globalSession` currently has loaded.
static Index _loadedBuiltinModuleCount(slang::IGlobalSession* globalSession)
{
    return static_cast<Session*>(globalSession)->coreModules.getCount();
}

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
    // Deliberately not `StaticUnitTestEnv`, which shares one global session across the whole
    // suite. This test asserts on the loaded-module count of a session whose history it knows,
    // so it must own that session: any other test in the process that triggered a supplement
    // load would otherwise decide this one's result, making the suite order-dependent.
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    const Index baseCoreModuleCount = _loadedBuiltinModuleCount(globalSession);

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
    SLANG_CHECK(_loadedBuiltinModuleCount(globalSession) == baseCoreModuleCount);

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
    SLANG_CHECK(_loadedBuiltinModuleCount(globalSession) == baseCoreModuleCount);

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
    SLANG_CHECK(_loadedBuiltinModuleCount(globalSession) == baseCoreModuleCount + 1);

    _loadModule(
        firstSession,
        "differentiableModule",
        "[ForwardDifferentiable] float f(float value) { return sin(value); }");
    SLANG_CHECK(_loadedBuiltinModuleCount(globalSession) == baseCoreModuleCount + 1);

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
    SLANG_CHECK(_loadedBuiltinModuleCount(globalSession) == baseCoreModuleCount + 1);
}

// The multi-linkage merge is idempotent: it does not append the supplement's declaration
// associations a second time to a context that already has them.
//
// The hazard is a `SharedSemanticsContext` that first builds its aggregate views *after* some
// other linkage already loaded the supplement into `Session::coreModules`. Normal construction
// then already includes the supplement, and a later differentiability trigger in that same
// context still calls `addLoadedAutodiffModule`. Without the containment check in
// `_mergeDeclAssociationsFromModule`, that second pass would duplicate every association the
// supplement contributes.
//
// `lazyAutodiffModuleLoading` cannot see this. It asserts on the loaded-module count, which is
// what stays the same whether or not the merge duplicates: the module is loaded once either way.
// Observing the invariant requires the context's own association list, so this test drives a
// `SharedSemanticsContext` directly -- reachable only because this suite links the compiler
// statically.
//
// Associations rather than candidate extensions are what the supplement contributes: the split
// left every `extension` declaration in the eager `autodiff-base` segment, while the registered
// `[ForwardDerivativeOf]`/`[BackwardDerivativeOf]` implementations that remain are recorded
// against the module owning the *derivative*, which is the supplement.
SLANG_UNIT_TEST(lazyAutodiffModuleMergeDoesNotDuplicateAssociations)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("sm_5_0");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    // A first linkage triggers the load, so the supplement is in `Session::coreModules` before
    // the linkage under test is created.
    ComPtr<slang::ISession> triggerSession;
    SLANG_CHECK_ABORT(
        globalSession->createSession(sessionDesc, triggerSession.writeRef()) == SLANG_OK);
    _loadModule(
        triggerSession,
        "triggerModule",
        "[ForwardDifferentiable] float f(float value) { return sin(value); }");

    Session* sessionImpl = static_cast<Session*>(globalSession.get());
    Module* supplement = sessionImpl->getBuiltinModule(slang::BuiltinModuleName::Autodiff);
    SLANG_CHECK_ABORT(supplement != nullptr);
    ModuleDecl* supplementDecl = supplement->getModuleDecl();

    // Any primal the supplement registers a derivative for will do; take the first.
    Decl* primalDecl = nullptr;
    for (auto& entry : supplementDecl->mapDeclToAssociatedDecls)
    {
        primalDecl = entry.key;
        break;
    }
    // If this fires, the supplement contributes no associations and the merge below has nothing
    // to duplicate -- the test would pass while asserting nothing.
    SLANG_CHECK_ABORT(primalDecl != nullptr);

    ComPtr<slang::ISession> reusedSession;
    SLANG_CHECK_ABORT(
        globalSession->createSession(sessionDesc, reusedSession.writeRef()) == SLANG_OK);
    Linkage* linkage = static_cast<Linkage*>(reusedSession.get());

    DiagnosticSink sink(linkage->getSourceManager(), nullptr);
    SharedSemanticsContext context(linkage, nullptr, &sink);

    // Building the view now picks the supplement up through `Session::coreModules`.
    const Index countBeforeMerge = context.getAssociatedDeclsForDecl(primalDecl).getCount();
    SLANG_CHECK(countBeforeMerge > 0);

    context.addLoadedAutodiffModule(supplementDecl);

    SLANG_CHECK(context.getAssociatedDeclsForDecl(primalDecl).getCount() == countBeforeMerge);
}
