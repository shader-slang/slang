// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Run with extras/validate-nvvm-surfaces.py; the host validates physical CUDA array bytes.
#ifndef SURFACE_DIM
#define SURFACE_DIM 2
#endif
#ifndef SURFACE_LANES
#define SURFACE_LANES 4
#endif
#ifndef SURFACE_HALF
#define SURFACE_HALF 1
#endif
#if SURFACE_LANES == 1
typealias Value = float;
#define HALF_FORMAT "r16f"
#elif SURFACE_LANES == 2
typealias Value = float2;
#define HALF_FORMAT "rg16f"
#else
typealias Value = float4;
#define HALF_FORMAT "rgba16f"
#endif
#if SURFACE_DIM == 1
#define Surface RWTexture1D<Value>
#else
#define Surface RWTexture2D<Value>
#endif
#if SURFACE_HALF
[format(HALF_FORMAT)]
#endif
Surface surface;
Surface observed;

float valueAt(uint index)
{
    switch (index % 24)
    {
    case 0: return asfloat(0x00000000u);
    case 1: return asfloat(0x80000000u);
    case 2: return asfloat(0x3f800000u);
    case 3: return asfloat(0xbf800000u);
    case 4: return asfloat(0x3f800fffu);
    case 5: return asfloat(0x3f801000u);
    case 6: return asfloat(0x3f801001u);
    case 7: return asfloat(0x3f803000u);
    case 8: return asfloat(0x33000000u);
    case 9: return asfloat(0x33000001u);
    case 10: return asfloat(0xb3000000u);
    case 11: return asfloat(0x33800000u);
    case 12: return asfloat(0x387fc000u);
    case 13: return asfloat(0x387fdfffu);
    case 14: return asfloat(0x387fe000u);
    case 15: return asfloat(0x38800000u);
    case 16: return asfloat(0x477fe000u);
    case 17: return asfloat(0x477fefffu);
    case 18: return asfloat(0x477ff000u);
    case 19: return asfloat(0xc77ff000u);
    case 20: return asfloat(0x7f800000u);
    case 21: return asfloat(0xff800000u);
    case 22: return asfloat(0x3eaaaaabu);
    default: return asfloat(0xbeaaaaabu);
    }
}

#define START_ACCESS                         \
    if (tid.x >= 24 || tid.y >= 2) return;    \
    uint index = tid.x + tid.y * 24;
#if SURFACE_DIM == 1
#define COORD int(tid.x)
#else
#define COORD int2(tid.xy)
#endif
#if SURFACE_LANES == 1
#define VALUES valueAt(index)
#elif SURFACE_LANES == 2
#define VALUES float2(valueAt(index), valueAt(index + 7))
#else
#define VALUES float4(valueAt(index), valueAt(index + 7), valueAt(index + 14), valueAt(index + 21))
#endif

[numthreads(1, 1, 1)]
void wholeStore(uint3 tid : SV_DispatchThreadID)
{
    START_ACCESS
    surface[COORD] = VALUES;
    observed[COORD] = Value(1234.0);
}

[numthreads(1, 1, 1)]
void componentStore(uint3 tid : SV_DispatchThreadID)
{
    START_ACCESS
#if SURFACE_LANES == 1
    surface[COORD] = VALUES;
#elif SURFACE_LANES == 2
    surface[COORD].x = valueAt(index);
#else
    surface[COORD].xz = float2(valueAt(index), valueAt(index + 14));
#endif
    observed[COORD] = Value(1234.0);
}

[numthreads(1, 1, 1)]
void loadValues(uint3 tid : SV_DispatchThreadID)
{
    START_ACCESS
    observed[COORD] = surface[COORD];
}

#ifdef SURFACE_LITERAL_STORE
// Unlike valueAt's merged result, these direct literals reach format legalization as constants.
// The first tie rounds down to even Half bits 0x3c00; the second rounds up to 0x3c02.
[numthreads(1, 1, 1)]
void literalStore(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x != 0) return;
    for (uint index = 0; index < 24; ++index)
    {
        if (index != 5 && index != 7)
            surface[int(index)] = valueAt(index);
        observed[int(index)] = 1234.0;
    }
    surface[5] = 1.00048828125f;
    surface[7] = 1.00146484375f;
}
#endif
