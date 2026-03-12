// unit-test-reflection-global-param-in-namespace.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Test that the findFieldByName recognizes a fully qualified name to disambiguate entries in
// namespaces.

SLANG_UNIT_TEST(reflectionGlobalParamInNamespace)
{
    const char* userSourceBody = R"(
        namespace NS {
           uniform int gParam;
        }
        uniform int4 gParam;

        [numthreads(1,1,1)] void computeMain() {}
        )";
    String userSource = userSourceBody;
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("sm_5_0");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    auto reflection = module->getLayout();
    auto globalVarTypeLayout =
        reflection->getGlobalParamsVarLayout()->getTypeLayout()->getElementTypeLayout();
    auto field0 = globalVarTypeLayout->findFieldIndexByName("gParam");
    SLANG_CHECK_ABORT(field0 != -1);
    SLANG_CHECK(globalVarTypeLayout->getFieldByIndex(field0)->getTypeLayout()->getSize() == 16);

    auto field1 = globalVarTypeLayout->findFieldIndexByName("NS::gParam");
    SLANG_CHECK_ABORT(field1 != -1 && field1 != field0);
    SLANG_CHECK(globalVarTypeLayout->getFieldByIndex(field1)->getTypeLayout()->getSize() == 4);

    auto field2 = globalVarTypeLayout->findFieldIndexByName("NS.gParam");
    SLANG_CHECK_ABORT(field1 == field2);
}
