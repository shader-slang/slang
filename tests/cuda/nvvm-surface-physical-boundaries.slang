// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Run with extras/validate-nvvm-surfaces.py; dynamic component admission is a separate contract.
#ifndef SURFACE_HALF
#define SURFACE_HALF 1
#endif
#ifndef PROBE_OOB
#define PROBE_OOB 0
#endif
#if PROBE_OOB
[format("r16f")]
RWTexture2D<float> surface;
RWTexture2D<float> observed;

int2 outsideCoordinate(uint index)
{
    switch (index)
    {
    case 0: return int2(-1, 0);
    case 1: return int2(8, 0);
    case 2: return int2(0, -1);
    default: return int2(0, 3);
    }
}

[numthreads(1, 1, 1)]
void loadValues(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x >= 4 || tid.y != 0) return;
    observed[int2(tid.xy)] = surface[outsideCoordinate(tid.x)];
}

[numthreads(1, 1, 1)]
void wholeStore(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x >= 4 || tid.y != 0) return;
    surface[outsideCoordinate(tid.x)] = 9.0;
    observed[int2(tid.xy)] = 1234.0;
}
#else
#if SURFACE_HALF
[format("rgba16f")]
#endif
RWTexture2D<float4> surface;
RWTexture2D<float4> observed;

[numthreads(1, 1, 1)]
void componentStore(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x >= 12 || tid.y >= 2) return;
    let p = int2(tid.xy);
    uint lane = tid.x & 3;
    surface[p][lane] = asfloat(0x3f801001u);
    observed[p] = float4(1234.0);
}
#endif
