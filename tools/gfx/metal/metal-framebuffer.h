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
    RefPtr<DeviceImpl> m_device;
    Desc m_desc;
#if 0
    MTL::RenderPassDescriptor* m_renderPass = nullptr;
    Array<MTL::RenderPassColorAttachmentDescriptor*, kMaxTargets> m_targetDescs;
    MTL::RenderPassDepthAttachmentDescriptor* m_depthAttachmentDesc = nullptr;
    MTL::RenderPassStencilAttachmentDescriptor* m_stencilAttachmentDesc = nullptr;
    bool m_hasDepthStencilTarget = false;
    uint32_t m_renderTargetCount = 0;
#endif

public:
    ~FramebufferLayoutImpl();
    Result init(DeviceImpl* device, const IFramebufferLayout::Desc& desc);
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
