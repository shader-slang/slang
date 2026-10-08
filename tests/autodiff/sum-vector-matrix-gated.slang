// Regression for https://github.com/shader-slang/slang/issues/11917.
//
// `lowerSumVectorMatrixInsts` is gated on `RequiredLoweringPassSet::sumVectorMatrix`. This test
// locks in that the gate FIRES when the module contains a `kIROp_SumVectorElements`. That opcode is
// produced only by the autodiff transpose pass: reverse-differentiating a scalar-to-vector broadcast
// (`float3(x)`) transposes to summing the incoming vector gradient's elements (see
// `transposeMakeVectorFromScalar` in slang-ir-autodiff-transpose.cpp). `lowerSumVectorMatrixInsts`
// must rewrite that sum into an explicit add tree; no emitter has a case for the opcode, so if the
// gate predicate were a false-negative the un-lowered `SumVectorElements` would reach the CPU
// emitter and compilation would fail — the CHECK below would not match.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

[Differentiable]
float f(float x)
{
    // Broadcast the scalar into a vector and reduce: the reverse pass of the broadcast is a
    // sum-of-elements over the incoming gradient, which lowers via lowerSumVectorMatrixInsts.
    float3 v = float3(x);
    return v.x + v.y + v.z;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    var p = diffPair(2.0f, 0.0f);
    bwd_diff(f)(p, 1.0f);
    // d/dx of (x+x+x) is 3.
    // CHECK: 3.0
    outputBuffer[0] = p.d;
}
