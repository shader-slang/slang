#include "slang-struct-tag-system.h"


namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StructTagCategoryInfo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

StructTagCategoryInfo::~StructTagCategoryInfo()
{
    for (auto type : m_types)
    {
        if (type)
        {
            type->~StructTagType();
        }
    }
}

void StructTagCategoryInfo::addType(StructTagType* type)
{
    auto typeIndex = StructTagUtil::getTypeIndex(type->m_tag);

    if (typeIndex >= m_types.getCount())
    {
        Index prevCount = m_types.getCount();
        m_types.setCount(typeIndex + 1);
        // Zero it
        ::memset(m_types.getBuffer() + prevCount, 0, sizeof(StructTagType*) * (m_types.getCount() - prevCount));
    }

    SLANG_ASSERT(m_types[typeIndex] == nullptr);
    m_types[typeIndex] = type;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StructTagSystem !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

StructTagSystem::~StructTagSystem()
{
    for (auto category : m_categories)
    {
        if (category)
        {
            category->~StructTagCategoryInfo();
        }
    }
}

StructTagCategoryInfo* StructTagSystem::getCategoryInfo(slang::StructTagCategory category) const
{
    const Index index = Index(category);
    return (index < m_categories.getCount()) ? m_categories[index] : nullptr;
}

StructTagCategoryInfo* StructTagSystem::addCategoryInfo(slang::StructTagCategory category, const String& name)
{
    StructTagCategoryInfo* categoryInfo = new (m_arena.allocateAligned(sizeof(StructTagCategoryInfo), SLANG_ALIGN_OF(StructTagCategoryInfo))) StructTagCategoryInfo(category, name);

    const Index index = Index(category);

    if (index >= m_categories.getCount())
    {
        m_categories.setCount(index + 1);
    }
    m_categories[index] = categoryInfo;
    return categoryInfo;
}

StructTagType* StructTagSystem::addType(slang::StructTag tag, const String& name, size_t sizeInBytes)
{
    auto category = StructTagUtil::getCategory(tag);
    auto categoryInfo = getCategoryInfo(category);

    auto structType = new (m_arena.allocate<StructTagType>()) StructTagType(tag, name, sizeInBytes);
    categoryInfo->addType(structType);

    return structType;
}

StructTagType* StructTagSystem::getType(slang::StructTag tag) const
{
    const auto category = StructTagUtil::getCategory(tag);
    auto categoryInfo = getCategoryInfo(category);
    if (categoryInfo)
    {
        auto typeIndex = StructTagUtil::getTypeIndex(tag);
        return categoryInfo->getType(typeIndex);
    }

    return nullptr;
}

bool StructTagSystem::canCast(slang::StructTag tag, const void* in) const
{
    if (in == nullptr)
    {
        return true;
    }

    const slang::TaggedStructBase* srcBase = reinterpret_cast<const slang::TaggedStructBase*>(in);
    // If the tag is identical we can cast
    if (srcBase->structTag == tag)
    {
        return true;
    }

    StructTagType* structType = getType(srcBase->structTag);
    if (!structType)
    {
        return nullptr;
    }

    // It's okay if the in is a later version.
    return StructTagUtil::isReadCompatible(tag, structType->m_tag);
}

void StructTagSystem::appendName(slang::StructTag tag, StringBuilder& out)
{
    auto info = StructTagUtil::getTypeInfo(tag);

    auto categoryInfo = getCategoryInfo(info.category);
    if (categoryInfo)
    {
        out << categoryInfo->m_name;
    }
    else
    {
        out << Index(info.category);
    }

    out << "::";

    auto type = categoryInfo->getType(info.typeIndex);

    if (type)
    {
        out << type->m_name;
    }
    else
    {
        out << Index(info.typeIndex);
    }

    out << "_";
    out << Index(info.majorVersion);
    out << ".";
    out << Index(info.minorVersion);
}

void StructTagSystem::setDefaultInstance(StructTagType* structType, const void* in)
{
    if (structType->m_defaultInstance == nullptr)
    {
        structType->m_defaultInstance = m_arena.allocate(structType->m_sizeInBytes);
    }

    // We can check if this seems plausible
    const slang::TaggedStructBase* srcBase = reinterpret_cast<const slang::TaggedStructBase*>(in);
    SLANG_UNUSED(srcBase);

    SLANG_ASSERT(srcBase->structTag == structType->m_tag);
    SLANG_ASSERT(srcBase->structSize == structType->m_sizeInBytes);

    // Copy it over
    ::memcpy(structType->m_defaultInstance, in, structType->m_sizeInBytes);
}

} // namespace Slang
