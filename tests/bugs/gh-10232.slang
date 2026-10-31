//DIAGNOSTIC_TEST:SIMPLE: -target wgsl -entry main -stage compute

// Regression test: double type on WGSL/Metal triggers ICE instead of diagnostic
// Issue: https://github.com/shader-slang/slang/issues/10232

RWStructuredBuffer<double> buffer;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID) {
    buffer[2] = buffer[0] + buffer[1];
}
