// metal-render-pass.cpp
#include "metal-render-pass.h"

//#include "metal-helper-functions.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

IRenderPassLayout* RenderPassLayoutImpl::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IRenderPassLayout)
        return static_cast<IRenderPassLayout*>(this);
    return nullptr;
}

RenderPassLayoutImpl::~RenderPassLayoutImpl()
{
}

static inline MTL::LoadAction translateLoadOp(IRenderPassLayout::TargetLoadOp loadOp)
{
    switch (loadOp)
    {
    case IRenderPassLayout::TargetLoadOp::Clear:
        return MTL::LoadAction::LoadActionClear;
    case IRenderPassLayout::TargetLoadOp::Load:
        return MTL::LoadAction::LoadActionLoad;
    case IRenderPassLayout::TargetLoadOp::DontCare:
    default:
        return MTL::LoadAction::LoadActionDontCare;
    }
}

static inline MTL::StoreAction translateStoreOp(IRenderPassLayout::TargetStoreOp storeOp)
{
    switch (storeOp)
    {
    case IRenderPassLayout::TargetStoreOp::Store:
        return MTL::StoreAction::StoreActionStore;
    case IRenderPassLayout::TargetStoreOp::DontCare:
    default:
        return MTL::StoreAction::StoreActionDontCare;
    }
}

Result RenderPassLayoutImpl::init(DeviceImpl* renderer, const IRenderPassLayout::Desc& desc)
{
    m_renderer = renderer;

    FramebufferLayoutImpl* framebufferLayout = static_cast<FramebufferLayoutImpl*>(desc.framebufferLayout);
    assert(framebufferLayout);

    // Initialize render pass descriptor, filling in attachment metadata, but leaving texture data unbound.
    m_renderPassDesc = MTL::RenderPassDescriptor::alloc()->init();
    m_renderPassDesc->setRenderTargetArrayLength(desc.renderTargetCount);

    MTL::RenderPassColorAttachmentDescriptorArray* colorAttachments = m_renderPassDesc->colorAttachments();
    for (GfxIndex i = 0; i < desc.renderTargetCount; ++i)
    {
        MTL::RenderPassColorAttachmentDescriptor* colorAttach = MTL::RenderPassColorAttachmentDescriptor::alloc()->init();
        colorAttach->setLoadAction(translateLoadOp(desc.renderTargetAccess[i].loadOp));
        colorAttach->setStoreAction(translateStoreOp(desc.renderTargetAccess[i].storeOp));
        // We set the texture when the render pass is executed, using the associated framebuffer.
        colorAttach->setTexture(nullptr);
        colorAttachments->setObject(colorAttach, i);
    }
    m_renderPassDesc->depthAttachment()->setLoadAction(translateLoadOp(desc.depthStencilAccess->loadOp));
    m_renderPassDesc->depthAttachment()->setStoreAction(translateStoreOp(desc.depthStencilAccess->storeOp));
    // We set the depth texture when the render pass is executed, using the associated framebuffer.
    m_renderPassDesc->depthAttachment()->setTexture(nullptr);
    //m_renderPassDesc->depthAttachment()->setClearDepth(1000000.);
    m_renderPassDesc->stencilAttachment()->setLoadAction(translateLoadOp(desc.depthStencilAccess->loadOp));
    m_renderPassDesc->stencilAttachment()->setStoreAction(translateStoreOp(desc.depthStencilAccess->storeOp));
    // We set the stencil texture when the render pass is executed, using the associated framebuffer.
    m_renderPassDesc->stencilAttachment()->setTexture(nullptr);
    return SLANG_OK;
}

} // namespace metal
} // namespace gfx
