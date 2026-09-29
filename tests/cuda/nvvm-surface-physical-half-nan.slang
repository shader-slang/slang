// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Run with extras/validate-nvvm-surfaces.py.
// Written NaNs are classified, with raw Half payloads retained for before/after comparison.
[format("r16f")]
RWTexture2D<float> surface;
RWTexture2D<float> observed;
float valueAt(uint index)
{
    switch (index)
    {
    case 0: return asfloat(0x7f800001u);
    case 1: return asfloat(0x7fa12345u);
    case 2: return asfloat(0x7fc12345u);
    case 3: return asfloat(0x7fffffffu);
    case 4: return asfloat(0xff800001u);
    case 5: return asfloat(0xffa12345u);
    case 6: return asfloat(0xffc12345u);
    case 7: return asfloat(0xffffffffu);
    default: return 0.0;
    }
}
[numthreads(1, 1, 1)]
void wholeStore(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x >= 8 || tid.y >= 2) return;
    let p = int2(tid.xy);
    surface[p] = valueAt(tid.x);
    observed[p] = 1234.0;
}
