#include "slang-json-rtti.h"

#include "../../slang-com-helper.h"

namespace Slang {

/* static */SlangResult JSONRttiUtil::nativeToJSON(JSONContainer* container, const RttiInfo* rttiInfo, const void* in, JSONValue& out)
{
    switch (rttiInfo->m_kind)
    {
        case RttiInfo::Kind::Invalid:   return SLANG_FAIL;
        case RttiInfo::Kind::I32:    out = JSONValue::makeInt(*(const int32_t*)in); break;
        case RttiInfo::Kind::U32:    out = JSONValue::makeInt(*(const uint32_t*)in); break;
        case RttiInfo::Kind::I64:    out = JSONValue::makeInt(*(const int64_t*)in); break;
        case RttiInfo::Kind::U64:    out = JSONValue::makeInt(*(const uint64_t*)in); break;
        case RttiInfo::Kind::F32:    out = JSONValue::makeFloat(*(const float*)in); break;
        case RttiInfo::Kind::F64:    out = JSONValue::makeFloat(*(const double*)in); break;
        case RttiInfo::Kind::Bool:   out = JSONValue::makeBool(*(const bool*)in); break;
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
            // We don't know how to identify a default value. 
            // We have to have that otherwise, we can't optionally write out

            break;
        }
        case RttiInfo::Kind::Enum:
        {
            return SLANG_E_NOT_IMPLEMENTED;
        }
        case RttiInfo::Kind::List:
        {
            // We need to have sizeof for all types, otherwise don't know how to iterate over the type.

            break;
        }
        case RttiInfo::Kind::Dictionary:
        {
            // We can *only* serialize this into a straight JSON object iff the key is a string-like type
            // We could turn into (say) an array of keys and values
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
    }

    return SLANG_OK;
}

/* static */SlangResult JSONRttiUtil::jsonToNative(JSONContainer* container, const JSONValue& in, const RttiInfo* rttiInfo, void* out)
{
}

} // namespace Slang
