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
        }

        public struct Impl : IBase
        {
            public void step(inout float f)
            {
                f += 1.0f;
            }
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
    if (func->isOverloaded())
    {
        List<slang::FunctionReflection*> candidates;
        for (uint32_t i = 0; i < func->getOverloadCount(); i++)
        {
            candidates.add(func->getOverload(i));
        }
        func = layout->tryResolveOverloadedFunction(
            (uint32_t)candidates.getCount(),
            candidates.getBuffer());
    }
    SLANG_CHECK_ABORT(!func->isOverloaded());

    SLANG_CHECK(String(func->getName()) == "step");
}
