//DIAGNOSTIC_TEST:SIMPLE: -target wgsl -entry main -stage compute

// Regression test: int8_t/uint8_t on WGSL triggers ICE instead of diagnostic
// Issue: https://github.com/shader-slang/slang/issues/10234

RWStructuredBuffer<int8_t> outputInt8;
RWStructuredBuffer<uint8_t> outputUint8;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID) {
    outputInt8[2] = outputInt8[0] + outputInt8[1];
    outputUint8[2] = outputUint8[0] + outputUint8[1];
}
