
#include "bind-location.h"

#include "../../slang-com-helper.h"

#define SLANG_PRELUDE_NAMESPACE CPPPrelude
#include "../../prelude/slang-cpp-types.h"

namespace renderer_test {
using namespace Slang;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BindSet !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

BindSet::BindSet():
    m_arena(4096, 16)
{
}

BindSet::Resource* BindSet::getAt(Resource* resource, size_t offset) const
{
    UniformLocation location;
    location.m_offset = offset;
    location.m_resource = resource;

    Resource** resourcePtr = m_uniformBindings.TryGetValue(location);
    return resourcePtr ? *resourcePtr : nullptr;
}

void BindSet::setAt(BindSet::Resource* resource, size_t offset, BindSet::Resource* value)
{
    UniformLocation location;
    location.m_offset = offset;
    location.m_resource = resource;

    // Note we don't remove when value == null, such that it is stored if should be nullptr
    Resource** resourcePtr = m_uniformBindings.TryGetValueOrAdd(location, value);
    if (resourcePtr)
    {
        *resourcePtr = value;
    }
}

BindSet::Resource* BindSet::getAt(SlangParameterCategory category, Slang::Index space, size_t offset)
{
    RegisterLocation location;
    location.m_category = category;
    location.m_offset = offset;
    location.m_space = space;

    Resource** resourcePtr = m_registerBindings.TryGetValue(location);
    return resourcePtr ? *resourcePtr : nullptr;
}

void BindSet::setAt(SlangParameterCategory category, Slang::Index space, size_t offset, Resource* value)
{
    RegisterLocation location;

    location.m_category = category;
    location.m_offset = offset;
    location.m_space = space;

    // Note we don't remove when value == null, such that it is stored if should be nullptr
    Resource** resourcePtr = m_registerBindings.TryGetValueOrAdd(location, value);
    if (resourcePtr)
    {
        *resourcePtr = value;
    }
}

BindSet::Resource* BindSet::getAt(const BindLocation& loc)
{
    SLANG_ASSERT(loc.isValid());
    if (loc.isInvalid())
    {
        return nullptr;
    }

    if (loc.m_resource)
    {
        return getAt(loc.m_resource, loc.m_offset);
    }
    else
    {
        // We should have a single category to be able to lookup
        SLANG_ASSERT(loc.m_bindPointSet == nullptr);
        if (loc.m_bindPointSet)
        {
            // It's ambiguous. I guess strictly speaking we could search to see if there is a unique entry
            // but getting to this point implies there is more than one option.
            return nullptr;
        }

        return getAt(loc.m_category, loc.m_space, loc.m_offset);
    }
}

BindSet::Resource* BindSet::newBufferResource(slang::TypeReflection::Kind kind, slang::TypeLayoutReflection* typeLayout, size_t sizeInBytes)
{
    SLANG_ASSERT(typeLayout == nullptr || typeLayout->getKind() == kind);

    Resource* resource = new (m_arena.allocateAligned(sizeof(Resource), SLANG_ALIGN_OF(Resource))) Resource();

    resource->m_kind = kind;
    resource->m_sizeInBytes = sizeInBytes;
    resource->m_elementCount = 0;
    resource->m_type = typeLayout;
    resource->m_userData = nullptr;

    uint8_t* data = (uint8_t*)m_arena.allocateAligned(sizeInBytes, 16);;
    // Clear it
    ::memset(data, 0, sizeInBytes);

    resource->m_data = data;

    m_resources.add(resource);
    return resource;
}

BindSet::Resource* BindSet::newBufferResource(slang::TypeLayoutReflection* typeLayout, size_t sizeInBytes)
{
    const auto kind = typeLayout->getKind();
    switch (kind)
    {
        case slang::TypeReflection::Kind::ParameterBlock:
        case slang::TypeReflection::Kind::ConstantBuffer:
        {
            return newBufferResource(kind, typeLayout, sizeInBytes);
        }
        case slang::TypeReflection::Kind::Resource:
        {
            auto type = typeLayout->getType();
            auto shape = type->getResourceShape();

            switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
            {
                case SLANG_STRUCTURED_BUFFER:
                {
                    auto elementTypeLayout = typeLayout->getElementTypeLayout();
                    size_t elementSize = elementTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM);

                    // We don't know the size of the buffer, but we can work it out, based on what is initialized
                    size_t elementCount = size_t((sizeInBytes + elementSize - 1) / elementSize);
                    size_t bufferSize = elementCount * elementSize;

                    Resource* resource = newBufferResource(kind, typeLayout, bufferSize);
                    resource->m_elementCount = elementCount;
                    return resource;
                }
                case SLANG_BYTE_ADDRESS_BUFFER:
                {
                    return newBufferResource(kind, typeLayout,  (sizeInBytes + 3) & ~size_t(3));
                }
            }
            break;
        }
        default: break;
    }
    return nullptr;
}

BindSet::Resource* BindSet::newBufferResource(slang::TypeLayoutReflection* type, size_t sizeInBytes, const void* initialData)
{
    Resource* resource = newBufferResource(type, sizeInBytes);
    if (!resource)
    {
        return resource;
    }

    SLANG_ASSERT(resource->m_sizeInBytes >= sizeInBytes);
    ::memcpy(resource->m_data, initialData, sizeInBytes);
    return resource;
}

BindSet::Resource* BindSet::newBufferResource(slang::TypeReflection::Kind kind, slang::TypeLayoutReflection* typeLayout, size_t sizeInBytes, const void* initialData)
{
    Resource* resource = newBufferResource(kind, typeLayout, sizeInBytes);
    if (!resource)
    {
        return resource;
    }

    SLANG_ASSERT(resource->m_sizeInBytes >= sizeInBytes);
    ::memcpy(resource->m_data, initialData, sizeInBytes);
    return resource;
}

SlangResult BindSet::init(slang::ShaderReflection* reflection, int entryPointIndex, List<BindLocation>& outRoots, Slang::List<slang::VariableLayoutReflection*>& outVars)
{
    outRoots.clear();
    outVars.clear();

    m_reflection = reflection;
    m_rootBuffer = nullptr;
    m_entryPointBuffer = nullptr;

    m_resources.clear();
    m_arena.deallocateAll();

    {
        size_t globalConstantBuffer = reflection->getGlobalConstantBufferSize();

        size_t rootSizeInBytes = 0;
        const int parameterCount = reflection->getParameterCount();
        for (int i = 0; i < parameterCount; ++i)
        {
            auto parameter = reflection->getParameterByIndex(i);

            auto offset = parameter->getOffset();

            auto typeLayout = parameter->getTypeLayout();
            auto sizeInBytes = typeLayout->getSize();

            size_t endOffset = offset + sizeInBytes;

            rootSizeInBytes = (endOffset > rootSizeInBytes) ? endOffset : rootSizeInBytes;
        }
        SLANG_ASSERT(rootSizeInBytes == globalConstantBuffer);

        if (rootSizeInBytes)
        {
            // Allocate the 'root' buffer
            m_rootBuffer = newBufferResource(slang::TypeReflection::Kind::ConstantBuffer, nullptr,  rootSizeInBytes);

            for (int i = 0; i < parameterCount; ++i)
            {
                auto parameter = reflection->getParameterByIndex(i);
                auto typeLayout = parameter->getTypeLayout();

                BindLocation location(this, typeLayout, m_rootBuffer, parameter->getOffset());
                outRoots.add(location);
                outVars.add(parameter);
            }
        }
    }

    {
        auto entryPointCount = int(reflection->getEntryPointCount());
        if (entryPointIndex < 0 || entryPointIndex >= entryPointCount)
        {
            SLANG_ASSERT(!"Entry point index out of range");
            return SLANG_FAIL;
        }

        m_entryPoint = reflection->getEntryPointByIndex(entryPointIndex);
        size_t entryPointParamsSizeInBytes = 0;

        const int parameterCount = int(m_entryPoint->getParameterCount());
        for (int i = 0; i < parameterCount; i++)
        {
            slang::VariableLayoutReflection* parameter = m_entryPoint->getParameterByIndex(i);

            // If has a semantic, then isn't uniform parameter
            if (auto semanticName = parameter->getSemanticName())
            {
                continue;
            }

            auto offset = parameter->getOffset();

            auto typeLayout = parameter->getTypeLayout();
            auto sizeInBytes = typeLayout->getSize();

            size_t endOffset = offset + sizeInBytes;
            entryPointParamsSizeInBytes = (endOffset > entryPointParamsSizeInBytes) ? endOffset : entryPointParamsSizeInBytes;
        }

        if (entryPointParamsSizeInBytes)
        {
            m_entryPointBuffer = newBufferResource(slang::TypeReflection::Kind::ConstantBuffer, nullptr, entryPointParamsSizeInBytes);

            for (int i = 0; i < parameterCount; ++i)
            {
                auto parameter = m_entryPoint->getParameterByIndex(i);
                // If has a semantic, then isn't uniform parameter
                if (auto semanticName = parameter->getSemanticName())
                {
                    continue;
                }

                auto typeLayout = parameter->getTypeLayout();

                BindLocation location(this, typeLayout, m_rootBuffer, parameter->getOffset());
                outRoots.add(location);
                outVars.add(parameter);
            }
        }
    }

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BindLocation !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

size_t BindLocation::getOffset(SlangParameterCategory category) const
{
    if (m_resource)
    {
        // TODO(JS): 
        // Might want to check if the category seems reasonable.
        // Can be one of a number of types
        return m_offset;
    }

    if (m_bindPointSet)
    {
        return m_bindPointSet->m_points.m_points[category].m_offset;
    }
    else
    {
        SLANG_ASSERT(m_category == category);
        return m_offset;
    }
}

BindLocation BindLocation::toField(const char* name) const
{
    if (!isValid())
    {
        return *this;
    }

    auto typeLayout = m_typeLayout;

    // TODO(JS): Is this reasonable in the general case? Constant buffer seems reasonable
    // parameter block not so much. We could just see if m_resource is set, and then do that in that case.
    //
    // Strip constantBuffer wrapping
    {
        const auto kind = typeLayout->getKind();
        if (kind == slang::TypeReflection::Kind::ConstantBuffer ||
            kind == slang::TypeReflection::Kind::ParameterBlock)
        {
            // Lookup the current location in the resource
            if (m_resource == nullptr)
            {
                // Invalid
                return BindLocation();
            }

            // Follow the pointer
            BindSet::Resource* value = m_bindSet->getAt(m_resource, m_offset);
            if (!value)
            {
                // There isn't anything attached there
                return BindLocation();
            }

            typeLayout = typeLayout->getElementTypeLayout();
            return BindLocation(m_bindSet, typeLayout, value, 0).toField(name);
        }
    }

    {
        const auto kind = typeLayout->getKind();
        if (kind == slang::TypeReflection::Kind::Struct)
        {
            slang::VariableLayoutReflection* varLayout = nullptr;
            auto fieldCount = typeLayout->getFieldCount();
            for (uint32_t ff = 0; ff < fieldCount; ++ff)
            {
                auto field = typeLayout->getFieldByIndex(ff);
                if (strcmp(field->getName(), name) == 0)
                {
                    if (m_resource)
                    {
                        // It must just be an offset
                        return BindLocation(m_bindSet, field->getTypeLayout(), m_resource, m_offset + field->getOffset());
                    }
                    else
                    {
                        if (field->getCategory() == SLANG_PARAMETER_CATEGORY_MIXED)
                        {
                            // Calculate the new ranges
                            SLANG_ASSERT(m_bindPointSet);
                            // Copy over what we have
                            BindPoints points(m_bindPointSet->m_points);

                            for (Index i = 0; i < SLANG_PARAMETER_CATEGORY_COUNT; ++i)
                            {
                                const auto category = SlangParameterCategory(i);

                                auto offset = field->getOffset(category);
                                auto space = field->getBindingSpace(category);

                                auto& point = points.m_points[i];

                                point.m_offset += size_t(offset);
                                // TODO(JS): Does the space just get replaced, or added to?
                                point.m_space = Index(space);
                            }

                            return BindLocation(m_bindSet, field->getTypeLayout(), points);
                        }
                        else
                        {
                            return BindLocation(m_bindSet, field->getTypeLayout(), field->getCategory(), getOffset(field->getCategory()), m_space);
                        }
                    }
                }
            }
        }
    }

    // Invalid
    return BindLocation();
}

BindLocation BindLocation::toIndex(int index) const
{
    if (!isValid())
    {
        return *this;
    }
    SLANG_ASSERT(index >= 0);
    if (index < 0)
    {
        return BindLocation();
    }

    auto typeLayout = m_typeLayout;
    
    const auto kind = typeLayout->getKind();
    switch (kind)
    {
        case slang::TypeReflection::Kind::Array:
        {
            auto elementTypeLayout = typeLayout->getElementTypeLayout();
            const auto elementCount = int(typeLayout->getElementCount());
            
            if (elementCount == 0)
            {
                // Indirects. It could be to a resource, or could be to a register range in a space
                // That breaks our simple model though... in so far that we don't directly know
                //
                // We could *assume* that if we are in m_resource, that we must be somehow 'bindless' and
                // so we can therefore lookup, using that. If not we could assume it is via spaces.
                // For that to be true we should find a non uniform strides. 

                const auto elementStride = typeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
                if (elementStride && m_resource == nullptr)
                {
                    // Must be in a resource
                    SLANG_ASSERT(!"A resource needed to hold array, none set");
                    return BindLocation();
                }

                if (m_resource)
                {
                    BindSet::Resource* value = m_bindSet->getAt(m_resource, m_offset);
                    if (!value)
                    {
                        return BindLocation();
                    }

                    return BindLocation(m_bindSet, elementTypeLayout, m_resource, elementStride * index);
                }
            }
            else
            {
                // Check it's in range
                if (index >= elementCount)
                {
                    SLANG_ASSERT(index < elementCount);
                    return BindLocation();
                }
            }

            if (m_resource)
            {
                const auto elementStride = typeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
                return BindLocation(m_bindSet, elementTypeLayout, m_resource, m_offset + elementStride * index);
            }
            else
            {
                if (m_bindPointSet)
                {
                    // Copy over what we have
                    BindPoints points(m_bindPointSet->m_points);

                    for (Index i = 0; i < SLANG_PARAMETER_CATEGORY_COUNT; ++i)
                    {
                        const auto category = SlangParameterCategory(i);

                        const auto elementStride = typeLayout->getElementStride(category);

                        auto& point = points.m_points[i];

                        point.m_offset += size_t(elementStride * index);
                        // TODO(JS): Looks like we don't do anything with the space
                    }

                    return BindLocation(m_bindSet, elementTypeLayout, points);
                }
                else
                {
                    // We should have a category
                    SLANG_ASSERT(m_category != SLANG_PARAMETER_CATEGORY_NONE);

                    const auto elementStride = typeLayout->getElementStride(m_category);
                    SLANG_ASSERT(elementStride);
                     
                    return BindLocation(m_bindSet, elementTypeLayout, m_category, m_offset + size_t(elementStride * index), m_space);
                }
            }
        }
        default: break;
    }

    // Invalid
    return BindLocation();
}



} // renderer_test
