// metal-framebuffer.cpp
#include "metal-framebuffer.h"
#include "metal-device.h"
#include "metal-resource-views.h"
#include "metal-helper-functions.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

FramebufferLayoutImpl::~FramebufferLayoutImpl()
{
    //m_renderPass->release();
}

Result FramebufferLayoutImpl::init(DeviceImpl* renderer, const IFramebufferLayout::Desc& desc)
{
    // Metal doesn't have a notion of Framebuffers or FramebufferLayouts per se.
    // We simply stash the desc and use it when creating the (convenience) Framebuffer
    m_renderer = renderer;
    m_desc = desc;
    return SLANG_OK;
}

FramebufferImpl::~FramebufferImpl()
{
}

Result FramebufferImpl::init(DeviceImpl* renderer, const IFramebuffer::Desc& desc)
{
    m_renderer = renderer;
    m_layout = static_cast<FramebufferLayoutImpl*>(desc.layout);
    m_width = m_height = 1;

    TextureResourceViewImpl* dsv = static_cast<TextureResourceViewImpl*>(desc.depthStencilView);

    // Get frame dimensions from attachments.
    if (dsv)
    {
        // If we have a depth attachment, get frame size from there.
        auto size = dsv->m_texture->getDesc()->size;
        auto viewDesc = dsv->getViewDesc();
        m_width = Math::Max(1u, uint32_t(size.width >> viewDesc->subresourceRange.mipLevel));
        m_height = Math::Max(1u, uint32_t(size.height >> viewDesc->subresourceRange.mipLevel));
    }
    else if (desc.renderTargetCount > 0)
    {
        // If we don't have a depth attachment, then we must have at least
        // one color attachment. Get frame dimension from there.
        auto viewImpl = static_cast<TextureResourceViewImpl*>(desc.renderTargetViews[0]);
        auto resourceDesc = viewImpl->m_texture->getDesc();
        auto viewDesc = viewImpl->getViewDesc();
        auto size = resourceDesc->size;
        m_width = Math::Max(1u, uint32_t(size.width >> viewDesc->subresourceRange.mipLevel));
        m_height = Math::Max(1u, uint32_t(size.height >> viewDesc->subresourceRange.mipLevel));
    }

    // Initialize depthstencil and render target views
    depthStencilView = desc.depthStencilView;

    renderTargetViews.setCount(desc.renderTargetCount);
    for (int i = 0; i < desc.renderTargetCount; ++i)
    {
        renderTargetViews[i] = desc.renderTargetViews[i];
    }

    return SLANG_OK;
}

} // namespace metal
} // namespace gfx
