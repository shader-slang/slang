//DIAGNOSTIC_TEST:SIMPLE: -target wgsl -entry computeMain -stage compute

// Regression test: WGSL backend rejects RWTexture2D<half> with internal assertion
// Issue: https://github.com/shader-slang/slang/issues/10247

RWTexture2D<half> halfTexture;

[shader("compute")]
[numthreads(4, 4, 1)]
void computeMain(uint3 dtid : SV_DispatchThreadID) {
    int2 pos = int2(dtid.xy);
    half h = halfTexture[pos];
    halfTexture[pos] = h + half(1.0);
}
