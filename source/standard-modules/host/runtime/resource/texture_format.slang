// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime.core.simple_assert;

/// Slang image formats used by HGVM textures and presentation surfaces.
///
/// The values are exactly the `format` generic argument values of Slang texture types. They
/// describe storage independently of the value type observed by shaders.
public namespace TextureFormat
{
public static const int unknown = 0;
public static const int rgba32Float = 1;
public static const int rgba16Float = 2;
public static const int rg32Float = 3;
public static const int rg16Float = 4;
public static const int r11G11B10Float = 5;
public static const int r32Float = 6;
public static const int r16Float = 7;
public static const int rgba16Unorm = 8;
public static const int rgb10A2Unorm = 9;
public static const int rgba8Unorm = 10;
public static const int rg16Unorm = 11;
public static const int rg8Unorm = 12;
public static const int r16Unorm = 13;
public static const int r8Unorm = 14;
public static const int rgba16Snorm = 15;
public static const int rgba8Snorm = 16;
public static const int rg16Snorm = 17;
public static const int rg8Snorm = 18;
public static const int r16Snorm = 19;
public static const int r8Snorm = 20;
public static const int rgba32Sint = 21;
public static const int rgba16Sint = 22;
public static const int rgba8Sint = 23;
public static const int rg32Sint = 24;
public static const int rg16Sint = 25;
public static const int rg8Sint = 26;
public static const int r32Sint = 27;
public static const int r16Sint = 28;
public static const int r8Sint = 29;
public static const int rgba32Uint = 30;
public static const int rgba16Uint = 31;
public static const int rgb10A2Uint = 32;
public static const int rgba8Uint = 33;
public static const int rg32Uint = 34;
public static const int rg16Uint = 35;
public static const int rg8Uint = 36;
public static const int r32Uint = 37;
public static const int r16Uint = 38;
public static const int r8Uint = 39;
public static const int r64Uint = 40;
public static const int r64Sint = 41;
public static const int bgra8Unorm = 42;
public static const int rgba8UnormSrgb = 43;
public static const int d32Float = 44;
public static const int d16Unorm = 45;
public static const int d32FloatS8Uint = 46;
}

/// Returns physical storage bytes per texel for an explicit uncompressed format, not sizeof(T).
/// Unknown formats cannot size raw uploads and are rejected. Device support is checked by
/// allocation.
public uint getTextureFormatBytesPerTexel(int format)
{
    switch (format)
    {
    case TextureFormat::rgba32Float:
    case TextureFormat::rgba32Sint:
    case TextureFormat::rgba32Uint:
        return 16;
    case TextureFormat::rgba16Float:
    case TextureFormat::rg32Float:
    case TextureFormat::rgba16Unorm:
    case TextureFormat::rgba16Snorm:
    case TextureFormat::rgba16Sint:
    case TextureFormat::rg32Sint:
    case TextureFormat::rgba16Uint:
    case TextureFormat::rg32Uint:
    case TextureFormat::r64Uint:
    case TextureFormat::r64Sint:
    case TextureFormat::d32FloatS8Uint:
        return 8;
    case TextureFormat::rg16Float:
    case TextureFormat::r11G11B10Float:
    case TextureFormat::r32Float:
    case TextureFormat::rgb10A2Unorm:
    case TextureFormat::rgba8Unorm:
    case TextureFormat::rg16Unorm:
    case TextureFormat::rgba8Snorm:
    case TextureFormat::rg16Snorm:
    case TextureFormat::rgba8Sint:
    case TextureFormat::rg16Sint:
    case TextureFormat::r32Sint:
    case TextureFormat::rgb10A2Uint:
    case TextureFormat::rgba8Uint:
    case TextureFormat::rg16Uint:
    case TextureFormat::r32Uint:
    case TextureFormat::bgra8Unorm:
    case TextureFormat::rgba8UnormSrgb:
    case TextureFormat::d32Float:
        return 4;
    case TextureFormat::r16Float:
    case TextureFormat::rg8Unorm:
    case TextureFormat::r16Unorm:
    case TextureFormat::rg8Snorm:
    case TextureFormat::r16Snorm:
    case TextureFormat::rg8Sint:
    case TextureFormat::r16Sint:
    case TextureFormat::rg8Uint:
    case TextureFormat::r16Uint:
    case TextureFormat::d16Unorm:
        return 2;
    case TextureFormat::r8Unorm:
    case TextureFormat::r8Snorm:
    case TextureFormat::r8Sint:
    case TextureFormat::r8Uint:
        return 1;
    default:
        break;
    }
    fail();
    return 0;
}
