
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
    for (auto value : m_values)
    {
        value->~Value();
    }
}

void BindSet::setAt(const BindLocation& loc, Value* value)
{
    SLANG_ASSERT(loc.isValid());
    if (loc.isInvalid())
    {
        return;
    }

    // Note we don't remove when value == null, such that it is stored if should be nullptr
    Value** valuePtr = m_bindings.TryGetValueOrAdd(loc, value);
    if (valuePtr)
    {
        *valuePtr = value;
    }
}

void BindSet::setAt(const BindLocation& loc, SlangParameterCategory category, Value* value)
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
            setAt(loc, value);
        }
        else
        {

            BindLocation catLoc(loc.m_typeLayout, category, *point, loc.m_value);
            setAt(catLoc, value);
        }
    }
    else
    {
        SLANG_ASSERT(!"Does not have category");
    }
}

BindSet::Value* BindSet::getAt(const BindLocation& loc) const
{
    SLANG_ASSERT(loc.isValid());
    if (loc.isInvalid())
    {
        return nullptr;
    }
    Value** valuePtr = m_bindings.TryGetValue(loc);
    return valuePtr ? *valuePtr : nullptr;
}

BindSet::Value* BindSet::_createBufferValue(slang::TypeReflection::Kind kind, slang::TypeLayoutReflection* typeLayout, size_t bufferSizeInBytes, size_t initialSizeInBytes, const void* initialData)
{
    SLANG_ASSERT(typeLayout == nullptr || typeLayout->getKind() == kind);

    Value* value = new (m_arena.allocateAligned(sizeof(Value), SLANG_ALIGN_OF(Value))) Value();

    value->m_kind = kind;
    value->m_sizeInBytes = bufferSizeInBytes;
    value->m_elementCount = 0;
    value->m_type = typeLayout;
    value->m_userIndex = -1;

    value->m_data = (uint8_t*)m_arena.allocateAligned(bufferSizeInBytes, 16);

    SLANG_ASSERT(initialSizeInBytes <= value->m_sizeInBytes);
    if (initialData)
    {
        ::memcpy(value->m_data, initialData, initialSizeInBytes);
        ::memset(value->m_data + initialSizeInBytes, 0, bufferSizeInBytes - initialSizeInBytes);
    }
    else
    {
        ::memset(value->m_data, 0, value->m_sizeInBytes);
    }

    m_values.add(value);
    return value;
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

BindSet::Value* BindSet::createTextureValue(slang::TypeLayoutReflection* typeLayout)
{
    if (!isTextureType(typeLayout))
    {
        SLANG_ASSERT(!"Not a texture type");
        return nullptr;
    }

    Value* value = new (m_arena.allocateAligned(sizeof(Value), SLANG_ALIGN_OF(Value))) Value();

    value->m_kind = typeLayout->getKind();
    value->m_sizeInBytes = 0;
    value->m_elementCount = 0;
    value->m_type = typeLayout;
    value->m_data = nullptr;
    value->m_userIndex = -1;

    m_values.add(value);

    return value;
}

BindSet::Value* BindSet::createBufferValue(slang::TypeReflection::Kind kind, size_t sizeInBytes, const void* initialData)
{
    return _createBufferValue(kind, nullptr, sizeInBytes, sizeInBytes, initialData);
}

BindSet::Value* BindSet::createBufferValue(slang::TypeLayoutReflection* typeLayout, size_t sizeInBytes, const void* initialData)
{
    const auto kind = typeLayout->getKind();
    switch (kind)
    {
        case slang::TypeReflection::Kind::ParameterBlock:
        case slang::TypeReflection::Kind::ConstantBuffer:
        {
            return _createBufferValue(kind, typeLayout, sizeInBytes, sizeInBytes, initialData);
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

                    Value* value = _createBufferValue(kind, typeLayout, bufferSize, sizeInBytes, initialData);
                    value->m_elementCount = elementCount;
                    return value;
                }
                case SLANG_BYTE_ADDRESS_BUFFER:
                {
                    return _createBufferValue(kind, typeLayout,  (sizeInBytes + 3) & ~size_t(3), sizeInBytes, initialData);
                }
            }
            break;
        }


        default: break;
    }

    SLANG_ASSERT(!"Unable to construct this type of buffer");
    return nullptr;
}

void BindSet::destroyValue(Value* value)
{
    // TODO(JS): NOTE we do not free the old buffer. This is not a memory leak, because
    // it is tracked elsewhere, but there is an argument to destroy it.
    const Index index = m_values.indexOf(value);
    SLANG_ASSERT(index >= 0);
    if (index >= 0)
    {
        m_values.fastRemoveAt(index);

        // I guess we should remove any bindings to it whilst we are at it
        List<BindLocation> locations;
        for (const auto& pair : m_bindings)
        {
            const auto& location = pair.Key;
            if (location.m_value == value)
            {
                locations.add(location);
            }
        }

        for (auto location : locations)
        {
            m_bindings.Remove(location);
        }

        // Run the dtor
        value->~Value();
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

void BindSet::calcValueLocations(const BindLocation& location, Slang::List<BindLocation>& outLocations)
{
    auto typeLayout = location.getTypeLayout();

    const auto kind = typeLayout->getKind();
    switch (kind)
    {
        case slang::TypeReflection::Kind::Array:
        {
            auto elementTypeLayout = typeLayout->getElementTypeLayout();
            auto elementCount = int(typeLayout->getElementCount());

            // If it's unbounded, it could point directly to a value/resource. We can't iterate over it
            // as 'children' because being an external value/resource (or in a register space) they
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

// Finds the first category from layout reflection that represents an actual value
// i.e. that is not ExistentialType or ExistentialObject.
template<typename LayoutReflectionType>
slang::ParameterCategory getFirstNonExistentialValueCategory(LayoutReflectionType* layout)
{
    slang::ParameterCategory category = slang::ParameterCategory::None;
    for (UInt i = 0; i < layout->getCategoryCount(); i++)
    {
        auto currentCategory = layout->getCategoryByIndex((unsigned int)i);
        if (currentCategory == slang::ParameterCategory::ExistentialTypeParam ||
            currentCategory == slang::ParameterCategory::ExistentialObjectParam)
            continue;
        category = currentCategory;
    }
    return category;
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
            auto category = field->getCategoryByIndex((unsigned int)i);

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

        return BindLocation(field->getTypeLayout(), bindPoints, loc.m_value);
    }
    else
    {
        slang::ParameterCategory category = getFirstNonExistentialValueCategory(field);
        SLANG_ASSERT(category != slang::ParameterCategory::None);

        // If I'm going from mixed, then I will have multiple items being tracked (so won't be here)
        // If I'm not, then I'm getting an inplace field. It must be relative
        // So it would seem I never need to call getBindingIndex, and since I can't do that it must be relative.
        // AND if it's relative well it must be in the same category.
      
        if (category == loc.m_category)
        {
            auto space = field->getBindingSpace(category);
            auto offset = field->getOffset(category);

            return BindLocation(field->getTypeLayout(), category, BindPoint(space, loc.m_point.m_offset + offset), loc.m_value);
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

    // Strip constantBuffer wrapping, only really applies when we have handles to value/resource
    // embedded in other types (like on CPU and CUDA)
    if (loc.m_value &&
        (kind == slang::TypeReflection::Kind::ConstantBuffer || kind == slang::TypeReflection::Kind::ParameterBlock))
    {
        // Follow the to associated value/resource
        BindSet::Value* value = getAt(loc);
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

    // Find where the uniform data will be held. If we have a unsized array, for some targets the actual content's might be in a different location
    BindSet::Value* uniformValue = loc.m_value;
    if (typeLayout->getElementCount() == 0)
    {
        // If we have a value/resource at this location, then we need to offset through that
        BindSet::Value* arrayValue = getAt(loc);
        if (arrayValue)
        {
            uniformValue = arrayValue;

            // Check it's in range.
            // NOTE we can't check this if the unbounded binding is in another space for example. 
            if (index >= Index(uniformValue->m_elementCount))
            {
                return BindLocation::Invalid;
            }
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
            auto category = elementTypeLayout->getCategoryByIndex((unsigned int)i);
            const auto elementStride = typeLayout->getElementStride(category);

            size_t baseOffset = loc.m_bindPointSet->m_points[category].m_offset;

            if (category == slang::ParameterCategory::Uniform && uniformValue != loc.m_value)
            {
                baseOffset = 0;
            }
             
            const auto& basePoint = loc.m_bindPointSet->m_points[category];
            SLANG_ASSERT(basePoint.isValid());
            bindPoints[category] = BindPoint(basePoint.m_space, baseOffset + elementStride * index);
        }

        return BindLocation(elementTypeLayout, bindPoints, uniformValue);
    }
    else
    {
        slang::ParameterCategory category = getFirstNonExistentialValueCategory(elementTypeLayout);
        SLANG_ASSERT(category != slang::ParameterCategory::None);

        const auto elementStride = typeLayout->getElementStride(category);

        size_t baseOffset = 0;
        if (category == slang::ParameterCategory::Uniform && uniformValue != loc.m_value)
        {
            // base of 0 is appropriate as it is the child value
        }
        else
        {
            // TODO(JS): 
            // Hmm, if its a different category, then not entirely clear what to do here.
            // Just zero as we can't use the base we have.
            // This might just be an error

            baseOffset = (category == loc.m_category) ? loc.m_point.m_offset : 0;
        }

        BindPoint point(loc.m_point.m_space, baseOffset + elementStride * index);

        return BindLocation(elementTypeLayout, category, point, uniformValue);
    }

    return BindLocation::Invalid;
}


SlangResult BindSet::setBufferContents(const BindLocation& loc, const void* initialData, size_t sizeInBytes) const
{
    BindSet::Value* value = getAt(loc);
    if (value)
    {
        // Truncate if initial data is larger than the buffer
        sizeInBytes = (sizeInBytes > value->m_sizeInBytes) ? value->m_sizeInBytes : sizeInBytes;

        SLANG_ASSERT(value->m_sizeInBytes >= sizeInBytes);
        ::memcpy(value->m_data, initialData, sizeInBytes);
        return SLANG_OK;
    }
    return SLANG_FAIL;
}

void BindSet::getBindings(List<BindLocation>& outLocations, List<Value*>& outResources) const
{
    outResources.clear();
    outLocations.clear();
    for (const auto& pair : m_bindings)
    {
        outLocations.add(pair.Key);
        outResources.add(pair.Value);
    }
}

void BindSet::releaseValueTargets()
{
    for (Value* value : m_values)
    {
        value->m_target.setNull();
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BindLocation !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

BindLocation::BindLocation(slang::TypeLayoutReflection* typeLayout, const BindPoints& points, BindSet_Value* value) :
    m_typeLayout(typeLayout),
    m_value(value)
{
    setPoints(points);
}

BindLocation::BindLocation(slang::TypeLayoutReflection* typeLayout, SlangParameterCategory category, const BindPoint& point, BindSet_Value* value) :
    m_category(category),
    m_point(point),
    m_typeLayout(typeLayout),
    m_value(value)
{
}

BindLocation::BindLocation(slang::VariableLayoutReflection* varLayout, BindSet_Value* value)
{    
    m_value = value;
    m_typeLayout = varLayout->getTypeLayout();

    const Index categoryCount = Index(varLayout->getCategoryCount());

    if (categoryCount <= 0)
    {
        *this = BindLocation::Invalid;
        return;
    }
    else if (categoryCount == 1)
    {
        const auto category = varLayout->getCategoryByIndex(0);

        const auto offset = varLayout->getOffset(category);
        const auto space = varLayout->getBindingSpace(category);

        m_category = category;
        m_point = BindPoint(Index(space), size_t(offset));
    }
    else
    {
        BindPoints points;
        points.setInvalid();

        for (Index i = 0; i < categoryCount; ++i)
        {
            const auto category = varLayout->getCategoryByIndex((unsigned int)i);

            const auto offset = varLayout->getOffset(category);
            const auto space = varLayout->getBindingSpace(category);

            BindPoint& point = points.m_points[category];

            point.m_offset = size_t(offset);
            point.m_space = Index(space);
        }

        setPoints(points);
    }
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
    if (m_value && point)
    {
        size_t offset = point->m_offset;
        // Make sure it's in range
        if (offset + sizeInBytes <= m_value->m_sizeInBytes)
        {
            return m_value->m_data + offset;
        }
    }
    return nullptr;
}

SlangResult BindLocation::setUniform(const void* data, size_t sizeInBytes) const
{
    // It has to be a location with uniform
    const BindPoint* point = getValidBindPointForCategory(SLANG_PARAMETER_CATEGORY_UNIFORM);
    if (m_value && point)
    {
        size_t offset = point->m_offset;
        ptrdiff_t maxSizeInBytes = m_value->m_sizeInBytes - offset;
        SLANG_ASSERT(maxSizeInBytes > 0);

        if (maxSizeInBytes <= 0)
        {
            return SLANG_FAIL;
        }

        // Clamp such that only fill in what's available to write
        sizeInBytes = sizeInBytes > size_t(maxSizeInBytes) ? size_t(maxSizeInBytes) : sizeInBytes;

        // Make sure it's in range
        SLANG_ASSERT(offset + sizeInBytes <= m_value->m_sizeInBytes);

        // Okay copy the contents
        ::memcpy(m_value->m_data + offset, data, sizeInBytes);
        return SLANG_OK;
    }
    return SLANG_FAIL;
}

bool BindLocation::operator==(const ThisType& rhs) const
{
    if (m_typeLayout != rhs.m_typeLayout ||
        m_value != rhs.m_value)
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

HashCode BindLocation::getHashCode() const
{
    if (!m_typeLayout)
    {
        return 1;
    }
    if (m_bindPointSet)
    {
        return m_bindPointSet->getHashCode();
    }
    else
    {
        return Slang::combineHash(Slang::combineHash(m_category, Slang::getHashCode(m_typeLayout)), m_point.getHashCode());
    }
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BindRoot !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult BindRoot::parse(const String& text, const String& sourcePath, WriterHelper outStream, BindLocation& outLocation)
{
    SLANG_ASSERT(m_bindSet);

    // We will parse the 'name' as may be path to a value/resource
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

slang::VariableLayoutReflection* BindRoot::getParameterByName(const char* name)
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

slang::VariableLayoutReflection* BindRoot::getEntryPointParameterByName(const char* name)
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

SlangResult BindRoot::init(BindSet* bindSet, slang::ShaderReflection* reflection, int entryPointIndex)
{
    m_bindSet = bindSet;
    m_reflection = reflection;
    m_entryPoint = nullptr;
    
    {
        auto entryPointCount = int(reflection->getEntryPointCount());
        if (entryPointIndex < 0 || entryPointIndex >= entryPointCount)
        {
            SLANG_ASSERT(!"Entry point index out of range");
            return SLANG_FAIL;
        }
        m_entryPoint = reflection->getEntryPointByIndex(entryPointIndex);
    }

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPULikeBindRoot !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult CPULikeBindRoot::init(BindSet* bindSet, slang::ShaderReflection* reflection, int entryPointIndex)
{
    m_rootValue = nullptr;
    m_entryPointValue = nullptr;

    SLANG_RETURN_ON_FAIL(Super::init(bindSet, reflection, entryPointIndex));

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
            m_rootValue = m_bindSet->createBufferValue(slang::TypeReflection::Kind::ConstantBuffer, rootSizeInBytes);
        }
    }

    {
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
            m_entryPointValue = m_bindSet->createBufferValue(slang::TypeReflection::Kind::ConstantBuffer, entryPointParamsSizeInBytes);
        }
    }

    return SLANG_OK;
}



BindLocation CPULikeBindRoot::find(const char* name)
{
    Value* value = nullptr;
    slang::VariableLayoutReflection* varLayout = nullptr;

    if (m_rootValue)
    {
        varLayout = getParameterByName(name);
        value = m_rootValue;
    }
        
    if (!varLayout && m_entryPointValue)
    {
        value = m_entryPointValue;
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

    return BindLocation(varLayout->getTypeLayout(), SLANG_PARAMETER_CATEGORY_UNIFORM, BindPoint(0, offset), value);
}

SlangResult CPULikeBindRoot::setArrayCount(const BindLocation& location, int count)
{
    if (!location.isValid())
    {
        return SLANG_FAIL;
    }

    // I can see if a resource has already been set
    Value* value = m_bindSet->getAt(location);
    
    auto typeLayout = location.getTypeLayout();
    const auto kind = typeLayout->getKind();

    if (!(typeLayout->getKind() == slang::TypeReflection::Kind::Array && typeLayout->getElementCount() == 0))
    {
        return SLANG_FAIL;
    }

    const size_t elementStride = typeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
    auto elementTypeLayout = typeLayout->getElementTypeLayout();

    if (value)
    {
        // Making smaller, just reduce the count.
        // NOTE! Nothing is done here about deallocating resources which are perhaps no longer reachable.
        // This isn't a leakage problem tho, as all buffers are released automatically when scope is left.
        if (count <= int(value->m_elementCount) || count <= int(value->m_sizeInBytes / elementStride))
        {
            value->m_elementCount = count;
            return SLANG_OK;
        }

        const size_t maxElementCount = (value->m_sizeInBytes / elementStride);
        if (size_t(count) <= maxElementCount)
        {
            // Just initialize the space
            ::memset(value->m_data + elementStride * value->m_elementCount, 0, (count - value->m_elementCount) * elementStride);
            value->m_elementCount = count;
            return SLANG_OK;
        }
    }

    // Ok allocate a buffer that can hold all the elements
    
    const size_t newBufferSize = count * elementStride;

    Value* newValue = m_bindSet->createBufferValue(slang::TypeReflection::Kind::Array, newBufferSize);
    newValue->m_elementCount = count;

    // Copy over the data from the old buffer if there is any
    if (value && value->m_elementCount)
    {
        ::memcpy(newValue->m_data, value->m_data, value->m_elementCount * elementStride);
    }

    // Remove the old buffer as no longer needed

    if (value)
    {
        m_bindSet->destroyValue(value);
    }

    // Set the new buffer
    m_bindSet->setAt(location, newValue);
    return SLANG_OK;
}


void CPULikeBindRoot::getRoots(Slang::List<BindLocation>& outLocations)
{
    if (m_entryPointValue)
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

            BindLocation location(parameter->getTypeLayout(), SLANG_PARAMETER_CATEGORY_UNIFORM, BindPoint(0, offset), m_entryPointValue);
            outLocations.add(location);
        }
    }

    if (m_rootValue)
    {
        const int parameterCount = m_reflection->getParameterCount();
        for (int i = 0; i < parameterCount; ++i)
        {
            auto parameter = m_reflection->getParameterByIndex(i);

            auto offset = parameter->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM);

            BindLocation location(parameter->getTypeLayout(), SLANG_PARAMETER_CATEGORY_UNIFORM, BindPoint(0, offset), m_rootValue);
            outLocations.add(location);
        }
    }
}

static void _addDefaultBuffersRec(BindSet* bindSet, const BindLocation& loc)
{
    // See if there is a value/resource attached there
    auto typeLayout = loc.getTypeLayout();

    const auto kind = typeLayout->getKind();
    switch (kind)
    {   
        case slang::TypeReflection::Kind::ParameterBlock:
        case slang::TypeReflection::Kind::ConstantBuffer:
        {
            BindSet::Value* value = bindSet->getAt(loc);

            auto elementTypeLayout = typeLayout->getElementTypeLayout();

            if (!value)
            {
                //SLANG_ASSERT(typeLayout->getSize() == sizeof(void*));
                const size_t elementSize = elementTypeLayout->getSize();

                // We create using typeLayout (as opposed to elementTypeLayout), because it also holds the wrapping
                // 'resource' type.
                value = bindSet->createBufferValue(typeLayout, elementSize);
                SLANG_ASSERT(value);

                bindSet->setAt(loc, value);
            }

            // Recurse into buffer, using the elementType
            BindLocation childLocation(elementTypeLayout, SLANG_PARAMETER_CATEGORY_UNIFORM, BindPoint(0, 0), value );
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

void CPULikeBindRoot::addDefaultValues()
{

    List<BindLocation> rootLocations;
    getRoots(rootLocations);

    for (auto& location : rootLocations)
    {
        _addDefaultBuffersRec(m_bindSet, location);
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! GPULikeBindRoot !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

BindLocation GPULikeBindRoot::find(const char* name)
{
    slang::VariableLayoutReflection* varLayout = nullptr;

    varLayout = getParameterByName(name);
    if (!varLayout)
    {
        varLayout = getEntryPointParameterByName(name);
    }

    if (!varLayout)
    {
        return BindLocation::Invalid;
    }

    return BindLocation(varLayout, nullptr);
}

SlangResult GPULikeBindRoot::setArrayCount(const BindLocation& location, int count)
{
    // TODO(JS):
    // Not 100% clear how to handle this. If the mechanism uses 'spaces' there is nothing to do.
    // If the size is an aspect of the binding, then we need to set up the binding information correctly. Depending on underlying
    // API. This could perhaps be handled with a base class for m_target which meant we could just call that and it would
    // do the right thing.
    //
    // For now, lets not worry.
    return SLANG_OK;
}

void GPULikeBindRoot::getRoots(Slang::List<BindLocation>& outLocations)
{
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

            BindLocation location(parameter, nullptr);
            SLANG_ASSERT(location.isValid());

            outLocations.add(location);
        }
    }
    {
        const int parameterCount = m_reflection->getParameterCount();
        for (int i = 0; i < parameterCount; ++i)
        {
            auto parameter = m_reflection->getParameterByIndex(i);

            BindLocation location(parameter, nullptr);
            SLANG_ASSERT(location.isValid());

            outLocations.add(location);
        }
    }
}

} // renderer_test
