// unit-test-get-layout-after-error.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// `IComponentType::getLayout` must fail on every call for a program whose layout has errors. The
// layout used to be cached even when generating it had reported errors, so a second call returned
// the cached layout together with an empty diagnostics blob, as if the program were valid.

static bool blobHasText(slang::IBlob* blob)
{
    return blob && blob->getBufferSize() != 0;
}

SLANG_UNIT_TEST(getLayoutAfterError)
{
    // Both parameters land on Vulkan binding 0 of space 0 once `-fvk-t-shift 0 all` is applied,
    // which is a layout-time error.
    const char* userSourceBody = R"(
        struct Data { float a; };
        [[vk::binding(0, 0)]] ConstantBuffer<Data> e;
        Texture2D x : register(t0);

        [shader("fragment")]
        float4 main() : SV_TARGET { return float4(1, 1, 1, 0); }
        )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_GLSL;
    targetDesc.profile = globalSession->findProfile("glsl_450");

    slang::CompilerOptionEntry option = {};
    option.name = slang::CompilerOptionName::VulkanBindShiftAll;
    option.value.kind = slang::CompilerOptionValueKind::Int;
    option.value.intValue0 = 2; // HLSLToVulkanLayoutOptions::Kind::ShaderResource
    option.value.intValue1 = 0; // shift 0

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = &option;
    sessionDesc.compilerOptionEntryCount = 1;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module =
        session->loadModuleFromSourceString("m", "m.slang", userSourceBody, diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(module->findEntryPointByName("main", entryPoint.writeRef()) == SLANG_OK);

    slang::IComponentType* components[] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> composite;
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(
            components,
            2,
            composite.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);
    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK_ABORT(composite->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    // The first call reports the error. Later calls must still fail instead of returning a layout
    // for the invalid program.
    for (int call = 0; call < 3; ++call)
    {
        ComPtr<slang::IBlob> layoutDiagnostics;
        auto layout = linked->getLayout(0, layoutDiagnostics.writeRef());
        SLANG_CHECK(layout == nullptr);
        if (call == 0)
            SLANG_CHECK(blobHasText(layoutDiagnostics));
    }
}
