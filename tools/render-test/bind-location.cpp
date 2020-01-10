
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

void BindSet::setAt(const BindLocation& loc, Resource* resource)
{
    SLANG_ASSERT(loc.isValid());
    if (loc.isInvalid())
    {
        return;
    }

    if (loc.m_resource)
    {
        setAt(loc.m_resource, loc.m_offset, resource);
    }
    else
    {
        // We should have a single category to be able to lookup
        SLANG_ASSERT(loc.m_bindPointSet == nullptr);
        if (loc.m_bindPointSet)
        {
            // It's ambiguous. I guess strictly speaking we could search to see if there is a unique entry
            // but getting to this point implies there is more than one option.

            SLANG_ASSERT(!"Setting is ambiguous, so nothing set");
            return;
        }
        else
        {
            setAt(loc.m_category, loc.m_space, loc.m_offset, resource);
        }
    }
}

void BindSet::setAt(const BindLocation& loc, SlangParameterCategory category, Resource* resource)
{
    SLANG_ASSERT(loc.isValid());
    if (loc.isInvalid())
    {
        return;
    }

    if (loc.m_resource)
    {
        setAt(loc.m_resource, loc.m_offset, resource);
    }
    else
    {
        // We should have a single category to be able to lookup
        SLANG_ASSERT(loc.m_bindPointSet == nullptr);
        if (loc.m_bindPointSet)
        {
            const auto& point = loc.m_bindPointSet->m_points.m_points[category];
            setAt(category, point.m_space, point.m_offset, resource);
        }
        else
        {
            SLANG_ASSERT(loc.m_category == category);
            setAt(loc.m_category, loc.m_space, loc.m_offset, resource);
        }
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

BindSet::Resource* BindSet::_createBufferResource(slang::TypeReflection::Kind kind, slang::TypeLayoutReflection* typeLayout, size_t bufferSizeInBytes, size_t initialSizeInBytes, const void* initialData)
{
    SLANG_ASSERT(typeLayout == nullptr || typeLayout->getKind() == kind);

    Resource* resource = new (m_arena.allocateAligned(sizeof(Resource), SLANG_ALIGN_OF(Resource))) Resource();

    resource->m_kind = kind;
    resource->m_sizeInBytes = bufferSizeInBytes;
    resource->m_elementCount = 0;
    resource->m_type = typeLayout;
    resource->m_userData = nullptr;

    resource->m_data = (uint8_t*)m_arena.allocateAligned(bufferSizeInBytes, 16);

    SLANG_ASSERT(sizeInBytes <= resource->m_sizeInBytes);
    if (initialData)
    {
        ::memcpy(resource->m_data, initialData, initialSizeInBytes);
        ::memset(resource->m_data + initialSizeInBytes, 0, bufferSizeInBytes - initialSizeInBytes);
    }
    else
    {
        ::memset(resource->m_data, 0, resource->m_sizeInBytes);
    }

    m_resources.add(resource);
    return resource;
}

BindSet::Resource* BindSet::createBufferResource(slang::TypeReflection::Kind kind, size_t sizeInBytes, const void* initialData)
{
    return _createBufferResource(kind, nullptr, sizeInBytes, sizeInBytes, initialData);
}

BindSet::Resource* BindSet::createBufferResource(slang::TypeLayoutReflection* typeLayout, size_t sizeInBytes, const void* initialData)
{
    const auto kind = typeLayout->getKind();
    switch (kind)
    {
        case slang::TypeReflection::Kind::ParameterBlock:
        case slang::TypeReflection::Kind::ConstantBuffer:
        {
            return _createBufferResource(kind, typeLayout, sizeInBytes, sizeInBytes, initialData);
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

                    Resource* resource = _createBufferResource(kind, typeLayout, bufferSize, sizeInBytes, initialData);
                    resource->m_elementCount = elementCount;
                    return resource;
                }
                case SLANG_BYTE_ADDRESS_BUFFER:
                {
                    return _createBufferResource(kind, typeLayout,  (sizeInBytes + 3) & ~size_t(3), sizeInBytes, initialData);
                }
            }
            break;
        }
        default: break;
    }
    return nullptr;
}

void BindSet::destroyResource(Resource* resource)
{
    // TODO(JS): NOTE we do not free the old buffer. This is not a memory leak, because
    // it is tracked elsewhere, but there is an argument to destroy it.
    const Index index = m_resources.indexOf(resource);
    SLANG_ASSERT(index >= 0);
    if (index >= 0)
    {
        m_resources.fastRemoveAt(index);
    }
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

bool BindLocation::hasCategory(SlangParameterCategory category) const
{
    if (m_resource)
    {
        // TODO(JS): I guess really we need to determine what types are applicable
        // here.. But for now we'll say it's ok
        return true;
    }
    else
    {
        if (m_bindPointSet)
        {
            // TODO(JS):
            // We don't actually know which members of the set are really being used, but
            // things won't break if we alter 
            return true;
        }
        else
        {
            return m_category == category;
        }
    }
}


void BindLocation::addOffset(SlangParameterCategory category, ptrdiff_t offset)
{
    if (m_resource)
    {
        // TODO(JS): I guess strictly speaking it could be something other than uniform
        // Must be in uniform if in a resource
        //SLANG_ASSERT(category == SLANG_PARAMETER_CATEGORY_UNIFORM);
        m_offset += offset;
    }
    else
    {
        if (m_bindPointSet)
        {
            m_bindPointSet->m_points.m_points[category].m_offset += offset;
        }
        else
        {
            SLANG_ASSERT(m_category == category);
            m_offset += offset;
        }
    }
}

BindLocation BindLocation::toField(const char* name) const
{
    if (!isValid())
    {
        return *this;
    }

    auto typeLayout = m_typeLayout;

    // Strip constantBuffer wrapping, only really applies when we have handles to resources
    // embedded in other types (like on CPU and CUDA)
    if (m_resource)
    {
        const auto kind = typeLayout->getKind();
        if (kind == slang::TypeReflection::Kind::ConstantBuffer ||
            kind == slang::TypeReflection::Kind::ParameterBlock)
        {
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

                                // TODO(JS): Offsetting seems appropriate as we are inside a struct
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
                // TODO(JS). This isn't right. That the elements could be implemented by bindings
                // in a separate space for example. 
                //

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

#if  0
void BindSet::calcResourceLocations(const BindLocation& location, List<BindLocation>& outLocations)
{
    auto typeLayout = location.getTypeLayout();

    const auto kind = typeLayout->getKind();
    switch (kind)
    {
        case slang::TypeReflection::Kind::Array:
        {
            auto elementTypeLayout = typeLayout->getElementTypeLayout();
            auto elementCount = int(typeLayout->getElementCount());

            if (elementCount == 0)
            {
                outLocations.add(location);
            }
            else
            {
                // Hmm... I guess only the ones with strides can have values to set

                for (Index i = 0; i < SLANG_PARAMETER_CATEGORY_COUNT; ++i)
                {
                    auto category = SlangParameterCategory(i);
                    const size_t stride = typeLayout->getElementStride(category);

                    if (stride)
                    {
                        BindLocation elementLocation(location);
                        for (int i = 0; i < elementCount; ++i)
                        {
                            elementLocation.addOffset(category, stride);
                            outLocations.add(elementLocation);
                        }
                    }
                }
            }
            break;
        }
        case slang::TypeReflection::Kind::Struct:
        {
            auto structTypeLayout = typeLayout;
            //auto name = structTypeLayout->getName();
            //SLANG_UNUSED(name);

            auto fieldCount = structTypeLayout->getFieldCount();
            for (uint32_t ff = 0; ff < fieldCount; ++ff)
            {
                auto field = structTypeLayout->getFieldByIndex(ff);

                Index categoryCount = field->getCategoryCount();

                if (categoryCount == 0)
                {
                    continue;
                }
                if (categoryCount == 1)
                {
                }

                ParameterCategory getCategoryByIndex(unsigned int index)

                for (Index i = 0; i < SLANG_PARAMETER_CATEGORY_COUNT; ++i)
                {
                    auto category = SlangParameterCategory(i);

                    if (location.hasCategory(category))

                    BindLocation fieldLocation(location);


                    if (fieldLocation)

                    auto offset = field->getOffset(category);

                    fieldLocation.addOffset(offset);

                Buffer fieldBuffer;
                SLANG_RETURN_ON_FAIL(_add(nullptr, field->getTypeLayout(), ((uint8_t*)dst) + offset, fieldBuffer));
            }
            break;
        }
        case slang::TypeReflection::Kind::ParameterBlock:
        case slang::TypeReflection::Kind::ConstantBuffer:
        {
            SLANG_ASSERT(typeLayout->getSize() == sizeof(void*));

            auto elementTypeLayout = typeLayout->getElementTypeLayout();
            const size_t elementSize = elementTypeLayout->getSize();

            outBuffer = _allocateBuffer(elementSize);

            // Constant buffers map to a pointer
            *(void**)dst = outBuffer.m_data;

            // On CPU constant buffers can contain pointers to other resources (including constant buffers)
            Buffer innerBuffer;
            SLANG_RETURN_ON_FAIL(_add(nullptr, elementTypeLayout, outBuffer.m_data, innerBuffer));
            break;
        }
        default: break;
    }
    return SLANG_OK;
}
#endif

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPULikeBindRoot !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult CPULikeBindRoot::init(BindSet* bindSet, slang::ShaderReflection* reflection, int entryPointIndex)
{
    m_bindSet = bindSet;
        
    m_reflection = reflection;
    m_rootBuffer = nullptr;
    m_entryPointBuffer = nullptr;

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
            m_rootBuffer = m_bindSet->createBufferResource(slang::TypeReflection::Kind::ConstantBuffer, rootSizeInBytes);

#if 0
            for (int i = 0; i < parameterCount; ++i)
            {
                auto parameter = reflection->getParameterByIndex(i);
                auto typeLayout = parameter->getTypeLayout();

                //BindLocation location(this, typeLayout, m_rootBuffer, parameter->getOffset());
                //outRoots.add(location);
                //outVars.add(parameter);
            }
#endif
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
            m_entryPointBuffer = m_bindSet->createBufferResource(slang::TypeReflection::Kind::ConstantBuffer, entryPointParamsSizeInBytes);

#if 0
            for (int i = 0; i < parameterCount; ++i)
            {
                auto parameter = m_entryPoint->getParameterByIndex(i);
                // If has a semantic, then isn't uniform parameter
                if (auto semanticName = parameter->getSemanticName())
                {
                    continue;
                }

                //auto typeLayout = parameter->getTypeLayout();

                //BindLocation location(this, typeLayout, m_rootBuffer, parameter->getOffset());
                //outRoots.add(location);
                //outVars.add(parameter);
            }
#endif
        }
    }

    return SLANG_OK;
}


slang::VariableLayoutReflection* CPULikeBindRoot::getParameterByName(const char* name)
{
    const int parameterCount = m_reflection->getParameterCount();
    for (int i = 0; i < parameterCount; ++i)
    {
        auto parameter = m_reflection->getParameterByIndex(i);
        const char* paramName = parameter->getName();
        if (strcmp(name, paramName) == 0)
        {
            return parameter;
        }
    }

    return nullptr;
}

slang::VariableLayoutReflection* CPULikeBindRoot::getEntryPointParameterByName(const char* name)
{
    const int parameterCount = int(m_entryPoint->getParameterCount());
    for (int i = 0; i < parameterCount; ++i)
    {
        auto parameter = m_entryPoint->getParameterByIndex(i);
        // If has a semantic we will ignore
        if (parameter->getSemanticName())
        {
            continue;
        }
        if (strcmp(parameter->getName(), name) == 0)
        {
            return parameter;
        }
    }
    return nullptr;
}

BindLocation CPULikeBindRoot::find(const char* name)
{
    auto varLayout = getParameterByName(name);
    if (varLayout)
    {
        return BindLocation(m_bindSet, varLayout->getTypeLayout(), m_rootBuffer, varLayout->getOffset());
    }

    varLayout = getEntryPointParameterByName(name);
    if (varLayout)
    {
        return BindLocation(m_bindSet, varLayout->getTypeLayout(), m_entryPointBuffer, varLayout->getOffset());
    }
    return BindLocation();
}

SlangResult CPULikeBindRoot::setArrayCount(const BindLocation& location, int count)
{
    if (!location.isValid())
    {
        return SLANG_FAIL;
    }

    // I can see if a resource has already been set
    Resource* resource = m_bindSet->getAt(location);
    
    auto typeLayout = location.getTypeLayout();
    const auto kind = typeLayout->getKind();

    if (!(typeLayout->getKind() == slang::TypeReflection::Kind::Array && typeLayout->getElementCount() == 0))
    {
        return SLANG_FAIL;
    }

    const size_t elementStride = typeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);

    if (resource)
    {
        // Making smaller, just reduce the count.
        // NOTE! Nothing is done here about deallocating resources which are perhaps no longer reachable.
        // This isn't a leakage problem tho, as all buffers are released automatically when scope is left.
        if (count <= int(resource->m_elementCount) || count <= int(resource->m_sizeInBytes / elementStride))
        {
            resource->m_elementCount = count;
            return SLANG_OK;
        }

        const size_t maxElementCount = (resource->m_sizeInBytes / elementStride);
        if (count <= maxElementCount)
        {
            // Just initialize the space
            memset(resource->m_data + elementStride * resource->m_elementCount, 0, (count - resource->m_elementCount) * elementStride);
            resource->m_elementCount = count;
            return SLANG_OK;
        }
    }

    // Ok allocate a buffer that can hold all the elements
    
    const size_t newBufferSize = count * elementStride;
    Resource* newBuffer = m_bindSet->createBufferResource(typeLayout, newBufferSize);

    // Copy over the data from the old buffer if there is any
    if (resource && resource->m_elementCount)
    {
        ::memcpy(newBuffer->m_data, resource->m_data, resource->m_elementCount * elementStride);
    }

    // Remove the old buffer as no longer needed

    if (resource)
    {
        m_bindSet->destroyResource(resource);
    }

    // Set the new buffer
    m_bindSet->setAt(location, newBuffer);
    return SLANG_OK;
}

} // renderer_test
