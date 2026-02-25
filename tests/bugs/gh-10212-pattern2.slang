//DIAGNOSTIC_TEST:SIMPLE: -target spirv -entry computeMain -stage compute

// Regression test: internal compiler error "unimplemented: assignment" on out/inout parameters
// Issue: https://github.com/shader-slang/slang/issues/10212
// Pattern 2 trigger: [Differentiable] compute shader with inout parameter flow.

uint tauStep(uint z, inout uint s1, uint s2, uint s3, uint M)
{
    uint b = (((z << s1) ^ z) >> s2);
    return (((z & M) << s3) ^ b);
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
[Differentiable]
void computeMain(uint3 dtid : SV_DispatchThreadID)
{
    uint idx = dtid.x;
    uint val = idx * 1099087573UL;
    out uint z1, z2, z3, z4;
    z1 = tauStep(val, 13, 19, 12, 4294967294);
    z2 = tauStep(val, 2, 25, 4, 4294967288);
    z3 = tauStep(val, 3, 11, 17, 4294967280);
    z4 = (1664525 * val + 1013904223);
    float result = float(z1 ^ z2 ^ z3 ^ z4) * 2.3283064365e-10;
    outputBuffer[idx] = result;
}
