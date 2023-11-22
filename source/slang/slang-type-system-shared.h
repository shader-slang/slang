#ifndef SLANG_TYPE_SYSTEM_SHARED_H
#define SLANG_TYPE_SYSTEM_SHARED_H

#include "../../slang.h"

namespace Slang
{
#define FOREACH_BASE_TYPE(X)    \
    X(Void)                     \
    X(Bool)                     \
    X(Int8)                     \
    X(Int16)                    \
    X(Int)                      \
    X(Int64)                    \
    X(UInt8)                    \
    X(UInt16)                   \
    X(UInt)                     \
    X(UInt64)                   \
    X(Half)                     \
    X(Float)                    \
    X(Double)                   \
    X(Char)                     \
    X(IntPtr)                   \
    X(UIntPtr)                  \
/* end */

    enum class BaseType
    {
#define DEFINE_BASE_TYPE(NAME) NAME,
FOREACH_BASE_TYPE(DEFINE_BASE_TYPE)
#undef DEFINE_BASE_TYPE

        CountOf,
    };

    enum class SamplerStateFlavor : uint8_t
    {
        SamplerState,
        SamplerComparisonState,
    };

    const int kStdlibResourceAccessReadOnly = 0;
    const int kStdlibResourceAccessReadWrite = 1;
    const int kStdlibResourceAccessRasterizerOrdered = 2;
    const int kStdlibResourceAccessFeedback = 3;

    const int kStdlibShapeIndex1D = 0;
    const int kStdlibShapeIndex2D = 1;
    const int kStdlibShapeIndex3D = 2;
    const int kStdlibShapeIndexCube = 3;
    const int kStdlibShapeIndexBuffer = 4;

    const int kStdlibTextureShapeParameterIndex = 1;
    const int kStdlibTextureIsArrayParameterIndex = 2;
    const int kStdlibTextureIsMultisampleParameterIndex = 3;
    const int kStdlibTextureSampleCountParameterIndex = 4;
    const int kStdlibTextureAccessParameterIndex = 5;
    const int kStdlibTextureIsShadowParameterIndex = 6;
    const int kStdlibTextureIsCombinedParameterIndex = 7;
    const int kStdlibTextureFormatParameterIndex = 8;
}

#endif
