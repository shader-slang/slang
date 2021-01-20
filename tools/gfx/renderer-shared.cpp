#include "renderer-shared.h"
#include "render-graphics-common.h"

namespace gfx
{

IResource* BufferResource::getInterface(const Slang::Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IResource ||
        guid == GfxGUID::IID_IBufferResource)
        return static_cast<IBufferResource*>(this);
    return nullptr;
}

SLANG_NO_THROW IResource::Type SLANG_MCALL BufferResource::getType() { return m_type; }
SLANG_NO_THROW IBufferResource::Desc* SLANG_MCALL BufferResource::getDesc() { return &m_desc; }


IResource* TextureResource::getInterface(const Slang::Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IResource ||
        guid == GfxGUID::IID_ITextureResource)
        return static_cast<ITextureResource*>(this);
    return nullptr;
}

SLANG_NO_THROW IResource::Type SLANG_MCALL TextureResource::getType() { return m_type; }
SLANG_NO_THROW ITextureResource::Desc* SLANG_MCALL TextureResource::getDesc() { return &m_desc; }


} // namespace gfx
