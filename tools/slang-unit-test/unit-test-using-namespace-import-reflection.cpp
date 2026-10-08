// unit-test-using-namespace-import-reflection.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// shader-slang/slang#11443: a `using namespace Foo;` in a module's PRIMARY source file
// must stay lookup-local to that module — it must not leak `Foo`'s members into an
// importing program's name-based (reflection / API) lookup. That lookup is backed by
// `ComponentType::_getOrCreateScopeForLegacyLookup`, the twin of the `import`-path
// re-export filter in `importModuleIntoScope`. The source-level `import` tests under
// tests/ only exercise the `import` path; this drives the legacy/API path via
// `IComponentType::getLayout()->findTypeByName` (the path
// findTypeByName -> getTypeFromString -> _getOrCreateScopeForLegacyLookup).
//
// Pins the `_getOrCreateScopeForLegacyLookup` mirror: reverting that hunk makes the
// unqualified `findTypeByName("Bar")` non-null again (the leak).
SLANG_UNIT_TEST(usingNamespaceImportReflectionNoLeak)
{
    // A module whose PRIMARY file pulls `Foo` into scope with a top-level `using`.
    const char* usingModSource = R"(
        namespace Foo
        {
            public struct Bar { int x; }
        }
        using namespace Foo;
    )";
    // An importer that imports the module above and adds no `using` of its own.
    const char* importerSource = R"(
        import usingmod;
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void csMain() {}
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("sm_5_0");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto usingMod = session->loadModuleFromSourceString(
        "usingmod",
        "usingmod.slang",
        usingModSource,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(usingMod != nullptr);

    auto importer = session->loadModuleFromSourceString(
        "importer",
        "importer.slang",
        importerSource,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(importer != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    importer->findAndCheckEntryPoint(
        "csMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    ComPtr<slang::IComponentType> program;
    slang::IComponentType* components[] = {importer, entryPoint.get()};
    session
        ->createCompositeComponentType(components, 2, program.writeRef(), diagnostics.writeRef());
    SLANG_CHECK_ABORT(program != nullptr);

    auto layout = program->getLayout();
    SLANG_CHECK_ABORT(layout != nullptr);

    // Qualified access through the imported module still resolves — the module's own
    // scope is still re-exported into the legacy lookup scope.
    auto qualified = layout->findTypeByName("Foo::Bar");
    SLANG_CHECK(qualified != nullptr);

    // The imported module's primary-file `using namespace Foo;` must NOT be re-exported
    // into this importer's name-based lookup, so `Bar` is not visible unqualified.
    auto leaked = layout->findTypeByName("Bar");
    SLANG_CHECK(leaked == nullptr);
}
