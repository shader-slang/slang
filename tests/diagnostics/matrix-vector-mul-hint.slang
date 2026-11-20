//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=2):

// `*` has no element-wise meaning between a matrix and a vector, or between two
// different-shape matrices. Fallback `operator*` declarations cover those shapes and
// carry `[__diagnoseOnUse]`, so referencing one is one clean error that names `mul`,
// replacing the old "no overload" error (E39999) and its candidate-note list. `m *= v`
// is unaffected because `*=` resolves against `operator*=` declarations, not the
// fallback `operator*`.

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
    // CHECK: error[E31233]: use of a declaration marked as an error on use
    // CHECK: use of '*' is an error: '*' does not define a matrix-vector product in Slang; use 'mul(m, v)' instead.
    // CHECK-NOT: E39999
    // CHECK-NOT: see overloads considered

    // vector * matrix: `mul(v, m)`.
    outputBuffer[0] = v * m;
    // CHECK: error[E31233]: use of a declaration marked as an error on use
    // CHECK: use of '*' is an error: '*' does not define a vector-matrix product in Slang; use 'mul(v, m)' instead.
    // CHECK-NOT: E39999
    // CHECK-NOT: see overloads considered

    // Different-shape matrices: `*` is element-wise and requires same shapes, so the matrix
    // product must be spelled `mul(m1, m2)`.
    float3x2 p = m34 * m42;
    // CHECK: error[E31233]: use of a declaration marked as an error on use
    // CHECK: use of '*' is an error: '*' is element-wise and requires same-shape matrices; use 'mul(m1, m2)' for the matrix product.
    // CHECK-NOT: E39999
    // CHECK-NOT: see overloads considered

    // Non-square shapes: a 3x4 matrix times a 4-vector fits the matrix-vector product,
    // so the fallback fires and names `mul(m, v)` (which accepts these shapes).
    float4 v4 = float4(1.0);
    float3 r34 = m34 * v4;
    // CHECK: error[E31233]: use of a declaration marked as an error on use
    // CHECK: use of '*' is an error: '*' does not define a matrix-vector product in Slang; use 'mul(m, v)' instead.
    // CHECK-NOT: E39999
    // CHECK-NOT: see overloads considered

    // Integer and half element matrices get the same hint from the same fallbacks.
    int4 vi = int4(1);
    int4x4 mi = int4x4(1);
    outputBuffer[0] = mi * vi;
    // CHECK: error[E31233]: use of a declaration marked as an error on use
    // CHECK: use of '*' is an error: '*' does not define a matrix-vector product in Slang; use 'mul(m, v)' instead.
    // CHECK-NOT: E39999
    // CHECK-NOT: see overloads considered

    half4 vh = half4(0.5);
    half4x4 mh = half4x4(0.5);
    outputBuffer[0] = mh * vh;
    // CHECK: error[E31233]: use of a declaration marked as an error on use
    // CHECK: use of '*' is an error: '*' does not define a matrix-vector product in Slang; use 'mul(m, v)' instead.
    // CHECK-NOT: E39999
    // CHECK-NOT: see overloads considered

    // `m *= v` keeps today's behavior: no `operator*=` covers it, so the error is E39999
    // with its candidate notes, not the fallback diagnostic. The fallback family
    // deliberately covers only the `*` operator for now; a matching `operator*=`
    // fallback would be a straightforward follow-up if the hint is wanted here too.
    m *= v;
    // CHECK: error[E39999]: no overload for '*=' applicable to arguments of type (matrix<float,4,4>, vector<float,4>)
    // CHECK-NOT: E40026
}

// Generic bodies over narrowly constrained element types keep working: the fallback family
// shares the real overloads' element constraint, so inside a generic with
// T : __BuiltinFloatingPointType, `matrix * scalar` resolves to the real broadcast
// overload without any error, while `matrix * vector` reports the fallback hint.
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
vector<T,N> genericBody(matrix<T,N,M> m, vector<T,M> v, T s)
{
    matrix<T,N,M> scaled = m * s; // the real scalar broadcast applies: no error
    vector<T,N> r = m * v;
    return r;
}
// CHECK: error[E31233]: use of a declaration marked as an error on use
// CHECK: use of '*' is an error: '*' does not define a matrix-vector product in Slang; use 'mul(m, v)' instead.
