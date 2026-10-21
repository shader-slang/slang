// Test that a specialization constant in numthreads still produces an error on the CPU target
// when the entry point calls WorkgroupSize(). As on HLSL, the numthreads operand names a global
// parameter that collectGlobalUniformParameters has already removed, so WorkgroupSize()
// materialization must not convert it (that read its cleared type and crashed the compiler).
// The CPU target reports the error while it computes the group extents for the entry point.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):-target cpp -entry main

[[vk::constant_id(0)]] const int X = 4;
RWStructuredBuffer<int3> output;

[numthreads(X, 1, 1)]
[shader("compute")]
void main()
/*CHECK:
     ^^^^ Specialization constants are not supported in the 'numthreads' attribute
*/
{
    output[0] = WorkgroupSize();
}
