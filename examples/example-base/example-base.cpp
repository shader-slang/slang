#include "example-base.h"

#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace Slang;
using namespace rhi;

Slang::Result WindowedAppBase::initializeBase(
    const char* title,
    int width,
    int height,
    DeviceType deviceType)
{
    DeviceDesc deviceDesc = {};
    deviceDesc.deviceType = deviceType;
#ifdef _DEBUG
    deviceDesc.enableValidation = true;
#endif
    gDevice = getRHI()->createDevice(deviceDesc);
    if (!gDevice) {
        return SLANG_FAIL;
    }

    gQueue = gDevice->getQueue(QueueType::Graphics);
    windowWidth = width;
    windowHeight = height;

    // Do not create swapchain and windows in test mode, because there won't be any display.
    if (!isTestMode())
    {
        // Create a window for our application to render into.
        //
        platform::WindowDesc windowDesc;
        windowDesc.title = title;
        windowDesc.width = width;
        windowDesc.height = height;
        windowDesc.style = platform::WindowStyle::Default;
        gWindow = platform::Application::createWindow(windowDesc);
        gWindow->events.mainLoop = [this]() { mainLoop(); };
        gWindow->events.sizeChanged = Slang::Action<>(this, &WindowedAppBase::windowSizeChanged);


        WindowHandle windowHandle = gWindow->getNativeHandle().convert<WindowHandle>();
        gSurface = gDevice->createSurface(windowHandle);

        auto deviceInfo = gDevice->getInfo();
        Slang::StringBuilder titleSb;
        titleSb << title << " (" << deviceInfo.apiName << ": " << deviceInfo.adapterName << ")";
        gWindow->setText(titleSb.getBuffer());

        rhi::SurfaceConfig surfaceConfig = {};

        surfaceConfig.format = Format::RGBA8Unorm;
        surfaceConfig.width = width;
        surfaceConfig.height = height;
        surfaceConfig.desiredImageCount = kSwapchainImageCount;
        gSurface->configure(surfaceConfig);
    }
    else
    {
        createOfflineTextures();
    }

    return SLANG_OK;
}

void WindowedAppBase::mainLoop()
{
    auto texture = gSurface->acquireNextImage();
    renderFrame(texture);
}

void WindowedAppBase::createOfflineTextures()
{
    for (uint32_t i = 0; i < kSwapchainImageCount; i++) {
        TextureDesc textureDesc = {};
        textureDesc.size.width = this->windowWidth;
        textureDesc.size.height = this->windowHeight;
        textureDesc.format = Format::RGBA8Unorm;
        textureDesc.mipCount = 1;
        textureDesc.usage = TextureUsage::UnorderedAccess | TextureUsage::CopySource;
        auto texture = gDevice->createTexture(textureDesc);
        gOfflineTextures.add(texture);
    }
}

void WindowedAppBase::offlineRender()
{
    SLANG_ASSERT(gOfflineTextures.getCount() > 0);
    renderFrame(gOfflineTextures[0]);
}

void WindowedAppBase::windowSizeChanged()
{
    // Wait for the GPU to finish.
    gQueue->waitOnHost();

    auto clientRect = gWindow->getClientRect();
    if (clientRect.width > 0 && clientRect.height > 0)
    {
        SurfaceConfig config = {};
        config.format = gSurface->getInfo().preferredFormat;
        config.width = clientRect.width;
        config.height = clientRect.height;
        config.vsync = false;
        gSurface->configure(config);
    }
}

int64_t getCurrentTime()
{
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

int64_t getTimerFrequency()
{
    return std::chrono::high_resolution_clock::period::den;
}

class DebugCallback : public IDebugCallback
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL
    handleMessage(DebugMessageType type, DebugMessageSource source, const char* message) override
    {
        const char* typeStr = "";
        switch (type)
        {
        case DebugMessageType::Info:
            typeStr = "INFO: ";
            break;
        case DebugMessageType::Warning:
            typeStr = "WARNING: ";
            break;
        case DebugMessageType::Error:
            typeStr = "ERROR: ";
            break;
        default:
            break;
        }
        const char* sourceStr = "[GraphicsLayer]: ";
        switch (source)
        {
        case DebugMessageSource::Slang:
            sourceStr = "[Slang]: ";
            break;
        case DebugMessageSource::Driver:
            sourceStr = "[Driver]: ";
            break;
        }
        printf("%s%s%s\n", sourceStr, typeStr, message);
#ifdef _WIN32
        OutputDebugStringA(sourceStr);
        OutputDebugStringA(typeStr);
        OutputDebugStringW(String(message).toWString());
        OutputDebugStringW(L"\n");
#endif
    }
};

#ifdef _WIN32
void _Win32OutputDebugString(const char* str)
{
    OutputDebugStringW(Slang::String(str).toWString().begin());
}
#endif
