#include "slang-struct-tag-system.h"

#include "slang-diagnostic-sink.h"
#include "slang-core-diagnostics.h"

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
        return false;
    }

    // It's okay if the in is a later version.
    return StructTagUtil::isReadCompatible(tag, structType->m_tag);
}

size_t StructTagSystem::getSize(const StructTagField::Type fieldType, slang::StructTag tag) const
{
    typedef StructTagField::Type FieldType;

    switch (fieldType)
    {
        case FieldType::TaggedStruct:
        {
            auto type = getType(tag);
            if (type)
            {
                return type->m_sizeInBytes;
            }
            return 0;
        }
        case FieldType::PtrTaggedStruct:
        case FieldType::PtrPtrTaggedStruct:
        {
            return sizeof(void*);
        }
        case FieldType::I32:     return sizeof(int32_t); 
        case FieldType::I64:     return sizeof(int64_t);
        default: break;
    }

    return 0;
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

SlangResult StructTagSystem::createCompatible(const StructTagDesc* descs, Index descsCount, DiagnosticSink* sink, RefPtr<StructTagSystem>& outSystem)
{
    RefPtr<StructTagSystem> dstSystem = new StructTagSystem;

    // We want to add all of the categories
    for (auto categoryInfo : m_categories)
    {
        dstSystem->addCategoryInfo(categoryInfo->m_category, categoryInfo->m_name);
    }

    for (Index i = 0; i < descsCount; ++i)
    {
        const auto& desc = descs[i];

        //
        auto category = StructTagUtil::getCategory(desc.structTag);

        auto dstCategoryInfo = dstSystem->getCategoryInfo(category);
        if (!dstCategoryInfo)
        {
            if (sink)
            {
                sink->diagnose(SourceLoc(), MiscDiagnostics::unknownStructTagCategory, Index(category));
            }
            return SLANG_FAIL;
        }

        // If it's in dst it must be in source
        auto srcCategoryInfo = getCategoryInfo(category);

        const Index typeIndex = StructTagUtil::getTypeIndex(desc.structTag);

        if (auto dstType = dstCategoryInfo->getType(typeIndex))
        {
            if (dstType->m_sizeInBytes != desc.sizeInBytes ||
                dstType->m_tag != desc.structTag)
            {
                if (sink)
                {
                    sink->diagnose(SourceLoc(), MiscDiagnostics::structTagInconsistent, dstSystem->getName(desc.structTag));
                }
                return SLANG_FAIL;
            }

            // Multiply defined, but consistent so ignore
            continue;
        }

        auto srcType = srcCategoryInfo->getType(typeIndex);
        if (srcType == nullptr)
        {
            // We don't have a definition...
            if (sink)
            {
                sink->diagnose(SourceLoc(), MiscDiagnostics::unknownStructTag, getName(desc.structTag));
            }
            return SLANG_FAIL;
        }

        if (!StructTagUtil::areSameMajorType(srcType->m_tag, desc.structTag))
        {
            if (sink)
            {
                sink->diagnose(SourceLoc(), MiscDiagnostics::cannotConvertStructTag, getName(desc.structTag), getName(srcType->m_tag));
            }
            return SLANG_E_STRUCT_TAG_INCOMPATIBLE;
        }

        // Okay lets copy
        auto dstType = dstSystem->addType(desc.structTag, srcType->m_name, desc.sizeInBytes);


        // Finally we need to copy the fiends which are still in range
        for (const auto& srcField : srcType->m_fields)
        {
            if (srcField.m_offset + getSize(srcField.m_type, slang::StructTag(0)) > desc.sizeInBytes ||
                (srcField.m_countType != StructTagField::Type::Unknown && srcField.m_countOffset + getSize(srcField.m_countType, slang::StructTag(0))))
            {
                // The field is not inside the destination
                continue;
            }

            dstType->m_fields.add(srcField);
        }
        
        // We need to work out which defaultInstance to use.

        StructTagDesc instanceDesc;
        const StructTagDesc currentDesc = StructTagDesc::make(srcType->m_tag, srcType->m_sizeInBytes, srcType->m_defaultInstance);

        // If both have defaultInstance, we want to copy from the latest one.
        if (srcType->m_defaultInstance && desc.defaultInstance)
        {
            instanceDesc = (StructTagUtil::getMinorVersion(srcType->m_tag) > StructTagUtil::getMinorVersion(desc.structTag)) ? currentDesc : desc;
        }
        else
        {
            instanceDesc = srcType->m_defaultInstance ? currentDesc : desc;
        }

        // Copy the default instance.
        if (instanceDesc.defaultInstance)
        {
            void* dstDefaultInstance = dstSystem->m_arena.allocateAndZero(desc.sizeInBytes);

            // Copy the minimum size
            auto size = std::min(dstType->m_sizeInBytes, instanceDesc.sizeInBytes);
            ::memcpy(dstDefaultInstance, instanceDesc.defaultInstance, size);

            // Set the default instance.
            dstType->m_defaultInstance = dstDefaultInstance;
        }
    }

    outSystem = dstSystem;
    return SLANG_OK;
}

} // namespace Slang
