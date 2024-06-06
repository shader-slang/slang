// metal-texture.h
#pragma once

#include "metal-base.h"
#include "metal-device.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

class TextureResourceImpl : public TextureResource
{
public:
    typedef TextureResource Parent;

    TextureResourceImpl(const Desc& desc, DeviceImpl* device);
    ~TextureResourceImpl();

    RefPtr<DeviceImpl> m_device;
    NS::SharedPtr<MTL::Texture> m_texture;
    // TODO still needed?
    // MTL::PixelFormat m_metalFormat = MTL::PixelFormat::PixelFormatInvalid;
    // bool m_isWeakImageReference = false;
    bool m_isCurrentDrawable = false;

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeResourceHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL setDebugName(const char* name) override;
};

} // namespace metal
} // namespace gfx
