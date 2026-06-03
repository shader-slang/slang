//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that an invalid NodeLaunch mode produces diagnostic E31401.

[shader("node")]
[NodeLaunch("invalid")]
//CHECK: E31401
[numthreads(1, 1, 1)]
void main(RWBuffer<int> output)
{
    output[0] = 0;
}

[shader("node")]
[NodeLaunch("Broadcasting")]
//CHECK: E31401
[numthreads(1, 1, 1)]
void mainCaseSensitive(RWBuffer<int> output)
{
    output[0] = 0;
}
