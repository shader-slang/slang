// debug-shader-object.cpp
#include "debug-shader-object.h"

#include "debug-resource-views.h"
#include "debug-sampler-state.h"

#include "debug-helper-functions.h"

namespace gfx
{
using namespace Slang;

namespace debug
{

ShaderObjectContainerType DebugShaderObject::getContainerType()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getContainerType();
}

slang::TypeLayoutReflection* DebugShaderObject::getElementTypeLayout()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getElementTypeLayout();
}

GfxCount DebugShaderObject::getEntryPointCount()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getEntryPointCount();
}

Result DebugShaderObject::getEntryPoint(GfxIndex index, IShaderObject** entryPoint)
{
    SLANG_GFX_API_FUNC;
    if (m_entryPoints.getCount() == 0)
    {
        for (GfxIndex i = 0; i < getEntryPointCount(); i++)
        {
            RefPtr<DebugShaderObject> entryPointObj = new DebugShaderObject();
            SLANG_RETURN_ON_FAIL(
                baseObject->getEntryPoint(i, entryPointObj->baseObject.writeRef()));
            m_entryPoints.add(entryPointObj);
        }
    }
    if (index > (GfxCount)m_entryPoints.getCount())
    {
        GFX_DIAGNOSE_ERROR("`index` must not exceed `entryPointCount`.");
        return SLANG_FAIL;
    }
    returnComPtr(entryPoint, m_entryPoints[index]);
    return SLANG_OK;
}

Result DebugShaderObject::setData(ShaderOffset const& offset, void const* data, Size size)
{
    SLANG_GFX_API_FUNC;
    return baseObject->setData(offset, data, size);
}

Result DebugShaderObject::getObject(ShaderOffset const& offset, IShaderObject** object)
{
    SLANG_GFX_API_FUNC;

    ComPtr<IShaderObject> innerObject;
    auto resultCode = baseObject->getObject(offset, innerObject.writeRef());
    SLANG_RETURN_ON_FAIL(resultCode);
    RefPtr<DebugShaderObject> debugShaderObject;
    if (m_objects.TryGetValue(ShaderOffsetKey{offset}, debugShaderObject))
    {
        if (debugShaderObject->baseObject == innerObject)
        {
            returnComPtr(object, debugShaderObject);
            return resultCode;
        }
    }
    debugShaderObject = new DebugShaderObject();
    debugShaderObject->baseObject = innerObject;
    debugShaderObject->m_typeName = innerObject->getElementTypeLayout()->getName();
    m_objects[ShaderOffsetKey{offset}] = debugShaderObject;
    returnComPtr(object, debugShaderObject);
    return resultCode;
}

Result DebugShaderObject::setObject(ShaderOffset const& offset, IShaderObject* object)
{
    SLANG_GFX_API_FUNC;
    auto objectImpl = static_cast<DebugShaderObject*>(object);
    m_objects[ShaderOffsetKey{offset}] = objectImpl;
    return baseObject->setObject(offset, objectImpl->baseObject.get());
}

Result DebugShaderObject::setResource(ShaderOffset const& offset, IResourceView* resourceView)
{
    SLANG_GFX_API_FUNC;
    auto viewImpl = static_cast<DebugResourceView*>(resourceView);
    m_resources[ShaderOffsetKey{offset}] = viewImpl;
    return baseObject->setResource(offset, viewImpl->baseObject.get());
}

Result DebugShaderObject::setSampler(ShaderOffset const& offset, ISamplerState* sampler)
{
    SLANG_GFX_API_FUNC;
    auto samplerImpl = static_cast<DebugSamplerState*>(sampler);
    m_samplers[ShaderOffsetKey{offset}] = samplerImpl;
    return baseObject->setSampler(offset, samplerImpl->baseObject.get());
}

Result DebugShaderObject::setCombinedTextureSampler(
    ShaderOffset const& offset,
    IResourceView* textureView,
    ISamplerState* sampler)
{
    SLANG_GFX_API_FUNC;
    auto samplerImpl = static_cast<DebugSamplerState*>(sampler);
    m_samplers[ShaderOffsetKey{offset}] = samplerImpl;
    auto viewImpl = static_cast<DebugResourceView*>(textureView);
    m_resources[ShaderOffsetKey{offset}] = viewImpl;
    return baseObject->setCombinedTextureSampler(
        offset, viewImpl->baseObject.get(), samplerImpl->baseObject.get());
}

Result DebugShaderObject::setSpecializationArgs(
    ShaderOffset const& offset,
    const slang::SpecializationArg* args,
    GfxCount count)
{
    SLANG_GFX_API_FUNC;
    return baseObject->setSpecializationArgs(offset, args, count);
}

Result DebugShaderObject::getCurrentVersion(
    ITransientResourceHeap* transientHeap, IShaderObject** outObject)
{
    SLANG_GFX_API_FUNC;
    ComPtr<IShaderObject> innerObject;
    SLANG_RETURN_ON_FAIL(baseObject->getCurrentVersion(getInnerObj(transientHeap), innerObject.writeRef()));
    RefPtr<DebugShaderObject> debugShaderObject = new DebugShaderObject();
    debugShaderObject->baseObject = innerObject;
    debugShaderObject->m_typeName = innerObject->getElementTypeLayout()->getName();
    returnComPtr(outObject, debugShaderObject);
    return SLANG_OK;
}

const void* DebugShaderObject::getRawData()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getRawData();
}

size_t DebugShaderObject::getSize()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getSize();
}

Result DebugShaderObject::setConstantBufferOverride(IBufferResource* constantBuffer)
{
    SLANG_GFX_API_FUNC;
    return baseObject->setConstantBufferOverride(getInnerObj(constantBuffer));
}

Result DebugRootShaderObject::setSpecializationArgs(
    ShaderOffset const& offset,
    const slang::SpecializationArg* args,
    GfxCount count)
{
    SLANG_GFX_API_FUNC;

    return baseObject->setSpecializationArgs(offset, args, count);
}

void DebugRootShaderObject::reset()
{
    m_entryPoints.clear();
    m_objects.Clear();
    m_resources.Clear();
    m_samplers.Clear();
    baseObject.detach();
}

} // namespace debug
} // namespace gfx
