
#include "bind-location.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-token-reader.h"

namespace renderer_test {
using namespace Slang;

/* static */const BindLocation BindLocation::Invalid;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BindSet !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

BindSet::BindSet():
    m_arena(4096, 16)
{
}

BindSet::~BindSet()
{
    for (auto resource : m_resources)
    {
        resource->~Resource();
    }
}

void BindSet::setAt(const BindLocation& loc, Resource* resource)
{
    SLANG_ASSERT(loc.isValid());
    if (loc.isInvalid())
    {
        return;
    }

    // Note we don't remove when value == null, such that it is stored if should be nullptr
    Resource** resourcePtr = m_bindings.TryGetValueOrAdd(loc, resource);
    if (resourcePtr)
    {
        *resourcePtr = resource;
    }
}

void BindSet::setAt(const BindLocation& loc, SlangParameterCategory category, Resource* resource)
{
    SLANG_ASSERT(loc.isValid());
    if (loc.isInvalid())
    {
        return;
    }

    const BindPoint* point = loc.getValidBindPointForCategory(category);
    if (point)
    {
        if (loc.m_bindPointSet == nullptr)
        {
            // Can only have one category, so just set on that
            setAt(loc, resource);
        }
        else
        {

            BindLocation catLoc(loc.m_typeLayout, category, *point, loc.m_resource);
            setAt(catLoc, resource);
        }
    }
    else
    {
        SLANG_ASSERT(!"Does not have category");
    }
}

BindSet::Resource* BindSet::getAt(const BindLocation& loc) const
{
    SLANG_ASSERT(loc.isValid());
    if (loc.isInvalid())
    {
        return nullptr;
    }
    Resource** resourcePtr = m_bindings.TryGetValue(loc);
    return resourcePtr ? *resourcePtr : nullptr;
}

BindSet::Resource* BindSet::_createBufferResource(slang::TypeReflection::Kind kind, slang::TypeLayoutReflection* typeLayout, size_t bufferSizeInBytes, size_t initialSizeInBytes, const void* initialData)
{
    SLANG_ASSERT(typeLayout == nullptr || typeLayout->getKind() == kind);

    Resource* resource = new (m_arena.allocateAligned(sizeof(Resource), SLANG_ALIGN_OF(Resource))) Resource();

    resource->m_kind = kind;
    resource->m_sizeInBytes = bufferSizeInBytes;
    resource->m_elementCount = 0;
    resource->m_type = typeLayout;
    resource->m_userIndex = -1;

    resource->m_data = (uint8_t*)m_arena.allocateAligned(bufferSizeInBytes, 16);

    SLANG_ASSERT(initialSizeInBytes <= resource->m_sizeInBytes);
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

/* static */bool BindSet::isTextureType(slang::TypeLayoutReflection* typeLayout)
{
    switch (typeLayout->getKind())
    {
        case slang::TypeReflection::Kind::Resource:
        {
            auto type = typeLayout->getType();
            auto shape = type->getResourceShape();

            switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
            {
                case SLANG_TEXTURE_2D:
                case SLANG_TEXTURE_1D:
                case SLANG_TEXTURE_3D:
                case SLANG_TEXTURE_CUBE:
                case SLANG_TEXTURE_BUFFER:
                {
                    return true;
                }
            }
        }
        default: break;
    }

    return false;
}

BindSet::Resource* BindSet::createTextureResource(slang::TypeLayoutReflection* typeLayout)
{
    if (!isTextureType(typeLayout))
    {
        SLANG_ASSERT(!"Not a texture type");
        return nullptr;
    }

    Resource* resource = new (m_arena.allocateAligned(sizeof(Resource), SLANG_ALIGN_OF(Resource))) Resource();

    resource->m_kind = typeLayout->getKind();
    resource->m_sizeInBytes = 0;
    resource->m_elementCount = 0;
    resource->m_type = typeLayout;
    resource->m_data = nullptr;
    resource->m_userIndex = -1;

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

    SLANG_ASSERT(!"Unable to construct this type of buffer");
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

        // I guess we should remove any bindings to it whilst we are at it
        List<BindLocation> locations;
        for (const auto& pair : m_bindings)
        {
            const auto& location = pair.Key;
            if (location.m_resource == resource)
            {
                locations.add(location);
            }
        }

        for (auto location : locations)
        {
            m_bindings.Remove(location);
        }

        // Run the dtor
        resource->~Resource();
    }
}

void BindSet::calcChildResourceLocations(const BindLocation& location, List<BindLocation>& outLocations)
{
    auto typeLayout = location.getTypeLayout();

    const auto kind = typeLayout->getKind();
    switch (kind)
    {
        case slang::TypeReflection::Kind::Array:
        {
            auto elementTypeLayout = typeLayout->getElementTypeLayout(); 
            auto elementCount = int(typeLayout->getElementCount());

            // We only iterate over the array, if it's a fixed array (not an unbounded array)
            // as it is then the elements are much like the fields of a struct and so 'children'.
            if (elementCount != 0)
            {
                for (Index i = 0; i < elementCount; ++i)
                {
                    BindLocation elementLocation = toIndex(location, i);
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
                BindLocation fieldLocation = toField(location, field);

                calcChildResourceLocations(fieldLocation, outLocations);
            }
            break;
        }

        default: break;
    }
}

void BindSet::calcResourceLocations(const BindLocation& location, Slang::List<BindLocation>& outLocations)
{
    auto typeLayout = location.getTypeLayout();

    const auto kind = typeLayout->getKind();
    switch (kind)
    {
        case slang::TypeReflection::Kind::Array:
        {
            auto elementTypeLayout = typeLayout->getElementTypeLayout();
            auto elementCount = int(typeLayout->getElementCount());

            // If it's unbounded, it could point directly to a resource. We can't iterate over it
            // as 'children' because being an external resource (or in a register space) they
            // are not part of the underling location. 
            if (elementCount == 0)
            {
                outLocations.add(location);
            }
            break;
        }

        case slang::TypeReflection::Kind::SamplerState:

        case slang::TypeReflection::Kind::ParameterBlock:
        case slang::TypeReflection::Kind::ConstantBuffer:
        case slang::TypeReflection::Kind::Resource:
        case slang::TypeReflection::Kind::TextureBuffer:
        case slang::TypeReflection::Kind::ShaderStorageBuffer:
        {
            //auto elementTypeLayout = typeLayout->getElementTypeLayout();
            //const size_t elementSize = elementTypeLayout->getSize();

            outLocations.add(location);
            break;
        }
        default:
        {
            calcChildResourceLocations(location, outLocations);
            break;
        }
    }
}

BindLocation BindSet::toField(const BindLocation& loc, slang::VariableLayoutReflection* field) const
{
    const Index categoryCount = Index(field->getCategoryCount());
    if (categoryCount == 0)
    {
        return BindLocation::Invalid;
    }

    if (loc.m_bindPointSet)
    {
        BindPoints bindPoints;
        bindPoints.setInvalid();

        // Copy over and add the ones found here
        for (Index i = 0; i < categoryCount; ++i)
        {
            auto category = field->getCategoryByIndex(unsigned int(i));

            auto const& point = loc.m_bindPointSet->m_points[category];
            if (point.isInvalid())
            {
                return BindLocation::Invalid;
            }

            auto space = field->getBindingSpace(category);
            auto offset = field->getOffset(category);

            // Set using new space, and offset
            bindPoints[category] = BindPoint(space, point.m_offset + offset);
        }

        return BindLocation(field->getTypeLayout(), bindPoints, loc.m_resource);
    }
    else
    {
        SLANG_ASSERT(categoryCount == 1);
        auto category = field->getCategoryByIndex(0);

        // If I'm going from mixed, then I will have multiple items being tracked (so won't be here)
        // If I'm not, then I'm getting an inplace field. It must be relative
        // So it would seem I never need to call getBindingIndex, and since I can't do that it must be relative.
        // AND if it's relative well it must be in the same category.
      
        if (category == loc.m_category)
        {
            auto space = field->getBindingSpace(category);
            auto offset = field->getOffset(category);

            return BindLocation(field->getTypeLayout(), category, BindPoint(space, loc.m_point.m_offset + offset), loc.m_resource);
        }
    }

    return BindLocation::Invalid;
}

BindLocation BindSet::toField(const BindLocation& loc, const char* name) const
{
    if (!loc.isValid())
    {
        return loc;
    }

    auto typeLayout = loc.m_typeLayout;
    const auto kind = typeLayout->getKind();

    // Strip constantBuffer wrapping, only really applies when we have handles to resources
    // embedded in other types (like on CPU and CUDA)
    if (loc.m_resource &&
        (kind == slang::TypeReflection::Kind::ConstantBuffer || kind == slang::TypeReflection::Kind::ParameterBlock))
    {
        // Follow the to associated resource
        BindSet::Resource* value = getAt(loc);
        if (value)
        {
            typeLayout = typeLayout->getElementTypeLayout();
            return toField(BindLocation(typeLayout, SLANG_PARAMETER_CATEGORY_UNIFORM, BindPoint(0, 0), value), name);
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
                return toField(loc, field);
            }
        }
    }

    // Invalid
    return BindLocation::Invalid; 
}

BindLocation BindSet::toIndex(const BindLocation& loc, Index index) const
{
    if (!loc.isValid())
    {
        return loc;
    }
    SLANG_ASSERT(index >= 0);
    if (index < 0)
    {
        return BindLocation::Invalid;
    }

    auto typeLayout = loc.m_typeLayout;
    const auto kind = typeLayout->getKind();

    // If it's a zero sized array, we may need to special case indirecting through a buffer that holds it's contents
    if (kind != slang::TypeReflection::Kind::Array)
    {
        return BindLocation::Invalid;
    }

    // Find where the uniform aspect will be held
    BindSet::Resource* uniformResource = loc.m_resource;
    if (typeLayout->getElementCount() == 0)
    {
        // If we have a resource at this location, then we need to offset through that
        BindSet::Resource* arrayResource = getAt(loc);
        if (arrayResource)
        {
            uniformResource = arrayResource;
        }
    }

    auto elementTypeLayout = typeLayout->getElementTypeLayout();

    const Index categoryCount = Index(elementTypeLayout->getCategoryCount());

    if (loc.m_bindPointSet)
    {
        BindPoints bindPoints;
        bindPoints.setInvalid();

        // Copy over and add the ones found here
        for (Index i = 0; i < categoryCount; ++i)
        {
            auto category = elementTypeLayout->getCategoryByIndex(unsigned int(i));
            const auto elementStride = typeLayout->getElementStride(category);
            const auto& basePoint = loc.m_bindPointSet->m_points[category];
            SLANG_ASSERT(basePoint.isValid());

            bindPoints[category] = BindPoint(basePoint.m_space, basePoint.m_offset + elementStride * index);
        }

        return BindLocation(elementTypeLayout, bindPoints, uniformResource);
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
        const size_t base = (category == loc.m_category) ? loc.m_point.m_offset : 0;

        BindPoint point(loc.m_point.m_space, base + elementStride * index);

        return BindLocation(elementTypeLayout, category, point, uniformResource);
    }

    return BindLocation::Invalid;
}


SlangResult BindSet::setBufferContents(const BindLocation& loc, const void* initialData, size_t sizeInBytes) const
{
    BindSet::Resource* resource = getAt(loc);
    if (resource)
    {
        // Truncate if initial data is larger than the buffer
        sizeInBytes = (sizeInBytes > resource->m_sizeInBytes) ? resource->m_sizeInBytes : sizeInBytes;

        SLANG_ASSERT(resource->m_sizeInBytes >= sizeInBytes);
        ::memcpy(resource->m_data, initialData, sizeInBytes);
        return SLANG_OK;
    }
    return SLANG_FAIL;
}

void BindSet::getBindings(List<BindLocation>& outLocations, List<Resource*>& outResources) const
{
    outResources.clear();
    outLocations.clear();
    for (const auto& pair : m_bindings)
    {
        outLocations.add(pair.Key);
        outResources.add(pair.Value);
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BindLocation !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

BindLocation::BindLocation(slang::TypeLayoutReflection* typeLayout, const BindPoints& points, BindSet_Resource* resource) :
    m_typeLayout(typeLayout)
{
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

BindLocation::BindLocation(slang::TypeLayoutReflection* typeLayout, SlangParameterCategory category, const BindPoint& point, BindSet_Resource* resource) :
    m_category(category),
    m_point(point),
    m_typeLayout(typeLayout),
    m_resource(resource)
{
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

const BindPoint* BindLocation::getValidBindPointForCategory(SlangParameterCategory category) const
{
    const BindPoint* point = nullptr;
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

void* BindLocation::getUniform(size_t sizeInBytes) const
{
    const BindPoint* point = getValidBindPointForCategory(SLANG_PARAMETER_CATEGORY_UNIFORM);
    if (m_resource && point)
    {
        size_t offset = point->m_offset;
        // Make sure it's in range
        if (offset + sizeInBytes <= m_resource->m_sizeInBytes)
        {
            return m_resource->m_data + offset;
        }
    }
    return nullptr;
}

SlangResult BindLocation::setUniform(const void* data, size_t sizeInBytes) const
{
    // It has to be a location with uniform
    const BindPoint* point = getValidBindPointForCategory(SLANG_PARAMETER_CATEGORY_UNIFORM);
    if (m_resource && point)
    {
        size_t offset = point->m_offset;
        // Make sure it's in range
        SLANG_ASSERT(offset + sizeInBytes <= m_resource->m_sizeInBytes);

        // Okay copy the contents
        ::memcpy(m_resource->m_data + offset, data, sizeInBytes);
        return SLANG_OK;
    }
    return SLANG_FAIL;
}

bool BindLocation::operator==(const ThisType& rhs) const
{
    if (m_typeLayout != rhs.m_typeLayout ||
        m_resource != rhs.m_resource)
    {
        return false;
    }

    // If same, then if it's set they must be equal
    // If not set, then must be the same category/point
    if (m_bindPointSet == rhs.m_bindPointSet)
    {
        return m_bindPointSet || (m_category == rhs.m_category && m_point == rhs.m_point);
    }

    // Only way these can be equal now, is if both are m_bindPointSet are different pointers, but same value
    return (m_bindPointSet && rhs.m_bindPointSet) && (m_bindPointSet->m_points == rhs.m_bindPointSet->m_points);
}

int BindLocation::GetHashCode() const
{
    if (!m_typeLayout)
    {
        return 1;
    }
    if (m_bindPointSet)
    {
        return m_bindPointSet->GetHashCode();
    }
    else
    {
        return Slang::combineHash(Slang::combineHash(m_category, Slang::GetHashCode(m_typeLayout)), m_point.GetHashCode());
    }
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BindRoot !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult BindRoot::parse(const String& text, const String& sourcePath, WriterHelper outStream, BindLocation& outLocation)
{
    SLANG_ASSERT(m_bindSet);

    // We will parse the 'name' as may be path to a resource
    TokenReader parser(text);

    BindLocation location = BindLocation::Invalid; 

    {
        Token nameToken = parser.ReadToken();
        if (nameToken.Type != TokenType::Identifier)
        {
            outStream.print("Invalid input syntax at line %d", int(parser.NextToken().Position.Line));
            return SLANG_FAIL;
        }
        location = find(nameToken.Content.getBuffer());
        if (location.isInvalid())
        {
            outStream.print("Unable to find entry in '%s' for '%s' (for CPU name must be specified) \n", sourcePath.getBuffer(), text.getBuffer());
            return SLANG_FAIL;
        }
    }

    while (!parser.IsEnd())
    {
        Token token = parser.NextToken(0);

        if (token.Type == TokenType::LBracket)
        {
            parser.ReadToken();
            int index = parser.ReadInt();
            SLANG_ASSERT(index >= 0);

            location = m_bindSet->toIndex(location, index);
            if (location.isInvalid())
            {
                outStream.print("Unable to find entry in '%d' in '%s'\n", index, text.getBuffer());
                return SLANG_FAIL;
            }
            parser.ReadMatchingToken(TokenType::RBracket);
        }
        else if (token.Type == TokenType::Dot)
        {
            parser.ReadToken();
            Token identifierToken = parser.ReadMatchingToken(TokenType::Identifier);

            location = m_bindSet->toField(location, identifierToken.Content.getBuffer());
            if (location.isInvalid())
            {
                outStream.print("Unable to find field '%s' in '%s'\n", identifierToken.Content.getBuffer(), text.getBuffer());
                return SLANG_FAIL;
            }
        }
        else if (token.Type == TokenType::Comma)
        {
            // Break out
            break;
        }
        else
        {
           return SLANG_FAIL;
        }
    }

    outLocation = location;
    return SLANG_OK;
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
        return BindLocation::Invalid;
    }

    // We don't need to worry about bindSpace because variable must be stored in the buffer 
    // auto space = varLayout->getBindingSpace();
    // TODO(JS): Where is getBindingIndex supposed to be used. It seems the offset here will do the right thing
    auto offset = varLayout->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM);

    return BindLocation(varLayout->getTypeLayout(), SLANG_PARAMETER_CATEGORY_UNIFORM, BindPoint(0, offset), resource);
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
    auto elementTypeLayout = typeLayout->getElementTypeLayout();

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
            ::memset(resource->m_data + elementStride * resource->m_elementCount, 0, (count - resource->m_elementCount) * elementStride);
            resource->m_elementCount = count;
            return SLANG_OK;
        }
    }

    // Ok allocate a buffer that can hold all the elements
    
    const size_t newBufferSize = count * elementStride;

    Resource* newBuffer = m_bindSet->createBufferResource(slang::TypeReflection::Kind::Array, newBufferSize);
    newBuffer->m_elementCount = count;

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

            BindLocation location(parameter->getTypeLayout(), SLANG_PARAMETER_CATEGORY_UNIFORM, BindPoint(0, offset), m_entryPointBuffer);
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

            BindLocation location(parameter->getTypeLayout(), SLANG_PARAMETER_CATEGORY_UNIFORM, BindPoint(0, offset), m_rootBuffer);
            outLocations.add(location);
        }
    }
}

static void _addDefaultBuffersRec(BindSet* bindSet, const BindLocation& loc)
{
    // See if there is a resource attached there
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

                // We create using typeLayout (as opposed to elementTypeLayout), because it also holds the wrapping
                // 'resource' type.
                resource = bindSet->createBufferResource(typeLayout, elementSize);
                SLANG_ASSERT(resource);

                bindSet->setAt(loc, resource);
            }

            // Recurse into buffer, using the elementType
            BindLocation childLocation(elementTypeLayout, SLANG_PARAMETER_CATEGORY_UNIFORM, BindPoint(0, 0), resource );
            _addDefaultBuffersRec(bindSet, childLocation);
            return;
        }
        default: break;    
    }

    // Recurse
    {
        List<BindLocation> childLocations;
        bindSet->calcChildResourceLocations(loc, childLocations);
        for (auto& childLocation : childLocations)
        {
            _addDefaultBuffersRec(bindSet, childLocation);
        }
    }
}

void CPULikeBindRoot::addDefaultBuffers()
{

    List<BindLocation> rootLocations;
    getRoots(rootLocations);

    for (auto& location : rootLocations)
    {
        _addDefaultBuffersRec(m_bindSet, location);
    }
}

} // renderer_test
