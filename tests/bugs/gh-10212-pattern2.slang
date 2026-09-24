//TEST:SIMPLE(filecheck=CHECK): -target spirv-assembly -entry computeMain -stage compute
// CHECK: OpEntryPoint GLCompute
// CHECK: computeMain

// Regression test: internal compiler error "unimplemented: assignment" on out/inout parameters
// Issue: https://github.com/shader-slang/slang/issues/10212
// Pattern 2 trigger: [Differentiable] compute shader with inout parameter flow.
// Intent: compile this entry point without internal compiler error.

uint tauStep(uint z, inout uint s1, uint s2, uint s3, uint M)
{
    uint b = (((z << s1) ^ z) >> s2);
    return (((z & M) << s3) ^ b);
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
// Intentionally marked [Differentiable] to preserve the original
// regression trigger shape, even though this shader uses integer/bitwise ops.
[Differentiable]
void computeMain(uint3 dtid : SV_DispatchThreadID)
{
    uint idx = dtid.x;
    uint val = idx * 1099087573u;
    uint z1, z2, z3, z4;
    uint s1a = 13u, s1b = 2u, s1c = 3u;
    z1 = tauStep(val, s1a, 19u, 12u, 4294967294u);
    z2 = tauStep(val, s1b, 25u, 4u, 4294967288u);
    z3 = tauStep(val, s1c, 11u, 17u, 4294967280u);
    z4 = (1664525u * val + 1013904223u);
    float result = float(z1 ^ z2 ^ z3 ^ z4) * 2.3283064365e-10;
    outputBuffer[idx] = result;
}
