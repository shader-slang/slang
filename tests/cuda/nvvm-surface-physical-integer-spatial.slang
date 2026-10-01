// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Run with extras/validate-nvvm-surfaces.py. Read the original physical input into
// independent native32 arrays before storing independently generated saturation edges.
// Layer/Z participates in the independent store phase, exposing paired address permutations.
#ifndef SURFACE_DIM
#define SURFACE_DIM 1
#endif
#ifndef SURFACE_OPERATION
#define SURFACE_OPERATION 0
#endif
#if SURFACE_DIM == 1
#define Surface RWTexture1DArray
#define COORD int2(tid.x, tid.z)
#elif SURFACE_DIM == 2
#define Surface RWTexture2DArray
#define COORD int3(tid.xyz)
#else
#define Surface RWTexture3D
#define COORD int3(tid.xyz)
#endif

[format("r8i")] Surface<int> surfaceSigned8R;
Surface<int> observedSigned8R;
[format("rg8i")] Surface<int2> surfaceSigned8RG;
Surface<int2> observedSigned8RG;
[format("rgba8i")] Surface<int4> surfaceSigned8RGBA;
Surface<int4> observedSigned8RGBA;
[format("r8ui")] Surface<uint> surfaceUnsigned8R;
Surface<uint> observedUnsigned8R;
[format("rg8ui")] Surface<uint2> surfaceUnsigned8RG;
Surface<uint2> observedUnsigned8RG;
[format("rgba8ui")] Surface<uint4> surfaceUnsigned8RGBA;
Surface<uint4> observedUnsigned8RGBA;
[format("r16i")] Surface<int> surfaceSigned16R;
Surface<int> observedSigned16R;
[format("rg16i")] Surface<int2> surfaceSigned16RG;
Surface<int2> observedSigned16RG;
[format("rgba16i")] Surface<int4> surfaceSigned16RGBA;
Surface<int4> observedSigned16RGBA;
[format("r16ui")] Surface<uint> surfaceUnsigned16R;
Surface<uint> observedUnsigned16R;
[format("rg16ui")] Surface<uint2> surfaceUnsigned16RG;
Surface<uint2> observedUnsigned16RG;
[format("rgba16ui")] Surface<uint4> surfaceUnsigned16RGBA;
Surface<uint4> observedUnsigned16RGBA;

int signedEdge(uint index, int low, int high)
{
    switch (index % 16)
    {
    case 0: return asint(0x80000000u);
    case 1: return low - 1;
    case 2: return low;
    case 3: return low + 1;
    case 4: return -2;
    case 5: return -1;
    case 6: return 0;
    case 7: return 1;
    case 8: return 2;
    case 9: return high - 1;
    case 10: return high;
    case 11: return high + 1;
    case 12: return 255;
    case 13: return 256;
    case 14: return 65536;
    default: return 0x7fffffff;
    }
}

uint unsignedEdge(uint index, uint high)
{
    switch (index % 16)
    {
    case 0: return 0;
    case 1: return 1;
    case 2: return 2;
    case 3: return 127;
    case 4: return 128;
    case 5: return 255;
    case 6: return 256;
    case 7: return 32767;
    case 8: return 32768;
    case 9: return 65535;
    case 10: return 65536;
    case 11: return high - 1;
    case 12: return high;
    case 13: return high + 1;
    case 14: return 0x80000000u;
    default: return 0xffffffffu;
    }
}

[numthreads(1, 1, 1)]
void exercise(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x < 1 || tid.x > 64 || tid.z < 1 || tid.z > 2)
        return;
#if SURFACE_DIM >= 2
    if (tid.y < 1 || tid.y > 3)
        return;
#endif
#if SURFACE_OPERATION == 2
    // Every edge value reaches every live component selector over 64 texels.
    uint index = (tid.x - 1) / 4 + tid.y * 7 + tid.z * 5;
#else
    uint index = tid.x - 1 + tid.y * 7 + tid.z * 5;
#endif

    observedSigned8R[COORD] = surfaceSigned8R[COORD];
    int valueSigned8R = signedEdge(index + 0, -128, 127);
    surfaceSigned8R[COORD] = valueSigned8R;

    observedSigned8RG[COORD] = surfaceSigned8RG[COORD];
    int2 valueSigned8RG = int2(signedEdge(index + 3, -128, 127), signedEdge(index + 8, -128, 127));
#if SURFACE_OPERATION == 0
    surfaceSigned8RG[COORD] = valueSigned8RG;
#elif SURFACE_OPERATION == 1
    surfaceSigned8RG[COORD].y = valueSigned8RG.y;
#else
    surfaceSigned8RG[COORD][(tid.x - 1) % 2] = valueSigned8RG[(tid.x - 1) % 2];
#endif

    observedSigned8RGBA[COORD] = surfaceSigned8RGBA[COORD];
    int4 valueSigned8RGBA = int4(signedEdge(index + 6, -128, 127), signedEdge(index + 11, -128, 127), signedEdge(index + 16, -128, 127), signedEdge(index + 21, -128, 127));
#if SURFACE_OPERATION == 0
    surfaceSigned8RGBA[COORD] = valueSigned8RGBA;
#elif SURFACE_OPERATION == 1
    surfaceSigned8RGBA[COORD].yw = valueSigned8RGBA.yw;
#else
    surfaceSigned8RGBA[COORD][(tid.x - 1) % 4] = valueSigned8RGBA[(tid.x - 1) % 4];
#endif

    observedUnsigned8R[COORD] = surfaceUnsigned8R[COORD];
    uint valueUnsigned8R = unsignedEdge(index + 9, 255u);
    surfaceUnsigned8R[COORD] = valueUnsigned8R;

    observedUnsigned8RG[COORD] = surfaceUnsigned8RG[COORD];
    uint2 valueUnsigned8RG = uint2(unsignedEdge(index + 12, 255u), unsignedEdge(index + 17, 255u));
#if SURFACE_OPERATION == 0
    surfaceUnsigned8RG[COORD] = valueUnsigned8RG;
#elif SURFACE_OPERATION == 1
    surfaceUnsigned8RG[COORD].y = valueUnsigned8RG.y;
#else
    surfaceUnsigned8RG[COORD][(tid.x - 1) % 2] = valueUnsigned8RG[(tid.x - 1) % 2];
#endif

    observedUnsigned8RGBA[COORD] = surfaceUnsigned8RGBA[COORD];
    uint4 valueUnsigned8RGBA = uint4(unsignedEdge(index + 15, 255u), unsignedEdge(index + 20, 255u), unsignedEdge(index + 25, 255u), unsignedEdge(index + 30, 255u));
#if SURFACE_OPERATION == 0
    surfaceUnsigned8RGBA[COORD] = valueUnsigned8RGBA;
#elif SURFACE_OPERATION == 1
    surfaceUnsigned8RGBA[COORD].yw = valueUnsigned8RGBA.yw;
#else
    surfaceUnsigned8RGBA[COORD][(tid.x - 1) % 4] = valueUnsigned8RGBA[(tid.x - 1) % 4];
#endif

    observedSigned16R[COORD] = surfaceSigned16R[COORD];
    int valueSigned16R = signedEdge(index + 18, -32768, 32767);
    surfaceSigned16R[COORD] = valueSigned16R;

    observedSigned16RG[COORD] = surfaceSigned16RG[COORD];
    int2 valueSigned16RG = int2(signedEdge(index + 21, -32768, 32767), signedEdge(index + 26, -32768, 32767));
#if SURFACE_OPERATION == 0
    surfaceSigned16RG[COORD] = valueSigned16RG;
#elif SURFACE_OPERATION == 1
    surfaceSigned16RG[COORD].y = valueSigned16RG.y;
#else
    surfaceSigned16RG[COORD][(tid.x - 1) % 2] = valueSigned16RG[(tid.x - 1) % 2];
#endif

    observedSigned16RGBA[COORD] = surfaceSigned16RGBA[COORD];
    int4 valueSigned16RGBA = int4(signedEdge(index + 24, -32768, 32767), signedEdge(index + 29, -32768, 32767), signedEdge(index + 34, -32768, 32767), signedEdge(index + 39, -32768, 32767));
#if SURFACE_OPERATION == 0
    surfaceSigned16RGBA[COORD] = valueSigned16RGBA;
#elif SURFACE_OPERATION == 1
    surfaceSigned16RGBA[COORD].yw = valueSigned16RGBA.yw;
#else
    surfaceSigned16RGBA[COORD][(tid.x - 1) % 4] = valueSigned16RGBA[(tid.x - 1) % 4];
#endif

    observedUnsigned16R[COORD] = surfaceUnsigned16R[COORD];
    uint valueUnsigned16R = unsignedEdge(index + 27, 65535u);
    surfaceUnsigned16R[COORD] = valueUnsigned16R;

    observedUnsigned16RG[COORD] = surfaceUnsigned16RG[COORD];
    uint2 valueUnsigned16RG = uint2(unsignedEdge(index + 30, 65535u), unsignedEdge(index + 35, 65535u));
#if SURFACE_OPERATION == 0
    surfaceUnsigned16RG[COORD] = valueUnsigned16RG;
#elif SURFACE_OPERATION == 1
    surfaceUnsigned16RG[COORD].y = valueUnsigned16RG.y;
#else
    surfaceUnsigned16RG[COORD][(tid.x - 1) % 2] = valueUnsigned16RG[(tid.x - 1) % 2];
#endif

    observedUnsigned16RGBA[COORD] = surfaceUnsigned16RGBA[COORD];
    uint4 valueUnsigned16RGBA = uint4(unsignedEdge(index + 33, 65535u), unsignedEdge(index + 38, 65535u), unsignedEdge(index + 43, 65535u), unsignedEdge(index + 48, 65535u));
#if SURFACE_OPERATION == 0
    surfaceUnsigned16RGBA[COORD] = valueUnsigned16RGBA;
#elif SURFACE_OPERATION == 1
    surfaceUnsigned16RGBA[COORD].yw = valueUnsigned16RGBA.yw;
#else
    surfaceUnsigned16RGBA[COORD][(tid.x - 1) % 4] = valueUnsigned16RGBA[(tid.x - 1) % 4];
#endif
}
