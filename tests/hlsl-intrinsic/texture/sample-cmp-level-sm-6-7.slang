// SampleCmpLevel's HLSL lowering requires SM 6.7 (DXC's minimum for the method), so a lower
// profile is diagnosed as implicitly upgraded rather than silently emitting a 6.7 instruction.
//DIAGNOSTIC_TEST:SIMPLE(diag=UPGRADE,non-exhaustive,exit-code=0):-target hlsl -profile sm_6_6 -stage fragment -entry fragmentMain
//DIAGNOSTIC_TEST:SIMPLE(diag=NO_UPGRADE,non-exhaustive,exit-code=0):-target hlsl -profile sm_6_7 -stage fragment -entry fragmentMain

Texture2D<float> tex;
SamplerComparisonState cmp;

[shader("fragment")]
float4 fragmentMain(float4 p: SV_Position) : SV_Target
/*UPGRADE:
       ^^^^^^^^^^^^ profile implicitly upgraded
       ^^^^^^^^^^^^ entry point 'fragmentMain' uses additional capabilities that are not part of the specified profile 'lib_6_6'. The profile setting is automatically updated to include these capabilities: 'sm_6_7'
*/
{
    return float4(tex.SampleCmpLevel(cmp, p.xy, 0.5, 0.0), 0, 0, 1);
}
