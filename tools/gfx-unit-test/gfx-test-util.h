#pragma once

#include "slang-gfx.h"
#include "source/core/slang-basic.h"

namespace gfx_test
{
        /// Helper function for print out diagnostic messages output by Slang compiler.
    void diagnoseIfNeeded(ISlangWriter* diagnosticWriter, slang::IBlob* diagnosticsBlob);

        /// Loads a compute shader module and produces a `gfx::IShaderProgram`.
    Slang::Result loadShaderProgram(
        gfx::IDevice* device,
        Slang::ComPtr<gfx::IShaderProgram>& outShaderProgram,
        const char* shaderModuleName,
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

#define GFX_CHECK_CALL(x) SLANG_CHECK(!SLANG_FAILED(x))
#define GFX_CHECK_CALL_ABORT(x) SLANG_CHECK_ABORT(!SLANG_FAILED(x))

}
