// unit-test-cuda-entry-point-param-size.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test that CUDA entry point parameter layout does not include trailing padding.

SLANG_UNIT_TEST(CudaEntryPointParamSizeWithoutPadding)
{
    // Shader with entry point parameters where the last parameter (uint) comes after
    // buffer handles with a larger size. Normally with a CUDA struct the layout would
    // have trailing padding to align to the larger size, but using that padding with
    // entry point parameters would cause cuLaunchKernel to fail.
    const char* userSourceBody = R"(
        [shader("compute")]
        [numthreads(4, 1, 1)]
        void computeMain(
            uint tid: SV_DispatchThreadID,
            uniform StructuredBuffer<float> input,
            uniform RWStructuredBuffer<float> output,
            uniform uint count)
        {
            if (tid >= count)
                return;
            output[tid] = input[tid];
        }
        )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_CUDA_SOURCE;
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
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), nullptr);
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    // Get program reflection
    auto programLayout = linkedProgram->getLayout();
    SLANG_CHECK_ABORT(programLayout != nullptr);

    // Get entry point reflection
    SLANG_CHECK_ABORT(programLayout->getEntryPointCount() == 1);
    auto entryPointLayout = programLayout->getEntryPointByIndex(0);
    SLANG_CHECK_ABORT(entryPointLayout != nullptr);

    // Get the entry point parameter type layout
    auto entryPointTypeLayout = entryPointLayout->getTypeLayout();
    SLANG_CHECK_ABORT(entryPointTypeLayout != nullptr);

    auto elementTypeLayout = entryPointTypeLayout->getElementTypeLayout();
    SLANG_CHECK_ABORT(elementTypeLayout != nullptr);

    // Get the size of the parameter struct (this is what CUDA kernel launch uses)
    size_t paramStructSize = elementTypeLayout->getSize();

    // The parameter struct should be 36 bytes:
    //   - input:  offset 0,  size 16 (CUDA buffer handle)
    //   - output: offset 16, size 16 (CUDA buffer handle)
    //   - count:  offset 32, size 4  (uint32_t)
    // Total: 36 bytes (no trailing padding)
    SLANG_CHECK(paramStructSize == 36);
}
