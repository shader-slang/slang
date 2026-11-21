// unit-test-type-nesting-exception-boundary.cpp

#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// A type nested deeper than `kMaxTypeNestingDepth` raises fatal diagnostic 39997
// (`maximum-type-nesting-level-exceeded`), which throws `AbortCompilationException` from inside
// layout or entry-point checking. The public methods tested here are `SLANG_NO_THROW`, so each must
// report that failure through its result and diagnostics blob instead of letting the exception
// cross the API boundary and terminate the host process.
//
// Gated on `SLANG_HAS_EXCEPTIONS`: with exceptions disabled a fatal diagnostic terminates the
// process in `handleSignal` instead of throwing, so there is no exception for the boundary to
// contain.
#if SLANG_HAS_EXCEPTIONS

// The chain is well past `kMaxTypeNestingDepth` (128, `slang-check.h`) so that the tests do not
// depend on its exact value.
static const int kStructChainDepth = 200;

// Return Slang source declaring `struct S0 { S1 inner; }` through `struct S<depth> { uint x; }`,
// so that `S0` is the outermost of `depth + 1` nested types.
static String makeStructChain(int depth)
{
    StringBuilder sb;
    sb << "struct S" << depth << " { uint x; }\n";
    for (int i = depth - 1; i >= 0; --i)
        sb << "struct S" << i << " { S" << (i + 1) << " inner; }\n";
    return sb.produceString();
}

static bool blobContains(slang::IBlob* blob, const char* text)
{
    if (!blob)
        return false;
    UnownedStringSlice contents((const char*)blob->getBufferPointer(), blob->getBufferSize());
    return contents.indexOf(UnownedStringSlice(text)) != -1;
}

static ComPtr<slang::ISession> createSession(
    slang::IGlobalSession* globalSession,
    SlangCompileTarget format)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = format;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    globalSession->createSession(sessionDesc, session.writeRef());
    return session;
}

SLANG_UNIT_TEST(getLayoutContainsTypeNestingAbort)
{
    ComPtr<slang::ISession> session =
        createSession(unitTestContext->slangGlobalSession, SLANG_SPIRV);
    SLANG_CHECK_ABORT(session != nullptr);

    // Only layout walks a `StructuredBuffer` element type, so the module itself still loads.
    String source = makeStructChain(kStructChainDepth) + "StructuredBuffer<S0> buffer;\n";
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "nestedLayout",
        "nestedLayout.slang",
        source.getBuffer(),
        diagnostics.writeRef()));
    SLANG_CHECK_ABORT(module != nullptr);

    // An abort caches neither a layout nor the layout's error state, so each later call runs layout
    // again and reports 39997 again.
    for (int call = 0; call < 2; ++call)
    {
        ComPtr<slang::IBlob> layoutDiagnostics;
        SLANG_CHECK(module->getLayout(0, layoutDiagnostics.writeRef()) == nullptr);
        SLANG_CHECK(blobContains(layoutDiagnostics, "39997"));
    }
    SLANG_CHECK(module->getLayout(0, nullptr) == nullptr);

    // The session stays usable after the abort.
    ComPtr<slang::IModule> shallowModule(session->loadModuleFromSourceString(
        "shallowLayout",
        "shallowLayout.slang",
        "StructuredBuffer<uint> buffer;\n",
        diagnostics.writeRef()));
    SLANG_CHECK_ABORT(shallowModule != nullptr);
    SLANG_CHECK(shallowModule->getLayout(0, nullptr) != nullptr);
}

SLANG_UNIT_TEST(findAndCheckEntryPointContainsTypeNestingAbort)
{
    ComPtr<slang::ISession> session =
        createSession(unitTestContext->slangGlobalSession, SLANG_SPIRV);
    SLANG_CHECK_ABORT(session != nullptr);

    // Neither function has a `[shader]` or `[numthreads]` attribute, so neither is discovered as an
    // entry point at load, and both lookups below go through `findAndValidateEntryPoint`.
    String source = makeStructChain(kStructChainDepth) +
                    "void deep(S0 value, uint3 id : SV_DispatchThreadID) {}\n"
                    "void shallow(uint3 id : SV_DispatchThreadID) {}\n";
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "nestedEntryPoint",
        "nestedEntryPoint.slang",
        source.getBuffer(),
        diagnostics.writeRef()));
    SLANG_CHECK_ABORT(module != nullptr);
    SLANG_CHECK_ABORT(module->getDefinedEntryPointCount() == 0);

    ComPtr<slang::IEntryPoint> entryPoint;
    ComPtr<slang::IBlob> entryPointDiagnostics;
    SlangResult result = module->findAndCheckEntryPoint(
        "deep",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        entryPointDiagnostics.writeRef());
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(entryPoint == nullptr);
    SLANG_CHECK(blobContains(entryPointDiagnostics, "39997"));

    // The aborted check registers no entry point, so a second call without a diagnostics blob
    // fails the same way.
    SLANG_CHECK(SLANG_FAILED(
        module
            ->findAndCheckEntryPoint("deep", SLANG_STAGE_COMPUTE, entryPoint.writeRef(), nullptr)));
    SLANG_CHECK(entryPoint == nullptr);

    // The module stays usable after the abort.
    ComPtr<slang::IEntryPoint> shallowEntryPoint;
    SLANG_CHECK(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "shallow",
        SLANG_STAGE_COMPUTE,
        shallowEntryPoint.writeRef(),
        nullptr)));
    SLANG_CHECK(shallowEntryPoint != nullptr);
}

SLANG_UNIT_TEST(getTargetHostCallableContainsTypeNestingAbort)
{
    ComPtr<slang::ISession> session =
        createSession(unitTestContext->slangGlobalSession, SLANG_SHADER_HOST_CALLABLE);
    SLANG_CHECK_ABORT(session != nullptr);

    // Layout of `buffer` runs before any downstream C++ compiler is invoked, so the test does not
    // need one.
    String source = makeStructChain(kStructChainDepth) + "StructuredBuffer<S0> buffer;\n";
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "nestedHostCallable",
        "nestedHostCallable.slang",
        source.getBuffer(),
        diagnostics.writeRef()));
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK_ABORT(module->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType2> linked2;
    SLANG_CHECK_ABORT(linked->queryInterface(SLANG_IID_PPV_ARGS(linked2.writeRef())) == SLANG_OK);

    ComPtr<ISlangSharedLibrary> library;
    ComPtr<slang::IBlob> callableDiagnostics;
    SlangResult result =
        linked2->getTargetHostCallable(0, library.writeRef(), callableDiagnostics.writeRef());
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(library == nullptr);
    SLANG_CHECK(blobContains(callableDiagnostics, "39997"));
}

#endif // SLANG_HAS_EXCEPTIONS
