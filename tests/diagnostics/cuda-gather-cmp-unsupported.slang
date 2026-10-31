// tex2Dgather takes no compare value and a SamplerComparisonState is a no-op placeholder on CUDA,
// so GatherCmp* is a capability error (E36107) there, as SampleCmp* is (#14073).

//DIAGNOSTIC_TEST:SIMPLE(filecheck=SEP): -target cuda -entry eSeparate -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=PTXSEP): -target ptx -entry eSeparate -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=RED): -target cuda -entry eSeparateRed -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=GREEN): -target cuda -entry eSeparateGreen -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=BLUE): -target cuda -entry eSeparateBlue -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ALPHA): -target cuda -entry eSeparateAlpha -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=OFF): -target cuda -entry eSeparateOffset -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=PTXOFF): -target ptx -entry eSeparateOffset -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=COMB): -target cuda -entry eCombined -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=PTXCOMB): -target ptx -entry eCombined -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=COMBOFF): -target cuda -entry eCombinedOffset -stage compute

Texture2D<float> dtex;
SamplerComparisonState csamp;
Sampler2DShadow stex;
RWStructuredBuffer<float4> outBuf;

//SEP: error[E36107]: unavailable features in entry point
//SEP: see using of 'GatherCmp'
//PTXSEP: error[E36107]: unavailable features in entry point
[shader("compute")][numthreads(1, 1, 1)]
void eSeparate() { outBuf[0] = dtex.GatherCmp(csamp, float2(0.5, 0.25), 0.75); }

//RED: error[E36107]: unavailable features in entry point
//RED: see using of 'GatherCmpRed'
[shader("compute")][numthreads(1, 1, 1)]
void eSeparateRed() { outBuf[0] = dtex.GatherCmpRed(csamp, float2(0.5, 0.25), 0.75); }

//GREEN: error[E36107]: unavailable features in entry point
//GREEN: see using of 'GatherCmpGreen'
[shader("compute")][numthreads(1, 1, 1)]
void eSeparateGreen() { outBuf[0] = dtex.GatherCmpGreen(csamp, float2(0.5, 0.25), 0.75); }

//BLUE: error[E36107]: unavailable features in entry point
//BLUE: see using of 'GatherCmpBlue'
[shader("compute")][numthreads(1, 1, 1)]
void eSeparateBlue() { outBuf[0] = dtex.GatherCmpBlue(csamp, float2(0.5, 0.25), 0.75); }

//ALPHA: error[E36107]: unavailable features in entry point
//ALPHA: see using of 'GatherCmpAlpha'
[shader("compute")][numthreads(1, 1, 1)]
void eSeparateAlpha() { outBuf[0] = dtex.GatherCmpAlpha(csamp, float2(0.5, 0.25), 0.75); }

//OFF: error[E36107]: unavailable features in entry point
//OFF: see using of 'GatherCmp'
//PTXOFF: error[E36107]: unavailable features in entry point
[shader("compute")][numthreads(1, 1, 1)]
void eSeparateOffset() { outBuf[0] = dtex.GatherCmp(csamp, float2(0.5, 0.25), 0.75, int2(1, 1)); }

//COMB: error[E36107]: unavailable features in entry point
//COMB: see using of 'GatherCmp'
//PTXCOMB: error[E36107]: unavailable features in entry point
[shader("compute")][numthreads(1, 1, 1)]
void eCombined() { outBuf[0] = stex.GatherCmp(float2(0.5, 0.25), 0.75); }

//COMBOFF: error[E36107]: unavailable features in entry point
//COMBOFF: see using of 'GatherCmp'
[shader("compute")][numthreads(1, 1, 1)]
void eCombinedOffset() { outBuf[0] = stex.GatherCmp(float2(0.5, 0.25), 0.75, int2(1, 1)); }
