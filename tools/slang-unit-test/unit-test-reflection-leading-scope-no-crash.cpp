// unit-test-reflection-leading-scope-no-crash.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// shader-slang/slang#13015: name-based type lookup (findTypeByName -> getTypeFromString) parses its
// string with no owning module (its synthetic legacy-lookup scope spans multiple modules), so the
// parser runs with `Parser::currentModule == null`. This pins three properties of that path:
//   1. inputs that reach a leading-`::` type scope or a language-version gate return cleanly (null)
//      instead of crashing;
//   2. a valid generic type still resolves through the same path; and
//   3. the parser uses the session's language version, not a hard-coded default.
SLANG_UNIT_TEST(reflectionLeadingScopeNoCrash)
{
    const char* moduleSource = R"(
        struct GlobalThing { int x; }
        struct MyGeneric<T> { T value; }
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void csMain() {}
    )";

    // Owns the whole session/program chain so the returned layout stays valid. A fresh global
    // session per case also isolates it from the module/reflection caches of the other cases.
    struct Reflection
    {
        ComPtr<slang::IGlobalSession> globalSession;
        ComPtr<slang::ISession> session;
        ComPtr<slang::IComponentType> program;
        slang::ProgramLayout* layout = nullptr;
    };

    // Builds a program layout for the module above in a session configured for `languageVersion`.
    auto makeReflection = [&](SlangLanguageVersion languageVersion, Reflection& out)
    {
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, out.globalSession.writeRef()) == SLANG_OK);

        slang::CompilerOptionEntry versionEntry = {};
        versionEntry.name = slang::CompilerOptionName::LanguageVersion;
        versionEntry.value.kind = slang::CompilerOptionValueKind::Int;
        versionEntry.value.intValue0 = languageVersion;

        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_HLSL;
        targetDesc.profile = out.globalSession->findProfile("sm_5_0");
        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;
        sessionDesc.compilerOptionEntryCount = 1;
        sessionDesc.compilerOptionEntries = &versionEntry;
        SLANG_CHECK_ABORT(
            out.globalSession->createSession(sessionDesc, out.session.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> diagnostics;
        auto module = out.session->loadModuleFromSourceString(
            "m",
            "m.slang",
            moduleSource,
            diagnostics.writeRef());
        SLANG_CHECK_ABORT(module != nullptr);

        ComPtr<slang::IEntryPoint> entryPoint;
        module->findAndCheckEntryPoint(
            "csMain",
            SLANG_STAGE_COMPUTE,
            entryPoint.writeRef(),
            diagnostics.writeRef());
        SLANG_CHECK_ABORT(entryPoint != nullptr);

        slang::IComponentType* components[] = {module, entryPoint.get()};
        out.session->createCompositeComponentType(
            components,
            2,
            out.program.writeRef(),
            diagnostics.writeRef());
        SLANG_CHECK_ABORT(out.program != nullptr);

        out.layout = out.program->getLayout();
        SLANG_CHECK_ABORT(out.layout != nullptr);
    };

    Reflection base;
    makeReflection(SLANG_LANGUAGE_VERSION_DEFAULT, base);

    // None of these is a valid type, so lookup must return null rather than crash. Each reaches a
    // guarded `currentModule` access on the module-less path:
    //   "x as ::Foo"                 `as` operator -> leading-`::` type scope;
    //   "(::Foo)0" / "()" / "(a,b)"  parenthesis language-version gates;
    //   "Foo<volatile ::Bar>"        `volatile` gate + a modifier-bearing leading-`::` generic arg;
    //   "x as struct[X] Foo {}"      two crashes on one input: the `struct` `[`-attribute
    //                                language-version gate (ParseStruct) and, once the inline decl
    //                                is parsed, the null-`containerDecl` deref in AddMember.
    const char* invalidInputs[] =
        {"x as ::Foo", "(::Foo)0", "()", "(a,b)", "Foo<volatile ::Bar>", "x as struct[X] Foo {}"};
    for (const char* input : invalidInputs)
    {
        SLANG_CHECK(base.layout->findTypeByName(input) == nullptr);
    }

    // A valid generic type reference must still resolve through the same module-less lookup path,
    // confirming the guards do not perturb normal name-based lookup.
    SLANG_CHECK(base.layout->findTypeByName("MyGeneric<GlobalThing>") != nullptr);

    // The parser must use the session's language version, not a hard-coded default. `(int, float)`
    // parses differently by version, and only if the session version reaches the parser (the
    // comma-vs-tuple gate in parseAtomicExpr):
    //   - Legacy: the comma is the C-style comma operator, so the parenthesized expression's type
    //   is
    //     its last operand `float` — a named type, which findTypeByName resolves (non-null).
    //   - Slang 2026: the comma builds a tuple, an unnamed structural type that findTypeByName does
    //     not resolve by this spelling (null).
    // (If 2026 tuple-type reflection ever becomes resolvable, the 2026 assertion below would flip —
    // an expected behavioural shift, not a regression in this fix.)
    // One differential assertion suffices: every module-less version gate (the `volatile` and
    // `struct[...]` gates above included) reads the version through the same
    // `Parser::getCurrentLanguageVersion()`, so this pins the shared threading mechanism. The other
    // gates only emit diagnostics — which `findTypeByName` discards — so they are not observable as
    // a resolve/no-resolve difference through this public entry point.
    Reflection legacy;
    makeReflection(SLANG_LANGUAGE_VERSION_LEGACY, legacy);
    SLANG_CHECK(legacy.layout->findTypeByName("(int, float)") != nullptr);

    Reflection v2026;
    makeReflection(SLANG_LANGUAGE_VERSION_2026, v2026);
    SLANG_CHECK(v2026.layout->findTypeByName("(int, float)") == nullptr);
}

// Changing a linkage's `-std` after reflection has started must not reuse a type cached under the
// previous language version. `getTypeFromString` caches by type string, and the parse is
// version-sensitive (`(int, float)` is a comma expression whose type is `float` in legacy, but a
// tuple in Slang 2026), so the cache key includes the language version. This drives the same string
// through one linkage across an in-place `-std` change to exercise that key.
SLANG_UNIT_TEST(reflectionTypeCacheHonorsLanguageVersion)
{
    auto session = spCreateSession();
    auto request = spCreateCompileRequest(session);

    spAddCodeGenTarget(request, SLANG_HLSL);
    int translationUnitIndex =
        spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "typeCacheVersion");
    spAddTranslationUnitSourceString(
        request,
        translationUnitIndex,
        "type-cache-version.slang",
        "int unused;");
    SLANG_CHECK_ABORT(spCompile(request) == SLANG_OK);

    auto layout = slang::ShaderReflection::get(request);
    SLANG_CHECK_ABORT(layout != nullptr);

    // Under legacy, `(int, float)` is a comma expression whose type is its last operand `float` — a
    // named type that resolves and is cached.
    SLANG_CHECK(layout->findTypeByName("(int, float)") != nullptr);

    // Switching to Slang 2026 makes `(int, float)` a tuple, an unnamed structural type that does
    // not resolve by this spelling. With a version-blind cache key the stale `float` from the
    // legacy lookup would be returned here instead of null.
    const char* slang2026Args[] = {"-std", "2026"};
    SLANG_CHECK_ABORT(
        spProcessCommandLineArguments(request, slang2026Args, SLANG_COUNT_OF(slang2026Args)) ==
        SLANG_OK);
    SLANG_CHECK(layout->findTypeByName("(int, float)") == nullptr);

    spDestroyCompileRequest(request);
    spDestroySession(session);
}

// The same rule holds for `findDeclFromString`/`m_decls`: a lookup *name* can carry
// version-sensitive grammar in a generic argument. `pick<(int, float)>` names `pick<float>` in
// legacy, but in Slang 2026 `(int, float)` parses as a tuple, so the same spelling no longer
// resolves to that function. This drives the same generic-function name through one linkage across
// an in-place `-std` change and confirms the specialization is not reused from the previous
// version.
SLANG_UNIT_TEST(reflectionDeclCacheHonorsLanguageVersion)
{
    auto session = spCreateSession();
    auto request = spCreateCompileRequest(session);

    spAddCodeGenTarget(request, SLANG_HLSL);
    int translationUnitIndex =
        spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "declCacheVersion");
    spAddTranslationUnitSourceString(
        request,
        translationUnitIndex,
        "decl-cache-version.slang",
        "T pick<T>(T x) { return x; }");
    SLANG_CHECK_ABORT(spCompile(request) == SLANG_OK);

    auto layout = slang::ShaderReflection::get(request);
    SLANG_CHECK_ABORT(layout != nullptr);

    // Legacy: the generic argument `(int, float)` is a comma expression of type `float`, so this
    // names `pick<float>`, which resolves and is cached.
    SLANG_CHECK(layout->findFunctionByName("pick<(int, float)>") != nullptr);

    // Slang 2026: `(int, float)` is a tuple rather than `float`, so the same spelling does not
    // resolve to `pick<float>`. With a version-blind `m_decls` key the stale `pick<float>` from the
    // legacy lookup would be returned here instead of null.
    const char* slang2026Args[] = {"-std", "2026"};
    SLANG_CHECK_ABORT(
        spProcessCommandLineArguments(request, slang2026Args, SLANG_COUNT_OF(slang2026Args)) ==
        SLANG_OK);
    SLANG_CHECK(layout->findFunctionByName("pick<(int, float)>") == nullptr);

    spDestroyCompileRequest(request);
    spDestroySession(session);
}
