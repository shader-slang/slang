#include "example-base.h"
#include <chrono>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

using namespace Slang;
using namespace gfx;

Slang::Result WindowedAppBase::initializeBase(
    const char* title,
    int width,
    int height,
    DeviceType deviceType)
{
    // Create a window for our application to render into.
    //
    platform::WindowDesc windowDesc;
    windowDesc.title = title;
    windowDesc.width = width;
    windowDesc.height = height;
    windowWidth = width;
    windowHeight = height;
    windowDesc.style = platform::WindowStyle::Default;
    gWindow = platform::Application::createWindow(windowDesc);
    gWindow->events.mainLoop = [this]() { mainLoop(); };
    gWindow->events.sizeChanged = Slang::Action<>(this, &WindowedAppBase::windowSizeChanged);

    // Initialize the rendering layer.
#ifdef _DEBUG
    // Enable debug layer in debug config.
    gfxEnableDebugLayer();
#endif
    IDevice::Desc deviceDesc = {};
    deviceDesc.deviceType = deviceType;
    gfx::Result res = gfxCreateDevice(&deviceDesc, gDevice.writeRef());
    if (SLANG_FAILED(res))
        return res;

    auto deviceInfo = gDevice->getDeviceInfo();
    Slang::StringBuilder titleSb;
    titleSb << title << " (" << deviceInfo.apiName << ": " << deviceInfo.adapterName << ")";
    gWindow->setText(titleSb.getBuffer());

    ICommandQueue::Desc queueDesc = {};
    queueDesc.type = ICommandQueue::QueueType::Graphics;
    gQueue = gDevice->createCommandQueue(queueDesc);

    // Create swapchain and framebuffers.
    gfx::ISwapchain::Desc swapchainDesc = {};
    swapchainDesc.format = gfx::Format::R8G8B8A8_UNORM;
    swapchainDesc.width = width;
    swapchainDesc.height = height;
    swapchainDesc.imageCount = kSwapchainImageCount;
    swapchainDesc.queue = gQueue;
    gfx::WindowHandle windowHandle = gWindow->getNativeHandle().convert<gfx::WindowHandle>();
    gSwapchain = gDevice->createSwapchain(swapchainDesc, windowHandle);

    IFramebufferLayout::TargetLayout renderTargetLayout = {gSwapchain->getDesc().format, 1};
    IFramebufferLayout::TargetLayout depthLayout = {gfx::Format::D32_FLOAT, 1};
    IFramebufferLayout::Desc framebufferLayoutDesc;
    framebufferLayoutDesc.renderTargetCount = 1;
    framebufferLayoutDesc.renderTargets = &renderTargetLayout;
    framebufferLayoutDesc.depthStencil = &depthLayout;
    SLANG_RETURN_ON_FAIL(
        gDevice->createFramebufferLayout(framebufferLayoutDesc, gFramebufferLayout.writeRef()));

    createSwapchainFramebuffers();

    for (uint32_t i = 0; i < kSwapchainImageCount; i++)
    {
        gfx::ITransientResourceHeap::Desc transientHeapDesc = {};
        transientHeapDesc.constantBufferSize = 4096 * 1024;
        auto transientHeap = gDevice->createTransientResourceHeap(transientHeapDesc);
        gTransientHeaps.add(transientHeap);
    }

    gfx::IRenderPassLayout::Desc renderPassDesc = {};
    renderPassDesc.framebufferLayout = gFramebufferLayout;
    renderPassDesc.renderTargetCount = 1;
    IRenderPassLayout::TargetAccessDesc renderTargetAccess = {};
    IRenderPassLayout::TargetAccessDesc depthStencilAccess = {};
    renderTargetAccess.loadOp = IRenderPassLayout::TargetLoadOp::Clear;
    renderTargetAccess.storeOp = IRenderPassLayout::TargetStoreOp::Store;
    renderTargetAccess.initialState = ResourceState::Undefined;
    renderTargetAccess.finalState = ResourceState::Present;
    depthStencilAccess.loadOp = IRenderPassLayout::TargetLoadOp::Clear;
    depthStencilAccess.storeOp = IRenderPassLayout::TargetStoreOp::Store;
    depthStencilAccess.initialState = ResourceState::Undefined;
    depthStencilAccess.finalState = ResourceState::DepthWrite;
    renderPassDesc.renderTargetAccess = &renderTargetAccess;
    renderPassDesc.depthStencilAccess = &depthStencilAccess;
    gRenderPass = gDevice->createRenderPassLayout(renderPassDesc);

    return SLANG_OK;
}

void WindowedAppBase::mainLoop()
{
    int frameBufferIndex = gSwapchain->acquireNextImage();
    if (frameBufferIndex == -1)
        return;

    gTransientHeaps[frameBufferIndex]->synchronizeAndReset();
    renderFrame(frameBufferIndex);
    gTransientHeaps[frameBufferIndex]->finish();
}

void WindowedAppBase::createSwapchainFramebuffers()
{
    gFramebuffers.clear();
    for (uint32_t i = 0; i < kSwapchainImageCount; i++)
    {
        gfx::ITextureResource::Desc depthBufferDesc;
        depthBufferDesc.type = IResource::Type::Texture2D;
        depthBufferDesc.size.width = gSwapchain->getDesc().width;
        depthBufferDesc.size.height = gSwapchain->getDesc().height;
        depthBufferDesc.size.depth = 1;
        depthBufferDesc.format = gfx::Format::D32_FLOAT;
        depthBufferDesc.defaultState = ResourceState::DepthWrite;
        depthBufferDesc.allowedStates = ResourceStateSet(ResourceState::DepthWrite);
        
        ComPtr<gfx::ITextureResource> depthBufferResource =
            gDevice->createTextureResource(depthBufferDesc, nullptr);
        ComPtr<gfx::ITextureResource> colorBuffer;
        gSwapchain->getImage(i, colorBuffer.writeRef());

        gfx::IResourceView::Desc colorBufferViewDesc;
        memset(&colorBufferViewDesc, 0, sizeof(colorBufferViewDesc));
        colorBufferViewDesc.format = gSwapchain->getDesc().format;
        colorBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
        colorBufferViewDesc.type = gfx::IResourceView::Type::RenderTarget;
        ComPtr<gfx::IResourceView> rtv =
            gDevice->createTextureView(colorBuffer.get(), colorBufferViewDesc);

        gfx::IResourceView::Desc depthBufferViewDesc;
        memset(&depthBufferViewDesc, 0, sizeof(depthBufferViewDesc));
        depthBufferViewDesc.format = gfx::Format::D32_FLOAT;
        depthBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
        depthBufferViewDesc.type = gfx::IResourceView::Type::DepthStencil;
        ComPtr<gfx::IResourceView> dsv =
            gDevice->createTextureView(depthBufferResource.get(), depthBufferViewDesc);

        gfx::IFramebuffer::Desc framebufferDesc;
        framebufferDesc.renderTargetCount = 1;
        framebufferDesc.depthStencilView = dsv.get();
        framebufferDesc.renderTargetViews = rtv.readRef();
        framebufferDesc.layout = gFramebufferLayout;
        ComPtr<gfx::IFramebuffer> frameBuffer = gDevice->createFramebuffer(framebufferDesc);
        gFramebuffers.add(frameBuffer);
    }
}

void WindowedAppBase::windowSizeChanged()
{
    // Wait for the GPU to finish.
    gQueue->waitOnHost();

    auto clientRect = gWindow->getClientRect();
    if (clientRect.width > 0 && clientRect.height > 0)
    {
        // Free all framebuffers before resizing swapchain.
        gFramebuffers = decltype(gFramebuffers)();

        // Resize swapchain.
        if (gSwapchain->resize(clientRect.width, clientRect.height) == SLANG_OK)
        {
            // Recreate framebuffers for each swapchain back buffer image.
            createSwapchainFramebuffers();
            windowWidth = clientRect.width;
            windowHeight = clientRect.height;
        }
    }
}

int64_t getCurrentTime() { return std::chrono::high_resolution_clock::now().time_since_epoch().count(); }

int64_t getTimerFrequency() { return std::chrono::high_resolution_clock::period::den; }

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

void initDebugCallback()
{
    static DebugCallback callback = {};
    gfxSetDebugCallback(&callback);
}

#ifdef _WIN32
void _Win32OutputDebugString(const char* str) { OutputDebugStringW(Slang::String(str).toWString().begin()); }
#endif
