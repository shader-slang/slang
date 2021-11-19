#ifndef SLANG_CORE_RTTI_UTIL_H
#define SLANG_CORE_RTTI_UTIL_H

#include "slang-rtti-info.h"

namespace Slang {
struct RttiUtil
{
    static SlangResult setInt(int64_t value, const RttiInfo* rttiInfo, void* dst);
    static int64_t getInt64(const RttiInfo* rttiInfo, const void* src);

    static double asDouble(const RttiInfo* rttiInfo, const void* src);

    static SlangResult setFromDouble(double v, const RttiInfo* rttiInfo, void* dst);

    static bool asBool(const RttiInfo* rttiInfo, const void* src);

    static bool isDefault(RttiDefaultValue defaultValue, const RttiInfo* rttiInfo, const void* src);

    static RttiTypeFuncs getTypeFuncs(const RttiInfo* rttiInfo);

        /// Set a list count
    static SlangResult setListCount(const RttiInfo* elementType, void* dst, Index count);

        /// Returns if the type can be zero initialized
    static bool canZeroInit(const RttiInfo* type);
        /// Returns true if the type needs dtor
    static bool hasDtor(const RttiInfo* type);
        /// Returns true if we can memcpy to copy 
    static bool canMemCpy(const RttiInfo* type);

    static void ctorArray(const RttiInfo* rttiInfo, void* inDst, ptrdiff_t stride, Index count);
    static void copyArray(const RttiInfo* rttiInfo, void* inDst, const void* inSrc, ptrdiff_t stride, Index count);
    static void dtorArray(const RttiInfo* rttiInfo, void* inDst, ptrdiff_t stride, Index count);
};

} // namespace Slang

#endif // SLANG_CORE_RTTI_UTIL_H
