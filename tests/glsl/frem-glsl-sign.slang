// Regression test for #12046: the GLSL text emitter lowered the
// floating-point `%` operator (`kIROp_FRem`, a truncated remainder whose sign
// follows the dividend) directly to the GLSL `mod()` builtin, which is a
// floor-modulus whose sign follows the divisor. The two disagree for operands
// of opposite sign, e.g. `-7.0 % 3.0` is `-1.0` but `mod(-7.0, 3.0)` is `2.0`.
//
// The emitter must instead recover the remainder as
// `sign(x) * mod(abs(x), abs(y))`, which is a truncated remainder and is valid
// for both scalar and vector operands. This is a GPU-free check of the emitted
// GLSL text.

//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage compute -entry computeMain -profile glsl_450

RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<float3> outputBufferVec;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float a = outputBuffer[0];
    float b = outputBuffer[1];
    // The scalar `%` must NOT lower to a bare `mod(a, b)`; it must use the
    // sign/abs construction that makes the result a truncated remainder.
    //CHECK: sign(
    //CHECK-SAME: * mod(abs(
    //CHECK-SAME: ), abs(
    outputBuffer[tid.x] = a % b;

    // The same construction must be emitted for vector operands, where a
    // boolean-vector `?:` would be invalid GLSL.
    float3 c = outputBufferVec[0];
    float3 d = outputBufferVec[1];
    //CHECK: sign(
    //CHECK-SAME: * mod(abs(
    //CHECK-SAME: ), abs(
    outputBufferVec[tid.x] = c % d;
}
