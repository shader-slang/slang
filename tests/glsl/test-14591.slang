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
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage compute -entry computeMain -profile glsl_450 -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=CHECK_SPV_DIRECT): -target spirv-asm -emit-spirv-directly -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -compute -entry computeMain -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -emit-spirv-via-glsl -output-using-type -shaderobj

//TEST:SIMPLE(filecheck=CHECK_GLSL_NSQ): -target glsl -stage compute -entry nonSquareMain -profile glsl_450 -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=CHECK_SPV_NSQ): -target spirv-asm -emit-spirv-via-glsl -stage compute -entry nonSquareMain

//TEST:SIMPLE(filecheck=CHECK_GLSL_VEC): -target glsl -stage compute -entry vectorMain -profile glsl_450 -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=CHECK_SPV_VEC): -target spirv-asm -emit-spirv-via-glsl -stage compute -entry vectorMain

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer

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

    float2x2 n = float2x2(0.75, 0.75, 0.75, 0.75);
    float2x2 r = m % n;
    float2x2 a = m % 0.75;
    float2x2 b = 0.75 % m;
    precise float2x2 p = a % n;

    // The whole matrix must never be passed to `sign`, `abs`, or `mod`, in
    // either argument of the `mod` call. The parenthesized pattern never
    // matches the valid row spelling (`abs(m_0[0])` has `[` where it has `)`).
    //CHECK_GLSL-NOT: sign(m_0)
    //CHECK_GLSL-NOT: abs(m_0)
    // Each row of `m` is passed on its own, so the emitted construction is
    // valid genType GLSL.
    //CHECK_GLSL: sign(m_0[0])
    //CHECK_GLSL-SAME: * mod(abs(m_0[0])
    //CHECK_GLSL: sign(m_0[1])
    //CHECK_GLSL-SAME: * mod(abs(m_0[1])
    // A scalar operand is splatted to the row vector type, because GLSL `mod`
    // has no `mod(float, genType)` overload for a scalar dividend.
    //CHECK_GLSL: vec2(0.75)
    //CHECK_GLSL: sign(_S{{[0-9]+}})
    //CHECK_GLSL-SAME: * mod(abs(_S{{[0-9]+}}), abs(m_0[0])
    // A `precise` result keeps its qualifier through the expansion.
    //CHECK_GLSL: precise

    // In the SPIR-V built via GLSL, the remainder runs one row at a time
    // (`OpFMod` on `v2float` rows) and the matrix is reassembled from the
    // row results. The direct SPIR-V path is not rewritten by the GLSL
    // legalization; its own emitter expands the rows as `OpFRem`.
    //CHECK_SPV: OpFMod %v2float
    //CHECK_SPV: OpCompositeConstruct %mat2v2float
    //CHECK_SPV_DIRECT: OpFRem %v2float

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
}

// A non-square matrix pins the row and column roles of the expansion: the
// rows of a `float3x2` are `float2`, so a dimensions mixup in the per-row
// rewrite (building `float3` rows instead) fails the via-GLSL round-trip.
// The helper is `[noinline]` so the `%` inside it stays in a non-entry
// function, pinning that the walk covers every function, not just entry
// points. The compound `%=` in the entry point and the `%` inside the `if`
// each lower to a matrix `FRem` in a different block of the same function.
[noinline]
float3x2 nonSquareHelper(float3x2 m)
{
    return m % 0.75;
}

[numthreads(1, 1, 1)]
void nonSquareMain(uint3 tid : SV_DispatchThreadID)
{
    float3x2 m = float3x2(-2.5, 2.5, 3.5, -3.5, 1.25, -1.25) * float(tid.x + 1);
    m %= 0.75;
    float3x2 n = nonSquareHelper(m);
    if (tid.x == 0)
    {
        outputBuffer[0] = n[2][0] + (m % 0.75)[2][1];
    }

    //CHECK_GLSL_NSQ-NOT: sign(m_0)
    //CHECK_GLSL_NSQ-NOT: abs(m_0)
    // The helper is a separate function (`[noinline]`), so its third row
    // existing at all pins that the walk rewrote a non-entry function.
    //CHECK_GLSL_NSQ: sign(m_0[2])
    //CHECK_GLSL_NSQ-SAME: * mod(abs(m_0[2])
    //CHECK_GLSL_NSQ: vec2(0.75)
    // The compound `%=` in the entry point lowers to the same per-row
    // expansion of its own matrix (m_1), on `v2float` rows.
    //CHECK_GLSL_NSQ: sign(m_1[2])
    //CHECK_GLSL_NSQ-SAME: * mod(abs(m_1[2])
    // Trailing guards cover the `if` block's matrix `%` as well.
    //CHECK_GLSL_NSQ-NOT: sign(m_1)
    //CHECK_GLSL_NSQ-NOT: abs(m_1)
    //CHECK_SPV_NSQ: OpFMod %v2float
    //CHECK_SPV_NSQ: OpCompositeConstruct %mat3v2float
}

// A scalar dividend against a vector divisor is the #14608 shape: GLSL `mod`
// has no `mod(float, genType)` overload, so the dividend must be splatted to
// the vector type by the same legalization. The reverse order (vector
// dividend, scalar divisor) already spells valid `mod(genType, float)`.
[numthreads(1, 1, 1)]
void vectorMain(uint3 tid : SV_DispatchThreadID)
{
    float3 c = float3(1.5, 2.5, 3.5) * float(tid.x + 1);
    float3 r = 0.75 % c;
    float3 s = c % 0.75;
    outputBuffer[0] = r.x + s.x;

    // The scalar dividend must never be passed to `sign`/`mod` bare; the
    // splat makes both operands vectors.
    //CHECK_GLSL_VEC-NOT: sign(0.75)
    //CHECK_GLSL_VEC-NOT: mod(abs(0.75)
    //CHECK_GLSL_VEC: vec3(0.75)
    //CHECK_GLSL_VEC-SAME: * mod(abs(vec3(0.75)
    // The control (vector dividend, scalar divisor) keeps its valid form.
    //CHECK_GLSL_VEC: sign(c_0)
    //CHECK_GLSL_VEC-SAME: * mod(abs(c_0), abs(0.75)
    //CHECK_SPV_VEC: OpFMod %v3float
}
