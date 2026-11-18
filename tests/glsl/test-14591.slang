// Regression test for #14591 and #14608: `%` on float matrices lowered to a
// single matrix-typed `kIROp_FRem`, and a scalar dividend against a vector
// divisor kept its scalar shape — the GLSL text emitter spells both as
// `sign(x) * mod(abs(x), abs(y))`, which glslang rejects for whole-matrix
// operands (no matrix overloads for `sign`/`abs`/`mod`) and for a scalar
// dividend (no `mod(float, genType)` overload). The GLSL FRem legalization
// must instead rewrite both shapes in the IR: matrices expand per row (scalar
// operands splatted to the row vector type), and a vector `FRem`'s scalar
// dividend is splatted to the vector type.

//TEST:SIMPLE(filecheck=CHECK_SPV): -target spirv-asm -emit-spirv-via-glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage compute -entry computeMain -profile glsl_450 -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=CHECK_SPV_DIRECT): -target spirv-asm -emit-spirv-directly -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -compute -entry computeMain -output-using-type -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -emit-spirv-via-glsl -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer

RWStructuredBuffer<float> outputBuffer;

// A non-entry helper holding a matrix `%`: the legalization's walk must cover
// every function, not just entry points. `[noinline]` keeps it a separate
// function all the way to emission.
[noinline]
float3x2 nonSquareHelper(float3x2 m)
{
    return m % 0.75;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Values chosen so every `%` result is exactly representable in float32.
    // The `tid.x` factor keeps the operands out of constant folding (with one
    // thread the factor is exactly 1.0 and the expected values are unchanged).

    // Square matrices: `mat % mat`, `mat % scalar`, `scalar % mat`, and a
    // `precise` result whose qualifier must survive the expansion.
    float2x2 m = float2x2(-2.5, 2.5, 3.5, -3.5) * float(tid.x + 1);
    float2x2 n = float2x2(0.75, 0.75, 0.75, 0.75);
    float2x2 r = m % n;
    float2x2 a = m % 0.75;
    float2x2 b = 0.75 % m;
    precise float2x2 p = a % n;

    // The whole matrix in the helper's scope must never reach `sign`/`abs`/
    // `mod`. The parenthesized patterns match the buggy whole-matrix
    // spelling and never the row spelling (`abs(m_0[0])` has `[` where they
    // have `)`).
    //CHECK_GLSL-NOT: sign(m_0)
    //CHECK_GLSL-NOT: abs(m_0)
    // A scalar operand is splatted to the row vector type, because GLSL
    // `mod` has no `mod(float, genType)` overload for a scalar dividend.
    //CHECK_GLSL: vec2(0.75)
    // Each row is passed on its own, so the construction is valid genType
    // GLSL. The third row pins the non-square shape and the `[noinline]`
    // helper (its expansion must exist inside a non-entry function).
    //CHECK_GLSL: sign(m_0[0])
    //CHECK_GLSL-SAME: * mod(abs(m_0[0])
    //CHECK_GLSL: sign(m_0[2])
    //CHECK_GLSL-SAME: * mod(abs(m_0[2])
    // Same for the entry point's matrices.
    //CHECK_GLSL-NOT: sign(m_1)
    //CHECK_GLSL-NOT: abs(m_1)
    //CHECK_GLSL: sign(m_1[0])
    //CHECK_GLSL-SAME: * mod(abs(m_1[0])
    //CHECK_GLSL: sign(_S{{[0-9]+}})
    //CHECK_GLSL-SAME: * mod(abs(_S{{[0-9]+}}), abs(m_1[0])
    // A `precise` result keeps its qualifier on the declaration that holds
    // the expanded expression.
    //CHECK_GLSL: precise mat2x2 p_{{[0-9]+}}

    // A non-square matrix pins the row and column roles: the rows of a
    // `float3x2` are `float2`, so a dimensions mixup in the per-row rewrite
    // fails the via-GLSL round-trip and the row checks below. The compound
    // `%=`, the `%` inside the `if`, and the helper call each lower to a
    // matrix `FRem` in a different function or block; the runtime (non-
    // constant) divisor matrix keeps the divisor's row extraction out of
    // constant folding.
    float3x2 m2 = float3x2(-2.5, 2.5, 3.5, -3.5, 1.25, -1.25) * float(tid.x + 1);
    m2 %= 0.75;
    float3x2 n2 = nonSquareHelper(m2);
    float3x2 d = float3x2(0.75, 0.75, 0.75, 0.75, 0.75, 0.75) * float(tid.x + 1);
    float3x2 u = m2 % d;

    //CHECK_GLSL-NOT: sign(m2_0)
    //CHECK_GLSL-NOT: abs(m2_0)
    //CHECK_GLSL: sign(m2_0[0])
    //CHECK_GLSL-SAME: * mod(abs(m2_0[0])
    //CHECK_GLSL: sign(m2_0[2])
    //CHECK_GLSL-SAME: * mod(abs(m2_0[2])
    // The runtime divisor's rows appear as `d_0[i]`, not folded constants.
    //CHECK_GLSL: sign(_S{{[0-9]+}}[0])
    //CHECK_GLSL-SAME: * mod(abs(_S{{[0-9]+}}[0]), abs(d_0[0])

    // Vectors: the #14608 shape — a scalar dividend against a vector divisor
    // has no `mod(float, genType)` overload, so the dividend must be splatted
    // upstream. The negative dividend pins the truncated-remainder semantics
    // (the result keeps the dividend's sign, where a floor-modulus would give
    // +0.75). The reverse order (vector dividend, scalar divisor) is already
    // valid `mod(genType, float)` and must keep its shape.
    float3 c = float3(1.5, 2.5, 3.5) * float(tid.x + 1);
    float3 rv = 0.75 % c;
    float3 sv = c % 0.75;
    float3 tv = -0.75 % c;

    //CHECK_GLSL: sign(vec3(0.75))
    //CHECK_GLSL-SAME: * mod(abs(vec3(0.75))
    //CHECK_GLSL: sign(c_0)
    //CHECK_GLSL-SAME: * mod(abs(c_0), abs(0.75)
    //CHECK_GLSL: sign(vec3(-0.75))
    //CHECK_GLSL-SAME: * mod(abs(vec3(-0.75))
    // Trailing guards: a scalar dividend must never reach `sign`/`mod` bare.
    //CHECK_GLSL-NOT: sign(0.75)
    //CHECK_GLSL-NOT: abs(0.75),
    //CHECK_GLSL-NOT: sign(-0.75)
    //CHECK_GLSL-NOT: abs(-0.75),

    // In the SPIR-V built via GLSL, the remainders run one row or vector at
    // a time (`OpFMod` on `v2float`/`v3float`) and the matrices are
    // reassembled from the row results. The direct SPIR-V path is not
    // rewritten by the GLSL legalization; its own emitter expands the rows as
    // `OpFRem`.
    //CHECK_SPV: OpFMod %v2float
    //CHECK_SPV: OpCompositeConstruct %mat2v2float
    //CHECK_SPV: OpCompositeConstruct %mat3v2float
    //CHECK_SPV: OpFMod %v3float
    //CHECK_SPV_DIRECT: OpFRem %v2float
    //CHECK_SPV_DIRECT: OpFRem %v3float

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
    outputBuffer[12] = p[1][1];
    outputBuffer[13] = n2[2][0];
    outputBuffer[14] = n2[0][0];
    outputBuffer[15] = u[0][0];
    if (tid.x == 0)
    {
        outputBuffer[16] = (m2 % 0.75)[2][1];
    }
    outputBuffer[17] = rv.x;
    outputBuffer[18] = rv.z;
    outputBuffer[19] = sv.y;
    outputBuffer[20] = sv.z;
    outputBuffer[21] = tv.x;
    outputBuffer[22] = tv.y;

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
    //BUF: -0.5
    //BUF: 0.5
    //BUF: -0.25
    //BUF: -0.25
    //BUF: -0.5
    //BUF: 0.75
    //BUF: 0.75
    //BUF: 0.25
    //BUF: 0.5
    //BUF: -0.75
    //BUF: -0.75
}
