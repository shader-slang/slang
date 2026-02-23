// unit-test-bindless-space-reflection.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test that the reflection API correctly returns the bindless space index.

SLANG_UNIT_TEST(bindlessSpaceReflection)
{
    const char* userSource = R"(
        RWStructuredBuffer<float> gInput;
        RWStructuredBuffer<float> gOutput;

        struct P
        {
            Texture2D.Handle t;
        };

        ParameterBlock<P> p1;

        [Shader("compute")]
        [NumThreads(4, 1, 1)]
        void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
        {
            uint tid = dispatchThreadID.x;
            gOutput[tid] = gInput[tid] + p1.t.Load(int3(0,0,0)).x;
        }
    )";

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
    auto module = session->loadModuleFromSourceString(
        "test",
        "test.slang",
        userSource,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(linkedProgram != nullptr);

    auto programLayout = linkedProgram->getLayout();
    SLANG_CHECK(programLayout != nullptr);

    // The shader has:
    // - gInput and gOutput at space 0 (default)
    // - ParameterBlock<P> p1 at space 1
    // So the bindless space should be allocated at space 2
    auto bindlessSpaceIndex = programLayout->getBindlessSpaceIndex();
    SLANG_CHECK(bindlessSpaceIndex == 2);
}
