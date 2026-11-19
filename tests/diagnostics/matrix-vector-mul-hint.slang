//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=2):

// Issue #14546: `*` has no element-wise meaning between a matrix and a vector, or between two
// different-shape matrices. Fake low-rank `operator*` overloads match exactly those shapes and
// carry `[__diagnoseOnUse]`, so referencing them is one clean error that names `mul`, replacing
// the old "no overload" error (E39999) and its candidate-note list. `m *= v` is unaffected
// because `*=` resolves against `operator*=` declarations, not the fake `operator*`.

RWStructuredBuffer<float4> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float4 v = float4(1.0, 2.0, 3.0, 4.0);
    float4x4 m = float4x4(1.0);
    float3x4 m34 = float3x4(2.0);
    float4x2 m42 = float4x2(3.0);

    // matrix * vector: the fallback overload suggests the `mul(m, v)` spelling.
    outputBuffer[0] = m * v;
    // CHECK: error[E31233]: use of 'operator*' is an error: '*' does not define a matrix-vector product in Slang; use 'mul(m, v)' instead.
    // CHECK-NOT: E39999
    // CHECK-NOT: note[

    // vector * matrix: `mul(v, m)`.
    outputBuffer[0] = v * m;
    // CHECK: error[E31233]: use of 'operator*' is an error: '*' does not define a vector-matrix product in Slang; use 'mul(v, m)' instead.
    // CHECK-NOT: E39999
    // CHECK-NOT: note[

    // Different-shape matrices: `*` is element-wise and requires same shapes, so the matrix
    // product must be spelled `mul(m1, m2)`.
    float3x2 p = m34 * m42;
    // CHECK: error[E31233]: use of 'operator*' is an error: '*' is element-wise and requires same-shape matrices; use 'mul(m1, m2)' for the matrix product.
    // CHECK-NOT: E39999
    // CHECK-NOT: note[

    // Integer matrices get the same hint (the fallback's integer flavor).
    int4 vi = int4(1);
    int4x4 mi = int4x4(1);
    outputBuffer[0] = mi * vi;
    // CHECK: error[E31233]: use of 'operator*' is an error: '*' does not define a matrix-vector product in Slang; use 'mul(m, v)' instead.
    // CHECK-NOT: E39999
    // CHECK-NOT: note[

    // `m *= v` keeps today's behavior: no `operator*=` covers it, so the error is E39999 with
    // its candidate notes, not the fallback diagnostic.
    m *= v;
    // CHECK: error[E39999]: no overload for '*=' applicable to arguments of type (float4x4, float4)
}
