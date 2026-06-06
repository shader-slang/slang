//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_BCAST,non-exhaustive): -entry broadcastingMain -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COAL,non-exhaustive): -entry coalescingMain -experimental-feature
import experimental.workgraph;

// Test that non-thread-launch node shaders still require a numthreads attribute.

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
//CHECK_BCAST: must specify a '[numthreads]' attribute
void broadcastingMain(RWBuffer<int> output)
{
    output[0] = 0;
}

[shader("node")]
[NodeLaunch("coalescing")]
//CHECK_COAL: must specify a '[numthreads]' attribute
void coalescingMain(RWBuffer<int> output)
{
    output[0] = 0;
}
