// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o -

struct Leaf { [format("rg8_snorm")] RWTexture2D<float2> image; }
struct Wrapper { Leaf items[2]; }
Wrapper collected;
RWStructuredBuffer<float2> output;

[numthreads(1,1,1)]
void computeMain(uniform Wrapper entry, uint3 tid : SV_DispatchThreadID)
{
    uint index = tid.x & 1;
    output[0] = entry.items[index].image.Load(int2(0));
    collected.items[index].image[int2(1)] = output[1];
}
// PTX: suld.b.2d.v2.b8.zero
// PTX: sust.b.2d.v2.b8.zero
