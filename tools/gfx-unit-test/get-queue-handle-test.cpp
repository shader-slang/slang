#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

using namespace gfx;

namespace gfx_test
{
    void getQueueHandleTestImpl(IDevice* device, UnitTestContext* context)
    {
        ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
        auto queue = device->createCommandQueue(queueDesc);
        ICommandQueue::NativeHandle handle;
        GFX_CHECK_CALL_ABORT(queue->getNativeHandle(&handle));
        if (device->getDeviceInfo().deviceType == gfx::DeviceType::Vulkan)
        {
        }
        else
        {
        }
    }

    void getQueueHandleTestAPI(UnitTestContext* context, Slang::RenderApiFlag::Enum api)
    {
        if ((api & context->enabledApis) == 0)
        {
            return SLANG_IGNORE_TEST;
        }
        Slang::ComPtr<IDevice> device;
        IDevice::Desc deviceDesc = {};
        switch (api)
        {
        case Slang::RenderApiFlag::D3D11:
            deviceDesc.deviceType = gfx::DeviceType::DirectX11;
            break;
        case Slang::RenderApiFlag::D3D12:
            deviceDesc.deviceType = gfx::DeviceType::DirectX12;
            break;
        case Slang::RenderApiFlag::Vulkan:
            deviceDesc.deviceType = gfx::DeviceType::Vulkan;
            break;
        default:
            return SLANG_IGNORE_TEST;
        }
        deviceDesc.slang.slangGlobalSession = context->slangGlobalSession;
        const char* searchPaths[] = { "", "../../tools/gfx-unit-test", "tools/gfx-unit-test" };
        deviceDesc.slang.searchPathCount = (SlangInt)SLANG_COUNT_OF(searchPaths);
        deviceDesc.slang.searchPaths = searchPaths;
        auto createDeviceResult = gfxCreateDevice(&deviceDesc, device.writeRef());
        if (SLANG_FAILED(createDeviceResult))
        {
            return SLANG_IGNORE_TEST;
        }

        getQueueHandleTestImpl(device, context);
    }

    SLANG_UNIT_TEST(getQueueHandleD3D12)
    {
        return getQueueHandleTestAPI(unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(getQueueHandleVulkan)
    {
        return getQueueHandleTestAPI(unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

}
