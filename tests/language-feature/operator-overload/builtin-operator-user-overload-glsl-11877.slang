// Companion to builtin-operator-user-overload-error-11877.slang, for https://github.com/shader-slang/slang/issues/11877
//
// A user overload of a builtin operator on builtin operand types is rejected at its declaration in
// the default (HLSL) scope -- but in GLSL operator scope the builtin-operator fast path defers
// matrix operators and vector equality to normal overload resolution, so a user matrix `operator*`
// IS honored there. GLSL operator scope is entered by importing the builtin `glsl` module
// (`import glsl;` sets `m_isGLSLModuleImported`; see `isGLSLOperatorScope` in slang-check-impl.h) or
// by the `-allow-glsl` compiler option. This test exercises the `import glsl;` route; the
// `-allow-glsl` command-line route is covered by builtin-operator-user-overload-allow-glsl-11877.slang.
// It must (a) suppress the declaration-site error and (b) select the user overload; if either fails,
// the run fails. (Note: `import glsl;` requires the builtin glsl module to be registered, i.e.
// `SlangGlobalSessionDesc::enableGLSL`; slangc/slang-test set it, so the import resolves here.)
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import glsl;

// User `operator*` on `float4x4` operands: a component-wise SUM with a `1234.0` marker in element
// [0][0], distinguishable from the builtin component-wise (Hadamard) product both by value and by
// the emitted marker.
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
    // the point of use, not constant-folded away.
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

    outputBuffer[0] = m[0][0]; // user: 1+3+1234 = 1238.0 ; builtin (dropped): 1*3 = 3.0
    // CHECK: 1238.0
}
