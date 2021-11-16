#include "slang-json-rtti.h"

#include "../../slang-com-helper.h"

#include "../core/slang-rtti-util.h"

namespace Slang {

static SlangResult _structNativeToJSON(JSONContainer* container, const StructRttiInfo* structRttiInfo, const void* src, List<JSONKeyValue>& outPairs)
{
    // Do the super class first
    if (structRttiInfo->m_super)
    {
        SLANG_RETURN_ON_FAIL(_structNativeToJSON(container, structRttiInfo, src, outPairs));
    }

    const Byte* base = (const Byte*)src;
    const Index count = structRttiInfo->m_fieldCount;

    for (Index i = 0; i < count; ++i)
    {
        const auto& field = structRttiInfo->m_fields[i];

        if (field.m_flags & StructRttiInfo::Flag::Optional)
        {
            const RttiDefaultValue defaultValue = RttiDefaultValue(field.m_flags & uint8_t(RttiDefaultValue::Mask));
            if (RttiUtil::isDefault(defaultValue, field.m_type, base + field.m_offset))
            {
                // If it's a default, we don't bother writing it
                continue;
            }
        }

        JSONKeyValue pair;

        pair.key = container->getKey(UnownedStringSlice(field.m_name));
        SLANG_RETURN_ON_FAIL(JSONRttiUtil::nativeToJSON(container, field.m_type, base + field.m_offset, pair.value));

        outPairs.add(pair);
    }

    return SLANG_OK;
}

/* static */SlangResult JSONRttiUtil::nativeToJSON(JSONContainer* container, const RttiInfo* rttiInfo, const void* in, JSONValue& out)
{
    if (RttiUtil::isIntegral(rttiInfo))
    {
        out = JSONValue::makeInt(RttiUtil::getInt64(rttiInfo, in));
    }
    else if (RttiUtil::isFloat(rttiInfo))
    {
        out = JSONValue::makeFloat(RttiUtil::asDouble(rttiInfo, in));
    }

    switch (rttiInfo->m_kind)
    {
        case RttiInfo::Kind::Invalid:   return SLANG_FAIL;
        case RttiInfo::Kind::Bool:      out = JSONValue::makeBool(RttiUtil::asBool(rttiInfo, in)); break;
        case RttiInfo::Kind::String:
        {
            const String& str = *(const String*)in;
            out = container->createString(str.getUnownedSlice());
            return SLANG_OK;
        }
        case RttiInfo::Kind::UnownedStringSlice:
        {
            const UnownedStringSlice& slice = *(const UnownedStringSlice*)in;
            out = container->createString(slice);
            return SLANG_OK;
        }
        case RttiInfo::Kind::Struct:
        {
            const StructRttiInfo* structRttiInfo = static_cast<const StructRttiInfo*>(rttiInfo);

            List<JSONKeyValue> pairs;
            SLANG_RETURN_ON_FAIL(_structNativeToJSON(container, structRttiInfo, in, pairs));
            out = container->createObject(pairs.getBuffer(), pairs.getCount());
            return SLANG_OK;
        }
        case RttiInfo::Kind::Enum:
        {
            return SLANG_E_NOT_IMPLEMENTED;
        }
        case RttiInfo::Kind::List:
        {
            const ListRttiInfo* listRttiInfo = static_cast<const ListRttiInfo*>(rttiInfo);
            const auto elementRttiInfo = listRttiInfo->m_elementType;

            // The src probably *doesn't* contain bytes, but can cast like this because
            // we only need the count (which doesn't depend on <T>), and the backing buffer
            const List<Byte>& srcValuesList = *(const List<Byte>*)in;

            const Index count = srcValuesList.getCount();
            const Byte* srcValues = srcValuesList.getBuffer();

            List<JSONValue> dstValues;
            dstValues.setCount(count);

            const size_t elementStride = elementRttiInfo->m_size;

            for (Index i = 0; i < count; ++i, srcValues += elementStride)
            {
                SLANG_RETURN_ON_FAIL(nativeToJSON(container, elementRttiInfo, srcValues, dstValues[i]));
            }

            out = container->createArray(dstValues.getBuffer(), count);
            return SLANG_OK;
        }
        case RttiInfo::Kind::Dictionary:
        {
            const DictionaryRttiInfo* listRttiInfo = static_cast<const DictionaryRttiInfo*>(rttiInfo);
            const auto keyRttiInfo = listRttiInfo->m_keyType;
            const auto valueRttiInfo = listRttiInfo->m_valueType;

            SLANG_UNUSED(keyRttiInfo);
            SLANG_UNUSED(valueRttiInfo);

            // We can *only* serialize this into a straight JSON object iff the key is a string-like type
            // We could turn into (say) an array of keys and values

            return SLANG_E_NOT_IMPLEMENTED;
        }
        case RttiInfo::Kind::Other:
        {
            if (rttiInfo == GetRttiInfo<JSONValue>::get())
            {
                // Do we need to copy into the container?
                // As it stands we have to assume src is stored in container.
                const JSONValue& src = *(const JSONValue*)in;

                out = src;
                return SLANG_OK;
            }
            return SLANG_FAIL;
        }
        default: return SLANG_FAIL;
    }

    return SLANG_OK;
}

static Index _getFieldCount(const StructRttiInfo* structRttiInfo)
{
    if (structRttiInfo->m_super)
    {
        return _getFieldCount(structRttiInfo->m_super) + structRttiInfo->m_fieldCount;
    }
    else
    {
        return structRttiInfo->m_fieldCount;
    }
}

static SlangResult _jsonToNative(JSONContainer* container, const ConstArrayView<JSONKeyValue>& pairs, const StructRttiInfo* structRttiInfo, void* out, Index& outFieldCount)
{
    Index fieldCount = 0;

    if (structRttiInfo->m_super)
    {
        SLANG_RETURN_ON_FAIL(_jsonToNative(container, pairs, structRttiInfo->m_super, out, fieldCount));
    }

    Byte* dst = (Byte*)out;

    const Index count = structRttiInfo->m_fieldCount;

    for (Index i = 0; i < count; ++i)
    {
        const auto& field = structRttiInfo->m_fields[i];

        auto key = container->findKey(UnownedStringSlice(field.m_name));

        if (key == 0)
        {
            if (field.m_flags & StructRttiInfo::Flag::Optional)
            {
                continue;
            }

            // Unable to find this key
            return SLANG_FAIL;
        }

        // If there are any of the pairs, that are not in the type.. it's an error
        const Index index = pairs.findFirstIndex([key](const JSONKeyValue& pair) -> bool { return pair.key == key; });
        if (index < 0)
        {
            if (field.m_flags & StructRttiInfo::Flag::Optional)
            {
                continue;
            }

            // Unable to find this key
            return SLANG_FAIL;
        }

        auto& pair = pairs[index];

        // Copy the field over
        SLANG_RETURN_ON_FAIL(JSONRttiUtil::jsonToNative(container, pair.value, field.m_type, dst + field.m_offset));

        // Field was handled
        ++fieldCount;
    }

    // Write off the amount of fields converted/handled. 
    outFieldCount = fieldCount;
    return SLANG_OK;
}

/* static */SlangResult JSONRttiUtil::jsonToNative(JSONContainer* container, const JSONValue& in, const RttiInfo* rttiInfo, void* out)
{
    SLANG_UNUSED(container);

    if (RttiUtil::isIntegral(rttiInfo))
    {
        return RttiUtil::setInt(in.asInteger(), rttiInfo, out);
    }
    else if (RttiUtil::isFloat(rttiInfo))
    {
        return RttiUtil::setFromDouble(in.asFloat(), rttiInfo, out);
    }

    switch (rttiInfo->m_kind)
    {
        case RttiInfo::Kind::Bool:
        {
            *(bool*)out = in.asBool();
            return SLANG_OK;
        }
        case RttiInfo::Kind::Struct:
        {
            if (in.getKind() != JSONValue::Kind::Object)
            {
                return SLANG_FAIL;
            }

            auto pairs = container->getObject(in);
            const StructRttiInfo* structRttiInfo = static_cast<const StructRttiInfo*>(rttiInfo);

            Index fieldCount = 0;
            SLANG_RETURN_ON_FAIL(_jsonToNative(container, pairs, structRttiInfo, out, fieldCount));

            if (fieldCount != pairs.getCount())
            {
                // If these are different then there are fields defined in the object that are *not* defined in class definition
                return SLANG_FAIL;
            }

            return SLANG_OK;
        }
        case RttiInfo::Kind::Enum:
        {
            return SLANG_E_NOT_IMPLEMENTED;
        }
        case RttiInfo::Kind::List:
        {
            if (in.getKind() != JSONValue::Kind::Array)
            {
                return SLANG_FAIL;
            }

            typedef List<Byte> Type;
            Type& list = *(Type*)out;

            auto arr = container->getArray(in);

            const Index count = arr.getCount();

            const ListRttiInfo* listRttiInfo = static_cast<const ListRttiInfo*>(rttiInfo);
            auto elementType = listRttiInfo->m_elementType;

            SLANG_RETURN_ON_FAIL(RttiUtil::setListCount(elementType, out, arr.getCount()));

            // Okay, we need to copy over one by one

            Byte* dstEles = list.getBuffer();
            for (Index i = 0; i < count; ++i, dstEles += elementType->m_size)
            {
                SLANG_RETURN_ON_FAIL(jsonToNative(container, arr[i], elementType, dstEles));
            }

            return SLANG_OK;
        }
        case RttiInfo::Kind::Dictionary:
        {
            // We can *only* serialize this into a straight JSON object iff the key is a string-like type
            // We could turn into (say) an array of keys and values
            break;
        }
        case RttiInfo::Kind::Other:
        {
            if (rttiInfo == GetRttiInfo<JSONValue>::get())
            {
                // Do we need to copy into the container?
                // As it stands we have to assume src is stored in container.
                *(JSONValue*)out = in;
                return SLANG_OK;
            }
            return SLANG_FAIL;
        }
        default: break;
    }
    return SLANG_FAIL;
}

} // namespace Slang
