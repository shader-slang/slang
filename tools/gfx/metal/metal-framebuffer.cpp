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

Result FramebufferLayoutImpl::init(const IFramebufferLayout::Desc& desc)
{
    for (Index i = 0; i < desc.renderTargetCount; ++i)
    {
        m_renderTargets.add(desc.renderTargets[i]);
    }
    if (desc.depthStencil)
    {
        m_depthStencil = *desc.depthStencil;
    }
    else
    {
        m_depthStencil = {};
    }
    return SLANG_OK;
}

Result FramebufferImpl::init(DeviceImpl* device, const IFramebuffer::Desc& desc)
{
    m_device = device;
    m_layout = static_cast<FramebufferLayoutImpl*>(desc.layout);
    m_renderTargetViews.setCount(desc.renderTargetCount);
    for (Index i = 0; i < desc.renderTargetCount; ++i)
    {
        m_renderTargetViews[i] = static_cast<TextureResourceViewImpl*>(desc.renderTargetViews[i]);
    }
    m_depthStencilView = static_cast<TextureResourceViewImpl*>(desc.depthStencilView);

    // Compute framebuffer dimensions from either the first render target view or the depth stencil view.
    TextureResourceViewImpl* view = (m_renderTargetViews.getCount() > 0) ? m_renderTargetViews[0] : m_depthStencilView;
    if (!view)
    {
        return SLANG_FAIL;
    }
    auto size = view->m_texture->getDesc()->size;
    auto viewDesc = view->getViewDesc();
    m_width = Math::Max(1u, uint32_t(size.width >> viewDesc->subresourceRange.mipLevel));
    m_height = Math::Max(1u, uint32_t(size.height >> viewDesc->subresourceRange.mipLevel));

    return SLANG_OK;
}

} // namespace metal
} // namespace gfx
