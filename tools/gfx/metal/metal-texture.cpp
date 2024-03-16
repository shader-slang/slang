// metal-texture.cpp
#include "metal-texture.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

TextureResourceImpl::TextureResourceImpl(const Desc& desc, DeviceImpl* device)
    : Parent(desc)
    , m_device(device)
{}

TextureResourceImpl::~TextureResourceImpl()
{
}

Result TextureResourceImpl::getNativeResourceHandle(InteropHandle* outHandle)
{
    outHandle->api = InteropHandleAPI::Metal;
    outHandle->handleValue = reinterpret_cast<intptr_t>(m_texture);
    return SLANG_OK;
}

Result TextureResourceImpl::getSharedHandle(InteropHandle* outHandle)
{
    return SLANG_OK;
}

Result TextureResourceImpl::setDebugName(const char* name)
{
    return SLANG_OK;
}

} // namespace metal
} // namespace gfx
