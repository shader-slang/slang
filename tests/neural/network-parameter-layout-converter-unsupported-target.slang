// DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl -profile cs_6_6 -entry computeMain -experimental-feature
// DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target wgsl -stage compute -entry computeMain -experimental-feature
// DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target glsl -stage compute -entry computeMain -experimental-feature
// DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target cpp -stage compute -entry computeMain -experimental-feature

import slang.neural;

uniform RWStructuredBuffer<float>.Handle portable;
uniform RWStructuredBuffer<float>.Handle optimal;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint tid : SV_GroupIndex)
{
    let portableAddress = BindlessAddress<float>(portable);
    let optimalAddress = BindlessAddress<float>(optimal);
    NetworkParameterLayoutConverter<float, 0, 1, 1>.toOptimalLayout(
        portableAddress,
        optimalAddress,
        tid,
        1);
}

// toOptimalLayout's [require]s (cuda/metal/spirv compute) fire at the entry
// point before the lowered body's own static_assert is reached, so the
// diagnostic names the call site rather than the converter's internals.
// CHECK: error[E36107]: unavailable features in entry point
// CHECK: see using of 'toOptimalLayout'
