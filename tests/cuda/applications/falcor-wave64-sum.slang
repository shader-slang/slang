// SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvrtc -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o -

// Reduced from Falcor2 accumulate_triangle_emission in
// slang/falcor2/render/kernels/emissive_geometry_kernels.slang at revision 4ab8df487e.
// Each lane contributes three unsigned64 fixed-point emission components. Summing these
// values must preserve both words and carry from the low word into the high word.
// The application subsequently atomically adds the result from the first active lane.
StructuredBuffer<uint64_t3> emission;
RWStructuredBuffer<uint64_t3> result;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint64_t3 sum = WaveActiveSum(emission[tid.x]);
    if (WaveIsFirstLane())
        result[tid.x / 32] = sum;
}

// PTX: .visible .entry
