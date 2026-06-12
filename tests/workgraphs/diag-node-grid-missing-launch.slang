//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that grid attributes require an explicit broadcasting launch mode.

[shader("node")]
[NodeDispatchGrid(1, 1, 1)]
//CHECK: E31403
void main(RWBuffer<int> output)
{
    output[0] = 0;
}
