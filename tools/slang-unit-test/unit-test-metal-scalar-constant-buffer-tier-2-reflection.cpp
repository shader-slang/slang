// unit-test-metal-scalar-constant-buffer-tier-2-reflection.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// A constant buffer that names `ScalarDataLayout` keeps natural layout when its
// enclosing struct is laid out with the Metal argument buffer tier 2 rules, and
// a default constant buffer next to it keeps the native MSL layout.
SLANG_UNIT_TEST(metalScalarConstantBufferTier2Reflection)
{
    const char* testSource = R"(
        struct Args
        {
            float3 A;
            float3 B;
        };

        struct Outer
        {
            ConstantBuffer<Args, ScalarDataLayout> cbScalar;
            ConstantBuffer<Args> cbDefault;
        };

        ParameterBlock<Outer> params;
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_METAL;
    targetDesc.profile = globalSession->findProfile("metal");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "test",
        "test.slang",
        testSource,
        diagnosticBlob.writeRef());
    if (diagnosticBlob)
        getTestReporter()->message(
            TestMessageType::Info,
            (const char*)diagnosticBlob->getBufferPointer());
    SLANG_CHECK_ABORT(module != nullptr);

    auto reflection = module->getLayout();
    auto outerType = reflection->findTypeByName("Outer");
    SLANG_CHECK_ABORT(outerType != nullptr);
    auto outerLayout =
        reflection->getTypeLayout(outerType, slang::LayoutRules::MetalArgumentBufferTier2);
    SLANG_CHECK_ABORT(outerLayout != nullptr);
    SLANG_CHECK_ABORT(outerLayout->getFieldCount() == 2);

    auto scalarArgs = outerLayout->getFieldByIndex(0)->getTypeLayout()->getElementTypeLayout();
    SLANG_CHECK_ABORT(scalarArgs != nullptr);
    SLANG_CHECK(scalarArgs->getFieldByIndex(0)->getOffset() == 0);
    SLANG_CHECK(scalarArgs->getFieldByIndex(1)->getOffset() == 12);
    SLANG_CHECK(scalarArgs->getFieldByIndex(1)->getTypeLayout()->getSize() == 12);

    auto defaultArgs = outerLayout->getFieldByIndex(1)->getTypeLayout()->getElementTypeLayout();
    SLANG_CHECK_ABORT(defaultArgs != nullptr);
    SLANG_CHECK(defaultArgs->getFieldByIndex(0)->getOffset() == 0);
    SLANG_CHECK(defaultArgs->getFieldByIndex(1)->getOffset() == 16);
    SLANG_CHECK(defaultArgs->getFieldByIndex(1)->getTypeLayout()->getSize() == 16);
}
