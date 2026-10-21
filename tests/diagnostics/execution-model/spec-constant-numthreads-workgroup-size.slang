// Test that a specialization constant in numthreads still produces an error on targets without
// specialization-constant support when the entry point calls WorkgroupSize(). On these targets
// the numthreads operand names a global parameter that collectGlobalUniformParameters has
// already removed, so WorkgroupSize() materialization must not convert it (that read its
// cleared type and crashed the compiler).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):-target hlsl -entry main

[[vk::constant_id(0)]] const int X = 4;
RWStructuredBuffer<int3> output;

[numthreads(X, 1, 1)]
/*CHECK:
 ^^^^^^^^^^ Specialization constants are not supported in the 'numthreads' attribute
*/
[shader("compute")]
void main()
{
    output[0] = WorkgroupSize();
}
