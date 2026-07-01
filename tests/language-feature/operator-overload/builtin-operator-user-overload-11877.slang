// Regression test for https://github.com/shader-slang/slang/issues/11877
//
// The builtin-operator fast path (#11493, `convertToBuiltinArithmeticOp` in slang-check-expr.cpp)
// rewrites `a OP b` on builtin scalar/vector/matrix operands to a component-wise
// `BuiltinOperatorExpr` before overload resolution runs. Since #11493 that silently shadowed a
// user-defined `operator OP` on those operand types (the user overload was never called, and no
// diagnostic was emitted).
//
// Here a user `operator*(float4x4, float4x4)` -- the exact case from the issue -- returns a
// component-wise SUM with a `1234.0` marker in element [0][0]; `a * b` must resolve to it, not the
// builtin component-wise (Hadamard) product. This is a front-end selection, so it is
// target-independent: checked on CPU by value, and in emitted HLSL / SPIR-V where the `1234`
// marker is present iff the user operator was selected.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv -entry computeMain -stage compute -emit-spirv-directly

// User overload: component-wise sum, plus a `1234.0` marker in element [0][0]. This is
// distinguishable from the builtin component-wise product both by value (CPU) and by the presence
// of the `1234` literal in emitted code. (The inner `a + b` is a builtin matrix add -- there is no
// user `operator+`, so it is not affected by this fix.)
float4x4 operator*(float4x4 a, float4x4 b)
{
    float4x4 r = a + b;
    r[0][0] = r[0][0] + 1234.0;
    return r;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Operands are runtime values (derived from a buffer read) so `a * b` is genuinely selected at
    // the point of use, not constant-folded away. `a` holds 1..16; `b` is all 3s.
    float base = outputBuffer[0]; // 0.0
    float4x4 a = float4x4(
        base + float4(1, 2, 3, 4),
        base + float4(5, 6, 7, 8),
        base + float4(9, 10, 11, 12),
        base + float4(13, 14, 15, 16));
    float4x4 b = float4x4(
        base + float4(3, 3, 3, 3),
        base + float4(3, 3, 3, 3),
        base + float4(3, 3, 3, 3),
        base + float4(3, 3, 3, 3));

    float4x4 m = a * b; // must resolve to the user operator*, not the builtin Hadamard multiply

    outputBuffer[0] = m[0][0]; // user: 1+3+1234 = 1238.0 ; builtin (dropped): 1*3  = 3.0
    outputBuffer[1] = m[3][2]; // user: 15+3    = 18.0   ; builtin (dropped): 15*3 = 45.0
    outputBuffer[2] = m[1][3]; // user: 8+3     = 11.0   ; builtin (dropped): 8*3  = 24.0
    outputBuffer[3] = m[2][0]; // user: 9+3     = 12.0   ; builtin (dropped): 9*3  = 27.0

    // The leading `1238.0` alone distinguishes the honored result from the dropped one.
    // CHECK: 1238.0
    // CHECK: 18.0
    // CHECK: 11.0
    // CHECK: 12.0

    // The user operator's `1234.0` marker survives into emitted code only if it was selected.
    // HLSL: 1234
    // SPV: 1234
}
