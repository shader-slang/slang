#include "slang-struct-tag-converter.h"


namespace Slang {

SlangResult StructTagConverter::getCurrentPtrArray(const void*const* in, Index count, const void*const** out)
{
    if (count == 0)
    {
        *out = in;
        return SLANG_OK;
    }

    List<const void*> dsts;

    for (Index i = 0; i < count; ++i)
    {
        const void* src = in[i];

        const void* dst;
        SLANG_RETURN_ON_FAIL(getCurrent(src, &dst));

        // Make space if not set up
        if (dst != src && dsts.getCount() == 0)
        {
            dsts.addRange(in, count);
        }
        dsts[i] = dst;
    }

    // We need to make a copy of the exts
    if (dsts.getCount())
    {
        if (!m_arena)
        {
            // We can't because we weren't given an area
            return SLANG_FAIL;
        }

        *out = (const void*const*)m_arena->allocateAndCopyArray(dsts.getBuffer(), count);
        return SLANG_OK;
    }

    *out = in;
    return SLANG_OK;
}
SlangResult StructTagConverter::copy(const StructTagType* structType, void* inDst, const void* inSrc)
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
        slang::PrimaryTaggedStruct* dstPrimary = reinterpret_cast<slang::PrimaryTaggedStruct*>(inDst);
        SLANG_RETURN_ON_FAIL(getCurrentPtrArray((const void*const*)dstPrimary->exts, dstPrimary->extsCount, (const void*const**)&dstPrimary->exts));
    }

    // It may have fields that need to be copied.


    return SLANG_OK;
}

SlangResult StructTagConverter::getCurrentArray(const void* in, Index count, const void** out)
{
    if (count == 0)
    {
        *out = in;
        return SLANG_OK;
    }

    if (count == 1)
    {
        return getCurrent(in, out);
    }

    const slang::TaggedStructBase* arr = reinterpret_cast<const slang::TaggedStructBase*>(in);

    // We assume all have the same size/type
    auto tag = arr[0].structTag;

    auto structType = m_system->getType(tag);

    if (StructTagUtil::isReadCompatible(tag, structType->m_tag))
    {
        if (m_sink)
        {
            // Incompatible types
        }
        return SLANG_E_STRUCT_TAG_INCOMPATIBLE;
    }

    if (structType->m_sizeInBytes == arr[0].structSize)
    {
        // Can just use what was passed in
        *out = in;
        return SLANG_OK;
    }

    if (m_arena)
    {
        if (m_sink)
        {
            // Needs an arena
        }

        return SLANG_FAIL;
    }

    const size_t dstStride = structType->m_sizeInBytes;
    uint8_t*const dstStart = (uint8_t*)m_arena->allocate(dstStride * count);
    uint8_t* dst = dstStart;

    size_t srcStride = arr[0].structSize;
    const uint8_t* src = reinterpret_cast<const uint8_t*>(in);

    for (Index i = 0; i < count; ++i)
    {
        copy(structType, dst, src);

        src += srcStride;
        dst += dstStride;
    }

    *out = (const void*)dstStart;
    return SLANG_OK;
}

SlangResult StructTagConverter::getCurrent(const void* in, const void** out)
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
                if (!getCurrent(exts[i]))
                {
                    return nullptr;
                }
            }
        }
    }

    // If can't find the type it's incompatible
    auto structType = m_system->getType(tag);
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

void* StructTagConverter::clone(const void* in)
{
    auto tag = reinterpret_cast<const slang::TaggedStructBase*>(in)->structTag;

    auto structType = m_system->getType(tag);
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
            dstExts = m_arena->allocateArray<slang::StructTag*>(primaryStruct->extsCount);

            for (Index i = 0; i < primaryStruct->extsCount; ++i)
            {
                auto dstExt = (slang::StructTag*)clone(&exts[i]);
                if (!dstExt)
                {
                    return nullptr;
                }

                dstExts[i] = dstExt;
            }
        }

        slang::PrimaryTaggedStruct* dstPrimaryStruct = (slang::PrimaryTaggedStruct*)m_arena->allocateAligned(structType->m_sizeInBytes, SLANG_ALIGN_OF(void*));
        ::memcpy(dstPrimaryStruct, primaryStruct, structType->m_sizeInBytes);

        dstPrimaryStruct->structTag = structType->m_tag;
        dstPrimaryStruct->structSize = structType->m_sizeInBytes;

        dstPrimaryStruct->exts = (const slang::StructTag**)dstExts;

        return dstPrimaryStruct;
    }
    else
    {
        slang::ExtensionTaggedStruct* dstExt = (slang::ExtensionTaggedStruct*)m_arena->allocateAligned(structType->m_sizeInBytes, SLANG_ALIGN_OF(void*));
        ::memcpy(dstExt, in, structType->m_sizeInBytes);
        dstExt->structTag = structType->m_tag;
        dstExt->structSize = structType->m_sizeInBytes;
        return dstExt;
    }
}

} // namespace Slang
