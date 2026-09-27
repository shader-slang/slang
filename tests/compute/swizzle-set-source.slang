// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:SIMPLE(filecheck=SCALAR):-target cuda -entry computeMain -stage compute -O3 -capability cuda_sm_8_0 -o -
//TEST:SIMPLE(filecheck=SCALAR):-target cpp -entry computeMain -stage compute -O3 -o -
//TEST:SIMPLE(filecheck=SCALAR):-target wgsl -entry computeMain -stage compute -O3 -o -
//TEST:SIMPLE(filecheck=SWIZZLE):-target hlsl -entry computeMain -stage compute -O3 -o -
//TEST:SIMPLE(filecheck=SWIZZLE):-target glsl -entry computeMain -stage compute -O3 -o -

StructuredBuffer<float4> inputs;
RWStructuredBuffer<float4> outputs;

// This mutable parameter preserves the existing SwizzledStore emission path.
[noinline]
float4 updateParameter(float4 value, float replacement)
{
    value.xyz = -value.zwx;
    value.xw = value.wy;
    value.z = replacement;
    return value;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // These local updates become SwizzleSet after SSA promotion. Keep the scalar field write
    // in updateParameter: taking the address of local.z here would prevent that promotion.
    float4 local = inputs[0];
    local.xyz = -local.zwx;
    local.xw = local.wy;
    vector<float, 1> single = vector<float, 1>(inputs[1].x);
    outputs[2] = float4(single.x);
    outputs[0] = local;
    outputs[1] = updateParameter(inputs[0], single.x);
}

// No multi-component lvalue is legal in these targets, in either emitted path.
// SCALAR-NOT: {{\.[xyzw]{2,4} =}}
// SCALAR: computeMain
// SCALAR-NOT: {{\.[xyzw]{2,4} =}}
// Targets that support swizzle lvalues retain their existing spelling.
// SWIZZLE: .xyz =
// SWIZZLE: .xw =
