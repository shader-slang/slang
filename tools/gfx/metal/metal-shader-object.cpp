// metal-shader-object.cpp

#include "metal-command-buffer.h"
#include "metal-command-encoder.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

Result ShaderObjectImpl::create(
    IDevice* device, ShaderObjectLayoutImpl* layout, ShaderObjectImpl** outShaderObject)
{
    auto object = RefPtr<ShaderObjectImpl>(new ShaderObjectImpl());
    SLANG_RETURN_ON_FAIL(object->init(device, layout));

    returnRefPtrMove(outShaderObject, object);
    return SLANG_OK;
}

RendererBase* ShaderObjectImpl::getDevice() { return m_layout->getDevice(); }

GfxCount ShaderObjectImpl::getEntryPointCount() { return 0; }

Result ShaderObjectImpl::getEntryPoint(GfxIndex index, IShaderObject** outEntryPoint)
{
    *outEntryPoint = nullptr;
    return SLANG_OK;
}

const void* ShaderObjectImpl::getRawData() { return m_data.getBuffer(); }

Size ShaderObjectImpl::getSize() { return (Size)m_data.getCount(); }

// TODO: Change size_t and Index to Size?
Result ShaderObjectImpl::setData(ShaderOffset const& inOffset, void const* data, size_t inSize)
{
    return SLANG_OK;
}

Result ShaderObjectImpl::setResource(ShaderOffset const& offset, IResourceView* resourceView)
{
    return SLANG_OK;
}

Result ShaderObjectImpl::setSampler(ShaderOffset const& offset, ISamplerState* sampler)
{
    return SLANG_OK;
}

Result ShaderObjectImpl::setCombinedTextureSampler(
    ShaderOffset const& offset, IResourceView* textureView, ISamplerState* sampler)
{
    return SLANG_OK;
}

Result ShaderObjectImpl::init(IDevice* device, ShaderObjectLayoutImpl* layout)
{
    return SLANG_OK;
}



Result EntryPointShaderObject::create(
    IDevice* device, EntryPointLayout* layout, EntryPointShaderObject** outShaderObject)
{
    RefPtr<EntryPointShaderObject> object = new EntryPointShaderObject();
    SLANG_RETURN_ON_FAIL(object->init(device, layout));

    returnRefPtrMove(outShaderObject, object);
    return SLANG_OK;
}


Result EntryPointShaderObject::init(IDevice* device, EntryPointLayout* layout)
{
    //SLANG_RETURN_ON_FAIL(Super::init(device, layout));
    return SLANG_OK;
}


GfxCount RootShaderObjectImpl::getEntryPointCount() { return (GfxCount)m_entryPoints.getCount(); }

Result RootShaderObjectImpl::getEntryPoint(GfxIndex index, IShaderObject** outEntryPoint)
{
    returnComPtr(outEntryPoint, m_entryPoints[index]);
    return SLANG_OK;
}

Result RootShaderObjectImpl::copyFrom(IShaderObject* object, ITransientResourceHeap* transientHeap)
{
    return SLANG_FAIL;
}

Result RootShaderObjectImpl::collectSpecializationArgs(ExtendedShaderObjectTypeList& args)
{
    return SLANG_OK;
}

Result RootShaderObjectImpl::init(IDevice* device, RootShaderObjectLayout* layout)
{
    return SLANG_OK;
}

} // namespace metal
} // namespace gfx
