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
#if 0
    MTL::RenderPassDescriptor* m_renderPass = nullptr;
    Array<MTL::RenderPassColorAttachmentDescriptor*, kMaxTargets> m_targetDescs;
    MTL::RenderPassDepthAttachmentDescriptor* m_depthAttachmentDesc = nullptr;
    MTL::RenderPassStencilAttachmentDescriptor* m_stencilAttachmentDesc = nullptr;
    bool m_hasDepthStencilTarget = false;
    uint32_t m_renderTargetCount = 0;
#endif
    DeviceImpl* m_renderer = nullptr;
    Desc m_desc;

public:
    ~FramebufferLayoutImpl();
    Result init(DeviceImpl* renderer, const IFramebufferLayout::Desc& desc);
};

class FramebufferImpl : public FramebufferBase
{
public:
    ShortList<ComPtr<IResourceView>> renderTargetViews;
    ComPtr<IResourceView> depthStencilView;
    uint32_t m_width;
    uint32_t m_height;
    BreakableReference<DeviceImpl> m_renderer;
    MTL::ClearColor m_clearValues[kMaxTargets];
    RefPtr<FramebufferLayoutImpl> m_layout;
#if 0
    Array<MTL::RenderPassColorAttachmentDescriptor*, kMaxTargets> m_colorTargetDescs;
    MTL::RenderPassDepthAttachmentDescriptor* m_depthAttachmentDesc = nullptr;
    MTL::RenderPassStencilAttachmentDescriptor* m_stencilAttachmentDesc = nullptr;
#endif

public:
    ~FramebufferImpl();

    Result init(DeviceImpl* renderer, const IFramebuffer::Desc& desc);
};

} // namespace metal
} // namespace gfx
