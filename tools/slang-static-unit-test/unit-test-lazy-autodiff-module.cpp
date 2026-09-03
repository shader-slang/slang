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

    // The base-surface autodiff symbols live in the eager `autodiff-base` segment folded into the
    // core module, so ordinary code may use them without pulling in the lazy supplement. This is
    // the guard against a base-surface declaration silently regressing to require the supplement.
    //
    // It has to be a count assertion here rather than in the companion
    // `tests/autodiff/lazy-load-base-surface.slang`: a `//TEST:SIMPLE:` shader can only show the
    // cluster still *compiles*, and would keep passing if a symbol started triggering the load,
    // because the load then just succeeds silently.
    //
    // Deliberately excluded: a concrete `struct : IDifferentiable`. Checking one loads the
    // supplement today -- see `lazyAutodiffConcreteDifferentiableConformanceLoadsSupplement`
    // below, which pins that separately. Every symbol listed here was individually confirmed not
    // to load it.
    _loadModule(
        firstSession,
        "baseSurfaceModule",
        R"(
            struct PlainTensorTypes
            {
                TensorView<float> view;
                DiffTensorView<float> diffView;
                TorchTensor<float> tensor;
            }

            float useAutodiffHelpersWithoutDifferentiating(float value)
            {
                var pair = diffPair(value, 1.0);
                updatePrimal(pair, value + 1.0);
                updateDiff(pair, 2.0);
                updatePair(pair, value + 2.0, 3.0);
                let values = makeArrayFromElement<float, 2>(pair.p);
                return detach(values[0]);
            }

            void useDifferentiableConstraint<T : IDifferentiable>(T value) { }

            interface IRefinesDifferentiable : IDifferentiable { }

            // The three aggregate conformances the base-surface header names as needed by
            // aggregate-conformance synthesis are Array, Optional and Tuple. Array and Optional
            // are covered above by `makeArrayFromElement` and the helper signatures; Tuple needs
            // its own use, and the pre-existing autodiff tests cannot stand in for it because they
            // all also carry `[Differentiable]` and so load the supplement regardless.
            struct HoldsTuple
            {
                Tuple<float, float> pair;
            }

            float useTuple(float value)
            {
                let pair = makeTuple(value, value);
                return pair._0;
            }

            // The `IDifferentiablePtrType` extensions for Array and Optional moved to the base
            // surface alongside the `IDifferentiable` ones.
            void useArrayPtrConstraint<T : IDifferentiablePtrType>(T[2] values) { }
            void useOptionalPtrConstraint<T : IDifferentiablePtrType>(Optional<T> value) { }
        )");
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
// Both halves of the merge are checked, because the supplement contributes both kinds of entry.
// `diff.meta.slang` contains no `extension` blocks -- the split left those in the eager
// `autodiff-base` segment -- but candidate extensions are not only written by `extension` syntax:
// checking a `[ForwardDerivativeOf]`/`[BackwardDerivativeOf]` declaration synthesizes one, and
// this PR's ownership fix registers it against the module owning the *derivative*, which is the
// supplement. Reading the maps directly rather than counting `extension` blocks in the source is
// what distinguishes the two.
SLANG_UNIT_TEST(lazyAutodiffModuleMergeDoesNotDuplicateEntries)
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
    Decl* extendedDecl = nullptr;
    for (const auto& entry : supplementDecl->mapDeclToCandidateExtensions)
    {
        extendedDecl = entry.first;
        break;
    }
    // If either fires, the supplement contributes no entry of that kind, the corresponding merge
    // below has nothing to duplicate, and the test would pass while asserting nothing.
    SLANG_CHECK_ABORT(primalDecl != nullptr);
    SLANG_CHECK_ABORT(extendedDecl != nullptr);

    ComPtr<slang::ISession> reusedSession;
    SLANG_CHECK_ABORT(
        globalSession->createSession(sessionDesc, reusedSession.writeRef()) == SLANG_OK);
    Linkage* linkage = static_cast<Linkage*>(reusedSession.get());

    DiagnosticSink sink(linkage->getSourceManager(), nullptr);
    SharedSemanticsContext context(linkage, nullptr, &sink);

    // Building the views now picks the supplement up through `Session::coreModules`.
    const Index associationsBeforeMerge = context.getAssociatedDeclsForDecl(primalDecl).getCount();
    const Index extensionsBeforeMerge =
        context.getCandidateExtensionsForTypeDecl(extendedDecl).getCount();
    SLANG_CHECK(associationsBeforeMerge > 0);
    SLANG_CHECK(extensionsBeforeMerge > 0);

    context.addLoadedAutodiffModule(supplementDecl);

    SLANG_CHECK(
        context.getAssociatedDeclsForDecl(primalDecl).getCount() == associationsBeforeMerge);
    SLANG_CHECK(
        context.getCandidateExtensionsForTypeDecl(extendedDecl).getCount() ==
        extensionsBeforeMerge);
}

// Declaring a concrete `struct : IDifferentiable` loads the supplement, even with no
// `[Differentiable]` member, no `fwd_diff`/`bwd_diff`, and no derivative of any kind written by
// hand. Conformance synthesis produces `dzero`/`dadd` carrying a differentiability modifier, and
// checking those is a load trigger.
//
// This is pinned rather than asserted-against because it is a live question, not a settled
// contract. The PR's own motivation offers `struct Parameters : IDifferentiable { float
// values[2]; Optional<float> optionalValue; }` as ordinary language surface, and the split does
// keep that surface *findable* -- it compiles, which is what
// `tests/autodiff/lazy-load-base-surface.slang` shows. What it does not do is keep it
// supplement-free, so a session that only declares differentiable data still materializes the
// derivative machinery it never differentiates with.
//
// The narrowing matters if this is revisited: a generic constraint `T : IDifferentiable` and an
// interface refining `IDifferentiable` both stay clean (asserted above). Only concrete conformance
// on a struct trips it.
SLANG_UNIT_TEST(lazyAutodiffConcreteDifferentiableConformanceLoadsSupplement)
{
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

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    _loadModule(
        session,
        "concreteConformanceModule",
        "struct PlainAggregate : IDifferentiable"
        "{"
        "    float values[2];"
        "    Optional<float> optionalValue;"
        "}");
    SLANG_CHECK(_loadedBuiltinModuleCount(globalSession) == baseCoreModuleCount + 1);
}

// `[MaybeDifferentiable]` on an interface requirement is the third disjunct of
// `_callableHasDifferentiabilityHeaderModifier`, and the only one that fires on a requirement
// rather than an implementation. It needs its own global session because the supplement, once
// loaded, stays loaded: sequencing this after any other trigger would assert nothing.
//
// Same purpose as the `primalSubstituteModule` check in `lazyAutodiffModuleLoading` — without a
// test that this disjunct still loads the supplement, a regression that stopped firing it would
// leave maybe-differentiable requirements checked against machinery that was never materialized,
// and nothing here would notice.
SLANG_UNIT_TEST(lazyAutodiffMaybeDifferentiableRequirementLoadsSupplement)
{
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

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    // No `[Differentiable]` implementation, no `fwd_diff`/`bwd_diff`, no primal substitute, and no
    // concrete `IDifferentiable` conformance: the requirement's modifier is the only thing here
    // that can drive the load.
    _loadModule(
        session,
        "maybeDifferentiableModule",
        "interface IThing"
        "{"
        "    [MaybeDifferentiable] float compute(float x);"
        "}");
    SLANG_CHECK(_loadedBuiltinModuleCount(globalSession) == baseCoreModuleCount + 1);
}

// When checking a module from source triggers the on-demand autodiff-supplement load,
// `SemanticsContext::ensureAutodiffModuleLoaded` records the supplement as a dependency of the
// module being checked, the same way `visitImportDecl` records a written `import`. Serializing
// that module and reloading it in a fresh global session -- one whose `coreModules` has never seen
// the supplement -- must therefore load the supplement automatically while deserializing the
// module's AST, through the ordinary `ImportedModule`/`findOrImportModule` mechanism used for any
// other cross-module reference.
//
// This mirrors the `slang.neural` scenario that originally motivated scanning the deserialized
// IR's mangled names for the supplement's module qualifier (see the removed
// `_importsAutodiffSupplementSymbol` in slang-session.cpp): a module with no source-visible
// `import` of the supplement that nonetheless calls a builtin registered derivative. Before the
// dependency-tracking fix this test pins, nothing loaded the supplement for a module reached only
// by deserialization, and linking a caller of such a module would fail with an unresolved external
// symbol for the derivative.
SLANG_UNIT_TEST(lazyAutodiffModuleDependencySurvivesSerialization)
{
    ComPtr<slang::IGlobalSession> sourceGlobalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, sourceGlobalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc sourceTargetDesc = {};
    sourceTargetDesc.format = SLANG_HLSL;
    sourceTargetDesc.profile = sourceGlobalSession->findProfile("sm_5_0");
    slang::SessionDesc sourceSessionDesc = {};
    sourceSessionDesc.targetCount = 1;
    sourceSessionDesc.targets = &sourceTargetDesc;

    ComPtr<slang::ISession> sourceSession;
    SLANG_CHECK_ABORT(
        sourceGlobalSession->createSession(sourceSessionDesc, sourceSession.writeRef()) ==
        SLANG_OK);

    ComPtr<slang::IModule> sourceModule = _loadModule(
        sourceSession,
        "usesBuiltinDerivative",
        "float useBuiltinBackwardDerivative(float value)"
        "{"
        "    var pair = diffPair(value, 0.0);"
        "    bwd_diff(sin)(pair, 1.0);"
        "    return pair.d;"
        "}");

    ComPtr<slang::IBlob> serializedModule;
    SLANG_CHECK_ABORT(sourceModule->serialize(serializedModule.writeRef()) == SLANG_OK);

    // A separate global session, so its `coreModules` starts without the autodiff supplement --
    // nothing in this process has triggered a load in it yet.
    ComPtr<slang::IGlobalSession> reloadGlobalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, reloadGlobalSession.writeRef()) == SLANG_OK);
    const Index baseCoreModuleCount = _loadedBuiltinModuleCount(reloadGlobalSession);

    slang::TargetDesc reloadTargetDesc = {};
    reloadTargetDesc.format = SLANG_HLSL;
    reloadTargetDesc.profile = reloadGlobalSession->findProfile("sm_5_0");
    slang::SessionDesc reloadSessionDesc = {};
    reloadSessionDesc.targetCount = 1;
    reloadSessionDesc.targets = &reloadTargetDesc;

    ComPtr<slang::ISession> reloadSession;
    SLANG_CHECK_ABORT(
        reloadGlobalSession->createSession(reloadSessionDesc, reloadSession.writeRef()) ==
        SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> reloadedModule(reloadSession->loadModuleFromIRBlob(
        "usesBuiltinDerivative",
        "usesBuiltinDerivative",
        serializedModule,
        diagnostics.writeRef()));
    SLANG_CHECK_ABORT(reloadedModule != nullptr);

    SLANG_CHECK(_loadedBuiltinModuleCount(reloadGlobalSession) == baseCoreModuleCount + 1);
}
