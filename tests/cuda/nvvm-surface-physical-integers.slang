// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Run with extras/validate-nvvm-surfaces.py; integer surfaces use native 32-bit channels.
#ifndef SURFACE_DIM
#define SURFACE_DIM 2
#endif
#ifndef SURFACE_LANES
#define SURFACE_LANES 4
#endif
#ifndef SURFACE_SIGNED
#define SURFACE_SIGNED 1
#endif
#if SURFACE_SIGNED
typealias Scalar = int;
#define FROM_BITS asint
#else
typealias Scalar = uint;
#define FROM_BITS uint
#endif
#if SURFACE_LANES == 1
typealias Value = Scalar;
#else
typealias Value = vector<Scalar, SURFACE_LANES>;
#endif
#if SURFACE_DIM == 1
#define Surface RWTexture1D<Value>
#define COORD int(tid.x)
#else
#define Surface RWTexture2D<Value>
#define COORD int2(tid.xy)
#endif
Surface surface;
Surface observed;

Scalar valueAt(uint index)
{
    uint bits;
    switch (index % 12)
    {
    case 0:
        bits = 0x00000000u;
        break;
    case 1:
        bits = 0xffffffffu;
        break;
    case 2:
        bits = 0x80000000u;
        break;
    case 3:
        bits = 0x7fffffffu;
        break;
    case 4:
        bits = 0x01020304u;
        break;
    case 5:
        bits = 0x89abcdefu;
        break;
    case 6:
        bits = 0xffffff80u;
        break;
    case 7:
        bits = 0x00000080u;
        break;
    case 8:
        bits = 0xffff8000u;
        break;
    case 9:
        bits = 0x00008000u;
        break;
    case 10:
        bits = 0x55555555u;
        break;
    default:
        bits = 0xaaaaaaaau;
        break;
    }
    return FROM_BITS(bits);
}

#if SURFACE_LANES == 1
#define VALUES valueAt(index)
#elif SURFACE_LANES == 2
#define VALUES Value(valueAt(index), valueAt(index + 5))
#else
#define VALUES Value(valueAt(index), valueAt(index + 5), valueAt(index + 10), valueAt(index + 15))
#endif

[numthreads(1, 1, 1)] void wholeStore(uint3 tid
                                      : SV_DispatchThreadID)
{
    if (tid.x >= 24 || tid.y >= 2)
        return;
    uint index = tid.x + tid.y * 24;
    surface[COORD] = VALUES;
    observed[COORD] = Value(0x12345678);
}

    [numthreads(1, 1, 1)] void componentStore(uint3 tid
                                              : SV_DispatchThreadID)
{
    if (tid.x >= 24 || tid.y >= 2)
        return;
    uint index = tid.x + tid.y * 24;
#if SURFACE_LANES == 1
    surface[COORD] = VALUES;
#elif SURFACE_LANES == 2
    surface[COORD].x = valueAt(index);
#else
    surface[COORD].xz = vector<Scalar, 2>(valueAt(index), valueAt(index + 10));
#endif
    observed[COORD] = Value(0x12345678);
}

[numthreads(1, 1, 1)] void loadValues(uint3 tid
                                      : SV_DispatchThreadID)
{
    if (tid.x >= 24 || tid.y >= 2)
        return;
    observed[COORD] = surface[COORD];
}
