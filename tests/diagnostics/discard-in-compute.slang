//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -entry main -profile cs_6_1
void test()
{
    discard; // This should lead to `test` having `fragment` capability requirement.
/*CHECK:
    ^^^^^^^ see using of 'discard'
*/
}

[shader("compute")]
[numthreads(1,1,1)]
void main()
/*CHECK:
     ^^^^ unavailable features in entry point
     ^^^^ entrypoint 'main' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
{
    test(); // compute shader cannot call `test` that require capabiltiy `fragment`.
}
