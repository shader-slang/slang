// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Run with extras/validate-nvvm-surfaces.py. Formats are inferred from native element types.
// Observe original values through explicit32-bit extension, then store independent native bits.
// These stores do not exercise the logical32-to-narrow saturation contract.
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

typealias Signed8R = int8_t;
Surface<Signed8R> surfaceSigned8R;
Surface<int> observedSigned8R;
typealias Signed8RG = vector<int8_t, 2>;
Surface<Signed8RG> surfaceSigned8RG;
Surface<int2> observedSigned8RG;
typealias Signed8RGBA = vector<int8_t, 4>;
Surface<Signed8RGBA> surfaceSigned8RGBA;
Surface<int4> observedSigned8RGBA;
typealias Unsigned8R = uint8_t;
Surface<Unsigned8R> surfaceUnsigned8R;
Surface<uint> observedUnsigned8R;
typealias Unsigned8RG = vector<uint8_t, 2>;
Surface<Unsigned8RG> surfaceUnsigned8RG;
Surface<uint2> observedUnsigned8RG;
typealias Unsigned8RGBA = vector<uint8_t, 4>;
Surface<Unsigned8RGBA> surfaceUnsigned8RGBA;
Surface<uint4> observedUnsigned8RGBA;
typealias Signed16R = int16_t;
Surface<Signed16R> surfaceSigned16R;
Surface<int> observedSigned16R;
typealias Signed16RG = vector<int16_t, 2>;
Surface<Signed16RG> surfaceSigned16RG;
Surface<int2> observedSigned16RG;
typealias Signed16RGBA = vector<int16_t, 4>;
Surface<Signed16RGBA> surfaceSigned16RGBA;
Surface<int4> observedSigned16RGBA;
typealias Unsigned16R = uint16_t;
Surface<Unsigned16R> surfaceUnsigned16R;
Surface<uint> observedUnsigned16R;
typealias Unsigned16RG = vector<uint16_t, 2>;
Surface<Unsigned16RG> surfaceUnsigned16RG;
Surface<uint2> observedUnsigned16RG;
typealias Unsigned16RGBA = vector<uint16_t, 4>;
Surface<Unsigned16RGBA> surfaceUnsigned16RGBA;
Surface<uint4> observedUnsigned16RGBA;

// Every result fits the corresponding unsigned physical width. The signed helpers only
// reinterpret those same-width bits; no out-of-range texture conversion is involved.
uint markerBits(uint index, uint signBit, uint mask)
{
    switch (index % 16)
    {
    case 0: return 0;
    case 1: return 1;
    case 2: return signBit - 1;
    case 3: return signBit;
    case 4: return signBit + 1;
    case 5: return mask;
    case 6: return mask - 1;
    case 7: return 2;
    case 8: return 0x55;
    case 9: return 0xaa;
    case 10: return mask / 3;
    case 11: return (mask / 3) * 2;
    case 12: return 3;
    case 13: return mask - 2;
    case 14: return 7;
    default: return mask - 7;
    }
}

int8_t signed8Marker(uint index)
{
    return bit_cast<int8_t>(uint8_t(markerBits(index, 128, 255)));
}

uint8_t unsigned8Marker(uint index)
{
    return uint8_t(markerBits(index, 128, 255));
}

int16_t signed16Marker(uint index)
{
    return bit_cast<int16_t>(uint16_t(markerBits(index, 32768, 65535)));
}

uint16_t unsigned16Marker(uint index)
{
    return uint16_t(markerBits(index, 32768, 65535));
}

[numthreads(1, 1, 1)]
void exercise(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x < 1 || tid.x > 64)
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
    // Every marker reaches every live component selector, including both signedness regions.
    uint index = (tid.x - 1) / 4 + tid.y * 7 + tid.z * 5;
#else
    uint index = tid.x - 1 + tid.y * 7 + tid.z * 5;
#endif

    observedSigned8R[COORD] = int(surfaceSigned8R[COORD]);
    Signed8R valueSigned8R = signed8Marker(index + 0);
    surfaceSigned8R[COORD] = valueSigned8R;

    observedSigned8RG[COORD] = int2(surfaceSigned8RG[COORD]);
    Signed8RG valueSigned8RG = Signed8RG(signed8Marker(index + 3), signed8Marker(index + 8));
#if SURFACE_OPERATION == 0
    surfaceSigned8RG[COORD] = valueSigned8RG;
#elif SURFACE_OPERATION == 1
    surfaceSigned8RG[COORD].y = valueSigned8RG.y;
#else
    surfaceSigned8RG[COORD][(tid.x - 1) % 2] = valueSigned8RG[(tid.x - 1) % 2];
#endif

    observedSigned8RGBA[COORD] = int4(surfaceSigned8RGBA[COORD]);
    Signed8RGBA valueSigned8RGBA = Signed8RGBA(signed8Marker(index + 6), signed8Marker(index + 11), signed8Marker(index + 16), signed8Marker(index + 21));
#if SURFACE_OPERATION == 0
    surfaceSigned8RGBA[COORD] = valueSigned8RGBA;
#elif SURFACE_OPERATION == 1
    surfaceSigned8RGBA[COORD].yw = valueSigned8RGBA.yw;
#else
    surfaceSigned8RGBA[COORD][(tid.x - 1) % 4] = valueSigned8RGBA[(tid.x - 1) % 4];
#endif

    observedUnsigned8R[COORD] = uint(surfaceUnsigned8R[COORD]);
    Unsigned8R valueUnsigned8R = unsigned8Marker(index + 9);
    surfaceUnsigned8R[COORD] = valueUnsigned8R;

    observedUnsigned8RG[COORD] = uint2(surfaceUnsigned8RG[COORD]);
    Unsigned8RG valueUnsigned8RG = Unsigned8RG(unsigned8Marker(index + 12), unsigned8Marker(index + 17));
#if SURFACE_OPERATION == 0
    surfaceUnsigned8RG[COORD] = valueUnsigned8RG;
#elif SURFACE_OPERATION == 1
    surfaceUnsigned8RG[COORD].y = valueUnsigned8RG.y;
#else
    surfaceUnsigned8RG[COORD][(tid.x - 1) % 2] = valueUnsigned8RG[(tid.x - 1) % 2];
#endif

    observedUnsigned8RGBA[COORD] = uint4(surfaceUnsigned8RGBA[COORD]);
    Unsigned8RGBA valueUnsigned8RGBA = Unsigned8RGBA(unsigned8Marker(index + 15), unsigned8Marker(index + 20), unsigned8Marker(index + 25), unsigned8Marker(index + 30));
#if SURFACE_OPERATION == 0
    surfaceUnsigned8RGBA[COORD] = valueUnsigned8RGBA;
#elif SURFACE_OPERATION == 1
    surfaceUnsigned8RGBA[COORD].yw = valueUnsigned8RGBA.yw;
#else
    surfaceUnsigned8RGBA[COORD][(tid.x - 1) % 4] = valueUnsigned8RGBA[(tid.x - 1) % 4];
#endif

    observedSigned16R[COORD] = int(surfaceSigned16R[COORD]);
    Signed16R valueSigned16R = signed16Marker(index + 18);
    surfaceSigned16R[COORD] = valueSigned16R;

    observedSigned16RG[COORD] = int2(surfaceSigned16RG[COORD]);
    Signed16RG valueSigned16RG = Signed16RG(signed16Marker(index + 21), signed16Marker(index + 26));
#if SURFACE_OPERATION == 0
    surfaceSigned16RG[COORD] = valueSigned16RG;
#elif SURFACE_OPERATION == 1
    surfaceSigned16RG[COORD].y = valueSigned16RG.y;
#else
    surfaceSigned16RG[COORD][(tid.x - 1) % 2] = valueSigned16RG[(tid.x - 1) % 2];
#endif

    observedSigned16RGBA[COORD] = int4(surfaceSigned16RGBA[COORD]);
    Signed16RGBA valueSigned16RGBA = Signed16RGBA(signed16Marker(index + 24), signed16Marker(index + 29), signed16Marker(index + 34), signed16Marker(index + 39));
#if SURFACE_OPERATION == 0
    surfaceSigned16RGBA[COORD] = valueSigned16RGBA;
#elif SURFACE_OPERATION == 1
    surfaceSigned16RGBA[COORD].yw = valueSigned16RGBA.yw;
#else
    surfaceSigned16RGBA[COORD][(tid.x - 1) % 4] = valueSigned16RGBA[(tid.x - 1) % 4];
#endif

    observedUnsigned16R[COORD] = uint(surfaceUnsigned16R[COORD]);
    Unsigned16R valueUnsigned16R = unsigned16Marker(index + 27);
    surfaceUnsigned16R[COORD] = valueUnsigned16R;

    observedUnsigned16RG[COORD] = uint2(surfaceUnsigned16RG[COORD]);
    Unsigned16RG valueUnsigned16RG = Unsigned16RG(unsigned16Marker(index + 30), unsigned16Marker(index + 35));
#if SURFACE_OPERATION == 0
    surfaceUnsigned16RG[COORD] = valueUnsigned16RG;
#elif SURFACE_OPERATION == 1
    surfaceUnsigned16RG[COORD].y = valueUnsigned16RG.y;
#else
    surfaceUnsigned16RG[COORD][(tid.x - 1) % 2] = valueUnsigned16RG[(tid.x - 1) % 2];
#endif

    observedUnsigned16RGBA[COORD] = uint4(surfaceUnsigned16RGBA[COORD]);
    Unsigned16RGBA valueUnsigned16RGBA = Unsigned16RGBA(unsigned16Marker(index + 33), unsigned16Marker(index + 38), unsigned16Marker(index + 43), unsigned16Marker(index + 48));
#if SURFACE_OPERATION == 0
    surfaceUnsigned16RGBA[COORD] = valueUnsigned16RGBA;
#elif SURFACE_OPERATION == 1
    surfaceUnsigned16RGBA[COORD].yw = valueUnsigned16RGBA.yw;
#else
    surfaceUnsigned16RGBA[COORD][(tid.x - 1) % 4] = valueUnsigned16RGBA[(tid.x - 1) % 4];
#endif
}
