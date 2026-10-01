// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Run with extras/validate-nvvm-surfaces.py. Host readback checks every physical layer.
#ifndef SURFACE_DIM
#define SURFACE_DIM 1
#endif
#if SURFACE_DIM == 1
#define Surface RWTexture1DArray
#define COORD int2(tid.x, tid.z)
#else
#define Surface RWTexture2DArray
#define COORD int3(tid.xyz)
#endif
#if SURFACE_DIM == 1
Surface<float> source0;
Surface<float> observed0;
Surface<float2> source1;
Surface<float2> observed1;
Surface<float4> source2;
Surface<float4> observed2;
Surface<int> source3;
Surface<int> observed3;
Surface<int2> source4;
Surface<int2> observed4;
Surface<int4> source5;
Surface<int4> observed5;
Surface<uint> source6;
Surface<uint> observed6;
Surface<uint2> source7;
Surface<uint2> observed7;
Surface<uint4> source8;
Surface<uint4> observed8;
#else
Surface<uint> source0;
Surface<uint> observed0;
#endif

[numthreads(1, 1, 1)]
void exercise(uint3 tid : SV_DispatchThreadID)
{
#if SURFACE_DIM == 1
    if (tid.x < 1 || tid.x >= 9 || tid.z >= 3)
        return;
#else
    // Both the correct (x=1,y=4,layer=2) and old misordered (1,2,4) addresses
    // are aligned and in bounds, so a mismatch demonstrates ordering rather than a fault.
    if (tid.x != 1 || tid.y != 4 || tid.z != 2)
        return;
#endif
    uint addressBits = (tid.z << 12) | (tid.y << 8) | (tid.x << 2);
#if SURFACE_DIM == 1
    observed0[COORD] = source0[COORD];
    source0[COORD] = asfloat(0x40000000u | addressBits | 0u);
    observed1[COORD] = source1[COORD];
    source1[COORD] = float2(
        asfloat(0x40020000u | addressBits | 0u),
        asfloat(0x40020000u | addressBits | 1u));
    observed2[COORD] = source2[COORD];
    source2[COORD] = float4(
        asfloat(0x40040000u | addressBits | 0u),
        asfloat(0x40040000u | addressBits | 1u),
        asfloat(0x40040000u | addressBits | 2u),
        asfloat(0x40040000u | addressBits | 3u));
    observed3[COORD] = source3[COORD];
    source3[COORD] = asint(0xa0060000u | addressBits | 0u);
    observed4[COORD] = source4[COORD];
    source4[COORD] = int2(
        asint(0xa0080000u | addressBits | 0u),
        asint(0xa0080000u | addressBits | 1u));
    observed5[COORD] = source5[COORD];
    source5[COORD] = int4(
        asint(0xa00a0000u | addressBits | 0u),
        asint(0xa00a0000u | addressBits | 1u),
        asint(0xa00a0000u | addressBits | 2u),
        asint(0xa00a0000u | addressBits | 3u));
    observed6[COORD] = source6[COORD];
    source6[COORD] = uint(0xa00c0000u | addressBits | 0u);
    observed7[COORD] = source7[COORD];
    source7[COORD] = uint2(
        uint(0xa00e0000u | addressBits | 0u),
        uint(0xa00e0000u | addressBits | 1u));
    observed8[COORD] = source8[COORD];
    source8[COORD] = uint4(
        uint(0xa0100000u | addressBits | 0u),
        uint(0xa0100000u | addressBits | 1u),
        uint(0xa0100000u | addressBits | 2u),
        uint(0xa0100000u | addressBits | 3u));
#else
    observed0[COORD] = source0[COORD];
    source0[COORD] = uint(0xa0000000u | addressBits | 0u);
#endif
}
