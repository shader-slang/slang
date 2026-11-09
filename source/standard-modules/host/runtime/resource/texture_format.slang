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
    // HGBC does not yet lower host switch statements.
    if (format == TextureFormat::rgba32Float || format == TextureFormat::rgba32Sint ||
        format == TextureFormat::rgba32Uint)
        return 16;
    if (format == TextureFormat::rgba16Float || format == TextureFormat::rg32Float ||
        format == TextureFormat::rgba16Unorm || format == TextureFormat::rgba16Snorm ||
        format == TextureFormat::rgba16Sint || format == TextureFormat::rg32Sint ||
        format == TextureFormat::rgba16Uint || format == TextureFormat::rg32Uint ||
        format == TextureFormat::r64Uint || format == TextureFormat::r64Sint ||
        format == TextureFormat::d32FloatS8Uint)
        return 8;
    if (format == TextureFormat::rg16Float || format == TextureFormat::r11G11B10Float ||
        format == TextureFormat::r32Float || format == TextureFormat::rgb10A2Unorm ||
        format == TextureFormat::rgba8Unorm || format == TextureFormat::rg16Unorm ||
        format == TextureFormat::rgba8Snorm || format == TextureFormat::rg16Snorm ||
        format == TextureFormat::rgba8Sint || format == TextureFormat::rg16Sint ||
        format == TextureFormat::r32Sint || format == TextureFormat::rgb10A2Uint ||
        format == TextureFormat::rgba8Uint || format == TextureFormat::rg16Uint ||
        format == TextureFormat::r32Uint || format == TextureFormat::bgra8Unorm ||
        format == TextureFormat::rgba8UnormSrgb || format == TextureFormat::d32Float)
        return 4;
    if (format == TextureFormat::r16Float || format == TextureFormat::rg8Unorm ||
        format == TextureFormat::r16Unorm || format == TextureFormat::rg8Snorm ||
        format == TextureFormat::r16Snorm || format == TextureFormat::rg8Sint ||
        format == TextureFormat::r16Sint || format == TextureFormat::rg8Uint ||
        format == TextureFormat::r16Uint || format == TextureFormat::d16Unorm)
        return 2;
    if (format == TextureFormat::r8Unorm || format == TextureFormat::r8Snorm ||
        format == TextureFormat::r8Sint || format == TextureFormat::r8Uint)
        return 1;
    fail();
    return 0;
}
