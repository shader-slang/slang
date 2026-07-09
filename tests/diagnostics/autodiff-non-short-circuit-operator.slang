//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that error 30055 is emitted when the non-short-circuiting ?: operator
// (vector condition) is used in a differentiable function.

[Differentiable]
float2 f(float2 x)
{
    return (x > float2(0)) ? x : -x;
//CHECK:                   ^ non-short-circuiting `?:` not allowed in differentiable function
//CHECK:                   ^ non-short-circuiting `?:` operator is not allowed in a differentiable function, use `select` instead.
}
