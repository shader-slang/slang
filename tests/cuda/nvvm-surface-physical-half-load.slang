// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Run with extras/validate-nvvm-surfaces.py.
// Every binary16 encoding is independently initialized by the host.
[format("r16f")]
RWTexture2D<float> surface;
RWTexture2D<float> observed;
[numthreads(1, 1, 1)]
void loadValues(uint3 tid : SV_DispatchThreadID)
{
    if (tid.y >= 256) return;
    observed[int2(tid.xy)] = surface[int2(tid.xy)];
}
