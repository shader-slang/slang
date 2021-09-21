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
        ISlangWriter* diagnosticWriter,
        const char* shaderModuleName,
        slang::ProgramLayout*& slangReflection);

        /// Reads back the content of `buffer` and compares it against `expectedResult`.
    Slang::Result _compareComputeResult(
        gfx::IDevice* device,
        gfx::IBufferResource* buffer,
        uint8_t* expectedResult,
        size_t expectedBufferSize);

    template<typename T, Slang::Index count>
    Slang::Result compareComputeResult(
        gfx::IDevice* device,
        gfx::IBufferResource* buffer,
        Slang::Array<T, count> expectedResult)
    {
        Slang::List<uint8_t> expectedBuffer;
        size_t bufferSize = sizeof(T) * count;
        expectedBuffer.setCount(bufferSize);
        memcpy(expectedBuffer.getBuffer(), expectedResult.begin(), bufferSize);
        return _compareComputeResult(device, buffer, expectedBuffer.getBuffer(), bufferSize);
    }
}
