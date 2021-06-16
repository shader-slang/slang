#include "slang-struct-tag-converter.h"


namespace Slang {

const void*const* StructTagConverter::maybeConvertCurrentPtrArray(const void*const* in, Index count)
{
    if (count == 0)
    {
        return in;
    }

    List<const void*> dsts;

    for (Index i = 0; i < count; ++i)
    {
        const void* src = in[i];
        const void* dst = maybeConvertCurrent(src);

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
            return nullptr;
        }
        return (const void*const*)m_arena->allocateAndCopyArray(dsts.getBuffer(), count);
    }
    return in;
}

static Index _getCount(const StructTagType::Field& field, const void* in)
{
    typedef StructTagType::FieldType FieldType;

    const uint8_t* ptr = (const uint8_t*)in;

    switch (field.m_countType)
    {
        case FieldType::I32:            return Index(*(const int32_t*)(ptr + field.m_countOffset));
        case FieldType::I64:            return Index(*(const int64_t*)(ptr + field.m_countOffset));
        default: break;
    }

    SLANG_ASSERT(!"Cannot access as count");
    return -1;
}

void StructTagConverter::setContained(Index stackIndex, const StructTagType* structType, void* out)
{
    if (StructTagUtil::isPrimary(structType->m_tag))
    {
        // Copy extensions if needed
        slang::PrimaryTaggedStruct* primary = reinterpret_cast<slang::PrimaryTaggedStruct*>(out);
        if (primary->extsCount > 0)
        {
            primary->exts = (const slang::StructTag**)m_stack[stackIndex++];
        }
    }

    {
        typedef StructTagType::FieldType FieldType;
        for (const auto& field : structType->m_fields)
        {
            switch (field.m_type)
            {
                case FieldType::PtrTaggedStruct:
                case FieldType::PtrPtrTaggedStruct:
                {
                    *(const void**)(reinterpret_cast<const uint8_t*>(out) + field.m_offset) = m_stack[stackIndex++];
                    break;
                }
                default: break;
            }
        }
    }
}

Index StructTagConverter::convertCurrentContained(const StructTagType* structType, const void* in)
{
    Index numConverted = 0;
    
    if (StructTagUtil::isPrimary(structType->m_tag))
    {
        // Copy extensions if needed
        const slang::PrimaryTaggedStruct* primary = reinterpret_cast<const slang::PrimaryTaggedStruct*>(in);
        const void*const* dstExts = (const void*const*)primary->exts;
        if (primary->extsCount > 0)
        {
            auto srcExts = (const void*const*)primary->exts;
            dstExts = maybeConvertCurrentPtrArray(srcExts, primary->extsCount);
            if (dstExts == nullptr)
            {
                return -1;
            }
            numConverted += (dstExts != srcExts);
        }
        m_stack.add(dstExts);
    }

    // It may have fields that need to be converted

    {
        typedef StructTagType::FieldType FieldType;

        for (const auto& field : structType->m_fields)
        {
            const Index count = _getCount(field, in);
            if (count <= 0)
            {
                m_stack.add(nullptr);
                continue;
            }

            const void* src = *(const void**)(reinterpret_cast<const uint8_t*>(in) + field.m_offset);
            SLANG_ASSERT(src);

            const void* dst = nullptr;
            switch (field.m_type)
            {
                case FieldType::PtrTaggedStruct:
                {
                    dst = maybeConvertCurrentArray(src, count);
                    break;
                }
                case FieldType::PtrPtrTaggedStruct:
                {
                    dst = maybeConvertCurrentPtrArray((const void*const*)src, count);
                    break;
                }
                default:
                {
                    return SLANG_FAIL;
                }
            }

            if (dst == nullptr)
            {
                return -1;
            }

            m_stack.add(dst);
            numConverted += (src != dst);
        }
    }

    return numConverted;
}

void StructTagConverter::copy(const StructTagType* structType, const void* src, void* dst)
{
    const slang::TaggedStructBase* srcBase = reinterpret_cast<const slang::TaggedStructBase*>(src);
    slang::StructSize size = std::min(structType->m_sizeInBytes, srcBase->structSize);

    // Copy
    ::memcpy(dst, src, size);
    // Zero any extra
    if (size < structType->m_sizeInBytes)
    {
        ::memset((char*)dst + size, 0, structType->m_sizeInBytes - size);
    }

    // Set the type and the size
    slang::TaggedStructBase* dstBase = reinterpret_cast<slang::TaggedStructBase*>(dst);
    dstBase->structTag = structType->m_tag;
    dstBase->structSize = structType->m_sizeInBytes;
}

void* StructTagConverter::allocateAndCopy(const StructTagType* structType, const void* src)
{
    uint8_t* dst = (uint8_t*)m_arena->allocate(structType->m_sizeInBytes);
    copy(structType, src, dst);
    return dst;
}

SlangResult StructTagConverter::convertCurrent(const StructTagType* structType, const void* src, void* dst)
{
    copy(structType, src, dst);
 
    const Index stackIndex = m_stack.getCount();

    const Index numConverted = convertCurrentContained(structType, dst);
    if (numConverted > 0)
    {
        setContained(stackIndex, structType, dst);
    }

    m_stack.setCount(stackIndex);
    if (numConverted < 0)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

const void* StructTagConverter::maybeConvertCurrentArray(const void* in, Index count)
{
    if (count == 0)
    {
        return in;
    }

    if (count == 1)
    {
        return maybeConvertCurrent(in);
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
        return nullptr;
    }

    if (structType->m_sizeInBytes == arr[0].structSize)
    {
        // Can just use what was passed in
        return in;
    }

    if (m_arena)
    {
        if (m_sink)
        {
            // Needs an arena
        }

        return nullptr;
    }

    const size_t dstStride = structType->m_sizeInBytes;
    uint8_t*const dstStart = (uint8_t*)m_arena->allocate(dstStride * count);
    uint8_t* dst = dstStart;

    size_t srcStride = arr[0].structSize;
    const uint8_t* src = reinterpret_cast<const uint8_t*>(in);

    for (Index i = 0; i < count; ++i)
    {
        if (SLANG_FAILED(convertCurrent(structType, src, dst)))
        {
            return nullptr;
        }

        src += srcStride;
        dst += dstStride;
    }

    return dstStart;
}

const void* StructTagConverter::maybeConvertCurrent(const void* in)
{
    auto base = reinterpret_cast<const slang::TaggedStructBase*>(in);
    auto tag = base->structTag;

    // If can't find the type it's incompatible
    auto structType = m_system->getType(tag);
    if (!structType)
    {
        // Unknown type
        if (m_sink)
        {
        }
        return nullptr;
    }

    if (!StructTagUtil::isReadCompatible(tag, structType->m_tag))
    {
        // Can't do a suitable conversion
        if (m_sink)
        {
        }
        return nullptr;
    }

    // Let's see if how everything contained converts
    const Index stackIndex = m_stack.getCount();
    const Index numConverted = convertCurrentContained(structType, in);
    if (numConverted <= 0)
    {
        m_stack.setCount(stackIndex);
        return numConverted == 0 ? in : nullptr;
    }

    // Okay we will need to allocate and copy
    void* dst = allocateAndCopy(structType, in);

    setContained(stackIndex, structType, dst);
    m_stack.setCount(stackIndex);

    return dst;
}

const void* StructTagConverter::maybeConvertCurrent(slang::StructTag tag, const void* in)
{
    auto base = reinterpret_cast<const slang::TaggedStructBase*>(in);
    auto inTag = base->structTag;

    if (!StructTagUtil::areSameType(inTag, tag))
    {
        if (m_sink)
        {
            // Incorrect types
        }
        return nullptr;
    }

    return maybeConvertCurrent(in);
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
