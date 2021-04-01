#include "example-base.h"

using namespace Slang;
using namespace gfx;

Slang::Result WindowedAppBase::initializeBase(const char* title, int width, int height)
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
    IDevice::Desc deviceDesc = {};
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
    swapchainDesc.format = gfx::Format::RGBA_Unorm_UInt8;
    swapchainDesc.width = width;
    swapchainDesc.height = height;
    swapchainDesc.imageCount = kSwapchainImageCount;
    swapchainDesc.queue = gQueue;
    gfx::WindowHandle windowHandle = gWindow->getNativeHandle().convert<gfx::WindowHandle>();
    gSwapchain = gDevice->createSwapchain(swapchainDesc, windowHandle);

    IFramebufferLayout::AttachmentLayout renderTargetLayout = {gSwapchain->getDesc().format, 1};
    IFramebufferLayout::AttachmentLayout depthLayout = {gfx::Format::D_Float32, 1};
    IFramebufferLayout::Desc framebufferLayoutDesc;
    framebufferLayoutDesc.renderTargetCount = 1;
    framebufferLayoutDesc.renderTargets = &renderTargetLayout;
    framebufferLayoutDesc.depthStencil = &depthLayout;
    SLANG_RETURN_ON_FAIL(
        gDevice->createFramebufferLayout(framebufferLayoutDesc, gFramebufferLayout.writeRef()));

    createSwapchainFramebuffers();

    for (uint32_t i = 0; i < kSwapchainImageCount; i++)
    {
        gfx::ITransientResourceHeap::Desc transientHeapDesc;
        transientHeapDesc.constantBufferSize = 4096 * 1024;
        auto transientHeap = gDevice->createTransientResourceHeap(transientHeapDesc);
        gTransientHeaps.add(transientHeap);
    }

    gfx::IRenderPassLayout::Desc renderPassDesc = {};
    renderPassDesc.framebufferLayout = gFramebufferLayout;
    renderPassDesc.renderTargetCount = 1;
    IRenderPassLayout::AttachmentAccessDesc renderTargetAccess = {};
    IRenderPassLayout::AttachmentAccessDesc depthStencilAccess = {};
    renderTargetAccess.loadOp = IRenderPassLayout::AttachmentLoadOp::Clear;
    renderTargetAccess.storeOp = IRenderPassLayout::AttachmentStoreOp::Store;
    renderTargetAccess.initialState = ResourceState::Undefined;
    renderTargetAccess.finalState = ResourceState::Present;
    depthStencilAccess.loadOp = IRenderPassLayout::AttachmentLoadOp::Clear;
    depthStencilAccess.storeOp = IRenderPassLayout::AttachmentStoreOp::Store;
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
}

void WindowedAppBase::createSwapchainFramebuffers()
{
    gFramebuffers.clear();
    for (uint32_t i = 0; i < kSwapchainImageCount; i++)
    {
        gfx::ITextureResource::Desc depthBufferDesc;
        depthBufferDesc.setDefaults(gfx::IResource::Usage::DepthWrite);
        depthBufferDesc.init2D(
            gfx::IResource::Type::Texture2D,
            gfx::Format::D_Float32,
            gSwapchain->getDesc().width,
            gSwapchain->getDesc().height,
            0);

        ComPtr<gfx::ITextureResource> depthBufferResource = gDevice->createTextureResource(
            gfx::IResource::Usage::DepthWrite, depthBufferDesc, nullptr);
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
        depthBufferViewDesc.format = gfx::Format::D_Float32;
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
    gQueue->wait();

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
