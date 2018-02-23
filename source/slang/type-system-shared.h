#ifndef SLANG_TYPE_SYSTEM_SHARED_H
#define SLANG_TYPE_SYSTEM_SHARED_H

#include "../../slang.h"

namespace Slang
{
    enum class BaseType
    {
        Void = 0,
        Bool,
        Int,
        UInt,
        UInt64,
        Half,
        Float,
        Double,
    };

    struct TextureFlavor
    {
        enum
        {
            // Mask for the overall "shape" of the texture
            ShapeMask = SLANG_RESOURCE_BASE_SHAPE_MASK,

            // Flag for whether the shape has "array-ness"
            ArrayFlag = SLANG_TEXTURE_ARRAY_FLAG,

            // Whether or not the texture stores multiple samples per pixel
            MultisampleFlag = SLANG_TEXTURE_MULTISAMPLE_FLAG,

            // Whether or not this is a shadow texture
            //
            // TODO(tfoley): is this even meaningful/used?
            // ShadowFlag		= 0x80, 
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

        uint16_t flavor;

        Shape GetBaseShape() const { return Shape(flavor & ShapeMask); }
        bool isArray() const { return (flavor & ArrayFlag) != 0; }
        bool isMultisample() const { return (flavor & MultisampleFlag) != 0; }
        //            bool isShadow() const { return (flavor & ShadowFlag) != 0; }

        SlangResourceShape getShape() const { return flavor & 0xFF; }
        SlangResourceAccess getAccess() const { return (flavor >> 8) & 0xFF; }

        TextureFlavor() = default;
        TextureFlavor(uint32_t tag) { flavor = (uint16_t)tag; }

        static TextureFlavor create(SlangResourceShape shape, SlangResourceAccess access);
    };

    enum class SamplerStateFlavor : uint8_t
    {
        SamplerState,
        SamplerComparisonState,
    };

}

#endif