// SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvrtc -entry constantCondition -stage compute -capability cuda_sm_8_0 -O3 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry constantCondition -stage compute -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry constantCondition -stage compute -capability cuda_sm_8_0 -O3 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvrtc -entry dynamicCondition -stage compute -capability cuda_sm_8_0 -O3 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry dynamicCondition -stage compute -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry dynamicCondition -stage compute -capability cuda_sm_8_0 -O3 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvrtc -entry vectorConditionControl -stage compute -capability cuda_sm_8_0 -O3 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry vectorConditionControl -stage compute -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry vectorConditionControl -stage compute -capability cuda_sm_8_0 -O3 -o -

// Reduced from Falcor2's Scene.make_triangle_surface_interaction in
// slang/falcor2/render/geometry/triangle_geometry.slang. During emissive-triangle setup,
// compute_triangle_max_emission reaches select(is_front_face_cw, -N, N), where the Boolean
// is_front_face_cw is false and N is a float3 normal. Selecting an entire vector with a scalar
// Boolean is intentional: core.meta.slang declares select<T>(bool, T, T).
//
// The constant and dynamic entries preserve that scalar-Bool/float3 IR shape. The control
// explicitly splats the condition to bool3 to distinguish it from per-lane vector selection.
// Desired output for n=(1,2,3): the constant entry returns n; the other two return n at index 0
// and -n at nonzero indices. Current corpus replay checks compilation and assembly, not execution.
// These compile regressions retain the original failure shape after scalar-predicate admission.

StructuredBuffer<float3> normals;
RWStructuredBuffer<float3> result;

[shader("compute")]
[numthreads(1, 1, 1)]
void constantCondition(uint3 tid : SV_DispatchThreadID)
{
    float3 n = normals[tid.x];
    result[tid.x] = select(false, -n, n);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void dynamicCondition(uint3 tid : SV_DispatchThreadID)
{
    float3 n = normals[tid.x];
    result[tid.x] = select(tid.x != 0, -n, n);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void vectorConditionControl(uint3 tid : SV_DispatchThreadID)
{
    float3 n = normals[tid.x];
    result[tid.x] = select(bool3(tid.x != 0), -n, n);
}

// PTX: .visible .entry
