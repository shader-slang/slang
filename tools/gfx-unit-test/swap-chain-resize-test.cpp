#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "tools/platform/window.h"
#include "source/core/slang-basic.h"

using namespace gfx;
using namespace Slang;

namespace gfx_test
{
    struct SwapchainResizeTest
    {
        IDevice* device;
        UnitTestContext* context;

        RefPtr<platform::Window> window;
        ComPtr<ICommandQueue> queue;
        ComPtr<ISwapchain> swapchain;

        static const GfxCount width = 500;
        static const GfxCount height = 500;
        static const int kSwapchainImageCount = 2;

        void init(IDevice* device, UnitTestContext* context)
        {
            this->device = device;
            this->context = context;
        }

        void createRequiredResources()
        {
            platform::Application::init();

            platform::WindowDesc windowDesc;
            windowDesc.title = "";
            windowDesc.width = width;
            windowDesc.height = height;
            windowDesc.style = platform::WindowStyle::Default;
            window = platform::Application::createWindow(windowDesc);

            ICommandQueue::Desc queueDesc = {};
            queueDesc.type = ICommandQueue::QueueType::Graphics;
            queue = device->createCommandQueue(queueDesc);

            ISwapchain::Desc swapchainDesc = {};
            swapchainDesc.format = Format::R8G8B8A8_UNORM;
            swapchainDesc.width = width;
            swapchainDesc.height = height;
            swapchainDesc.imageCount = kSwapchainImageCount;
            swapchainDesc.queue = queue;
            WindowHandle windowHandle = window->getNativeHandle().convert<WindowHandle>();
            if (SLANG_FAILED(device->createSwapchain(swapchainDesc, windowHandle, swapchain.writeRef())))
                SLANG_IGNORE_TEST;
        }

        void run()
        {
            createRequiredResources();
            // TODO: Extend test by drawing for a few frames before and after resize to ensure swapchain remains usable
            GFX_CHECK_CALL(swapchain->resize(700, 700));
        }
    };

    void swapchainResizeTestImpl(IDevice* device, UnitTestContext* context)
    {
        SwapchainResizeTest t;
        t.init(device, context);
        t.run();
    }

    SLANG_UNIT_TEST(swapchainResizeD3D12)
    {
        runTestImpl(swapchainResizeTestImpl, unitTestContext, RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(swapchainResizeVulkan)
    {
        runTestImpl(swapchainResizeTestImpl, unitTestContext, RenderApiFlag::Vulkan);
    }

}
