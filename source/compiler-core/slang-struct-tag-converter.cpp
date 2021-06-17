#include "slang-struct-tag-converter.h"


namespace Slang {

static Index _getCount(const StructTagType::Field& field, const void* in)
{
    typedef StructTagField::Type FieldType;

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

SlangResult StructTagConverter::_requireArena()
{
    if (!m_arena)
    {
        if (m_sink)
        {
            // Diagnose that we need an arena
        }
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

SlangResult StructTagConverter::_diagnoseCantConvert(slang::StructTag tag, StructTagType* structType)
{
    SLANG_UNUSED(tag);
    SLANG_UNUSED(structType);

    if (m_sink)
    {
        // Diagnose why the tag couldn't be converted
    }

    return SLANG_E_STRUCT_TAG_INCOMPATIBLE;
}

SlangResult StructTagConverter::_diagnoseUnknownType(slang::StructTag tag)
{
    SLANG_UNUSED(tag);

    if (m_sink)
    {
    }

    // Perhaps this isn't quite right - but it's probably the most suitable error
    return SLANG_E_STRUCT_TAG_INCOMPATIBLE;
}

SlangResult StructTagConverter::_diagnoseDifferentTypes(slang::StructTag tagA, slang::StructTag tagB)
{
    SLANG_UNUSED(tagA);
    SLANG_UNUSED(tagB);

    if (m_sink)
    {
    }

    // Perhaps this isn't quite right - but it's probably the most suitable error
    return SLANG_E_STRUCT_TAG_INCOMPATIBLE;
}

bool StructTagConverter::canConvertToCurrent(slang::StructTag tag, StructTagType* type) const
{
    // Means can be used without any modification
    if (StructTagUtil::isReadCompatible(tag, type->m_tag))
    {
        return true;
    }

    // We may want to allow zero extension, or initialization
    // We can accept for conversion if it's the same type with only difference being the minor version.
    return StructTagUtil::areSameType(tag, type->m_tag);
}

void** StructTagConverter::convertCurrentPtrArray(const void*const* in, Index count)
{
    SLANG_ASSERT(count > 0);
 
    void** dst = (void**)m_arena->allocate(count * sizeof(void*));
    for (Index i = 0; i < count; ++i)
    {
        const void* src = in[i];
        void* converted = convertCurrent(src);
        if (converted == nullptr)
        {
            return nullptr;
        }
        dst[i] = converted;
    }
    return dst;
}

void* StructTagConverter::convertCurrentArray(const void* in, Index count)
{
    SLANG_ASSERT(count > 0);
    if (count == 1)
    {
        return convertCurrent(in);
    }

    const slang::TaggedStructBase* arr = reinterpret_cast<const slang::TaggedStructBase*>(in);

    // We assume all have the same size/type
    auto tag = arr[0].structTag;

    auto structType = m_system->getType(tag);

    //If we can't convert then we are done
    if (!canConvertToCurrent(tag, structType))
    {
        if (m_sink)
        {
            // Incompatible types
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
        copy(structType, src, dst);
        if (SLANG_FAILED(convertCurrentContained(structType, dst)))
        {
            return nullptr;
        }
        src += srcStride;
        dst += dstStride;
    }

    return dstStart;
}

SlangResult StructTagConverter::convertCurrentContained(const StructTagType* structType, void* inout)
{
    // Convert primary

    if (StructTagUtil::isPrimary(structType->m_tag))
    {
        // Copy extensions if needed
        slang::PrimaryTaggedStruct* primary = reinterpret_cast<slang::PrimaryTaggedStruct*>(inout);
        if (primary->extsCount > 0)
        {
            auto dstExts = convertCurrentPtrArray((const void*const*)primary->exts, primary->extsCount);
            if (!dstExts)
            {
                return SLANG_FAIL;
            }
            primary->exts = (const slang::StructTag**)dstExts;
        }
    }

    // It may have fields that need to be converted

    {
        for (const auto& field : structType->m_fields)
        {
            const Index count = _getCount(field, inout);
            if (count > 0)
            {
                const void*& ptrRef = *(const void**)(reinterpret_cast<const uint8_t*>(inout) + field.m_offset);
                SLANG_ASSERT(ptrRef);

                const void* ptr = ptrRef;

                const void* dst = nullptr;
                switch (field.m_type)
                {
                    case FieldType::PtrTaggedStruct:
                    {
                        dst = convertCurrentArray(ptr, count);
                        break;
                    }
                    case FieldType::PtrPtrTaggedStruct:
                    {
                        dst = convertCurrentPtrArray((const void*const*)ptr, count);
                        break;
                    }
                    default:
                    {
                        return SLANG_FAIL;
                    }
                }

                if (dst == nullptr)
                {
                    return SLANG_FAIL;
                }
                ptrRef = dst;
            }
        }
    }
    return SLANG_OK;
}

void* StructTagConverter::convertCurrent(const void* in)
{
    auto tag = reinterpret_cast<const slang::TaggedStructBase*>(in)->structTag;

    auto structType = m_system->getType(tag);
    if (!structType)
    {
        if (m_sink)
        {
            // Unknown type
        }
        return nullptr;
    }

    if (!canConvertToCurrent(tag, structType))
    {
        if (m_sink)
        {
            // Can't convert
        }
        return nullptr;
    }

    slang::TaggedStructBase* dst = (slang::TaggedStructBase*)allocateAndCopy(structType, in);
    convertCurrentContained(structType, dst);
    return dst;
}

SlangResult StructTagConverter::maybeConvertCurrentPtrArray(const void*const* in, Index count, const void*const*& out)
{
    if (count == 0)
    {
        out = in;
        return SLANG_OK;
    }

    Index numConverted = 0;
    ScopeStack stackScope(this);

    for (Index i = 0; i < count; ++i)
    {
        const void* src = in[i];
        const void* dst;
        SLANG_RETURN_ON_FAIL(maybeConvertCurrent(src, dst));

        // Make space if not set up
        numConverted += Index(dst != src);
        m_convertStack.add(dst);
    }

    // We need to make a copy of the exts
    if (numConverted)
    {
        SLANG_RETURN_ON_FAIL(_requireArena());
        out = (const void*const*)m_arena->allocateAndCopyArray(m_convertStack.getBuffer() + stackScope.getStartIndex(), count);
    }
    else
    {
        out = in;
    }

    return SLANG_OK;
}


void StructTagConverter::setContainedConverted(const StructTagType* structType, Index stackIndex, BitField fieldsSet, void* out)
{
    if (fieldsSet == 0)
    {
        return;
    }

    if (StructTagUtil::isPrimary(structType->m_tag))
    {
        if (fieldsSet & 1)
        {
            // Copy extensions if needed
            slang::PrimaryTaggedStruct* primary = reinterpret_cast<slang::PrimaryTaggedStruct*>(out);
            primary->exts = (const slang::StructTag**)m_convertStack[stackIndex++];
        }
        fieldsSet >>= 1;
    }

    {
        for (const auto& field : structType->m_fields)
        {
            if (fieldsSet == 0)
            {
                return;
            }

            // If the field is set, copy
            if (fieldsSet & 1)
            {
                switch (field.m_type)
                {
                    case FieldType::PtrTaggedStruct:
                    case FieldType::PtrPtrTaggedStruct:
                    {
                        *(const void**)(reinterpret_cast<const uint8_t*>(out) + field.m_offset) = m_convertStack[stackIndex++];
                        break;
                    }
                    default: break;
                }
            }

            // Remove the bit
            fieldsSet >>= 1;
        }
    }
}

SlangResult StructTagConverter::maybeConvertCurrentContained(const StructTagType* structType, const void* in, BitField* outFieldsSet)
{
    BitField fieldsSet = 0;
    BitField bit = 1;

    if (StructTagUtil::isPrimary(structType->m_tag))
    {
        // Copy extensions if needed
        const slang::PrimaryTaggedStruct* primary = reinterpret_cast<const slang::PrimaryTaggedStruct*>(in);
        if (primary->extsCount > 0)
        {
            auto srcExts = (const void*const*)primary->exts;
            const void*const* dstExts;
            SLANG_RETURN_ON_FAIL(maybeConvertCurrentPtrArray(srcExts, primary->extsCount, dstExts));

            if (dstExts != srcExts)
            {
                m_convertStack.add(dstExts);
                fieldsSet |= bit;
            }
        }

        bit += bit;
    }

    // It may have fields that need to be converted

    {
        for (const auto& field : structType->m_fields)
        {
            const Index count = _getCount(field, in);
            if (count > 0)
            {
                const void* src = *(const void**)(reinterpret_cast<const uint8_t*>(in) + field.m_offset);
                SLANG_ASSERT(src);

                const void* dst = nullptr;
                switch (field.m_type)
                {
                    case FieldType::PtrTaggedStruct:
                    {
                        SLANG_RETURN_ON_FAIL(maybeConvertCurrentArray(src, count, dst));
                        break;
                    }
                    case FieldType::PtrPtrTaggedStruct:
                    {
                        const void*const* ptrDst;
                        SLANG_RETURN_ON_FAIL(maybeConvertCurrentPtrArray((const void*const*)src, count, ptrDst));
                        dst = ptrDst;
                        break;
                    }
                    default:    return SLANG_FAIL;
                }

                if (dst != src)
                {
                    // Set the and add to the stack
                    m_convertStack.add(dst);
                    fieldsSet |= bit;
                }
            }

            bit += bit;
        }
    }

    *outFieldsSet = fieldsSet;
    return SLANG_OK;
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

SlangResult StructTagConverter::maybeConvertCurrentArray(const void* in, Index count, const void*& out)
{
    if (count == 0)
    {
        out = in;
        return SLANG_OK;
    }

    if (count == 1)
    {
        return maybeConvertCurrent(in, out);
    }

    const slang::TaggedStructBase* arr = reinterpret_cast<const slang::TaggedStructBase*>(in);

    // We assume all have the same size/type
    auto tag = arr[0].structTag;

    auto structType = m_system->getType(tag);

    if (!canConvertToCurrent(tag, structType))
    {
        return _diagnoseCantConvert(tag, structType);
    }

    if (structType->m_sizeInBytes == arr[0].structSize)
    {
        // Can just use what was passed in
        out = in;
        return SLANG_OK;
    }

    SLANG_RETURN_ON_FAIL(_requireArena());

    const size_t dstStride = structType->m_sizeInBytes;
    uint8_t*const dstStart = (uint8_t*)m_arena->allocate(dstStride * count);
    uint8_t* dst = dstStart;

    size_t srcStride = arr[0].structSize;
    const uint8_t* src = reinterpret_cast<const uint8_t*>(in);

    for (Index i = 0; i < count; ++i)
    {
        // Do the straight copy
        copy(structType, src, dst);

        // Work out what was converted
        ScopeStack stackScope(this);
        BitField fieldsSet;
        SLANG_RETURN_ON_FAIL(maybeConvertCurrentContained(structType, in, &fieldsSet));

        // Copy anything converted
        setContainedConverted(structType, stackScope, fieldsSet, dst);

        src += srcStride;
        dst += dstStride;
    }

    out = dstStart;
    return SLANG_OK;
}

SlangResult StructTagConverter::maybeConvertCurrent(const void* in, const void*& out)
{
    auto base = reinterpret_cast<const slang::TaggedStructBase*>(in);
    auto tag = base->structTag;

    // If can't find the type it's incompatible
    auto structType = m_system->getType(tag);
    if (!structType)
    {
        return _diagnoseUnknownType(tag);
    }

    if (StructTagUtil::isReadCompatible(tag, structType->m_tag))
    {
        out = in;
        return SLANG_OK;
    }

    if (!canConvertToCurrent(tag, structType))
    {
        return _diagnoseCantConvert(tag, structType);
    }

    // Let's see if how everything contained converts
    ScopeStack stackScope(this);
    BitField fieldsSet;
    SLANG_RETURN_ON_FAIL(maybeConvertCurrentContained(structType, in, &fieldsSet));

    // Okay we will need to allocate and copy
    void* dst = allocateAndCopy(structType, in);

    // Copy anything converted
    setContainedConverted(structType, stackScope, fieldsSet, dst);
    out = dst;
    return SLANG_OK;
}

SlangResult StructTagConverter::maybeConvertCurrent(slang::StructTag tag, const void* in, const void*& out)
{
    auto base = reinterpret_cast<const slang::TaggedStructBase*>(in);
    auto inTag = base->structTag;

    if (!StructTagUtil::areSameType(inTag, tag))
    {
        return _diagnoseDifferentTypes(inTag, tag);
    }

    return maybeConvertCurrent(in, out);
}

} // namespace Slang
