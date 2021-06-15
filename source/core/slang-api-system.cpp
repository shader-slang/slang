#include "slang-api-system.h"

namespace Slang {

void ApiSystem::addCategory(uint32_t category, const String& name)
{
    if (Index(category) >= m_categories.getCount())
    {
        m_categories.setCount(Index(category) + 1);
    }

    CategoryInfo& info = m_categories[Index(category)];
    info.m_name = name;
}

void ApiSystem::addType(slang::ApiStructTypeValue typeValue, const String qualifiedName, size_t sizeInBytes)
{
    auto categoryAndType = ApiUtil::getCategoryAndType(typeValue);

    // Look up in the dictionary
    if (auto indexPtr = m_typeToInfoIndex.TryGetValueOrAdd(categoryAndType, m_typeInfos.getCount()))
    {
        TypeInfo& info = m_typeInfos[*indexPtr];

        info.m_type = typeValue;
        info.m_name = qualifiedName;
        info.m_sizeInBytes = sizeInBytes;
    }
    else
    {
        TypeInfo info;

        info.m_type = typeValue;
        info.m_name = qualifiedName;
        info.m_sizeInBytes = sizeInBytes;

        m_typeInfos.add(info); 
    }
}

const ApiSystem::TypeInfo* ApiSystem::getTypeInfo(slang::ApiStructTypeValue value)
{
    const auto categoryAndType = ApiUtil::getCategoryAndType(value);
    if (auto indexPtr = m_typeToInfoIndex.TryGetValue(categoryAndType))
    {
        return &m_typeInfos[*indexPtr];
    }
    return nullptr;
}

const void* ApiSystem::getReadCompatible(const void* in, MemoryArena& arena)
{
    slang::ApiStructTypeValue type = *(const slang::ApiStructTypeValue*)in;

    // TODO(JS): Note here we may need to upgrade or downgrade a type. For the moment we don't worry
    // about that and just test that they are 'readable' and only accept if all the associated types are read compatible

    if (slang::kApiPrimaryMask & type)
    {
        const slang::ApiPrimaryStruct* primaryStruct = (const slang::ApiPrimaryStruct*)in;

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

    auto categoryAndType = ApiUtil::getCategoryAndType(type);

    const Index* indexPtr = m_typeToInfoIndex.TryGetValue(categoryAndType);
    if (!indexPtr)
    {
        return nullptr;
    }

    {
        const TypeInfo& typeInfo = m_typeInfos[*indexPtr];
        if (!ApiUtil::isReadCompatible(type, typeInfo.m_type))
        {
            return nullptr;
        }
    }

    // TOODO(JS): If we succeed here, we don't need to have actually copied anything.
    return in;
}

void* ApiSystem::clone(const void* in, MemoryArena& arena)
{
    slang::ApiStructTypeValue type = *(const slang::ApiStructTypeValue*)in;

    auto typeInfo = getTypeInfo(type);
    if (!typeInfo || !ApiUtil::isReadCompatible(type, typeInfo->m_type))
    {
        return nullptr;
    }

    slang::ApiExtensionType** dstExts = nullptr;

    if (slang::kApiPrimaryMask & type)
    {
        const slang::ApiPrimaryStruct* primaryStruct = (const slang::ApiPrimaryStruct*)in;
        auto exts = primaryStruct->exts;

        if (primaryStruct->extsCount > 0)
        {
            dstExts = arena.allocateArray<slang::ApiExtensionType*>(primaryStruct->extsCount);

            for (Index i = 0; i < primaryStruct->extsCount; ++i)
            {
                auto dstExt = (slang::ApiExtensionType*)clone(&exts[i], arena);
                if (!dstExt)
                {
                    return nullptr;
                }

                dstExts[i] = dstExt;
            }
        }

        slang::ApiPrimaryStruct* dstPrimaryStruct = (slang::ApiPrimaryStruct*)arena.allocateAligned(typeInfo->m_sizeInBytes, SLANG_ALIGN_OF(void*));
        ::memcpy(dstPrimaryStruct, primaryStruct, typeInfo->m_sizeInBytes);

        dstPrimaryStruct->apiType = slang::ApiPrimaryType(typeInfo->m_type);
        dstPrimaryStruct->exts = (const slang::ApiExtensionType**)dstExts;

        return dstPrimaryStruct;
    }
    else
    {
        slang::ApiExtensionStruct* dstExt = (slang::ApiExtensionStruct*)arena.allocateAligned(typeInfo->m_sizeInBytes, SLANG_ALIGN_OF(void*));
        ::memcpy(dstExt, in, typeInfo->m_sizeInBytes);
        dstExt->abiType = slang::ApiExtensionType(typeInfo->m_type);
        return dstExt;
    }
}

} // namespace Slang
