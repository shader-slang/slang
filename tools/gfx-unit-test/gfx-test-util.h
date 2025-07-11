#pragma once

#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "core/slang-render-api-util.h"
#include "core/slang-test-tool-util.h"
#include "slang-rhi.h"
#include "span.h"
#include "unit-test/slang-unit-test.h"

// GFX_CHECK_CALL and GFX_CHECK_CALL_ABORT are used to check SlangResult
#define GFX_CHECK_CALL(x) SLANG_CHECK(!SLANG_FAILED(x))
#define GFX_CHECK_CALL_ABORT(x) SLANG_CHECK_ABORT(!SLANG_FAILED(x))

using namespace rhi;

namespace gfx_test
{
enum class PrecompilationMode
{
    None,
    SlangIR,
    InternalLink,
    ExternalLink,
};
/// Helper function for print out diagnostic messages output by Slang compiler.
void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob);

/// Loads a compute shader module and produces a `rhi::IShaderProgram`.
Slang::Result loadComputeProgram(
    rhi::IDevice* device,
    Slang::ComPtr<rhi::IShaderProgram>& outShaderProgram,
    const char* shaderModuleName,
    const char* entryPointName,
    slang::ProgramLayout*& slangReflection);

Slang::Result loadComputeProgram(
    rhi::IDevice* device,
    slang::ISession* slangSession,
    Slang::ComPtr<rhi::IShaderProgram>& outShaderProgram,
    const char* shaderModuleName,
    const char* entryPointName,
    slang::ProgramLayout*& slangReflection);

Slang::Result loadComputeProgramFromSource(
    rhi::IDevice* device,
    Slang::ComPtr<rhi::IShaderProgram>& outShaderProgram,
    std::string_view source);

Slang::Result loadGraphicsProgram(
    rhi::IDevice* device,
    Slang::ComPtr<rhi::IShaderProgram>& outShaderProgram,
    const char* shaderModuleName,
    const char* vertexEntryPointName,
    const char* fragmentEntryPointName,
    slang::ProgramLayout*& slangReflection);

template<typename T>
void compareResultFuzzy(const T* result, const T* expectedResult, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        SLANG_CHECK(abs(result[i] - expectedResult[i]) < 0.01f);
    }
}

template<typename T>
void compareResult(const T* result, const T* expectedResult, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        SLANG_CHECK(result[i] == expectedResult[i]);
    }
}

template<typename T>
void compareComputeResult(rhi::IDevice* device, rhi::IBuffer* buffer, span<T> expectedResult)
{
    size_t bufferSize = expectedResult.size() * sizeof(T);
    // Read back the results.`
    ComPtr<ISlangBlob> bufferData;
    SLANG_CHECK(SLANG_SUCCEEDED(device->readBuffer(buffer, 0, bufferSize, bufferData.writeRef())));
    SLANG_CHECK(bufferData->getBufferSize() == bufferSize);
    const T* result = reinterpret_cast<const T*>(bufferData->getBufferPointer());

    if constexpr (std::is_same<T, float>::value || std::is_same<T, double>::value)
        compareResultFuzzy(result, expectedResult.data(), expectedResult.size());
    else
        compareResult<T>(result, expectedResult.data(), expectedResult.size());
}

template<typename T, size_t Count>
void compareComputeResult(
    rhi::IDevice* device,
    rhi::IBuffer* buffer,
    std::array<T, Count> expectedResult)
{
    compareComputeResult(device, buffer, span<T>(expectedResult.data(), Count));
}

template<typename T>
void compareComputeResult(
    rhi::IDevice* device,
    rhi::ITexture* texture,
    uint32_t layer,
    uint32_t mip,
    span<T> expectedResult)
{
    size_t bufferSize = expectedResult.size() * sizeof(T);
    // Read back the results.
    ComPtr<ISlangBlob> textureData;
    rhi::SubresourceLayout layout;
    SLANG_CHECK(
        SLANG_SUCCEEDED(device->readTexture(texture, layer, mip, textureData.writeRef(), &layout)));
    SLANG_CHECK(textureData->getBufferSize() >= bufferSize);

    uint8_t* buffer = (uint8_t*)textureData->getBufferPointer();
    for (uint32_t z = 0; z < layout.size.depth; z++)
    {
        for (uint32_t y = 0; y < layout.size.height; y++)
        {
            for (uint32_t x = 0; x < layout.size.width; x++)
            {
                const uint8_t* src = reinterpret_cast<const uint8_t*>(
                    buffer + z * layout.slicePitch + y * layout.rowPitch + x * layout.colPitch);
                uint8_t* dst = reinterpret_cast<uint8_t*>(
                    buffer +
                    (((z * layout.size.depth + y) * layout.size.width) + x) * layout.colPitch);
                ::memcpy(dst, src, layout.colPitch);
            }
        }
    }

    const T* result = reinterpret_cast<const T*>(textureData->getBufferPointer());

    if constexpr (std::is_same<T, float>::value)
        compareResultFuzzy(result, expectedResult.data(), expectedResult.size());
    else
        compareResult<T>(result, expectedResult.data(), expectedResult.size());
}

template<typename T, size_t Count>
void compareComputeResult(
    rhi::IDevice* device,
    rhi::ITexture* texture,
    uint32_t layer,
    uint32_t mip,
    std::array<T, Count> expectedResult)
{
    compareComputeResult(device, texture, layer, mip, span<T>(expectedResult.data(), Count));
}

Slang::ComPtr<rhi::IDevice> createTestingDevice(
    UnitTestContext* context,
    rhi::DeviceType deviceType,
    Slang::List<const char*> additionalSearchPaths = {});

Slang::List<const char*> getSlangSearchPaths();

void initializeRenderDoc();
void renderDocBeginFrame();
void renderDocEndFrame();

template<typename T, typename... Args>
auto makeArray(Args... args)
{
    return std::array<T, sizeof...(Args)>{static_cast<T>(args)...};
}

inline bool deviceTypeInEnabledApis(rhi::DeviceType deviceType, Slang::RenderApiFlags enabledApis)
{
    switch (deviceType)
    {
    case rhi::DeviceType::Default:
        return true;
    case rhi::DeviceType::CPU:
        return enabledApis & Slang::RenderApiFlag::CPU;
    case rhi::DeviceType::CUDA:
        return enabledApis & Slang::RenderApiFlag::CUDA;
    case rhi::DeviceType::Metal:
        return enabledApis & Slang::RenderApiFlag::Metal;
    case rhi::DeviceType::WGPU:
        return enabledApis & Slang::RenderApiFlag::WebGPU;
    case rhi::DeviceType::Vulkan:
        return enabledApis & Slang::RenderApiFlag::Vulkan;
    case rhi::DeviceType::D3D11:
        return enabledApis & Slang::RenderApiFlag::D3D11;
    case rhi::DeviceType::D3D12:
        return enabledApis & Slang::RenderApiFlag::D3D12;
    }
    return true;
}


template<typename ImplFunc>
void runTestImpl(
    const ImplFunc& f,
    UnitTestContext* context,
    rhi::DeviceType deviceType,
    Slang::List<const char*> searchPaths = {})
{
    if (!deviceTypeInEnabledApis(deviceType, context->enabledApis))
    {
        SLANG_IGNORE_TEST
    }

    auto device = createTestingDevice(context, deviceType, searchPaths);
    if (!device)
    {
        SLANG_IGNORE_TEST
    }
#if SLANG_WIN32
    // Skip d3d12 tests on x86 now since dxc doesn't function correctly there on Windows 11.
    if (rhi::DeviceType == rhi::DeviceType::D3D12)
    {
        SLANG_IGNORE_TEST
    }
#endif
    // Skip d3d11 tests when we don't have DXBC support as they're bound to
    // fail without a backend compiler
    if (deviceType == rhi::DeviceType::D3D11 && !SLANG_ENABLE_DXBC_SUPPORT)
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

} // namespace gfx_test
