//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -experimental-feature
import experimental.workgraph;

// Test that node entry points must specify a NodeLaunch mode.

[shader("node")]
//CHECK: must specify a '[NodeLaunch]' attribute
[numthreads(1, 1, 1)]
void main(RWBuffer<int> output)
{
    output[0] = 0;
}
