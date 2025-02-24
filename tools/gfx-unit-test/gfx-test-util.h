#pragma once

#include "core/slang-basic.h"
#include "core/slang-render-api-util.h"
#include "slang-gfx.h"
#include "unit-test/slang-unit-test.h"

namespace gfx_test
{
/// Helper function for print out diagnostic messages output by Slang compiler.
void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob);

/// Loads a compute shader module and produces a `gfx::IShaderProgram`.
Slang::Result loadComputeProgram(
    gfx::IDevice* device,
    Slang::ComPtr<gfx::IShaderProgram>& outShaderProgram,
    const char* shaderModuleName,
    const char* entryPointName,
    slang::ProgramLayout*& slangReflection);

Slang::Result loadComputeProgram(
    gfx::IDevice* device,
    slang::ISession* slangSession,
    Slang::ComPtr<gfx::IShaderProgram>& outShaderProgram,
    const char* shaderModuleName,
    const char* entryPointName,
    slang::ProgramLayout*& slangReflection);

Slang::Result loadComputeProgramFromSource(
    gfx::IDevice* device,
    Slang::ComPtr<gfx::IShaderProgram>& outShaderProgram,
    Slang::String source);

Slang::Result loadGraphicsProgram(
    gfx::IDevice* device,
    Slang::ComPtr<gfx::IShaderProgram>& outShaderProgram,
    const char* shaderModuleName,
    const char* vertexEntryPointName,
    const char* fragmentEntryPointName,
    slang::ProgramLayout*& slangReflection);

/// Reads back the content of `buffer` and compares it against `expectedResult`.
void compareComputeResult(
    gfx::IDevice* device,
    gfx::IBufferResource* buffer,
    size_t offset,
    const void* expectedResult,
    size_t expectedBufferSize);

/// Reads back the content of `texture` and compares it against `expectedResult`.
void compareComputeResult(
    gfx::IDevice* device,
    gfx::ITextureResource* texture,
    gfx::ResourceState state,
    void* expectedResult,
    size_t expectedResultRowPitch,
    size_t rowCount);

void compareComputeResultFuzzy(
    const float* result,
    float* expectedResult,
    size_t expectedBufferSize);

/// Reads back the content of `buffer` and compares it against `expectedResult` with a set
/// tolerance.
void compareComputeResultFuzzy(
    gfx::IDevice* device,
    gfx::IBufferResource* buffer,
    float* expectedResult,
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
    if (std::is_same<T, float>::value)
        return compareComputeResultFuzzy(
            device,
            buffer,
            (float*)expectedBuffer.getBuffer(),
            bufferSize);
    return compareComputeResult(device, buffer, 0, expectedBuffer.getBuffer(), bufferSize);
}

Slang::ComPtr<gfx::IDevice> createTestingDevice(
    UnitTestContext* context,
    Slang::RenderApiFlag::Enum api,
    Slang::List<const char*> additionalSearchPaths = {},
    gfx::IDevice::ShaderCacheDesc shaderCache = {});

Slang::List<const char*> getSlangSearchPaths();

void initializeRenderDoc();
void renderDocBeginFrame();
void renderDocEndFrame();

template<typename ImplFunc>
void runTestImpl(
    const ImplFunc& f,
    UnitTestContext* context,
    Slang::RenderApiFlag::Enum api,
    Slang::List<const char*> searchPaths = {},
    gfx::IDevice::ShaderCacheDesc shaderCache = {})
{
    if ((api & context->enabledApis) == 0)
    {
        SLANG_IGNORE_TEST
    }
    printf("File, Line: %s, %d\n", __FILE__, __LINE__);
    auto device = createTestingDevice(context, api, searchPaths, shaderCache);
    printf("File, Line: %s, %d\n", __FILE__, __LINE__);
    if (!device)
    {
        printf("File, Line: %s, %d\n", __FILE__, __LINE__);
        SLANG_IGNORE_TEST
    }
#if SLANG_WIN32
    // Skip d3d12 tests on x86 now since dxc doesn't function correctly there on Windows 11.
    if (api == Slang::RenderApiFlag::D3D12)
    {
        SLANG_IGNORE_TEST
    }
#endif
    // Skip d3d11 tests when we don't have DXBC support as they're bound to
    // fail without a backend compiler
    printf("File, Line: %s, %d\n", __FILE__, __LINE__);
    if (api == Slang::RenderApiFlag::D3D11 && !SLANG_ENABLE_DXBC_SUPPORT)
    {
        printf("File, Line: %s, %d\n", __FILE__, __LINE__);
        SLANG_IGNORE_TEST
    }
    try
    {
        printf("File, Line: %s, %d\n", __FILE__, __LINE__);
        fflush(stdout);
        renderDocBeginFrame();
        f(device, context);
        printf("File, Line: %s, %d\n", __FILE__, __LINE__);
        fflush(stdout);
    }
    catch (AbortTestException& e)
    {
        printf("EXCEPTION! File, Line: %s, %d\n", __FILE__, __LINE__);
        fflush(stdout);
        renderDocEndFrame();
        throw e;
    }
    printf("Dun.. File, Line: %s, %d\n", __FILE__, __LINE__);
    fflush(stdout);
    renderDocEndFrame();
    printf("File, Line: %s, %d\n", __FILE__, __LINE__);
    fflush(stdout);
}

#define GFX_CHECK_CALL(x) SLANG_CHECK(!SLANG_FAILED(x))
#define GFX_CHECK_CALL_ABORT(x) SLANG_CHECK_ABORT(!SLANG_FAILED(x))

} // namespace gfx_test
