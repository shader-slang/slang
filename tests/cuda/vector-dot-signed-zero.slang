//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj

// Unrolling the `dot` fallback removes its `T(0)` accumulator seed, because the additive-identity
// peephole folds `0 + x` to `x` and `isZero` treats `-0.0` as zero. For operands whose products are
// all negatively signed zeros that changes the result from `+0.0` to `-0.0`: the loop form computed
// `(0.0 + -0.0) + -0.0`, which is `+0.0` under IEEE-754, and the unrolled form computes
// `-0.0 + -0.0`, which is `-0.0`. This pins the sign bit so the behaviour cannot drift unnoticed.
//
// Operands come from a buffer because an all-literal `dot` is constant-folded before it reaches the
// fallback at all.

//TEST_INPUT:ubuffer(data=[0x80000000 0x80000000 0x3F800000 0x3F800000], stride=4):name=inputBuffer
RWStructuredBuffer<float> inputBuffer;
//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float2 a = float2(inputBuffer[0], inputBuffer[1]);
    float2 b = float2(inputBuffer[2], inputBuffer[3]);
    // CHECK: 80000000
    outputBuffer[0] = asuint(dot(a, b));
}
