// unit-test-generic-entrypoint-layout.cpp

// Test that getLayout() does not crash on generic compute shader entry points
// whose parameters reference the generic value parameter.
// Regression test for shader-slang/slangpy#788.

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(genericEntryPointLayout)
{
    const char* userSourceBody = R"(
        struct MyData<int N>
        {
            float value;
        }

        [shader("compute")]
        [numthreads(8, 1, 1)]
        void my_compute<int N>(
            uint3 tid : SV_DispatchThreadID,
            RWStructuredBuffer<MyData<N>> output)
        {
            MyData<N> d;
            d.value = float(tid.x);
            output[tid.x] = d;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    // 1. getLayout() on the unspecialized entry point must not crash.
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        module->findEntryPointByName("my_compute", entryPoint.writeRef());
        SLANG_CHECK_ABORT(entryPoint != nullptr);

        auto layout = entryPoint->getLayout();
        SLANG_CHECK(layout != nullptr);
    }

    // 2. getLayout() via getDefinedEntryPoint must not crash.
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        module->getDefinedEntryPoint(0, entryPoint.writeRef());
        SLANG_CHECK_ABORT(entryPoint != nullptr);

        auto layout = entryPoint->getLayout();
        SLANG_CHECK(layout != nullptr);
    }

    // 3. getLayout() on a specialized entry point produced via specialize() must not crash.
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        module->findEntryPointByName("my_compute", entryPoint.writeRef());
        SLANG_CHECK_ABORT(entryPoint != nullptr);

        ComPtr<slang::IComponentType> specializedEntryPoint;
        slang::SpecializationArg args[] = {slang::SpecializationArg::fromExpr("4")};
        SLANG_CHECK_ABORT(
            entryPoint->specialize(
                args,
                1,
                specializedEntryPoint.writeRef(),
                diagnosticBlob.writeRef()) == SLANG_OK);
        SLANG_CHECK_ABORT(specializedEntryPoint != nullptr);

        auto layout = specializedEntryPoint->getLayout();
        SLANG_CHECK(layout != nullptr);

        auto entryPointLayout = layout->getEntryPointByIndex(0);
        SLANG_CHECK(entryPointLayout != nullptr);
    }

    // 4. The specialized version via findAndCheckEntryPoint should still work end-to-end.
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        module->findAndCheckEntryPoint(
            "my_compute<4>",
            SLANG_STAGE_COMPUTE,
            entryPoint.writeRef(),
            diagnosticBlob.writeRef());
        SLANG_CHECK_ABORT(entryPoint != nullptr);

        auto layout = entryPoint->getLayout();
        SLANG_CHECK(layout != nullptr);

        auto entryPointLayout = layout->getEntryPointByIndex(0);
        SLANG_CHECK(entryPointLayout != nullptr);
    }
}
