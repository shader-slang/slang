// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Run with extras/validate-nvvm-surfaces.py. Observe independently initialized normalized bytes
// before writing boundary inputs; unselected channels must preserve their exact physical bits.
#ifndef SURFACE_DIM
#define SURFACE_DIM 1
#endif
#ifndef SURFACE_ARRAY
#define SURFACE_ARRAY 0
#endif
#ifndef SURFACE_OPERATION
#define SURFACE_OPERATION 0
#endif
#if SURFACE_DIM == 1
#if SURFACE_ARRAY
#define Surface RWTexture1DArray
#define COORD int2(tid.x, tid.z)
#else
#define Surface RWTexture1D
#define COORD int(tid.x)
#endif
#elif SURFACE_DIM == 2
#if SURFACE_ARRAY
#define Surface RWTexture2DArray
#define COORD int3(tid.xyz)
#else
#define Surface RWTexture2D
#define COORD int2(tid.xy)
#endif
#else
#define Surface RWTexture3D
#define COORD int3(tid.xyz)
#endif

#ifndef SURFACE_LOGICAL_HALF
#define SURFACE_LOGICAL_HALF 0
#endif
#if SURFACE_LOGICAL_HALF
#define Scalar half
#else
#define Scalar float
#endif
typealias Signed8R = Scalar;
[format("r8_snorm")] Surface<Signed8R> surfaceSigned8R;
Surface<float> observedSigned8R;
typealias Signed8RG = vector<Scalar, 2>;
[format("rg8_snorm")] Surface<Signed8RG> surfaceSigned8RG;
Surface<float2> observedSigned8RG;
typealias Signed8RGBA = vector<Scalar, 4>;
[format("rgba8_snorm")] Surface<Signed8RGBA> surfaceSigned8RGBA;
Surface<float4> observedSigned8RGBA;
typealias Unsigned8R = Scalar;
[format("r8")] Surface<Unsigned8R> surfaceUnsigned8R;
Surface<float> observedUnsigned8R;
typealias Unsigned8RG = vector<Scalar, 2>;
[format("rg8")] Surface<Unsigned8RG> surfaceUnsigned8RG;
Surface<float2> observedUnsigned8RG;
typealias Unsigned8RGBA = vector<Scalar, 4>;
[format("rgba8")] Surface<Unsigned8RGBA> surfaceUnsigned8RGBA;
Surface<float4> observedUnsigned8RGBA;
typealias Signed16R = Scalar;
[format("r16_snorm")] Surface<Signed16R> surfaceSigned16R;
Surface<float> observedSigned16R;
typealias Signed16RG = vector<Scalar, 2>;
[format("rg16_snorm")] Surface<Signed16RG> surfaceSigned16RG;
Surface<float2> observedSigned16RG;
typealias Signed16RGBA = vector<Scalar, 4>;
[format("rgba16_snorm")] Surface<Signed16RGBA> surfaceSigned16RGBA;
Surface<float4> observedSigned16RGBA;
typealias Unsigned16R = Scalar;
[format("r16")] Surface<Unsigned16R> surfaceUnsigned16R;
Surface<float> observedUnsigned16R;
typealias Unsigned16RG = vector<Scalar, 2>;
[format("rg16")] Surface<Unsigned16RG> surfaceUnsigned16RG;
Surface<float2> observedUnsigned16RG;
typealias Unsigned16RGBA = vector<Scalar, 4>;
[format("rgba16")] Surface<Unsigned16RGBA> surfaceUnsigned16RGBA;
Surface<float4> observedUnsigned16RGBA;

float inputValue(uint index)
{
    switch (index % 64)
    {
    case 0: return asfloat(0x00000000u);
    case 1: return asfloat(0x80000000u);
    case 2: return asfloat(0x3f800000u);
    case 3: return asfloat(0xbf800000u);
    case 4: return asfloat(0x40000000u);
    case 5: return asfloat(0xc0000000u);
    case 6: return asfloat(0x7f800000u);
    case 7: return asfloat(0xff800000u);
    case 8: return asfloat(0x7fc12345u);
    case 9: return asfloat(0xffc12345u);
    case 10: return asfloat(0x00000001u);
    case 11: return asfloat(0x80000001u);
    case 12: return asfloat(0x3f000000u);
    case 13: return asfloat(0xbf000000u);
    case 14: return asfloat(0x3eaaaaabu);
    case 15: return asfloat(0xbeaaaaabu);
    case 16: return asfloat(0x3b810203u);
    case 17: return asfloat(0x3b810204u);
    case 18: return asfloat(0x3b810205u);
    case 19: return asfloat(0xbb810203u);
    case 20: return asfloat(0xbb810204u);
    case 21: return asfloat(0xbb810205u);
    case 22: return asfloat(0x3effffffu);
    case 23: return asfloat(0x3f000000u);
    case 24: return asfloat(0x3f000001u);
    case 25: return asfloat(0xbeffffffu);
    case 26: return asfloat(0xbf000000u);
    case 27: return asfloat(0xbf000001u);
    case 28: return asfloat(0x3b008080u);
    case 29: return asfloat(0x3b008081u);
    case 30: return asfloat(0x3b008082u);
    case 31: return asfloat(0xbb008080u);
    case 32: return asfloat(0xbb008081u);
    case 33: return asfloat(0xbb008082u);
    case 34: return asfloat(0x3effffffu);
    case 35: return asfloat(0x3f000000u);
    case 36: return asfloat(0x3f000001u);
    case 37: return asfloat(0xbeffffffu);
    case 38: return asfloat(0xbf000000u);
    case 39: return asfloat(0xbf000001u);
    case 40: return asfloat(0x378000ffu);
    case 41: return asfloat(0x37800100u);
    case 42: return asfloat(0x37800101u);
    case 43: return asfloat(0xb78000ffu);
    case 44: return asfloat(0xb7800100u);
    case 45: return asfloat(0xb7800101u);
    case 46: return asfloat(0x3effffffu);
    case 47: return asfloat(0x3f000000u);
    case 48: return asfloat(0x3f000001u);
    case 49: return asfloat(0xbeffffffu);
    case 50: return asfloat(0xbf000000u);
    case 51: return asfloat(0xbf000001u);
    case 52: return asfloat(0x3700007fu);
    case 53: return asfloat(0x37000080u);
    case 54: return asfloat(0x37000081u);
    case 55: return asfloat(0xb700007fu);
    case 56: return asfloat(0xb7000080u);
    case 57: return asfloat(0xb7000081u);
    case 58: return asfloat(0x3effffffu);
    case 59: return asfloat(0x3f000000u);
    case 60: return asfloat(0x3f000001u);
    case 61: return asfloat(0xbeffffffu);
    case 62: return asfloat(0xbf000000u);
    case 63: return asfloat(0xbf000001u);
    default: return 0;
    }
}
[numthreads(1, 1, 1)]
void exercise(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x < 1 || tid.x > 256)
        return;
#if SURFACE_ARRAY || SURFACE_DIM == 3
    if (tid.z < 1 || tid.z > 2)
        return;
#endif
#if SURFACE_DIM >= 2
    if (tid.y < 1 || tid.y > 3)
        return;
#endif
#if SURFACE_OPERATION == 2
    // Every boundary input reaches every live component selector, including both signedness regions.
    uint index = (tid.x - 1) / 4 + tid.y * 7 + tid.z * 5;
#else
    uint index = tid.x - 1 + tid.y * 7 + tid.z * 5;
#endif

    observedSigned8R[COORD] = float(surfaceSigned8R[COORD]);
    Signed8R valueSigned8R = Scalar(inputValue(index + 0));
    surfaceSigned8R[COORD] = valueSigned8R;

    observedSigned8RG[COORD] = float2(surfaceSigned8RG[COORD]);
    Signed8RG valueSigned8RG = Signed8RG(Scalar(inputValue(index + 3)), Scalar(inputValue(index + 8)));
#if SURFACE_OPERATION == 0
    surfaceSigned8RG[COORD] = valueSigned8RG;
#elif SURFACE_OPERATION == 1
    surfaceSigned8RG[COORD].y = valueSigned8RG.y;
#else
    surfaceSigned8RG[COORD][(tid.x - 1) % 2] = valueSigned8RG[(tid.x - 1) % 2];
#endif

    observedSigned8RGBA[COORD] = float4(surfaceSigned8RGBA[COORD]);
    Signed8RGBA valueSigned8RGBA = Signed8RGBA(Scalar(inputValue(index + 6)), Scalar(inputValue(index + 11)), Scalar(inputValue(index + 16)), Scalar(inputValue(index + 21)));
#if SURFACE_OPERATION == 0
    surfaceSigned8RGBA[COORD] = valueSigned8RGBA;
#elif SURFACE_OPERATION == 1
    surfaceSigned8RGBA[COORD].yw = valueSigned8RGBA.yw;
#else
    surfaceSigned8RGBA[COORD][(tid.x - 1) % 4] = valueSigned8RGBA[(tid.x - 1) % 4];
#endif

    observedUnsigned8R[COORD] = float(surfaceUnsigned8R[COORD]);
    Unsigned8R valueUnsigned8R = Scalar(inputValue(index + 9));
    surfaceUnsigned8R[COORD] = valueUnsigned8R;

    observedUnsigned8RG[COORD] = float2(surfaceUnsigned8RG[COORD]);
    Unsigned8RG valueUnsigned8RG = Unsigned8RG(Scalar(inputValue(index + 12)), Scalar(inputValue(index + 17)));
#if SURFACE_OPERATION == 0
    surfaceUnsigned8RG[COORD] = valueUnsigned8RG;
#elif SURFACE_OPERATION == 1
    surfaceUnsigned8RG[COORD].y = valueUnsigned8RG.y;
#else
    surfaceUnsigned8RG[COORD][(tid.x - 1) % 2] = valueUnsigned8RG[(tid.x - 1) % 2];
#endif

    observedUnsigned8RGBA[COORD] = float4(surfaceUnsigned8RGBA[COORD]);
    Unsigned8RGBA valueUnsigned8RGBA = Unsigned8RGBA(Scalar(inputValue(index + 15)), Scalar(inputValue(index + 20)), Scalar(inputValue(index + 25)), Scalar(inputValue(index + 30)));
#if SURFACE_OPERATION == 0
    surfaceUnsigned8RGBA[COORD] = valueUnsigned8RGBA;
#elif SURFACE_OPERATION == 1
    surfaceUnsigned8RGBA[COORD].yw = valueUnsigned8RGBA.yw;
#else
    surfaceUnsigned8RGBA[COORD][(tid.x - 1) % 4] = valueUnsigned8RGBA[(tid.x - 1) % 4];
#endif

    observedSigned16R[COORD] = float(surfaceSigned16R[COORD]);
    Signed16R valueSigned16R = Scalar(inputValue(index + 18));
    surfaceSigned16R[COORD] = valueSigned16R;

    observedSigned16RG[COORD] = float2(surfaceSigned16RG[COORD]);
    Signed16RG valueSigned16RG = Signed16RG(Scalar(inputValue(index + 21)), Scalar(inputValue(index + 26)));
#if SURFACE_OPERATION == 0
    surfaceSigned16RG[COORD] = valueSigned16RG;
#elif SURFACE_OPERATION == 1
    surfaceSigned16RG[COORD].y = valueSigned16RG.y;
#else
    surfaceSigned16RG[COORD][(tid.x - 1) % 2] = valueSigned16RG[(tid.x - 1) % 2];
#endif

    observedSigned16RGBA[COORD] = float4(surfaceSigned16RGBA[COORD]);
    Signed16RGBA valueSigned16RGBA = Signed16RGBA(Scalar(inputValue(index + 24)), Scalar(inputValue(index + 29)), Scalar(inputValue(index + 34)), Scalar(inputValue(index + 39)));
#if SURFACE_OPERATION == 0
    surfaceSigned16RGBA[COORD] = valueSigned16RGBA;
#elif SURFACE_OPERATION == 1
    surfaceSigned16RGBA[COORD].yw = valueSigned16RGBA.yw;
#else
    surfaceSigned16RGBA[COORD][(tid.x - 1) % 4] = valueSigned16RGBA[(tid.x - 1) % 4];
#endif

    observedUnsigned16R[COORD] = float(surfaceUnsigned16R[COORD]);
    Unsigned16R valueUnsigned16R = Scalar(inputValue(index + 27));
    surfaceUnsigned16R[COORD] = valueUnsigned16R;

    observedUnsigned16RG[COORD] = float2(surfaceUnsigned16RG[COORD]);
    Unsigned16RG valueUnsigned16RG = Unsigned16RG(Scalar(inputValue(index + 30)), Scalar(inputValue(index + 35)));
#if SURFACE_OPERATION == 0
    surfaceUnsigned16RG[COORD] = valueUnsigned16RG;
#elif SURFACE_OPERATION == 1
    surfaceUnsigned16RG[COORD].y = valueUnsigned16RG.y;
#else
    surfaceUnsigned16RG[COORD][(tid.x - 1) % 2] = valueUnsigned16RG[(tid.x - 1) % 2];
#endif

    observedUnsigned16RGBA[COORD] = float4(surfaceUnsigned16RGBA[COORD]);
    Unsigned16RGBA valueUnsigned16RGBA = Unsigned16RGBA(Scalar(inputValue(index + 33)), Scalar(inputValue(index + 38)), Scalar(inputValue(index + 43)), Scalar(inputValue(index + 48)));
#if SURFACE_OPERATION == 0
    surfaceUnsigned16RGBA[COORD] = valueUnsigned16RGBA;
#elif SURFACE_OPERATION == 1
    surfaceUnsigned16RGBA[COORD].yw = valueUnsigned16RGBA.yw;
#else
    surfaceUnsigned16RGBA[COORD][(tid.x - 1) % 4] = valueUnsigned16RGBA[(tid.x - 1) % 4];
#endif
}
