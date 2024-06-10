// metal-framebuffer.h
#pragma once

#include "metal-base.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

enum
{
    kMaxRenderTargets = 8,
    kMaxTargets = kMaxRenderTargets + 1,
};

class FramebufferLayoutImpl : public FramebufferLayoutBase
{
public:
    List<IFramebufferLayout::TargetLayout> m_renderTargets;
    IFramebufferLayout::TargetLayout m_depthStencil;

public:
    Result init(const IFramebufferLayout::Desc& desc);
};

class FramebufferImpl : public FramebufferBase
{
public:
    BreakableReference<DeviceImpl> m_device;
    ShortList<ComPtr<IResourceView>> renderTargetViews;
    ComPtr<IResourceView> depthStencilView;
    uint32_t m_width;
    uint32_t m_height;
    MTL::ClearColor m_clearValues[kMaxTargets];
    RefPtr<FramebufferLayoutImpl> m_layout;
#if 0
    Array<MTL::RenderPassColorAttachmentDescriptor*, kMaxTargets> m_colorTargetDescs;
    MTL::RenderPassDepthAttachmentDescriptor* m_depthAttachmentDesc = nullptr;
    MTL::RenderPassStencilAttachmentDescriptor* m_stencilAttachmentDesc = nullptr;
#endif

public:
    ~FramebufferImpl();

    Result init(DeviceImpl* device, const IFramebuffer::Desc& desc);
};

} // namespace metal
} // namespace gfx
