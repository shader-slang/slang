// Regression test for #14591: `%` on float matrices lowered to a single
// matrix-typed `kIROp_FRem`, and the GLSL text emitter wrote the
// `sign(x) * mod(abs(x), abs(y))` construction with the whole matrix as the
// argument. GLSL has no matrix overloads for `sign`, `abs`, or `mod`, so
// glslang rejected the output. The matrix FRem must instead be expanded per
// row in the IR so that `sign`/`abs`/`mod` only ever see vector operands.
// Covers `mat % mat`, `mat % scalar`, and `scalar % mat`: the scalar variants
// reach codegen with a scalar operand (not broadcast to a matrix), so the
// expansion must splat the scalar operand per row.

//TEST:SIMPLE: -target spirv -emit-spirv-via-glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage compute -entry computeMain -profile glsl_450
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -compute -entry computeMain -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -emit-spirv-via-glsl -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Values chosen so every `%` result is exactly representable in float32:
    // the truncated remainder of -2.5/2.5/3.5/-3.5 by 0.75 is
    // -0.25/0.25/0.5/-0.5, and 0.75 % x is 0.75 whenever |x| > 0.75.
    float2x2 m = float2x2(-2.5, 2.5, 3.5, -3.5);

    float2x2 r = m % float2x2(0.75, 0.75, 0.75, 0.75);
    float2x2 a = m % 0.75;
    float2x2 b = 0.75 % m;

    outputBuffer[0] = r[0][0];
    outputBuffer[1] = r[1][0];
    outputBuffer[2] = r[0][1];
    outputBuffer[3] = r[1][1];
    outputBuffer[4] = a[0][0];
    outputBuffer[5] = a[1][0];
    outputBuffer[6] = a[0][1];
    outputBuffer[7] = a[1][1];
    outputBuffer[8] = b[0][0];
    outputBuffer[9] = b[1][0];
    outputBuffer[10] = b[0][1];
    outputBuffer[11] = b[1][1];

    //BUF: -0.25
    //BUF: 0.5
    //BUF: 0.25
    //BUF: -0.5
    //BUF: -0.25
    //BUF: 0.5
    //BUF: 0.25
    //BUF: -0.5
    //BUF: 0.75
    //BUF: 0.75
    //BUF: 0.75
    //BUF: 0.75
}
