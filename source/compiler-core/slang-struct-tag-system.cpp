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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ApiSystem !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

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

StructTagSystem::CompatibilityResult StructTagSystem::calcPtrArrayCompatible(const void*const* in, Index count)
{
    auto current = CompatibilityResult::Compatible;

    for (Index i = 0; i < count; ++i)
    {
        auto result = calcCompatible(in[i]);
        if (result == CompatibilityResult::Incompatible)
        {
            return result;
        }

        if (Index(result) > Index(current))
        {
            current = result;
        }
    }
    return current;
}

StructTagSystem::CompatibilityResult StructTagSystem::calcArrayCompatible(const void* in, Index count)
{
    SLANG_UNUSED(count);
   

    const slang::TaggedStructBase* srcBase = reinterpret_cast<const slang::TaggedStructBase*>(in);

    const StructTagType* structType = getType(srcBase->structTag);
    if (!structType)
    {
        return CompatibilityResult::Incompatible;
    }

    //if (srcBase->apiSizeInBytes 

    return CompatibilityResult::Incompatible;
}

StructTagSystem::CompatibilityResult StructTagSystem::calcCompatible(const void* in)
{
    const slang::TaggedStructBase* srcBase = reinterpret_cast<const slang::TaggedStructBase*>(in);
    SLANG_UNUSED(srcBase);

#if 0

    const ApiStructType* structType = getStructType(srcBase->apiType);
    if (!structType)
    {
        return CompatibilityResult::Incompatible;
    }

    if (slang::kApiPrimaryMask & structType->m_typeValue)
    {

        // Copy extensions if needed

    }


    // We know we will need a copy
    if (srcBase->apiSizeInBytes != structType->m_sizeInBytes)
    {
    }

    // Set the type and the size
    dstBase->apiType = structType->m_typeValue;
    dstBase->apiSizeInBytes = int32_t(structType->m_sizeInBytes);

    

    // It may have fields that need to be copied.
#endif


    return CompatibilityResult::Incompatible;
}


StructTagCategoryInfo* StructTagSystem::getCategoryInfo(slang::StructTagCategory category)
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

StructTagType* StructTagSystem::getType(slang::StructTag tag)
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

SlangResult StructTagSystem::copy(const StructTagType* structType, void* inDst, const void* inSrc)
{
    uint8_t* dst = (uint8_t*)inDst;
    const slang::TaggedStructBase* srcBase = reinterpret_cast<const slang::TaggedStructBase*>(inSrc);

    slang::StructSize size = std::min(structType->m_sizeInBytes, srcBase->structSize);

    // Copy
    ::memcpy(dst, inSrc, size);
    // Zero any extra
    if (size < structType->m_sizeInBytes)
    {
        ::memset(dst + size, 0, structType->m_sizeInBytes - size);
    }

    // Set the type and the size
    slang::TaggedStructBase* dstBase = reinterpret_cast<slang::TaggedStructBase*>(dst);
    dstBase->structTag = structType->m_tag;
    dstBase->structSize = structType->m_sizeInBytes;

    if (StructTagUtil::isPrimary(structType->m_tag))
    {
        // Copy extensions if needed


    }

    // It may have fields that need to be copied.


    return SLANG_OK;
}

const void* StructTagSystem::getReadArray(const void* in, Index count, MemoryArena& arena)
{
    if (count == 0)
    {
        return in;
    }

    if (count == 1)
    {
        return getReadCompatible(in, arena);
    }

    const slang::TaggedStructBase* arr = reinterpret_cast<const slang::TaggedStructBase*>(in);

    // We assume all have the same size/type
    auto tag = arr[0].structTag;

    auto structType = getType(tag);

    if (StructTagUtil::isReadCompatible(tag, structType->m_tag))
    {
        if (structType->m_sizeInBytes == arr[0].structSize)
        {
            // Can just use what was passed in
            return in;
        }

        const size_t dstStride = structType->m_sizeInBytes;
        uint8_t* dst = (uint8_t*)arena.allocate(dstStride * count);

        size_t srcStride = arr[0].structSize;
        const uint8_t* src = reinterpret_cast<const uint8_t*>(in);

        for (Index i = 0; i < count; ++i)
        {
            copy(structType, dst, src);

            src += srcStride;
            dst += dstStride;
        }


        // How large of a thing to copy

        // We need to allocate memory for all the output

    }

    return nullptr;
}


const void* StructTagSystem::getReadCompatible(const void* in, MemoryArena& arena)
{
    auto tag = reinterpret_cast<const slang::TaggedStructBase*>(in)->structTag;

    // TODO(JS): Note here we may need to upgrade or downgrade a type. For the moment we don't worry
    // about that and just test that they are 'readable' and only accept if all the associated types are read compatible

    if (StructTagUtil::isPrimary(tag))
    {
        const slang::PrimaryTaggedStruct* primaryStruct = (const slang::PrimaryTaggedStruct*)in;

        if (primaryStruct->extsCount > 0)
        {
            auto exts = primaryStruct->exts;

            // We have to make sure all the exts are compatible.
            for (Index i = 0; i < primaryStruct->extsCount; ++i)
            {
                if (!getReadCompatible(exts[i], arena))
                {
                    return nullptr;
                }
            }
        }
    }

    // If can't find the type it's incompatible
    auto structType = getType(tag);
    if (!structType)
    {
        return nullptr;
    }

    if (!StructTagUtil::isReadCompatible(tag, structType->m_tag))
    {
        return nullptr;
    }

    // TOODO(JS): If we succeed here, we don't need to have actually copied anything.
    return in;
}

void* StructTagSystem::clone(const void* in, MemoryArena& arena)
{
    auto tag = reinterpret_cast<const slang::TaggedStructBase*>(in)->structTag;

    auto structType = getType(tag);
    if (!structType || !StructTagUtil::isReadCompatible(tag, structType->m_tag))
    {
        return nullptr;
    }

    slang::StructTag** dstExts = nullptr;

    if (StructTagUtil::isPrimary(tag))
    {
        const slang::PrimaryTaggedStruct* primaryStruct = (const slang::PrimaryTaggedStruct*)in;
        auto exts = primaryStruct->exts;

        if (primaryStruct->extsCount > 0)
        {
            dstExts = arena.allocateArray<slang::StructTag*>(primaryStruct->extsCount);

            for (Index i = 0; i < primaryStruct->extsCount; ++i)
            {
                auto dstExt = (slang::StructTag*)clone(&exts[i], arena);
                if (!dstExt)
                {
                    return nullptr;
                }

                dstExts[i] = dstExt;
            }
        }

        slang::PrimaryTaggedStruct* dstPrimaryStruct = (slang::PrimaryTaggedStruct*)arena.allocateAligned(structType->m_sizeInBytes, SLANG_ALIGN_OF(void*));
        ::memcpy(dstPrimaryStruct, primaryStruct, structType->m_sizeInBytes);

        dstPrimaryStruct->structTag = structType->m_tag;
        dstPrimaryStruct->structSize = structType->m_sizeInBytes;

        dstPrimaryStruct->exts = (const slang::StructTag**)dstExts;

        return dstPrimaryStruct;
    }
    else
    {
        slang::ExtensionTaggedStruct* dstExt = (slang::ExtensionTaggedStruct*)arena.allocateAligned(structType->m_sizeInBytes, SLANG_ALIGN_OF(void*));
        ::memcpy(dstExt, in, structType->m_sizeInBytes);
        dstExt->structTag = structType->m_tag;
        dstExt->structSize = structType->m_sizeInBytes;
        return dstExt;
    }
}

} // namespace Slang
