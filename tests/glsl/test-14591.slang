// Regression test for #14591: `%` on float matrices lowered to a single
// matrix-typed `kIROp_FRem`, and the GLSL text emitter wrote the
// `sign(x) * mod(abs(x), abs(y))` construction with the whole matrix as the
// argument. GLSL has no matrix overloads for `sign`, `abs`, or `mod`, so
// glslang rejected the output. The matrix FRem must instead be expanded per
// row in the IR so that `sign`/`abs`/`mod` only ever see vector operands.
// Covers `mat % mat`, `mat % scalar`, and `scalar % mat`: the scalar variants
// reach codegen with a scalar operand (not broadcast to a matrix), so the
// expansion must splat the scalar operand per row.

//TEST:SIMPLE(filecheck=CHECK_SPV): -target spirv-asm -emit-spirv-via-glsl -stage compute -entry computeMain
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
    // -0.25/0.25/0.5/-0.5, and 0.75 % x is 0.75 whenever |x| > 0.75. The
    // `tid.x` factor keeps the rows out of constant folding (with one thread,
    // the factor is exactly 1.0 and the expected values are unchanged).
    float2x2 m = float2x2(-2.5, 2.5, 3.5, -3.5) * float(tid.x + 1);

    float2x2 r = m % float2x2(0.75, 0.75, 0.75, 0.75);
    float2x2 a = m % 0.75;
    float2x2 b = 0.75 % m;

    // The whole matrix must never be passed to `sign`, `abs`, or `mod`.
    //CHECK_GLSL-NOT: sign(m_0)
    //CHECK_GLSL-NOT: mod(abs(m_0)
    // Each row of `m` is passed on its own, so the emitted construction is
    // valid genType GLSL.
    //CHECK_GLSL: sign(m_0[0])
    //CHECK_GLSL-SAME: * mod(abs(m_0[0])
    //CHECK_GLSL: sign(m_0[1])
    //CHECK_GLSL-SAME: * mod(abs(m_0[1])
    // A scalar operand is splatted to the row vector type, because GLSL `mod`
    // has no `mod(float, genType)` overload for a scalar dividend.
    //CHECK_GLSL: vec2(0.75)
    //CHECK_GLSL: sign(_S2)
    //CHECK_GLSL-SAME: * mod(abs(_S2), abs(m_0[0])

    // In the SPIR-V built via GLSL, the remainder runs one row at a time
    // (`OpFMod` on `v2float` rows) and the matrix is reassembled from the
    // row results.
    //CHECK_SPV: OpFMod %v2float
    //CHECK_SPV: OpCompositeConstruct %mat2v2float

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
