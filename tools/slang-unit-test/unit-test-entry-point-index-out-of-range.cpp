#include "core/slang-io.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// An out-of-range `entryPointIndex` must be rejected at the public-API boundary with
// `SLANG_E_INVALID_ARG` and the out-of-range diagnostic, rather than passed into code generation.

static ComPtr<slang::ISession> createSessionForFormat(
    slang::IGlobalSession* globalSession,
    SlangCompileTarget format,
    const char* profileName)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = format;
    if (profileName)
        targetDesc.profile = globalSession->findProfile(profileName);
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
    return session;
}

static void loadModuleWithEntryPoint(
    slang::ISession* session,
    ComPtr<slang::IModule>& outModule,
    ComPtr<slang::IEntryPoint>& outEntryPoint)
{
    const char* userSourceBody = R"(
        RWStructuredBuffer<float> outputBuffer;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            outputBuffer[tid.x] = float(tid.x);
        }
        )";

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);
    outModule = module;

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(entryPoint != nullptr);
    outEntryPoint = entryPoint;
}

static ComPtr<slang::IComponentType> composeAndLink(
    slang::ISession* session,
    slang::IComponentType** components,
    SlangInt componentCount)
{
    ComPtr<slang::IBlob> diagnosticBlob;
    ComPtr<slang::IComponentType> composite;
    session->createCompositeComponentType(
        components,
        componentCount,
        composite.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(composite != nullptr);

    ComPtr<slang::IComponentType> linked;
    composite->link(linked.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(linked != nullptr);
    return linked;
}

static bool blobContains(slang::IBlob* blob, const char* needle)
{
    if (!blob || blob->getBufferSize() == 0)
        return false;
    UnownedStringSlice text(
        (const char*)blob->getBufferPointer(),
        (const char*)blob->getBufferPointer() + blob->getBufferSize());
    return text.indexOf(UnownedStringSlice(needle)) != -1;
}

// Identify the diagnostic by its error code (E38015) so the test cannot pass on an unrelated error,
// and confirm it carries the "out of range" wording that names the failure to the user.
static bool blobReportsIndexOutOfRange(slang::IBlob* blob)
{
    return blobContains(blob, "E38015") && blobContains(blob, "out of range");
}

static void checkAllApisRejectIndex(slang::IComponentType* linked, SlangInt entryPointIndex)
{
    {
        ComPtr<slang::IBlob> code, diagnostics;
        auto result =
            linked->getEntryPointCode(entryPointIndex, 0, code.writeRef(), diagnostics.writeRef());
        SLANG_CHECK(result == SLANG_E_INVALID_ARG);
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(blobReportsIndexOutOfRange(diagnostics));
    }
    {
        ComPtr<slang::IComponentType2> componentType2;
        linked->queryInterface(SLANG_IID_PPV_ARGS(componentType2.writeRef()));
        SLANG_CHECK(componentType2 != nullptr);
        ComPtr<slang::ICompileResult> compileResult;
        ComPtr<slang::IBlob> diagnostics;
        auto result = componentType2->getEntryPointCompileResult(
            entryPointIndex,
            0,
            compileResult.writeRef(),
            diagnostics.writeRef());
        SLANG_CHECK(result == SLANG_E_INVALID_ARG);
        SLANG_CHECK(compileResult == nullptr);
        SLANG_CHECK(blobReportsIndexOutOfRange(diagnostics));
    }
    {
        ComPtr<slang::IMetadata> metadata;
        ComPtr<slang::IBlob> diagnostics;
        auto result = linked->getEntryPointMetadata(
            entryPointIndex,
            0,
            metadata.writeRef(),
            diagnostics.writeRef());
        SLANG_CHECK(result == SLANG_E_INVALID_ARG);
        SLANG_CHECK(metadata == nullptr);
        SLANG_CHECK(blobReportsIndexOutOfRange(diagnostics));
    }
    {
        ComPtr<ISlangSharedLibrary> library;
        ComPtr<slang::IBlob> diagnostics;
        auto result = linked->getEntryPointHostCallable(
            (int)entryPointIndex,
            0,
            library.writeRef(),
            diagnostics.writeRef());
        SLANG_CHECK(result == SLANG_E_INVALID_ARG);
        SLANG_CHECK(library == nullptr);
        SLANG_CHECK(blobReportsIndexOutOfRange(diagnostics));
    }
}

SLANG_UNIT_TEST(entryPointIndexOutOfRange)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // Composite omits the entry point, so index 0 is out of range on a SPIR-V target.
    {
        auto session = createSessionForFormat(globalSession, SLANG_SPIRV, "spirv_1_5");
        ComPtr<slang::IModule> module;
        ComPtr<slang::IEntryPoint> entryPoint;
        loadModuleWithEntryPoint(session, module, entryPoint);

        slang::IComponentType* components[] = {module.get()};
        auto linked = composeAndLink(session, components, 1);

        checkAllApisRejectIndex(linked, 0);
    }

    // A source target (HLSL) reaches a different emit path than SPIR-V for the same
    // out-of-range index.
    {
        auto session = createSessionForFormat(globalSession, SLANG_HLSL, nullptr);
        ComPtr<slang::IModule> module;
        ComPtr<slang::IEntryPoint> entryPoint;
        loadModuleWithEntryPoint(session, module, entryPoint);

        slang::IComponentType* components[] = {module.get()};
        auto linked = composeAndLink(session, components, 1);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        auto result = linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
        SLANG_CHECK(result == SLANG_E_INVALID_ARG);
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(blobReportsIndexOutOfRange(diagnostics));
    }

    // Index 1 into a 1-entry-point program: the bound is the entry-point count, not emptiness.
    {
        auto session = createSessionForFormat(globalSession, SLANG_SPIRV, "spirv_1_5");
        ComPtr<slang::IModule> module;
        ComPtr<slang::IEntryPoint> entryPoint;
        loadModuleWithEntryPoint(session, module, entryPoint);

        slang::IComponentType* components[] = {module.get(), entryPoint.get()};
        auto linked = composeAndLink(session, components, 2);

        checkAllApisRejectIndex(linked, 1);
    }

    // A negative index takes the same diagnosed path as a too-large one.
    {
        auto session = createSessionForFormat(globalSession, SLANG_SPIRV, "spirv_1_5");
        ComPtr<slang::IModule> module;
        ComPtr<slang::IEntryPoint> entryPoint;
        loadModuleWithEntryPoint(session, module, entryPoint);

        slang::IComponentType* components[] = {module.get(), entryPoint.get()};
        auto linked = composeAndLink(session, components, 2);

        checkAllApisRejectIndex(linked, -1);
    }

    // Control: a valid index into a properly composed program still compiles.
    {
        auto session = createSessionForFormat(globalSession, SLANG_SPIRV, "spirv_1_5");
        ComPtr<slang::IModule> module;
        ComPtr<slang::IEntryPoint> entryPoint;
        loadModuleWithEntryPoint(session, module, entryPoint);

        slang::IComponentType* components[] = {module.get(), entryPoint.get()};
        auto linked = composeAndLink(session, components, 2);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        auto result = linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
        SLANG_CHECK(result == SLANG_OK);
        SLANG_CHECK(code != nullptr && code->getBufferSize() != 0);
    }
}
