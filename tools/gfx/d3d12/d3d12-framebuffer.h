// d3d12-framebuffer.h
#pragma once

#include "../renderer-shared.h"
#include "d3d12-resource-views.h"
#include "descriptor-heap-d3d12.h"

namespace gfx
{
namespace d3d12
{

using namespace Slang;

class FramebufferLayoutImpl : public FramebufferLayoutBase
{
public:
    ShortList<IFramebufferLayout::AttachmentLayout> m_renderTargets;
    bool m_hasDepthStencil = false;
    IFramebufferLayout::AttachmentLayout m_depthStencil;
};

class FramebufferImpl : public FramebufferBase
{
public:
    ShortList<RefPtr<ResourceViewImpl>> renderTargetViews;
    RefPtr<ResourceViewImpl> depthStencilView;
    ShortList<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors;
    struct Color4f
    {
        float values[4];
    };
    ShortList<Color4f> renderTargetClearValues;
    D3D12_CPU_DESCRIPTOR_HANDLE depthStencilDescriptor;
    DepthStencilClearValue depthStencilClearValue;
};

} // namespace d3d12
} // namespace gfx
