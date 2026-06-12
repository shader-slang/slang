// DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that node shader entry points require an explicit NodeLaunch attribute.

[shader("node")]
[numthreads(1, 1, 1)]
void main(RWBuffer<int> output)
/*CHECK:
     ^^^^ node shader entry point requires a NodeLaunch attribute
*/
{
    output[0] = 0;
}

//CHECK: E31410
