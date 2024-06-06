// metal-util.cpp
#include "metal-util.h"
#include "core/slang-math.h"

#include <stdlib.h>
#include <stdio.h>

namespace gfx {

using namespace MTL;

MTL::VertexFormat MetalUtil::translateVertexFormat(Format format)
{
    switch (format)
    {
    case Format::R8G8_UINT:                 return VertexFormatUChar2;
    // VertexFormatUChar3
    case Format::R8G8B8A8_UINT:             return VertexFormatUChar4;
    case Format::R8G8_SINT:                 return VertexFormatChar2;
    // return VertexFormatChar3
    case Format::R8G8B8A8_SINT:             return VertexFormatChar4;
    case Format::R8G8_UNORM:                return VertexFormatUChar2Normalized;
    // return VertexFormatUChar3Normalized;
    case Format::R8G8B8A8_UNORM:            return VertexFormatUChar4Normalized;
    case Format::R8G8_SNORM:                return VertexFormatChar2Normalized;
    // return VertexFormatChar3Normalized
    case Format::R8G8B8A8_SNORM:            return VertexFormatChar4Normalized;
    case Format::R16G16_UINT:               return VertexFormatUShort2;
    // return VertexFormatUShort3;
    case Format::R16G16B16A16_UINT:         return VertexFormatUShort4;
    case Format::R16G16_SINT:               return VertexFormatShort2;
    // return VertexFormatShort3;
    case Format::R16G16B16A16_SINT:         return VertexFormatShort4;
    case Format::R16G16_UNORM:              return VertexFormatUShort2Normalized;
    // return VertexFormatUShort3Normalized;
    case Format::R16G16B16A16_UNORM:        return VertexFormatUShort4Normalized;
    case Format::R16G16_SNORM:              return VertexFormatShort2Normalized;
    // return VertexFormatShort3Normalized;
    case Format::R16G16B16A16_SNORM:        return VertexFormatShort4Normalized;
    case Format::R16G16_FLOAT:              return VertexFormatHalf2;
    // return VertexFormatHalf3;
    case Format::R16G16B16A16_FLOAT:        return VertexFormatHalf4;
    case Format::R32_FLOAT:                 return VertexFormatFloat;
    case Format::R32G32_FLOAT:              return VertexFormatFloat2;
    case Format::R32G32B32_FLOAT:           return VertexFormatFloat3;
    case Format::R32G32B32A32_FLOAT:        return VertexFormatFloat4;
    case Format::R32_SINT:                  return VertexFormatInt;
    case Format::R32G32_SINT:               return VertexFormatInt2;
    case Format::R32G32B32_SINT:            return VertexFormatInt3;
    case Format::R32G32B32A32_SINT:         return VertexFormatInt4;
    case Format::R32_UINT:                  return VertexFormatUInt;
    case Format::R32G32_UINT:               return VertexFormatUInt2;
    case Format::R32G32B32_UINT:            return VertexFormatUInt3;
    case Format::R32G32B32A32_UINT:         return VertexFormatUInt4;
    // return VertexFormatInt1010102Normalized;
    case Format::R10G10B10A2_UNORM:         return VertexFormatUInt1010102Normalized;
    case Format::B4G4R4A4_UNORM:            return VertexFormatUChar4Normalized_BGRA;
    case Format::R8_UINT:                   return VertexFormatUChar;
    case Format::R8_SINT:                   return VertexFormatChar;
    case Format::R8_UNORM:                  return VertexFormatUCharNormalized;
    case Format::R8_SNORM:                  return VertexFormatCharNormalized;
    case Format::R16_UINT:                  return VertexFormatUShort;
    case Format::R16_SINT:                  return VertexFormatShort;
    case Format::R16_UNORM:                 return VertexFormatUShortNormalized;
    case Format::R16_SNORM:                 return VertexFormatShortNormalized;
    case Format::R16_FLOAT:                 return VertexFormatHalf;
    case Format::R11G11B10_FLOAT:           return VertexFormatFloatRG11B10;
    case Format::R9G9B9E5_SHAREDEXP:        return VertexFormatFloatRGB9E5;
    default:                                return VertexFormatInvalid;
    }
}

/* static */MTL::PixelFormat MetalUtil::getMetalPixelFormat(Format format)
{
    switch (format)
    {
    case Format::R32G32B32A32_TYPELESS:     return PixelFormatRGBA32Float;
    case Format::R32G32B32_TYPELESS:        return PixelFormatInvalid;
    case Format::R32G32_TYPELESS:           return PixelFormatRG32Float;
    case Format::R32_TYPELESS:              return PixelFormatR32Float;

    case Format::R16G16B16A16_TYPELESS:     return PixelFormatRGBA16Float;
    case Format::R16G16_TYPELESS:           return PixelFormatRG16Float;
    case Format::R16_TYPELESS:              return PixelFormatR16Float;

    case Format::R8G8B8A8_TYPELESS:         return PixelFormatRGBA8Unorm;
    case Format::R8G8_TYPELESS:             return PixelFormatRG8Unorm;
    case Format::R8_TYPELESS:               return PixelFormatR8Unorm;
    case Format::B8G8R8A8_TYPELESS:         return PixelFormatBGRA8Unorm;

    case Format::R32G32B32A32_FLOAT:        return PixelFormatRGBA32Float;
    case Format::R32G32B32_FLOAT:           return PixelFormatInvalid;
    case Format::R32G32_FLOAT:              return PixelFormatRG32Float;
    case Format::R32_FLOAT:                 return PixelFormatR32Float;

    case Format::R16G16B16A16_FLOAT:        return PixelFormatRGBA16Float;
    case Format::R16G16_FLOAT:              return PixelFormatRG16Float;
    case Format::R16_FLOAT:                 return PixelFormatR16Float;

    case Format::R32G32B32A32_UINT:         return PixelFormatRGBA32Uint;
    case Format::R32G32B32_UINT:            return PixelFormatInvalid;
    case Format::R32G32_UINT:               return PixelFormatRG32Uint;
    case Format::R32_UINT:                  return PixelFormatR32Uint;

    case Format::R16G16B16A16_UINT:         return PixelFormatRGBA16Uint;
    case Format::R16G16_UINT:               return PixelFormatRG16Uint;
    case Format::R16_UINT:                  return PixelFormatR16Uint;

    case Format::R8G8B8A8_UINT:             return PixelFormatRGBA8Uint;
    case Format::R8G8_UINT:                 return PixelFormatRG8Uint;
    case Format::R8_UINT:                   return PixelFormatR8Uint;

    case Format::R32G32B32A32_SINT:         return PixelFormatRGBA32Sint;
    case Format::R32G32B32_SINT:            return PixelFormatInvalid;
    case Format::R32G32_SINT:               return PixelFormatRG32Sint;
    case Format::R32_SINT:                  return PixelFormatR32Sint;

    case Format::R16G16B16A16_SINT:         return PixelFormatRGBA16Sint;
    case Format::R16G16_SINT:               return PixelFormatRG16Sint;
    case Format::R16_SINT:                  return PixelFormatR16Sint;

    case Format::R8G8B8A8_SINT:             return PixelFormatRGBA8Sint;
    case Format::R8G8_SINT:                 return PixelFormatRG8Sint;
    case Format::R8_SINT:                   return PixelFormatR8Sint;

    case Format::R16G16B16A16_UNORM:        return PixelFormatRGBA16Unorm;
    case Format::R16G16_UNORM:              return PixelFormatRG16Unorm;
    case Format::R16_UNORM:                 return PixelFormatR16Unorm;

    case Format::R8G8B8A8_UNORM:            return PixelFormatRGBA8Unorm;
    case Format::R8G8B8A8_UNORM_SRGB:       return PixelFormatRGBA8Unorm_sRGB;
    case Format::R8G8_UNORM:                return PixelFormatRG8Unorm;
    case Format::R8_UNORM:                  return PixelFormatR8Unorm;
    case Format::B8G8R8A8_UNORM:            return PixelFormatBGRA8Unorm;
    case Format::B8G8R8A8_UNORM_SRGB:       return PixelFormatBGRA8Unorm_sRGB;
    case Format::B8G8R8X8_UNORM:            return PixelFormatInvalid;
    case Format::B8G8R8X8_UNORM_SRGB:       return PixelFormatInvalid;

    case Format::R16G16B16A16_SNORM:        return PixelFormatRGBA16Snorm;
    case Format::R16G16_SNORM:              return PixelFormatRG16Snorm;
    case Format::R16_SNORM:                 return PixelFormatR16Snorm;

    case Format::R8G8B8A8_SNORM:            return PixelFormatRGBA8Snorm;
    case Format::R8G8_SNORM:                return PixelFormatRG8Snorm;
    case Format::R8_SNORM:                  return PixelFormatR8Snorm;

    case Format::D32_FLOAT:                 return PixelFormatDepth32Float;
    case Format::D16_UNORM:                 return PixelFormatDepth16Unorm;
    case Format::D32_FLOAT_S8_UINT:         return PixelFormatDepth32Float_Stencil8;
    case Format::R32_FLOAT_X32_TYPELESS:    return PixelFormatInvalid;

    case Format::B4G4R4A4_UNORM:            return PixelFormatABGR4Unorm;
    case Format::B5G6R5_UNORM:              return PixelFormatB5G6R5Unorm;
    case Format::B5G5R5A1_UNORM:            return PixelFormatA1BGR5Unorm;

    case Format::R9G9B9E5_SHAREDEXP:        return PixelFormatRGB9E5Float;
    case Format::R10G10B10A2_TYPELESS:      return PixelFormatInvalid;
    case Format::R10G10B10A2_UINT:          return PixelFormatRGB10A2Uint;
    case Format::R10G10B10A2_UNORM:         return PixelFormatRGB10A2Unorm;
    case Format::R11G11B10_FLOAT:           return PixelFormatRG11B10Float;

    case Format::BC1_UNORM:                 return PixelFormatBC1_RGBA;
    case Format::BC1_UNORM_SRGB:            return PixelFormatBC1_RGBA_sRGB;
    case Format::BC2_UNORM:                 return PixelFormatBC2_RGBA;
    case Format::BC2_UNORM_SRGB:            return PixelFormatBC2_RGBA_sRGB;
    case Format::BC3_UNORM:                 return PixelFormatBC3_RGBA;
    case Format::BC3_UNORM_SRGB:            return PixelFormatBC3_RGBA_sRGB;
    case Format::BC4_UNORM:                 return PixelFormatBC4_RUnorm;
    case Format::BC4_SNORM:                 return PixelFormatBC4_RSnorm;
    case Format::BC5_UNORM:                 return PixelFormatBC5_RGUnorm;
    case Format::BC5_SNORM:                 return PixelFormatBC5_RGSnorm;
    case Format::BC6H_UF16:                 return PixelFormatBC6H_RGBUfloat;
    case Format::BC6H_SF16:                 return PixelFormatBC6H_RGBFloat;
    case Format::BC7_UNORM:                 return PixelFormatBC7_RGBAUnorm;
    case Format::BC7_UNORM_SRGB:            return PixelFormatBC7_RGBAUnorm_sRGB;

    default:                                return PixelFormatInvalid;
    }
}

MTL::SamplerMinMagFilter MetalUtil::translateSamplerMinMagFilter(TextureFilteringMode mode)
{
    switch (mode)
    {
    case TextureFilteringMode::Point:
        return MTL::SamplerMinMagFilterNearest;
    case TextureFilteringMode::Linear:
        return MTL::SamplerMinMagFilterLinear;
    default:
        return MTL::SamplerMinMagFilter(0);
    }
}

MTL::SamplerMipFilter MetalUtil::translateSamplerMipFilter(TextureFilteringMode mode)
{
    switch (mode)
    {
    case TextureFilteringMode::Point:
        return MTL::SamplerMipFilterNearest;
    case TextureFilteringMode::Linear:
        return MTL::SamplerMipFilterLinear;
    default:
        return MTL::SamplerMipFilter(0);
    }    
}

MTL::SamplerAddressMode MetalUtil::translateSamplerAddressMode(TextureAddressingMode mode)
{
    switch (mode)
    {
    case TextureAddressingMode::Wrap:
        return MTL::SamplerAddressModeRepeat;
    case TextureAddressingMode::ClampToEdge:
        return MTL::SamplerAddressModeClampToEdge;
    case TextureAddressingMode::ClampToBorder:
        return MTL::SamplerAddressModeClampToBorderColor;
    case TextureAddressingMode::MirrorRepeat:
        return MTL::SamplerAddressModeMirrorRepeat;
    case TextureAddressingMode::MirrorOnce:
        return MTL::SamplerAddressModeMirrorClampToEdge;
    default:
        return MTL::SamplerAddressMode(0);
    }
}

MTL::CompareFunction MetalUtil::translateCompareFunction(ComparisonFunc func)
{
    switch (func)
    {
    case ComparisonFunc::Never:
        return MTL::CompareFunctionNever;
    case ComparisonFunc::Less:
        return MTL::CompareFunctionLess;
    case ComparisonFunc::Equal:
        return MTL::CompareFunctionEqual;
    case ComparisonFunc::LessEqual:
        return MTL::CompareFunctionLessEqual;
    case ComparisonFunc::Greater:
        return MTL::CompareFunctionGreater;
    case ComparisonFunc::NotEqual:
        return MTL::CompareFunctionNotEqual;
    case ComparisonFunc::GreaterEqual:
        return MTL::CompareFunctionGreaterEqual;
    case ComparisonFunc::Always:
        return MTL::CompareFunctionAlways;
    default:
        return MTL::CompareFunction(0);
    }
}

MTL::VertexStepFunction MetalUtil::translateVertexStepFunction(InputSlotClass slotClass)
{
    switch (slotClass)
    {
    case InputSlotClass::PerVertex:
        return MTL::VertexStepFunctionPerVertex;
    case InputSlotClass::PerInstance:
        return MTL::VertexStepFunctionPerInstance;
    default:
        return MTL::VertexStepFunctionPerVertex;
    }
}

} // namespace gfx
