#include "example-base.h"

#include "slang.h"

#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace Slang;
using namespace rhi;

class DebugCallback : public rhi::IDebugCallback
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL handleMessage(
        rhi::DebugMessageType type,
        rhi::DebugMessageSource source,
        const char* message) override
    {
        const char* typeStr = "";
        switch (type)
        {
        case rhi::DebugMessageType::Info:
            typeStr = "INFO: ";
            break;
        case rhi::DebugMessageType::Warning:
            typeStr = "WARNING: ";
            break;
        case rhi::DebugMessageType::Error:
            typeStr = "ERROR: ";
            break;
        default:
            break;
        }
        const char* sourceStr = "[GraphicsLayer]: ";
        switch (source)
        {
        case rhi::DebugMessageSource::Slang:
            sourceStr = "[Slang]: ";
            break;
        case rhi::DebugMessageSource::Driver:
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


Slang::Result WindowedAppBase::initializeBase(
    const char* title,
    int width,
    int height,
    DeviceType deviceType)
{
    DeviceDesc deviceDesc = {};
    deviceDesc.deviceType = deviceType;

    // Enable validation when not in test mode to avoid output pollution during testing
    deviceDesc.enableValidation = !isTestMode();

    // Set debug callback (only used when validation is enabled, i.e., non-test mode)
    static DebugCallback debugCallback;
    deviceDesc.debugCallback = &debugCallback;

    slang::CompilerOptionEntry slangOptions[] = {
        {slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1}},
        {slang::CompilerOptionName::DebugInformation,
         {slang::CompilerOptionValueKind::Int, SLANG_DEBUG_INFO_LEVEL_STANDARD}}};
    deviceDesc.slang.compilerOptionEntries = slangOptions;

    // When in test mode, don't include debug information to avoid altering hash values during
    // testing Otherwise, include debug information for better debugging experience
    deviceDesc.slang.compilerOptionEntryCount = isTestMode() ? 1 : 2;

    gDevice = getRHI()->createDevice(deviceDesc);
    if (!gDevice)
    {
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

        surfaceConfig.format = gSurface->getInfo().preferredFormat;
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


ComPtr<ITextureView> WindowedAppBase::createTextureFromFile(
    String fileName,
    int& textureWidth,
    int& textureHeight)
{
    int channelsInFile = 0;
    auto textureContent =
        stbi_load(fileName.getBuffer(), &textureWidth, &textureHeight, &channelsInFile, 4);
    TextureDesc textureDesc = {};
    textureDesc.type = TextureType::Texture2D;
    textureDesc.usage = TextureUsage::ShaderResource;
    textureDesc.format = Format::RGBA8Unorm;
    textureDesc.mipCount = Math::Log2Ceil(Math::Min(textureWidth, textureHeight)) + 1;
    textureDesc.size.width = textureWidth;
    textureDesc.size.height = textureHeight;
    textureDesc.size.depth = 1;
    List<SubresourceData> subresData;
    List<List<uint32_t>> mipMapData;
    mipMapData.setCount(textureDesc.mipCount);
    subresData.setCount(textureDesc.mipCount);
    mipMapData[0].setCount(textureWidth * textureHeight);
    memcpy(mipMapData[0].getBuffer(), textureContent, textureWidth * textureHeight * 4);
    stbi_image_free(textureContent);
    subresData[0].data = mipMapData[0].getBuffer();
    subresData[0].rowPitch = textureWidth * 4;
    subresData[0].slicePitch = textureWidth * textureHeight * 4;

    // Build mipmaps.
    struct RGBA
    {
        uint8_t v[4];
    };
    auto castToRGBA = [](uint32_t v)
    {
        RGBA result;
        memcpy(&result, &v, 4);
        return result;
    };
    auto castToUint = [](RGBA v)
    {
        uint32_t result;
        memcpy(&result, &v, 4);
        return result;
    };

    int lastMipWidth = textureWidth;
    int lastMipHeight = textureHeight;
    for (uint32_t m = 1; m < textureDesc.mipCount; m++)
    {
        auto lastMipmapData = mipMapData[m - 1].getBuffer();
        int w = lastMipWidth / 2;
        int h = lastMipHeight / 2;
        mipMapData[m].setCount(w * h);
        subresData[m].data = mipMapData[m].getBuffer();
        subresData[m].rowPitch = w * 4;
        subresData[m].slicePitch = h * w * 4;
        for (int x = 0; x < w; x++)
        {
            for (int y = 0; y < h; y++)
            {
                auto pix1 = castToRGBA(lastMipmapData[(y * 2) * lastMipWidth + (x * 2)]);
                auto pix2 = castToRGBA(lastMipmapData[(y * 2) * lastMipWidth + (x * 2 + 1)]);
                auto pix3 = castToRGBA(lastMipmapData[(y * 2 + 1) * lastMipWidth + (x * 2)]);
                auto pix4 = castToRGBA(lastMipmapData[(y * 2 + 1) * lastMipWidth + (x * 2 + 1)]);
                RGBA pix;
                for (int c = 0; c < 4; c++)
                {
                    pix.v[c] =
                        (uint8_t)(((uint32_t)pix1.v[c] + pix2.v[c] + pix3.v[c] + pix4.v[c]) / 4);
                }
                mipMapData[m][y * w + x] = castToUint(pix);
            }
        }
        lastMipWidth = w;
        lastMipHeight = h;
    }

    auto texture = gDevice->createTexture(textureDesc, subresData.getBuffer());

    TextureViewDesc viewDesc = {};
    return gDevice->createTextureView(texture.get(), viewDesc);
}

void WindowedAppBase::createOfflineTextures()
{
    for (uint32_t i = 0; i < kSwapchainImageCount; i++)
    {
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


#ifdef _WIN32
void _Win32OutputDebugString(const char* str)
{
    OutputDebugStringW(Slang::String(str).toWString().begin());
}
#endif
