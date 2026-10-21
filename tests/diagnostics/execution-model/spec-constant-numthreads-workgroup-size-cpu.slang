// Test that a specialization constant in numthreads still produces an error, rather than a
// crash, on the CPU target when the entry point calls WorkgroupSize(). The CPU target reports
// the error while it computes the group extents for the entry point.

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
