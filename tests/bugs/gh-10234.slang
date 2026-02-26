//DIAGNOSTIC_TEST:SIMPLE: -target wgsl -entry main -stage compute

// Regression test: int8_t on WGSL triggers ICE instead of diagnostic
// Issue: https://github.com/shader-slang/slang/issues/10234

RWStructuredBuffer<int8_t> outputBuffer;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID) {
    outputBuffer[2] = outputBuffer[0] + outputBuffer[1];
}
