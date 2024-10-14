// unit-test-find-entrypoint-nested.cpp

#include "slang.h"

#include <stdio.h>
#include <stdlib.h>

#include "tools/unit-test/slang-unit-test.h"
#include "slang-com-ptr.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"

using namespace Slang;

// Test that the IModule::findAndCheckEntryPoint API works with modules that
// defines two entrypoints, where one entrypoint calls the other.

SLANG_UNIT_TEST(findEntryPointNested)
{
    // Source for a module that contains an undecorated entrypoint.
    const char* userSourceBody = R"(
        [shader("raygeneration")]
        void inner()
        {
        }
        [shader("raygeneration")]
        void outer()
        {
            inner();
        }
        )";

    auto moduleName = "moduleG" + String(Process::getId());
    String userSource = "import " + moduleName + ";\n" + userSourceBody;
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString("m", "m.slang", userSourceBody, diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint("outer", SLANG_STAGE_RAY_GENERATION, entryPoint.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = { module, entryPoint.get() };
    session->createCompositeComponentType(components, 2, compositeProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(linkedProgram != nullptr);

    ComPtr<slang::IBlob> code;
    linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(code != nullptr);
    SLANG_CHECK(code->getBufferSize() != 0);
}

