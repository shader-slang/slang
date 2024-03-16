// metal-render-pass.h
#pragma once

#include "metal-base.h"
#include "metal-device.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

class RenderPassLayoutImpl
    : public IRenderPassLayout
    , public ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    IRenderPassLayout* getInterface(const Guid& guid);

public:
    MTL::RenderPassDescriptor* m_renderPassDesc = nullptr;
    RefPtr<DeviceImpl> m_renderer;
    ~RenderPassLayoutImpl();

    Result init(DeviceImpl* renderer, const IRenderPassLayout::Desc& desc);
};

} // namespace metal
} // namespace gfx
