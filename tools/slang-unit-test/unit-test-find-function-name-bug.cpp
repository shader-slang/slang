// unit-test-find-function-name-bug.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test case for issue where FunctionReflection.getName() returns NULL
// when found via findFunctionByNameInType
SLANG_UNIT_TEST(findFunctionNameBug)
{
    const char* testSource = R"(
        public interface IBase
        {
            public void step(inout float f);
        }

        public struct Impl : IBase
        {
            public void step(inout float f)
            {
                f += 1.0f;
            }
        }
    )";

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
        "testModule",
        "testModule.slang",
        testSource,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    // Find the Impl type
    auto implType = module->getLayout()->findTypeByName("Impl");
    SLANG_CHECK_ABORT(implType != nullptr);

    // Find the step function in the Impl type
    auto stepFunc = module->getLayout()->findFunctionByNameInType(implType, "step");
    SLANG_CHECK(stepFunc != nullptr);

    // Check if the function is overloaded or not
    if (stepFunc->isOverloaded())
    {
        // If overloaded, check the individual overloads
        SLANG_CHECK(stepFunc->getOverloadCount() >= 1);
        auto overload = stepFunc->getOverload(0);
        SLANG_CHECK(overload != nullptr);
        auto funcName = overload->getName();
        SLANG_CHECK(funcName != nullptr);
        SLANG_CHECK(strcmp(funcName, "step") == 0);
    }
    else
    {
        // If not overloaded, the base function should have a name
        auto funcName = stepFunc->getName();
        SLANG_CHECK(funcName != nullptr);
        SLANG_CHECK(strcmp(funcName, "step") == 0);
    }
    
    // Also test that the original problematic call now works
    // (This was the original bug report - calling getName() on the base function)
    auto funcName = stepFunc->getName();
    SLANG_CHECK(funcName != nullptr);
    SLANG_CHECK(strcmp(funcName, "step") == 0);
}