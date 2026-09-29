// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Run with extras/validate-nvvm-surfaces.py. All sources are initialized by the host;
// no invocation reads a destination, so host readback tests each conversion independently.
#ifndef SURFACE_DIM
#define SURFACE_DIM 2
#endif
#if SURFACE_DIM == 1
#define Surface RWTexture1D
#define COORD int(tid.x)
#else
#define Surface RWTexture2D
#define COORD int2(tid.xy)
#endif
Surface<float4> sourceNative;
[format("r16f")] Surface<float> sourceR;
[format("rg16f")] Surface<float2> sourceRG;
[format("rgba16f")] Surface<float4> sourceRGBA;
Surface<float4> resultNative;
[format("r16f")] Surface<float> resultR;
[format("rg16f")] Surface<float2> resultRG;
[format("rgba16f")] Surface<float4> resultRGBA;

[numthreads(1, 1, 1)] void wholeCopies(uint3 tid
                                       : SV_DispatchThreadID)
{
    if (tid.x >= 24 || tid.y >= 2)
        return;
    float4 native = sourceNative[COORD];
    resultNative[COORD] = float4(sourceR[COORD], sourceRG[COORD].y, sourceRGBA[COORD].zw);
    resultR[COORD] = native.x;
    resultRG[COORD] = native.xy;
    resultRGBA[COORD] = native;
}

    [numthreads(1, 1, 1)] void componentCopies(uint3 tid
                                               : SV_DispatchThreadID)
{
    if (tid.x >= 24 || tid.y >= 2)
        return;
    float4 native = sourceNative[COORD];
    // Preserve both finite and NaN payload bits in all unselected result channels.
    resultNative[COORD].yw = float2(sourceRG[COORD].y, sourceRGBA[COORD].w);
    resultR[COORD] = native.x;
    resultRG[COORD].y = native.y;
    resultRGBA[COORD].yw = native.yw;
}
