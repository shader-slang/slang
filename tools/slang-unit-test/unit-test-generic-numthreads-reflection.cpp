// unit-test-generic-numthreads-reflection.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test that `getComputeThreadGroupSize` works on compute shader entrypoints that has
// generic/unknown workgroup sizes without crashing.

SLANG_UNIT_TEST(genericNumThreadsReflection)
{
    const char* userSourceBody = R"(
        [SpecializationConstant]
        const uint THREADGROUP_SIZE_X = 8;
        static const uint THREADGROUP_SIZE_Y = 8;

        [shader("compute")]
        [numthreads(THREADGROUP_SIZE_X, THREADGROUP_SIZE_Y+1, z+2)]
        void computeMain<uint8_t z>()
        {
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

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain<3>",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    SlangUInt sizes[3];
    entryPoint->getLayout()->getEntryPointByIndex(0)->getComputeThreadGroupSize(3, sizes);
    SLANG_CHECK(sizes[0] == 0);
    SLANG_CHECK(sizes[1] == 9);
    SLANG_CHECK(sizes[2] == 5);

    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);
    entryPoint->getLayout()->getEntryPointByIndex(0)->getComputeThreadGroupSize(3, sizes);
    SLANG_CHECK(sizes[0] == 0);
    SLANG_CHECK(sizes[1] == 9);
    SLANG_CHECK(sizes[2] == 0);
}
