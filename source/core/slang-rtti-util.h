#ifndef SLANG_CORE_RTTI_UTIL_H
#define SLANG_CORE_RTTI_UTIL_H

#include "slang-rtti-info.h"

namespace Slang {
struct RttiUtil
{
    static bool isIntegral(RttiInfo::Kind kind) { return Index(kind) >= Index(RttiInfo::Kind::I32) && Index(kind) <= Index(RttiInfo::Kind::U64); }
    static bool isFloat(RttiInfo::Kind kind) { return kind == RttiInfo::Kind::F32 || kind == RttiInfo::Kind::F64;  }

    static bool isIntegral(const RttiInfo* rttiInfo) { return isIntegral(rttiInfo->m_kind); }
    static bool isFloat(const RttiInfo* rttiInfo) { return isFloat(rttiInfo->m_kind); }

    static SlangResult setInt(int64_t value, const RttiInfo* rttiInfo, void* dst);
    static int64_t getInt64(const RttiInfo* rttiInfo, const void* src);

    static double asDouble(const RttiInfo* rttiInfo, const void* src);

    static SlangResult setFromDouble(double v, const RttiInfo* rttiInfo, void* dst);

    static bool asBool(const RttiInfo* rttiInfo, const void* src);

};

} // namespace Slang

#endif // SLANG_CORE_RTTI_UTIL_H
