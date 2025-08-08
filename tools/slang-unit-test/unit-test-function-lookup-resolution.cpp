// unit-test-function-lookup-resolution.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

static String getTypeFullName(slang::TypeReflection* type)
{
    ComPtr<ISlangBlob> blob;
    type->getFullName(blob.writeRef());
    return String((const char*)blob->getBufferPointer());
}

// Test that the reflection API provides correctly resolved lookup results.

SLANG_UNIT_TEST(functionLookupResolution)
{
    // Source for a module that contains an undecorated entrypoint.
    const char* userSourceBody = R"(
        public interface IBase
        {
            public void step(inout float f);
            public void method(int x) {}
        }

        public struct Impl : IBase
        {
            public void step(inout float f)
            {
                f += 1.0f;
            }
            public override void method(int x) {}
        }
        public extension<T : IBase> T {
            public void method(int x) {}
        }
        )";

    auto moduleName = "moduleG" + String(Process::getId());
    String userSource = "import " + moduleName + ";\n" + userSourceBody;
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

    auto layout = module->getLayout();
    auto type = layout->findTypeByName("Impl");
    SLANG_CHECK_ABORT(type != nullptr);

    auto func = layout->findFunctionByNameInType(type, "step");
    SLANG_CHECK_ABORT(func && !func->isOverloaded());


    auto func1 = layout->findFunctionByNameInType(type, "method");
    SLANG_CHECK_ABORT(func1->isOverloaded());
    SLANG_CHECK(func1->getOverloadCount() == 3);
    // Test that overloaded function containers return the correct name
    SLANG_CHECK(func1->getName() != nullptr);
    SLANG_CHECK(String(func1->getName()) == "method");
    if (func1->isOverloaded())
    {
        List<slang::FunctionReflection*> candidates;
        for (uint32_t i = 0; i < func1->getOverloadCount(); i++)
        {
            candidates.add(func1->getOverload(i));
        }
        func1 = layout->tryResolveOverloadedFunction(
            (uint32_t)candidates.getCount(),
            candidates.getBuffer());
    }
    SLANG_CHECK(!func1->isOverloaded());
    SLANG_CHECK(String(func1->getName()) == "method");
}
