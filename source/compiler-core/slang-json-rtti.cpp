#include "slang-json-rtti.h"

#include "../../slang-com-helper.h"

#include "../core/slang-rtti-util.h"

namespace Slang {

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
        default: return SLANG_FAIL;
    }

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
