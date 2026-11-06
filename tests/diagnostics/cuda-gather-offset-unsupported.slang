// tex2Dgather has no offset parameter, so the Gather* overloads that take a texel offset are a
// capability error (E36107) on the CUDA targets, as the Sample* offset overloads are (#14073).

//DIAGNOSTIC_TEST:SIMPLE(filecheck=SEP,exit-code=-1): -target cuda -entry eSeparate -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=PTXSEP,exit-code=-1): -target ptx -entry eSeparate -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ALPHA,exit-code=-1): -target cuda -entry eSeparateAlpha -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=COMB,exit-code=-1): -target cuda -entry eCombined -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=PTXCOMB,exit-code=-1): -target ptx -entry eCombined -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=GREEN,exit-code=-1): -target cuda -entry eCombinedGreen -stage compute

Texture2D<float4> tex;
SamplerState samp;
Sampler2D<float4> stex;
RWStructuredBuffer<float4> outBuf;

//SEP: error[E36107]: unavailable features in entry point
//SEP: see using of 'Gather'
//PTXSEP: error[E36107]: unavailable features in entry point
[shader("compute")][numthreads(1, 1, 1)]
void eSeparate() { outBuf[0] = tex.Gather(samp, float2(0.5, 0.25), int2(1, 1)); }

//ALPHA: error[E36107]: unavailable features in entry point
//ALPHA: see using of 'GatherAlpha'
[shader("compute")][numthreads(1, 1, 1)]
void eSeparateAlpha() { outBuf[0] = tex.GatherAlpha(samp, float2(0.5, 0.25), int2(1, 1)); }

//COMB: error[E36107]: unavailable features in entry point
//COMB: see using of 'Gather'
//PTXCOMB: error[E36107]: unavailable features in entry point
[shader("compute")][numthreads(1, 1, 1)]
void eCombined() { outBuf[0] = stex.Gather(float2(0.5, 0.25), int2(1, 1)); }

//GREEN: error[E36107]: unavailable features in entry point
//GREEN: see using of 'GatherGreen'
[shader("compute")][numthreads(1, 1, 1)]
void eCombinedGreen() { outBuf[0] = stex.GatherGreen(float2(0.5, 0.25), int2(1, 1)); }
