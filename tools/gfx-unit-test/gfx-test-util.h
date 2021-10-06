#pragma once

#include "slang-gfx.h"
#include "source/core/slang-basic.h"
#include "source/core/slang-render-api-util.h"
#include "tools/unit-test/slang-unit-test.h"

namespace gfx_test
{
        /// Helper function for print out diagnostic messages output by Slang compiler.
    void diagnoseIfNeeded(ISlangWriter* diagnosticWriter, slang::IBlob* diagnosticsBlob);

        /// Loads a compute shader module and produces a `gfx::IShaderProgram`.
    Slang::Result loadComputeProgram(
        gfx::IDevice* device,
        Slang::ComPtr<gfx::IShaderProgram>& outShaderProgram,
        const char* shaderModuleName,
        const char* entryPointName,
        slang::ProgramLayout*& slangReflection);

        /// Reads back the content of `buffer` and compares it against `expectedResult`.
    void compareComputeResult(
        gfx::IDevice* device,
        gfx::IBufferResource* buffer,
        uint8_t* expectedResult,
        size_t expectedBufferSize);

    template<typename T, Slang::Index count>
    void compareComputeResult(
        gfx::IDevice* device,
        gfx::IBufferResource* buffer,
        Slang::Array<T, count> expectedResult)
    {
        Slang::List<uint8_t> expectedBuffer;
        size_t bufferSize = sizeof(T) * count;
        expectedBuffer.setCount(bufferSize);
        memcpy(expectedBuffer.getBuffer(), expectedResult.begin(), bufferSize);
        return compareComputeResult(device, buffer, expectedBuffer.getBuffer(), bufferSize);
    }
    
    Slang::ComPtr<gfx::IDevice> createTestingDevice(UnitTestContext* context, Slang::RenderApiFlag::Enum api);

    void initializeRenderDoc();
    void renderDocBeginFrame();
    void renderDocEndFrame();

    template<typename ImplFunc>
    void runTestImpl(const ImplFunc& f, UnitTestContext* context, Slang::RenderApiFlag::Enum api)
    {
        if ((api & context->enabledApis) == 0)
        {
            SLANG_IGNORE_TEST
        }
        auto device = createTestingDevice(context, api);
        if (!device)
        {
            SLANG_IGNORE_TEST
        }
        try
        {
            renderDocBeginFrame();
            f(device, context);
        }
        catch (AbortTestException& e)
        {
            renderDocEndFrame();
            throw e;
        }
        renderDocEndFrame();
    }

#define GFX_CHECK_CALL(x) SLANG_CHECK(!SLANG_FAILED(x))
#define GFX_CHECK_CALL_ABORT(x) SLANG_CHECK_ABORT(!SLANG_FAILED(x))

}
