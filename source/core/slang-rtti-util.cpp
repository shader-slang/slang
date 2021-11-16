#include "slang-rtti-util.h"

namespace Slang {

/* static */SlangResult RttiUtil::setInt(int64_t value, const RttiInfo* rttiInfo, void* dst)
{
    SLANG_ASSERT(isIntegral(rttiInfo));

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
    SLANG_ASSERT(isIntegral(rttiInfo));

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
    if (isIntegral(rttiInfo))
    {
        return (double)getInt64(rttiInfo, src);
    }
    else if (isFloat(rttiInfo))
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
    if (isIntegral(rttiInfo))
    {
        return setInt(int64_t(v), rttiInfo, dst);
    }
    else if (isFloat(rttiInfo))
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

    if (isIntegral(rttiInfo))
    {
        return getInt64(rttiInfo, src) != 0;
    }
    else if (isFloat(rttiInfo))
    {
        return asDouble(rttiInfo, src) != 0.0;
    }

    SLANG_ASSERT(!"Cannot convert to bool");
    return false;
}

} // namespace Slang
