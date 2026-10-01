// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Run with extras/validate-nvvm-surfaces.py. Results retain original copies/conversions;
// independent source-marker stores expose matching load/store address permutations.
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

Surface<half> sourceHalfR;
Surface<half> resultHalfR;
Surface<half2> sourceHalfRG;
Surface<half2> resultHalfRG;
Surface<half4> sourceHalfRGBA;
Surface<half4> resultHalfRGBA;
[format("r16f")] Surface<float> sourceFormatR;
[format("r16f")] Surface<float> resultFormatR;
[format("rg16f")] Surface<float2> sourceFormatRG;
[format("rg16f")] Surface<float2> resultFormatRG;
[format("rgba16f")] Surface<float4> sourceFormatRGBA;
[format("rgba16f")] Surface<float4> resultFormatRGBA;
Surface<float4> sourceFloat;
Surface<float4> resultFloat;

half marker(uint group, uint3 tid, uint lane)
{
    // Six groups, three layers, at most five rows and eleven columns fit 12 bits.
    uint code = (((group * 3 + tid.z) * 5 + tid.y) * 11 + tid.x) * 4 + lane;
    return bit_cast<half>(uint16_t(0x3000u | code));
}

half4 markers(uint group, uint3 tid)
{
    return half4(marker(group, tid, 0), marker(group, tid, 1),
                 marker(group, tid, 2), marker(group, tid, 3));
}

void copyTexel(uint3 tid, bool partial)
{
    if (tid.x < 1 || tid.x >= 9 || tid.z >= 2)
        return;
#if SURFACE_DIM == 2
    if (tid.y < 1 || tid.y >= 4)
        return;
#endif
    // Load original values before any source marker overwrites this invocation's texel.
    half hR = sourceHalfR[COORD];
    half2 hRG = sourceHalfRG[COORD];
    half4 hRGBA = sourceHalfRGBA[COORD];
    float fR = sourceFormatR[COORD];
    float2 fRG = sourceFormatRG[COORD];
    float4 fRGBA = sourceFormatRGBA[COORD];
    float4 input = sourceFloat[COORD];
    if (partial)
    {
        resultHalfR[COORD] = hR;
        resultHalfRG[COORD].y = hRG.y;
        resultHalfRGBA[COORD].yw = hRGBA.yw;
        resultFormatR[COORD] = input.x;
        resultFormatRG[COORD].y = input.y;
        resultFormatRGBA[COORD].yw = input.yw;
        resultFloat[COORD].yw = float2(fRG.y, fRGBA.w);
    }
    else
    {
        resultHalfR[COORD] = hR;
        resultHalfRG[COORD] = hRG;
        resultHalfRGBA[COORD] = hRGBA;
        resultFormatR[COORD] = input.x;
        resultFormatRG[COORD] = input.xy;
        resultFormatRGBA[COORD] = input;
        resultFloat[COORD] = float4(fR, fRG.y, fRGBA.zw);
    }

    half4 m0 = markers(0, tid);
    half4 m1 = markers(1, tid);
    half4 m2 = markers(2, tid);
    float4 m3 = float4(markers(3, tid));
    float4 m4 = float4(markers(4, tid));
    float4 m5 = float4(markers(5, tid));
    if (partial)
    {
        sourceHalfR[COORD] = m0.x;
        sourceHalfRG[COORD].y = m1.y;
        sourceHalfRGBA[COORD].yw = m2.yw;
        sourceFormatR[COORD] = m3.x;
        sourceFormatRG[COORD].y = m4.y;
        sourceFormatRGBA[COORD].yw = m5.yw;
    }
    else
    {
        sourceHalfR[COORD] = m0.x;
        sourceHalfRG[COORD] = m1.xy;
        sourceHalfRGBA[COORD] = m2;
        sourceFormatR[COORD] = m3.x;
        sourceFormatRG[COORD] = m4.xy;
        sourceFormatRGBA[COORD] = m5;
    }
}

[numthreads(1, 1, 1)]
void wholeCopies(uint3 tid : SV_DispatchThreadID)
{
    copyTexel(tid, false);
}

[numthreads(1, 1, 1)]
void componentCopies(uint3 tid : SV_DispatchThreadID)
{
    copyTexel(tid, true);
}
