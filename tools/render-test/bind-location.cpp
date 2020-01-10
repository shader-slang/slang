
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

BindSet::Resource* BindSet::getAt(SlangParameterCategory category, const BindPoint& point)
{
    RegisterLocation location;
    location.m_category = category;
    location.m_point = point;
    
    Resource** resourcePtr = m_registerBindings.TryGetValue(location);
    return resourcePtr ? *resourcePtr : nullptr;
}

void BindSet::setAt(SlangParameterCategory category, const BindPoint& point, Resource* value)
{
    RegisterLocation location;

    location.m_category = category;
    location.m_point = point;
    
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
        setAt(loc.m_resource, loc.m_point.m_offset, resource);
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
            setAt(loc.m_category, loc.m_point, resource);
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
        setAt(loc.m_resource, loc.m_point.m_offset, resource);
    }
    else
    {
        // We should have a single category to be able to lookup
        SLANG_ASSERT(loc.m_bindPointSet == nullptr);
        if (loc.m_bindPointSet)
        {
            const auto& point = loc.m_bindPointSet->m_points.m_points[category];
            setAt(category, point, resource);
        }
        else
        {
            SLANG_ASSERT(loc.m_category == category);
            setAt(loc.m_category, loc.m_point, resource);
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
        return getAt(loc.m_resource, loc.m_point.m_offset);
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

        return getAt(loc.m_category, loc.m_point);
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

BindLocation::BindLocation(BindSet* bindSet, slang::TypeLayoutReflection* typeLayout, const BindPoints& points, BindSet_Resource* resource) :
    m_bindSet(bindSet),
    m_typeLayout(typeLayout)
{
    SLANG_ASSERT(bindSet);
    Slang::Index categoryIndex = points.findSingle();
    if (categoryIndex >= 0)
    {
        m_category = SlangParameterCategory(categoryIndex);
        m_point = points.m_points[categoryIndex];
    }
    else
    {
        m_category = SLANG_PARAMETER_CATEGORY_NONE;
        m_point.setInvalid();
        m_bindPointSet = new BindPointSet(points);
    }
}

BindLocation::BindLocation(BindSet* bindSet, slang::TypeLayoutReflection* typeLayout, SlangParameterCategory category, const BindPoint& point, BindSet_Resource* resource) :
    m_bindSet(bindSet),
    m_category(category),
    m_point(point),
    m_typeLayout(typeLayout),
    m_resource(resource)
{
    SLANG_ASSERT(bindSet);
}

BindPoint* BindLocation::getValidBindPointForCategory(SlangParameterCategory category) 
{
    BindPoint* point = nullptr;
    if (m_bindPointSet)
    {
         point = &m_bindPointSet->m_points.m_points[category];
    }
    else if (m_category == category)
    {
        point = &m_point;
    }
    return (point && point->isValid()) ? point : nullptr;
}

BindPoint BindLocation::getBindPointForCategory(SlangParameterCategory category) const
{
    if (m_bindPointSet)
    {
        return m_bindPointSet->m_points.m_points[category];
    }
    else if (m_category == category)
    {
        return m_point;
    }
    return BindPoint::makeInvalid();
}

void BindLocation::setPoints(const BindPoints& points)
{
    Index found;
    auto const validCount = points.calcValidCount(&found);

    // There is nothing tracked, so we are done.
    if (validCount == 0)
    {
        setEmptyBinding();
        return;
    }

    if (validCount == 1)
    {
        m_bindPointSet.setNull();
        m_point = points.m_points[found];
        m_category = SlangParameterCategory(found);
        return;
    }

    if (m_bindPointSet->isUniquelyReferenced())
    {
        m_bindPointSet->m_points = points;
    }
    else
    {
        m_bindPointSet = new BindPointSet(points);
    }
}

void BindLocation::addOffset(SlangParameterCategory category, ptrdiff_t offset)
{
    BindPoint* point = getValidBindPointForCategory(category);
    if (point)
    {
        point->m_offset += offset;
    }
}

BindLocation BindLocation::toField(slang::VariableLayoutReflection* field) const
{
    const Index categoryCount = Index(field->getCategoryCount());
    if (categoryCount == 0)
    {
        return BindLocation();
    }

    if (m_bindPointSet)
    {
        BindPoints bindPoints;
        bindPoints.setInvalid();

        // Copy over and add the ones found here
        for (Index i = 0; i < categoryCount; ++i)
        {
            auto category = field->getCategoryByIndex(unsigned int(i));

            auto const& point = m_bindPointSet->m_points[category];
            if (point.isInvalid())
            {
                return BindLocation();
            }

            auto space = field->getBindingSpace(category);
            auto offset = field->getOffset(category);

            // Set using new space, and offset
            bindPoints[category] = BindPoint(space, point.m_offset + offset);
        }

        return BindLocation(m_bindSet, field->getTypeLayout(), bindPoints, m_resource);
    }
    else
    {
        SLANG_ASSERT(categoryCount == 1);
        auto category = field->getCategoryByIndex(0);
 
        // TODO(JS): This is a field so I don't need to call
        // If I'm going from mixed, then I will have multiple items being tracked (so won't be here)
        // If I'm not, then I'm getting an inplace field. It must be relative
        // So it would seem I never need to call, and since I can't do that it must be relative.
        // AND if it's relative well it must be in the same category.
        // var->getBindingIndex()

        if (category == m_category)
        {
            auto space = field->getBindingSpace(category);
            auto offset = field->getOffset(category);

            return BindLocation(m_bindSet, field->getTypeLayout(), category, BindPoint(space, m_point.m_offset + offset), m_resource);
        }
    }

    // Invalid
    return BindLocation();
}

BindLocation BindLocation::toField(const char* name) const
{
    if (!isValid())
    {
        return *this;
    }

    auto typeLayout = m_typeLayout;
    const auto kind = typeLayout->getKind();

    // Strip constantBuffer wrapping, only really applies when we have handles to resources
    // embedded in other types (like on CPU and CUDA)
    if (m_resource && 
        (kind == slang::TypeReflection::Kind::ConstantBuffer || kind == slang::TypeReflection::Kind::ParameterBlock))
    {
        // Follow the pointer
        BindSet::Resource* value = m_bindSet->getAt(m_resource, m_point.m_offset);
        if (value)
        {
            typeLayout = typeLayout->getElementTypeLayout();
            return BindLocation(m_bindSet, typeLayout, SLANG_PARAMETER_CATEGORY_UNIFORM, BindPoint(0, 0), value).toField(name);
        }
    }

    if (kind == slang::TypeReflection::Kind::Struct)
    {
        slang::VariableLayoutReflection* varLayout = nullptr;
        auto fieldCount = typeLayout->getFieldCount();
        for (uint32_t ff = 0; ff < fieldCount; ++ff)
        {
            auto field = typeLayout->getFieldByIndex(ff);
            if (strcmp(field->getName(), name) == 0)
            {
                return toField(field);
            }
        }
    }

    // Invalid
    return BindLocation();
}

BindLocation BindLocation::toIndex(Index index) const
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

    // If it's a zero sized array, we may need to special case indirecting through a buffer that holds it's contents
    if (kind != slang::TypeReflection::Kind::Array)
    {
        return BindLocation();
    }

    // Find where the uniform aspect will be held
    BindSet::Resource* uniformResource = m_resource;
    if (typeLayout->getElementCount() == 0)
    {
        // If we have a resource at this location, then we need to offset through that
        BindSet::Resource* arrayResource = m_bindSet->getAt(*this);
        if (arrayResource)
        {
            uniformResource = arrayResource;
        }
    }

    auto elementTypeLayout = typeLayout->getElementTypeLayout();

    const Index categoryCount = Index(elementTypeLayout->getCategoryCount());

    if (m_bindPointSet)
    {
        BindPoints bindPoints;
        bindPoints.setInvalid();

        // Copy over and add the ones found here
        for (Index i = 0; i < categoryCount; ++i)
        {
            auto category = elementTypeLayout->getCategoryByIndex(unsigned int(i));
            const auto elementStride = typeLayout->getElementStride(category);
            const auto& basePoint = m_bindPointSet->m_points[category];
            SLANG_ASSERT(basePoint.isValid());

            bindPoints[category] = BindPoint(basePoint.m_space, basePoint.m_offset + elementStride * index);
        }

        return BindLocation(m_bindSet, elementTypeLayout, bindPoints, uniformResource);
    }
    else
    {
        SLANG_ASSERT(categoryCount == 1);
        auto category = elementTypeLayout->getCategoryByIndex(0);

        const auto elementStride = typeLayout->getElementStride(category);

        // TODO(JS): 
        // Hmm, if its a different category, then not entirely clear what to do here.
        // Just zero as we can't use the base we have.
        // This might just be an error
        const size_t base = (category == m_category) ? m_point.m_offset : 0;

        BindPoint point(m_point.m_space, base + elementStride * index);

        return BindLocation(m_bindSet, elementTypeLayout, category, point, uniformResource);
    }

    // Invalid
    return BindLocation();
}

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
                for (Index i = 0; i < elementCount; ++i)
                {
                    BindLocation elementLocation = location.toIndex(i);
                    calcChildResourceLocations(elementLocation, outLocations);
                }
            }
            break;
        }
        case slang::TypeReflection::Kind::Struct:
        {
            auto structTypeLayout = typeLayout;
           
            auto fieldCount = structTypeLayout->getFieldCount();
            for (uint32_t ff = 0; ff < fieldCount; ++ff)
            {
                auto field = structTypeLayout->getFieldByIndex(ff);
                BindLocation fieldLocation = location.toField(field);

                calcChildResourceLocations(fieldLocation, outLocations);
            }
            break;
        }
        
        default: break;
    }
}

void BindSet::calcChildResourceLocations(const BindLocation& location, Slang::List<BindLocation>& outLocations)
{
    auto typeLayout = location.getTypeLayout();

    const auto kind = typeLayout->getKind();
    switch (kind)
    {
        case slang::TypeReflection::Kind::ParameterBlock:
        case slang::TypeReflection::Kind::ConstantBuffer:
        {
            auto elementTypeLayout = typeLayout->getElementTypeLayout();
            const size_t elementSize = elementTypeLayout->getSize();

            outLocations.add(location);
            break;
        }
        default:
        {
            calcResourceLocations(location, outLocations);
            break;
        }
    }
}

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
    Resource* resource = nullptr;
    slang::VariableLayoutReflection* varLayout = nullptr;

    if (m_rootBuffer)
    {
        varLayout = getParameterByName(name);
        resource = m_rootBuffer;
    }
        
    if (!varLayout && m_entryPointBuffer)
    {
        resource = m_entryPointBuffer;
        varLayout = getEntryPointParameterByName(name);
    }

    if (!varLayout)
    {
        return BindLocation();
    }

    // We don't need to worry about bindSpace because variable must be stored in the buffer 
    // auto space = varLayout->getBindingSpace();
    // TODO(JS): Where is getBindingIndex supposed to be used. It seems the offset here will do the right thing
    auto offset = varLayout->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM);

    return BindLocation(m_bindSet, varLayout->getTypeLayout(), SLANG_PARAMETER_CATEGORY_UNIFORM, BindPoint(0, offset), resource);
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


void CPULikeBindRoot::getRoots(Slang::List<BindLocation>& outLocations)
{
    if (m_entryPointBuffer)
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

            auto offset = parameter->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM);

            BindLocation location(m_bindSet, parameter->getTypeLayout(), SLANG_PARAMETER_CATEGORY_UNIFORM, BindPoint(0, offset), m_entryPointBuffer);
            outLocations.add(location);
        }
    }

    if (m_rootBuffer)
    {
        const int parameterCount = m_reflection->getParameterCount();
        for (int i = 0; i < parameterCount; ++i)
        {
            auto parameter = m_reflection->getParameterByIndex(i);

            auto offset = parameter->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM);

            BindLocation location(m_bindSet, parameter->getTypeLayout(), SLANG_PARAMETER_CATEGORY_UNIFORM, BindPoint(0, offset), m_rootBuffer);
            outLocations.add(location);
        }
    }
}

static void _addDefaultBuffersRec(const BindLocation& loc)
{
    // See if there is a resource attached there
    BindSet* bindSet = loc.getBindSet();
    auto typeLayout = loc.getTypeLayout();

    const auto kind = typeLayout->getKind();
    switch (kind)
    {   
        case slang::TypeReflection::Kind::ParameterBlock:
        case slang::TypeReflection::Kind::ConstantBuffer:
        {
            BindSet::Resource* resource = bindSet->getAt(loc);

            auto elementTypeLayout = typeLayout->getElementTypeLayout();

            if (!resource)
            {
                //SLANG_ASSERT(typeLayout->getSize() == sizeof(void*));
                const size_t elementSize = elementTypeLayout->getSize();

                resource = bindSet->createBufferResource(elementTypeLayout, elementSize);
                bindSet->setAt(loc, resource);
            }

            // Recurse into buffer
            BindLocation childLocation(bindSet, elementTypeLayout, SLANG_PARAMETER_CATEGORY_UNIFORM, BindPoint(0, 0), resource );
            _addDefaultBuffersRec(childLocation);
            return;
        }
        default: break;    
    }

    // Recurse
    {
        List<BindLocation> childLocations;
        bindSet->calcResourceLocations(loc, childLocations);
        for (auto& childLocation : childLocations)
        {
            _addDefaultBuffersRec(childLocation);
        }
    }
}

void CPULikeBindRoot::addDefaultBuffers()
{
    List<BindLocation> rootLocations;
    getRoots(rootLocations);

    for (auto& location : rootLocations)
    {
        _addDefaultBuffersRec(location);
    }
}

} // renderer_test
