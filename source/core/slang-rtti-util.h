#ifndef SLANG_CORE_RTTI_UTIL_H
#define SLANG_CORE_RTTI_UTIL_H

#include "slang-rtti-info.h"

namespace Slang {
struct RttiUtil
{
    static bool isIntegral(RttiInfo::Kind kind) { return Index(kind) >= Index(RttiInfo::Kind::I32) && Index(kind) <= Index(RttiInfo::Kind::U64); }
    static bool isFloat(RttiInfo::Kind kind) { return kind == RttiInfo::Kind::F32 || kind == RttiInfo::Kind::F64;  }

    static bool isBuiltIn(RttiInfo::Kind kind) { return kind == RttiInfo::Kind::I32 || kind == RttiInfo::Kind::Bool; }

    static bool isIntegral(const RttiInfo* rttiInfo) { return isIntegral(rttiInfo->m_kind); }
    static bool isFloat(const RttiInfo* rttiInfo) { return isFloat(rttiInfo->m_kind); }
    static bool isBuiltIn(const RttiInfo* rttiInfo) { return isBuiltIn(rttiInfo->m_kind); }

    static SlangResult setInt(int64_t value, const RttiInfo* rttiInfo, void* dst);
    static int64_t getInt64(const RttiInfo* rttiInfo, const void* src);

    static double asDouble(const RttiInfo* rttiInfo, const void* src);

    static SlangResult setFromDouble(double v, const RttiInfo* rttiInfo, void* dst);

    static bool asBool(const RttiInfo* rttiInfo, const void* src);

    static bool isDefault(RttiDefaultValue defaultValue, const RttiInfo* rttiInfo, const void* src);

    static RttiTypeFuncs getTypeFuncs(const RttiInfo* rttiInfo);

        /// Set a list count
    static SlangResult setListCount(const RttiInfo* elementType, void* dst, Index count);

};

} // namespace Slang

#endif // SLANG_CORE_RTTI_UTIL_H
