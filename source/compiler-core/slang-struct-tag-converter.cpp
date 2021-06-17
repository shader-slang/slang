#include "slang-struct-tag-converter.h"

#include "slang-core-diagnostics.h"

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

SlangResult StructTagConverterBase::_requireArena()
{
    if (!m_arena)
    {
        if (m_sink)
        {
            // Diagnose that we need an arena
            m_sink->diagnose(SourceLoc(), MiscDiagnostics::structTagConversionFailureNoArena);
        }
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

SlangResult StructTagConverterBase::_diagnoseCantConvert(slang::StructTag tag, StructTagType* structType)
{
    SLANG_UNUSED(tag);
    SLANG_UNUSED(structType);

    if (m_sink)
    {
        // Diagnose why the tag couldn't be converted
        StringBuilder from, to;
        m_system->appendName(tag, from);
        m_system->appendName(structType->m_tag, to);

        m_sink->diagnose(SourceLoc(), MiscDiagnostics::cannotConvertStructTag, from, to);
    }

    return SLANG_E_STRUCT_TAG_INCOMPATIBLE;
}

SlangResult StructTagConverterBase::_diagnoseUnknownType(slang::StructTag tag)
{
    SLANG_UNUSED(tag);

    if (m_sink)
    {
        StringBuilder buf;
        m_system->appendName(tag, buf);
        m_sink->diagnose(SourceLoc(), MiscDiagnostics::unknownStructTag, buf);
    }

    // Perhaps this isn't quite right - but it's probably the most suitable error
    return SLANG_E_STRUCT_TAG_INCOMPATIBLE;
}

SlangResult StructTagConverterBase::_diagnoseDifferentTypes(slang::StructTag tagA, slang::StructTag tagB)
{
    if (m_sink)
    {
        StringBuilder a, b;
        m_system->appendName(tagA, a);
        m_system->appendName(tagB, b);

        m_sink->diagnose(SourceLoc(), MiscDiagnostics::cannotConvertDifferentStructTag, a, b);
    }

    // Perhaps this isn't quite right - but it's probably the most suitable error
    return SLANG_E_STRUCT_TAG_INCOMPATIBLE;
}

bool StructTagConverterBase::canConvertToCurrent(slang::StructTag tag, StructTagType* type) const
{
    // Means can be used without any modification
    if (StructTagUtil::isReadCompatible(tag, type->m_tag))
    {
        return true;
    }

    // We may want to allow zero extension, or initialization
    // We can accept for conversion if it's the same type with only difference being the minor version.
    return StructTagUtil::areSameMajorType(tag, type->m_tag);
}

void StructTagConverterBase::copy(const StructTagType* structType, const void* src, void* dst)
{
    const slang::TaggedStructBase* srcBase = reinterpret_cast<const slang::TaggedStructBase*>(src);

    const slang::StructSize size = std::min(structType->m_sizeInBytes, srcBase->structSize);

    // Copy
    ::memcpy(dst, src, size);

    // TODO(JS): Alternatively if we have the default set on the structType, we could initialize
    // other fields with the default values. For the moment we zero

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

void* StructTagConverterBase::allocateAndCopy(const StructTagType* structType, const void* src)
{
    uint8_t* dst = (uint8_t*)m_arena->allocate(structType->m_sizeInBytes);
    copy(structType, src, dst);
    return dst;
}

SlangResult StructTagConverterBase::convertCurrent(slang::StructTag tag, const void* in, void*& out)
{
    auto base = reinterpret_cast<const slang::TaggedStructBase*>(in);
    auto inTag = base->structTag;

    // Currently we can only convert if same major type.
    // In future it might be possible to improve on this
    if (!StructTagUtil::areSameMajorType(inTag, tag))
    {
        return _diagnoseDifferentTypes(inTag, tag);
    }

    return convertCurrent(in, out);
}

SlangResult StructTagConverterBase::convertArrayField(const FieldType type, const void* in, Index count, void*& out)
{
    if (count <= 0)
    {
        out = const_cast<void*>(in);
        return SLANG_OK;
    }

    SLANG_ASSERT(in);
    switch (type)
    {
        case FieldType::PtrTaggedStruct:        return convertCurrentArray(in, count, out);
        case FieldType::PtrPtrTaggedStruct:     return convertCurrentPtrArray((const void*const*)in, count, (void**&)out);
        default: break;
    }
    return SLANG_FAIL;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                  CopyStructTagConverter

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

SlangResult CopyStructTagConverter::convertCurrentPtrArray(const void*const* in, Index count, void**& out)
{
    if (count == 0)
    {
        out = nullptr;
        return SLANG_OK;
    }
    void** dst = (void**)m_arena->allocate(count * sizeof(void*));
    for (Index i = 0; i < count; ++i)
    {
        SLANG_RETURN_ON_FAIL(convertCurrent(in[i], dst[i]));
    }
    out = dst;
    return SLANG_OK;
}

SlangResult CopyStructTagConverter::convertCurrentArray(const void* in, Index count, void*& out)
{
    if (count <= 0)
    {
        out = nullptr;
        return SLANG_OK;
    }

    if (count == 1)
    {
        return convertCurrent(in, out);
    }

    const slang::TaggedStructBase* arr = reinterpret_cast<const slang::TaggedStructBase*>(in);

    // We assume all have the same size/type
    auto tag = arr[0].structTag;

    auto structType = m_system->getType(tag);

    if (!structType)
    {
        return _diagnoseUnknownType(tag);
    }
    if (!canConvertToCurrent(tag, structType))
    {
        return _diagnoseCantConvert(tag, structType);
    }

    const size_t dstStride = structType->m_sizeInBytes;
    uint8_t*const dstStart = (uint8_t*)m_arena->allocate(dstStride * count);
    uint8_t* dst = dstStart;

    size_t srcStride = arr[0].structSize;
    const uint8_t* src = reinterpret_cast<const uint8_t*>(in);

    for (Index i = 0; i < count; ++i)
    {
        copy(structType, src, dst);
        SLANG_RETURN_ON_FAIL(convertCurrentContained(structType, dst));

        src += srcStride;
        dst += dstStride;
    }

    out = dstStart;
    return SLANG_OK;
}

SlangResult CopyStructTagConverter::convertCurrentContained(const StructTagType* structType, void* inout)
{
    // Convert primary
    if (StructTagUtil::isPrimary(structType->m_tag))
    {
        // Copy extensions if needed
        slang::PrimaryTaggedStruct* primary = reinterpret_cast<slang::PrimaryTaggedStruct*>(inout);
        if (primary->extsCount > 0)
        {
            void** dstExts;
            SLANG_RETURN_ON_FAIL(convertCurrentPtrArray((const void*const*)primary->exts, primary->extsCount, dstExts));
            primary->exts = (const slang::StructTag**)dstExts;
        }
    }

    // It may have fields that need to be converted
    for (const auto& field : structType->m_fields)
    {
        const Index count = _getCount(field, inout);
        if (count)
        {
            void*& ptrRef = *(void**)(reinterpret_cast<uint8_t*>(inout) + field.m_offset);
            SLANG_RETURN_ON_FAIL(convertArrayField(field.m_type, ptrRef, count, ptrRef));
        }
    }

    return SLANG_OK;
}

SlangResult CopyStructTagConverter::convertCurrent(const void* in, void*& out)
{
    auto tag = reinterpret_cast<const slang::TaggedStructBase*>(in)->structTag;

    auto structType = m_system->getType(tag);
    if (!structType)
    {
        return _diagnoseUnknownType(tag);
    }
    if (!canConvertToCurrent(tag, structType))
    {
        return _diagnoseCantConvert(tag, structType);
    }

    slang::TaggedStructBase* dst = (slang::TaggedStructBase*)allocateAndCopy(structType, in);
    SLANG_RETURN_ON_FAIL(convertCurrentContained(structType, dst));
    out = dst;
    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                  LazyStructTagConverter

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

SlangResult LazyStructTagConverter::convertCurrentPtrArray(const void*const* in, Index count, void**& out)
{
    if (count == 0)
    {
        out = (void**)in;
        return SLANG_OK;
    }

    Index numConverted = 0;
    ScopeStack stackScope(this);

    for (Index i = 0; i < count; ++i)
    {
        const void* src = in[i];
        void* dst;
        SLANG_RETURN_ON_FAIL(convertCurrent(src, dst));

        // Make space if not set up
        numConverted += Index(dst != src);
        m_convertStack.add(dst);
    }

    // We need to make a copy of the exts
    if (numConverted)
    {
        SLANG_RETURN_ON_FAIL(_requireArena());
        out = (void**)m_arena->allocateAndCopyArray(m_convertStack.getBuffer() + stackScope.getStartIndex(), count);
    }
    else
    {
        out = (void**)in;
    }

    return SLANG_OK;
}

void LazyStructTagConverter::setContainedConverted(const StructTagType* structType, Index stackIndex, BitField fieldsSet, void* out)
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


SlangResult LazyStructTagConverter::maybeConvertCurrentContained(const StructTagType* structType, const void* in, BitField* outFieldsSet)
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
            void** dstExts;
            SLANG_RETURN_ON_FAIL(convertCurrentPtrArray(srcExts, primary->extsCount, dstExts));

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
                void* dst = nullptr;
                const void* src = *(const void**)(reinterpret_cast<const uint8_t*>(in) + field.m_offset);
                SLANG_RETURN_ON_FAIL(convertArrayField(field.m_type, src, count, dst));

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

SlangResult LazyStructTagConverter::convertCurrentArray(const void* in, Index count, void*& out)
{
    if (count == 0)
    {
        out = (void*)in;
        return SLANG_OK;
    }

    if (count == 1)
    {
        return convertCurrent(in, out);
    }

    const slang::TaggedStructBase* arr = reinterpret_cast<const slang::TaggedStructBase*>(in);

    // We assume all have the same size/type
    auto tag = arr[0].structTag;

    auto structType = m_system->getType(tag);

    if (!structType)
    {
        return _diagnoseUnknownType(tag);
    }

    if (StructTagUtil::areSameMajorType(tag, structType->m_tag) && structType->m_sizeInBytes == arr[0].structSize)
    {
        // Can just use what was passed in
        out = (void*)in;
        return SLANG_OK;
    }

    if (!canConvertToCurrent(tag, structType))
    {
        return _diagnoseCantConvert(tag, structType);
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
        ScopeStack scopeStack(this);
        BitField fieldsSet;
        SLANG_RETURN_ON_FAIL(maybeConvertCurrentContained(structType, in, &fieldsSet));

        // Copy anything converted
        setContainedConverted(structType, scopeStack, fieldsSet, dst);

        src += srcStride;
        dst += dstStride;
    }

    out = dstStart;
    return SLANG_OK;
}

SlangResult LazyStructTagConverter::convertCurrent(const void* in, void*& out)
{
    auto base = reinterpret_cast<const slang::TaggedStructBase*>(in);
    auto tag = base->structTag;

    // If can't find the type it's incompatible
    auto structType = m_system->getType(tag);
    if (!structType)
    {
        return _diagnoseUnknownType(tag);
    }

    if (!canConvertToCurrent(tag, structType))
    {
        return _diagnoseCantConvert(tag, structType);
    }

    // Let's see if how everything contained converts
    ScopeStack stackScope(this);
    BitField fieldsSet;
    SLANG_RETURN_ON_FAIL(maybeConvertCurrentContained(structType, in, &fieldsSet));

    // If there were no fields set
    if (fieldsSet == 0)
    {
        // And it's the same major type, and is at least as big a the current struct
        // We can just use as is
        if (StructTagUtil::areSameMajorType(tag, structType->m_tag) && base->structSize >= structType->m_sizeInBytes)
        {
            out = (void*)in;
            return SLANG_OK;
        }
    }

    // Okay we will need to allocate and copy
    void* dst = allocateAndCopy(structType, in);

    // Copy anything converted
    setContainedConverted(structType, stackScope, fieldsSet, dst);

    out = dst;
    return SLANG_OK;
}

} // namespace Slang
