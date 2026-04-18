//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that an invalid NodeLaunch mode produces diagnostic E31150.

[shader("node")]
[NodeLaunch("invalid")]
//CHECK: E31150
[numthreads(1, 1, 1)]
void main(RWBuffer<int> output)
{
    output[0] = 0;
}
