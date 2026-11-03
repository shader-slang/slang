// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:SIMPLE(filecheck=HLSL): -O0 -target hlsl -entry interpolateColor -entry main -whole-program -profile lib_6_6 -validate-ir

// A directly called fragment entry point has an ordinary-function clone as well as its launch
// function. Both contain interpolation, so varying-reference resolution must process the clone
// after the entry-point call has been redirected. We keep the callee separate to expose both forms.
// HLSL: float4 [[CALLEE:interpolateColor_[0-9]+]](float3 [[CLONE_COLOR:color_[0-9]+]] : COLOR)
// HLSL: EvaluateAttributeAtCentroid([[CLONE_COLOR]])
// HLSL: float4 interpolateColor(float3 [[LAUNCH_COLOR:color_[0-9]+]] : COLOR)
// HLSL: EvaluateAttributeAtCentroid([[LAUNCH_COLOR]])
// HLSL: float4 main(float3 [[CALLER_COLOR:color_[0-9]+]] : COLOR)
// HLSL: [[CALLEE]]([[CALLER_COLOR]]);

// Interpolation uses the caller's fragment varying when this function is called directly.
[noinline]
[shader("fragment")]
float4 interpolateColor(float3 color : COLOR) : SV_Target
{
    return float4(EvaluateAttributeAtCentroid(color), 1.0);
}

// The call makes the ordinary clone reachable while both fragment launches remain in the library.
[shader("fragment")]
float4 main(float3 color : COLOR) : SV_Target
{
    return interpolateColor(color);
}
