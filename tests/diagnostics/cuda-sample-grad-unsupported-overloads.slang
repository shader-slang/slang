// The SampleGrad overloads that also take a texel offset or a minimum-LOD clamp have no
// CUDA `tex*Grad` counterpart, so on the CUDA targets they are a capability error
// (E36107), as docs/cuda-target.md states.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=OFF): -target cuda -entry eOffset -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CLAMP): -target cuda -entry eClamp -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=COMB): -target cuda -entry eCombinedOffset -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=PTXOFF): -target ptx -entry eOffset -stage compute

Texture2D<float4> gTex;
Sampler2D<float4> gSampler;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

//OFF: error[E36107]: unavailable features in entry point
//OFF: see using of 'SampleGrad'
//PTXOFF: error[E36107]: unavailable features in entry point
[shader("compute")][numthreads(1, 1, 1)]
void eOffset() { outBuf[0] = gTex.SampleGrad(samp, float2(0.5f), float2(0.25f, 0.0f), float2(0.0f, 0.125f), int2(1, 1)); }

//CLAMP: error[E36107]: unavailable features in entry point
//CLAMP: see using of 'SampleGrad'
[shader("compute")][numthreads(1, 1, 1)]
void eClamp() { outBuf[0] = gTex.SampleGrad(samp, float2(0.5f), float2(0.25f, 0.0f), float2(0.0f, 0.125f), int2(0, 0), 1.0f); }

//COMB: error[E36107]: unavailable features in entry point
//COMB: see using of 'SampleGrad'
[shader("compute")][numthreads(1, 1, 1)]
void eCombinedOffset() { outBuf[0] = gSampler.SampleGrad(float2(0.5f), float2(0.25f, 0.0f), float2(0.0f, 0.125f), int2(1, 1)); }
