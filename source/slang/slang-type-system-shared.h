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
/* end */

    enum class BaseType
    {
#define DEFINE_BASE_TYPE(NAME) NAME,
FOREACH_BASE_TYPE(DEFINE_BASE_TYPE)
#undef DEFINE_BASE_TYPE

        CountOf,
    };

    struct TextureFlavor
    {
        typedef TextureFlavor ThisType;
        enum
        {
            // Mask for the overall "shape" of the texture
            BaseShapeMask = SLANG_RESOURCE_BASE_SHAPE_MASK,

            // Flag for whether the shape has "array-ness"
            ArrayFlag = SLANG_TEXTURE_ARRAY_FLAG,

            // Whether or not the texture stores multiple samples per pixel
            MultisampleFlag = SLANG_TEXTURE_MULTISAMPLE_FLAG,

            // Whether or not this is a shadow texture
            //
            // TODO(tfoley): is this even meaningful/used?
            // ShadowFlag		= 0x80,

            // For feedback texture
            FeedbackFlag = SLANG_TEXTURE_FEEDBACK_FLAG,
        };

        enum Shape : uint8_t
        {
            Shape1D = SLANG_TEXTURE_1D,
            Shape2D = SLANG_TEXTURE_2D,
            Shape3D = SLANG_TEXTURE_3D,
            ShapeCube = SLANG_TEXTURE_CUBE,
            ShapeBuffer = SLANG_TEXTURE_BUFFER,
            
            Shape1DArray = Shape1D | ArrayFlag,
            Shape2DArray = Shape2D | ArrayFlag,
            // No Shape3DArray
            ShapeCubeArray = ShapeCube | ArrayFlag,
        };

        enum
        {
            // This the total number of expressible flavors,
            // which is *not* to say that every expressible
            // flavor is actual valid.
            Count = 0x10000,
        };

        uint16_t flavor;

        Shape getBaseShape() const { return Shape(flavor & BaseShapeMask); }
        bool isArray() const { return (flavor & ArrayFlag) != 0; }
        bool isMultisample() const { return (flavor & MultisampleFlag) != 0; }
        bool isFeedback() const { return (flavor & FeedbackFlag) != 0; }
        //            bool isShadow() const { return (flavor & ShadowFlag) != 0; }

        SLANG_FORCE_INLINE bool operator==(const ThisType& rhs) const { return flavor == rhs.flavor; }
        SLANG_FORCE_INLINE bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        SlangResourceShape getShape() const { return SlangResourceShape(flavor & 0xFF); }
        SlangResourceAccess getAccess() const { return SlangResourceAccess((flavor >> 8) & 0xFF); }

        TextureFlavor() = default;
        TextureFlavor(uint32_t tag) { flavor = (uint16_t)tag; }

        static TextureFlavor create(SlangResourceShape shape, SlangResourceAccess access);
        static TextureFlavor create(SlangResourceShape shape, SlangResourceAccess access, int flags);

        static TextureFlavor create(TextureFlavor::Shape shape, SlangResourceAccess access) { return create(SlangResourceShape(shape), access); }
        static TextureFlavor create(TextureFlavor::Shape shape, SlangResourceAccess access, int flags) { return create(SlangResourceShape(shape), access, flags); }

    };

    enum class SamplerStateFlavor : uint8_t
    {
        SamplerState,
        SamplerComparisonState,
    };

}

#endif
