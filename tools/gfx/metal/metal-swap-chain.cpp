// metal-swap-chain.cpp
#include "metal-swap-chain.h"

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

void SwapchainImpl::getWindowSize(int& widthOut, int& heightOut) const
{
    CocoaUtil::getNSWindowContentSize((void*)m_windowHandle.handleValues[0], &widthOut, &heightOut);
}

SwapchainImpl::~SwapchainImpl()
{
    m_images.clear();
    CocoaUtil::destroyMetalLayer(m_metalLayer);
}

Result SwapchainImpl::init(DeviceImpl* device, const ISwapchain::Desc& desc, WindowHandle window)
{
    m_device = device;
    m_desc = desc;
    m_windowHandle = window;
    m_metalFormat = MetalUtil::translatePixelFormat(desc.format);
    m_currentImageIndex = 0;

    getWindowSize(m_desc.width, m_desc.height);

    m_metalLayer = (CA::MetalLayer*)CocoaUtil::createMetalLayer((void*)window.handleValues[0]);
    if (!m_metalLayer)
    {
        return SLANG_FAIL;
    }
    m_metalLayer->setPixelFormat(m_metalFormat);
    m_metalLayer->setDevice(m_device->m_device.get());
    m_metalLayer->setDrawableSize(CGSize{(float)m_desc.width, (float)m_desc.height});
    m_metalLayer->setFramebufferOnly(true);

    return SLANG_OK;
}

Result SwapchainImpl::getImage(GfxIndex index, ITextureResource** outResource)
{
    if (index < 0 || index != m_currentImageIndex)
        return SLANG_FAIL;
    returnComPtr(outResource, m_images[index]);
    return SLANG_OK;
}

Result SwapchainImpl::resize(GfxCount width, GfxCount height)
{
    SLANG_UNUSED(width);
    SLANG_UNUSED(height);
    m_images.clear();
    m_currentImageIndex = -1;
    m_currentDrawable.reset();
    getWindowSize(m_desc.width, m_desc.height);
    m_metalLayer->setDrawableSize(CGSize{(float)m_desc.width, (float)m_desc.height});
    return SLANG_OK;
}

Result SwapchainImpl::present()
{
    if (!m_currentDrawable)
    {
        return SLANG_FAIL;
    }

    MTL::CommandBuffer* commandBuffer = m_device->m_commandQueue->commandBuffer();
    commandBuffer->presentDrawable(m_currentDrawable.get());
    commandBuffer->commit();
    m_currentDrawable.reset();
    return SLANG_OK;

    // // TODO: Expose controls via some other means
    // static uint32_t frameCount = 0;
    // static uint32_t maxFrameCount = 32;
    // ++frameCount;
    // if (m_device->captureEnabled() && frameCount == maxFrameCount)
    // {
    //     MTL::CaptureManager* captureManager = MTL::CaptureManager::sharedCaptureManager();
    //     captureManager->stopCapture();
    //     exit(1);
    // }
    // return SLANG_OK;
}

int SwapchainImpl::acquireNextImage()
{
    AUTORELEASEPOOL

    CA::MetalDrawable* drawable = m_metalLayer->nextDrawable();
    if (drawable == nullptr)
    {
        return -1;
    }

    m_currentDrawable = NS::RetainPtr(drawable);
    MTL::Texture* texture = drawable->texture();

    // Check if we got a texture we've seen before.
    for (Index i = 0; i < m_images.getCount(); i++)
    {
        if (m_images[i]->m_texture.get() == texture)
        {
            m_currentImageIndex = i;
            return m_currentImageIndex;
        }
    }

    // Create a new texture object to wrap the drawable's texture.

    ITextureResource::Desc desc = {};
    desc.allowedStates = ResourceStateSet(
        ResourceState::Present, ResourceState::RenderTarget, ResourceState::CopyDestination);
    desc.type = IResource::Type::Texture2D;
    desc.arraySize = 0;
    desc.format = m_desc.format; // TODO use actual pixelformat
    desc.size.width = texture->width();
    desc.size.height = texture->height();
    desc.size.depth = 1;
    desc.numMipLevels = 1;
    desc.defaultState = ResourceState::Present;
    RefPtr<TextureResourceImpl> image = new TextureResourceImpl(desc, m_device);
    image->m_texture = NS::RetainPtr(texture);

    m_currentImageIndex = m_images.getCount();
    m_images.add(image);

    return m_currentImageIndex;
}

Result SwapchainImpl::setFullScreenMode(bool mode) { return SLANG_FAIL; }

} // namespace metal 
} // namespace gfx
