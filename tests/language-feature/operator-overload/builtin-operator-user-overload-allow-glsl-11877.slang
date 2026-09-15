// Companion to builtin-operator-user-overload-glsl-11877.slang, for
// https://github.com/shader-slang/slang/issues/11877
//
// The `import glsl;` companion covers the in-source route into GLSL operator scope; this test covers
// the `-allow-glsl` *command-line* route the diagnostic also recommends. The source deliberately does
// NOT `import glsl;`, so GLSL operator scope here comes solely from the `-allow-glsl` option. In that
// scope the builtin-operator fast path defers a matrix operator to normal overload resolution, so the
// user `operator*(float4x4, float4x4)` must be (a) legal at its declaration (no E30073) and (b)
// selected at the use site.
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -allow-glsl

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
