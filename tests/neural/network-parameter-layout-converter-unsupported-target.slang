// DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -profile cs_6_0 -entry computeMain -experimental-feature
// DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target wgsl -stage compute -entry computeMain -experimental-feature
// DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target glsl -stage compute -entry computeMain -experimental-feature
// DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -experimental-feature

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

// CHECK: static assertion failed
// CHECK: NetworkParameterLayoutConverter.toOptimalLayout
