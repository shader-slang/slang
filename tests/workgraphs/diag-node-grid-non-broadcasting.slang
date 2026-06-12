//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that grid attributes are only accepted on broadcasting-launch nodes.

[shader("node")]
[NodeLaunch("thread")]
[NodeDispatchGrid(1, 1, 1)]
//CHECK: E31403
void main(RWBuffer<int> output)
{
    output[0] = 0;
}
