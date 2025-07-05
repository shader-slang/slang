#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

#include <slang-rhi.h>

#if SLANG_WINDOWS_FAMILY
#include <d3d12.h>
#endif

using namespace rhi;

namespace gfx_test
{
void getBufferResourceHandleTestImpl(IDevice* device, UnitTestContext* context)
{
    const int numberCount = 1;
    BufferDesc bufferDesc = {};
    bufferDesc.size = numberCount * sizeof(float);
    bufferDesc.format = rhi::Format::Undefined;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;
    bufferDesc.usage = BufferUsage::UnorderedAccess | BufferUsage::ShaderResource |
                       BufferUsage::CopySource | BufferUsage::CopyDestination;

    ComPtr<IBuffer> buffer;
    GFX_CHECK_CALL_ABORT(device->createBuffer(bufferDesc, nullptr, buffer.writeRef()));

    NativeHandle handle;
    GFX_CHECK_CALL_ABORT(buffer->getNativeHandle(&handle));
    if (device->getInfo().deviceType == rhi::DeviceType::Vulkan)
    {
        SLANG_CHECK(handle.value != 0);
    }
#if SLANG_WINDOWS_FAMILY
    else
    {
        auto d3d12Handle = (ID3D12Resource*)handle.value;
        Slang::ComPtr<IUnknown> testHandle1;
        GFX_CHECK_CALL_ABORT(d3d12Handle->QueryInterface<IUnknown>(testHandle1.writeRef()));
        Slang::ComPtr<ID3D12Resource> testHandle2;
        GFX_CHECK_CALL_ABORT(testHandle1->QueryInterface<ID3D12Resource>(testHandle2.writeRef()));
        SLANG_CHECK(d3d12Handle == testHandle2.get());
    }
#endif
}

void getBufferResourceHandleTestAPI(UnitTestContext* context, Slang::RenderApiFlag::Enum api)
{
    if (context->enableDebugLayers)
        getRHI()->enableDebugLayers();
    if ((api & context->enabledApis) == 0)
    {
        SLANG_IGNORE_TEST;
    }
    Slang::ComPtr<IDevice> device;
    DeviceDesc deviceDesc = {};
    switch (api)
    {
    case Slang::RenderApiFlag::D3D11:
        deviceDesc.deviceType = rhi::DeviceType::D3D11;
        break;
    case Slang::RenderApiFlag::D3D12:
        deviceDesc.deviceType = rhi::DeviceType::D3D12;
        break;
    case Slang::RenderApiFlag::Vulkan:
        deviceDesc.deviceType = rhi::DeviceType::Vulkan;
        break;
    default:
        SLANG_IGNORE_TEST;
    }
    deviceDesc.slang.slangGlobalSession = context->slangGlobalSession;
    const char* searchPaths[] = {"", "../../tools/gfx-unit-test", "tools/gfx-unit-test"};
    deviceDesc.slang.searchPathCount = (SlangInt)SLANG_COUNT_OF(searchPaths);
    deviceDesc.slang.searchPaths = searchPaths;
    auto createDeviceResult = getRHI()->createDevice(deviceDesc, device.writeRef());
    if (SLANG_FAILED(createDeviceResult))
    {
        SLANG_IGNORE_TEST;
    }
    // Ignore this test on swiftshader. Swiftshader seems to have a bug that causes the test
    // to crash.
    if (Slang::String(device->getInfo().adapterName).toLower().contains("swiftshader"))
    {
        SLANG_IGNORE_TEST;
    }

    getBufferResourceHandleTestImpl(device, context);
}

SLANG_UNIT_TEST(getBufferResourceHandleD3D12)
{
    return getBufferResourceHandleTestAPI(unitTestContext, Slang::RenderApiFlag::D3D12);
}

SLANG_UNIT_TEST(getBufferResourceHandleVulkan)
{
    return getBufferResourceHandleTestAPI(unitTestContext, Slang::RenderApiFlag::Vulkan);
}

} // namespace gfx_test
