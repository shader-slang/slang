#include "slang-rtti-util.h"

namespace Slang {

/* static */SlangResult RttiUtil::setInt(int64_t value, const RttiInfo* rttiInfo, void* dst)
{
    SLANG_ASSERT(rttiInfo->isIntegral());

    // We could check ranges are appropriate, but for now we just write.
    // Passing in rttiInfo allows for other more complex types to be econverted
    switch (rttiInfo->m_kind)
    {
        case RttiInfo::Kind::I32:   *(int32_t*)dst = int32_t(value); break;
        case RttiInfo::Kind::U32:   *(uint32_t*)dst = uint32_t(value); break;
        case RttiInfo::Kind::I64:   *(int64_t*)dst = int64_t(value); break;
        case RttiInfo::Kind::U64:   *(uint64_t*)dst = uint64_t(value); break;
        default: return SLANG_FAIL;
    }
    return SLANG_OK;
}

/* static */int64_t RttiUtil::getInt64(const RttiInfo* rttiInfo, const void* src)
{
    SLANG_ASSERT(rttiInfo->isIntegral());

    switch (rttiInfo->m_kind)
    {
        case RttiInfo::Kind::I32:   return *(const int32_t*)src;
        case RttiInfo::Kind::U32:   return *(const uint32_t*)src;
        case RttiInfo::Kind::I64:   return *(const int64_t*)src;
        case RttiInfo::Kind::U64:   return *(const uint64_t*)src;
        default: break;
    }

    SLANG_ASSERT(!"Not integral!");
    return -1;
}

/* static */double RttiUtil::asDouble(const RttiInfo* rttiInfo, const void* src)
{
    if (rttiInfo->isIntegral())
    {
        return (double)getInt64(rttiInfo, src);
    }
    else if (rttiInfo->isFloat())
    {
        switch (rttiInfo->m_kind)
        {
            case RttiInfo::Kind::F32:   return *(const float*)src;
            case RttiInfo::Kind::F64:   return *(const double*)src;
            default: break;
        }
    }

    SLANG_ASSERT(!"Cannot convert to float");
    return 0.0;
}

/* static */SlangResult RttiUtil::setFromDouble(double v, const RttiInfo* rttiInfo, void* dst)
{
    if (rttiInfo->isIntegral())
    {
        return setInt(int64_t(v), rttiInfo, dst);
    }
    else if (rttiInfo->isFloat())
    {
        switch (rttiInfo->m_kind)
        {
            case RttiInfo::Kind::F32:   *(float*)dst = float(v); return SLANG_OK;
            case RttiInfo::Kind::F64:   *(double*)dst = v; return SLANG_OK;
            default: break;
        }
    }

    return SLANG_FAIL;
}

/* static */bool RttiUtil::asBool(const RttiInfo* rttiInfo, const void* src)
{
    if (rttiInfo->m_kind == RttiInfo::Kind::Bool)
    {
        return *(const bool*)src;
    }

    if (rttiInfo->isIntegral())
    {
        return getInt64(rttiInfo, src) != 0;
    }
    else if (rttiInfo->isFloat())
    {
        return asDouble(rttiInfo, src) != 0.0;
    }

    SLANG_ASSERT(!"Cannot convert to bool");
    return false;
}

static int64_t _getIntDefaultValue(RttiDefaultValue value)
{
    switch (value)
    {
        default:
        case RttiDefaultValue::Normal:      return 0;
        case RttiDefaultValue::One:         return 1;
        case RttiDefaultValue::MinusOne:    return -1;
    }
}

static bool _isStructDefault(const StructRttiInfo* type, const void* src)
{
    if (type->m_super)
    {
        if (!_isStructDefault(type->m_super, src))
        {
            return false;
        }
    }

    const Byte* base = (const Byte*)src;

    const Index count = type->m_fieldCount;
    for (Index i = 0; i < count; ++i)
    {
        const auto& field = type->m_fields[i];

        const RttiDefaultValue defaultValue = RttiDefaultValue(field.m_flags & uint8_t(RttiDefaultValue::Mask));

        if (!RttiUtil::isDefault(defaultValue, field.m_type, base + field.m_offset))
        {
            return false;
        }
    }

    return true;
}

/* static */bool RttiUtil::isDefault(RttiDefaultValue defaultValue, const RttiInfo* rttiInfo, const void* src)
{
    if (rttiInfo->isIntegral())
    {
        const auto value = getInt64(rttiInfo, src);
        return _getIntDefaultValue(defaultValue) == value;
    }
    else if (rttiInfo->isFloat())
    {
        const auto value = asDouble(rttiInfo, src);
        return _getIntDefaultValue(defaultValue) == value;
    }

    switch (rttiInfo->m_kind)
    {
        case RttiInfo::Kind::Invalid:       return true;
        case RttiInfo::Kind::Bool:          return *(const bool*)src == (_getIntDefaultValue(defaultValue) != 0);
        case RttiInfo::Kind::String:
        {
            return ((const String*)src)->getLength() == 0;
        }
        case RttiInfo::Kind::UnownedStringSlice:
        {
            return ((const UnownedStringSlice*)src)->getLength() == 0;
        }
        case RttiInfo::Kind::Struct:
        {
            return _isStructDefault(static_cast<const StructRttiInfo*>(rttiInfo), src);
        }
        case RttiInfo::Kind::Enum:
        {
            SLANG_ASSERT(!"Not implemented yet");
            return false;
        }
        case RttiInfo::Kind::List:
        {
            const auto& v = *(const List<Byte>*)src;
            return v.getCount() == 0;
        }
        case RttiInfo::Kind::Dictionary:
        {
            const auto& v = *(const Dictionary<Byte, Byte>*)src;
            return v.Count() == 0;
        }
        case RttiInfo::Kind::Other:
        {
            const OtherRttiInfo* otherRttiInfo = static_cast<const OtherRttiInfo*>(rttiInfo);
            return otherRttiInfo->m_isDefaultFunc && otherRttiInfo->m_isDefaultFunc(rttiInfo, src);
        }
        default:
        {
            return false;
        }
    }
}

template <typename T>
struct GetRttiTypeFuncsForBuiltIn
{
    static void ctorArray(const RttiInfo* rttiInfo, void* dst, Index count) { SLANG_UNUSED(rttiInfo);  ::memset(dst, 0, sizeof(T) * count); }
    static void dtorArray(const RttiInfo* rttiInfo, void* dst, Index count) { SLANG_UNUSED(rttiInfo); SLANG_UNUSED(dst); SLANG_UNUSED(count); }
    static void copyArray(const RttiInfo* rttiInfo, void* dst, const void* src, Index count) { SLANG_UNUSED(rttiInfo); ::memcpy(dst, src, sizeof(T) * count); }

    static RttiTypeFuncs getFuncs()
    {
        RttiTypeFuncs funcs;
        funcs.copyArray = &copyArray;
        funcs.dtorArray = &dtorArray;
        funcs.ctorArray = &ctorArray;
        return funcs;
    }
};

struct ListFuncs
{
    static void ctorArray(const RttiInfo* rttiInfo, void* inDst, Index count)
    {
        SLANG_ASSERT(rttiInfo->m_kind == RttiInfo::Kind::List);
        //const ListRttiInfo* listRttiInfo = static_cast<const ListRttiInfo*>(rttiInfo);

        // We don't care about the element type, as we can just initialize them all as List<Byte>

        typedef List<Byte> Type;

        Type* dst = (Type*)inDst;

        for (Index i = 0; i < count; ++i)
        {
            new (dst + i) Type;
        }
    }
    static void copyArray(const RttiInfo* rttiInfo, void* inDst, const void* inSrc, Index count)
    {
        SLANG_ASSERT(rttiInfo->m_kind == RttiInfo::Kind::List);
        const ListRttiInfo* listRttiInfo = static_cast<const ListRttiInfo*>(rttiInfo);
        const auto elementType = listRttiInfo->m_elementType;

        // We need to get the type funcs
        auto typeFuncs = RttiUtil::getTypeFuncs(elementType);
        SLANG_ASSERT(typeFuncs.isValid());

        // We need a type that we can get information from the list from - List<Byte> gives us the functions we need.
        typedef List<Byte> Type;

        Type* dst = (Type*)inDst;
        const Type* src = (const Type*)inSrc;

        for (Index i = 0; i < count; ++i)
        {
            auto& dstList = dst[i];
            auto& srcList = src[i];

            const Index srcCount = srcList.getCount();

            if (srcCount > dstList.getCount())
            {
                // Allocate new memory

                auto dynamicArrayFuncs = RttiInfo::getDynamicArrayFuncs(elementType->m_alignment, elementType->m_size);
                SLANG_ASSERT(dynamicArrayFuncs.isValid());

                const Index dstCapacity = dstList.getCapacity();
                void* oldBuffer = dstList.detachBuffer();

                void* newBuffer = dynamicArrayFuncs.newFunc(count);
                // Initialize it all first
                typeFuncs.ctorArray(elementType, newBuffer, count);
                typeFuncs.copyArray(elementType, newBuffer, oldBuffer, count);

                // Attach the new buffer
                dstList.attachBuffer((Byte*)newBuffer, count, count);

                // Free the old buffer
                if (oldBuffer)
                {
                    typeFuncs.dtorArray(elementType, oldBuffer, dstCapacity);
                    dynamicArrayFuncs.deleteFunc(oldBuffer);
                }
            }
            else
            {
                typeFuncs.copyArray(elementType, dstList.getBuffer(), srcList.getBuffer(), srcCount);
                dstList.unsafeShrinkToCount(srcCount);
            }
        }
    }

    static void dtorArray(const RttiInfo* rttiInfo, void* inDst, Index count)
    {
        SLANG_ASSERT(rttiInfo->m_kind == RttiInfo::Kind::List);
        const ListRttiInfo* listRttiInfo = static_cast<const ListRttiInfo*>(rttiInfo);

        const auto elementType = listRttiInfo->m_elementType;

        auto dynamicArrayFuncs = RttiInfo::getDynamicArrayFuncs(elementType->m_alignment, elementType->m_size);
        SLANG_ASSERT(dynamicArrayFuncs.isValid());

        // We need to get the type funcs
        auto typeFuncs = RttiUtil::getTypeFuncs(elementType);
        SLANG_ASSERT(typeFuncs.isValid());

        typedef List<Byte> Type;
        Type* dst = (Type*)inDst;

        for (Index i = 0; i < count; ++i)
        {
            auto& dstList = dst[i];

            const Index capacity = dstList.getCapacity();
            Byte* buffer = dstList.detachBuffer();

            if (buffer)
            {
                typeFuncs.dtorArray(elementType, buffer, capacity);
                dynamicArrayFuncs.deleteFunc(buffer);
            }
        }
    }

    static RttiTypeFuncs getFuncs()
    {
        RttiTypeFuncs funcs;
        funcs.copyArray = &copyArray;
        funcs.dtorArray = &dtorArray;
        funcs.ctorArray = &ctorArray;
        return funcs;
    }
};

RttiTypeFuncs RttiUtil::getTypeFuncs(const RttiInfo* rttiInfo)
{
    if (rttiInfo->isBuiltIn())
    {
        switch (rttiInfo->m_size)
        {
            case 1: return GetRttiTypeFuncsForBuiltIn<uint8_t>::getFuncs();
            case 2: return GetRttiTypeFuncsForBuiltIn<uint16_t>::getFuncs();
            case 4: return GetRttiTypeFuncsForBuiltIn<uint32_t>::getFuncs();
            case 8: return GetRttiTypeFuncsForBuiltIn<uint64_t>::getFuncs();
        }
        return RttiTypeFuncs{};
    }

    switch (rttiInfo->m_kind)
    {
        case RttiInfo::Kind::String:                return GetRttiTypeFuncs<String>::getFuncs();
        case RttiInfo::Kind::UnownedStringSlice:    return GetRttiTypeFuncs<UnownedStringSlice>::getFuncs();
        case RttiInfo::Kind::List:                  return ListFuncs::getFuncs();
        default: break;
    }

    return RttiTypeFuncs{};
}

/* static */SlangResult RttiUtil::setListCount(const RttiInfo* elementType, void* dst, Index count)
{
    // NOTE! The following only works because List<T> has capacity initialized members, and
    // setting the count if it is <= capacity just sets the count (ie things aren't released(!)).
    
    List<Byte>& dstList = *(List<Byte>*)dst;
    const Index dstCount = dstList.getCount();
    if (dstCount == count)
    {
        return SLANG_OK;
    }
    if (count < dstCount)
    {
        dstList.unsafeShrinkToCount(count);
        return SLANG_OK;
    }

    // Get funcs needed
    const auto dynamicArrayFuncs = RttiInfo::getDynamicArrayFuncs(elementType->m_alignment, elementType->m_size);
    const auto typeFuncs = RttiUtil::getTypeFuncs(elementType);

    if (!dynamicArrayFuncs.isValid() || !typeFuncs.isValid())
    {
        return SLANG_FAIL;
    }

    const Index dstCapacity = dstList.getCapacity();
    void* oldBuffer = dstList.detachBuffer();

    void* newBuffer = dynamicArrayFuncs.newFunc(count);
    // Initialize it all first
    typeFuncs.ctorArray(elementType, newBuffer, count);
    typeFuncs.copyArray(elementType, newBuffer, oldBuffer, count);

    // Attach the new buffer
    dstList.attachBuffer((Byte*)newBuffer, count, count);

    // Free the old buffer
    if (oldBuffer)
    {
        typeFuncs.dtorArray(elementType, oldBuffer, dstCapacity);
        dynamicArrayFuncs.deleteFunc(oldBuffer);
    }

    return SLANG_OK;
}

} // namespace Slang
