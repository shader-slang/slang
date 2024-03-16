// metal-swap-chain.cpp
#include "metal-swap-chain.h"

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
#include <AppKit/AppKit.hpp>
#include "metal-util.h"
#include "../apple/cocoa-util.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

ISwapchain* SwapchainImpl::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ISwapchain)
        return static_cast<ISwapchain*>(this);
    return nullptr;
}

void SwapchainImpl::destroySwapchainAndImages()
{
    m_images.clear();
}

void SwapchainImpl::getWindowSize(int& widthOut, int& heightOut) const
{
    CocoaUtil::getNSWindowContentSize((void*)m_windowHandle.handleValues[0], &widthOut, &heightOut);
}

Result SwapchainImpl::createSwapchainAndImages()
{
    getWindowSize(m_desc.width, m_desc.height);
    // Note that we do not actually create/assign textures here, as metal requires that one do so JIT,
    // rather than ahead of time.
    //m_drawables.setCount(m_desc.imageCount);
    for (GfxIndex i = 0; i < m_desc.imageCount; i++)
    {
        //CA::MetalDrawable* drawable = m_metalLayer->nextDrawable();
        //if (drawable == nullptr)
        //{
        //    assert(drawable);
        // }
        // m_drawables[i] = drawable;
        //MTL::Texture* tex = drawable->texture();

        ITextureResource::Desc imageDesc = {};
        imageDesc.allowedStates = ResourceStateSet(
            ResourceState::Present, ResourceState::RenderTarget, ResourceState::CopyDestination);
        imageDesc.type = IResource::Type::Texture2D;
        imageDesc.arraySize = 0;
        imageDesc.format = m_desc.format;
        imageDesc.size.width = m_desc.width;
        imageDesc.size.height = m_desc.height;
        imageDesc.size.depth = 1;
        imageDesc.numMipLevels = 1;
        imageDesc.defaultState = ResourceState::Present;
        RefPtr<TextureResourceImpl> image = new TextureResourceImpl(imageDesc, m_renderer);
        //image->m_texture = tex;
        image->m_texture = nullptr;
        m_images.add(image);
    }
    return SLANG_OK;
}

SwapchainImpl::~SwapchainImpl()
{
    destroySwapchainAndImages();
    CocoaUtil::destroyMetalLayer(m_renderer->m_metalLayer);
}

Result SwapchainImpl::init(DeviceImpl* renderer, const ISwapchain::Desc& desc, WindowHandle window)
{
    m_renderer = renderer;
    m_api = &renderer->m_api;
    m_queue = static_cast<CommandQueueImpl*>(desc.queue);
    m_windowHandle = window;
    m_metalFormat = MetalUtil::getMetalPixelFormat(desc.format);

    int width, height;
    getWindowSize(width, height);
    CGSize windowSize = {(float)width, (float)height};

    NS::Window* nswin = (NS::Window*)m_windowHandle.handleValues[0];
    m_desc = desc;

    m_renderer->m_metalLayer = (CA::MetalLayer*)CocoaUtil::createMetalLayer((void*)window.handleValues[0]);
    m_renderer->m_metalLayer->setPixelFormat(m_metalFormat);
    m_renderer->m_metalLayer->setDevice(renderer->m_device);
    m_renderer->m_metalLayer->setDrawableSize(windowSize);
    m_renderer->m_metalLayer->setFramebufferOnly(true);

    createSwapchainAndImages();

    return SLANG_OK;
}

Result SwapchainImpl::getImage(GfxIndex index, ITextureResource** outResource)
{
    if (m_images.getCount() <= (Index)index)
        return SLANG_FAIL;
    // TODO: iff index == current
    m_images[index]->m_isCurrentDrawable = true;
    returnComPtr(outResource, m_images[index]);
    return SLANG_OK;
}

Result SwapchainImpl::resize(GfxCount width, GfxCount height)
{
    SLANG_UNUSED(width);
    SLANG_UNUSED(height);
    destroySwapchainAndImages();
    return createSwapchainAndImages();
}

Result SwapchainImpl::present()
{
    // TODO: Expose controls via some other means
    static uint32_t frameCount = 0;
    static uint32_t maxFrameCount = 32;
    ++frameCount;
    if (m_renderer->captureEnabled() && frameCount == maxFrameCount)
    {
        MTL::CaptureManager* captureManager = MTL::CaptureManager::sharedCaptureManager();
        captureManager->stopCapture();
        exit(1);
    }
    return SLANG_OK;
}

int SwapchainImpl::acquireNextImage()
{
    // TODO: hardcoded 0
    CA::MetalDrawable* d = m_renderer->m_metalLayer->nextDrawable();
    m_images[0]->m_texture = d->texture();
    m_renderer->m_drawable = d;

    return 0;
}

Result SwapchainImpl::setFullScreenMode(bool mode) { return SLANG_FAIL; }

} // namespace metal 
} // namespace gfx
